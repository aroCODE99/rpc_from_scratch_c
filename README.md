# Simple RPC From Scratch in C

A minimal **Remote Procedure Call (RPC) system built from scratch in C** using TCP sockets.

The project started as a small manually implemented client/server RPC system and is evolving into a lightweight **RPC code generator**. The goal is to understand what happens underneath higher-level RPC frameworks such as gRPC by implementing the important pieces ourselves:

* RPC interface definitions
* Lexing and parsing
* Abstract Syntax Trees
* Code generation
* Client-side RPC stubs
* Server-side RPC handling
* Serialization and deserialization
* TCP socket communication
* Network byte order
* Request/response protocols

The project intentionally avoids frameworks such as gRPC and Protocol Buffers.

---

# What is RPC?

RPC (Remote Procedure Call) allows a program to call a function that executes in another process or on another machine.

From the caller's perspective, an RPC can look similar to a normal function call:

```c
bool result = MathService_isPrime(17);
```

However, the function does not execute directly inside the client process.

Conceptually:

```text
Client                                      Server

MathService_isPrime(17)
        |
        | serialize request
        |
        | ----------- TCP ------------>
        |                              |
        |                              | deserialize request
        |                              |
        |                              | dispatch method
        |                              |
        |                              | isPrime(17)
        |                              |
        |                              | result = true
        |                              |
        | <--------- TCP ------------- |
        |
        | deserialize response
        |
        v
     return true
```

The purpose of this project is to understand how the layers between the function call and the network actually work.

---

# Current Architecture

The project is now structured around an RPC definition file.

For example:

```text
service MathService {
    rpc isPrime(int32 num) returns bool;
}
```

The compiler processes this definition:

```text
              .rpc file
                  |
                  v
              ┌────────┐
              │ Lexer  │
              └────┬───┘
                   |
                   v
              ┌────────┐
              │ Parser │
              └────┬───┘
                   |
                   v
                 AST
                   |
             ┌─────┴─────┐
             v           v
       Server Generator  Client Generator
             |           |
             v           v
       rpc_server.c  rpc_client.c
```

The generated client and server communicate using TCP.

---

# RPC Definition Language

The project has a small RPC interface definition language.

Currently supported concepts include:

```text
service
rpc
returns
int32
bool
string
```

A definition can look like:

```text
service MathService {
    rpc isPrime(int32 num) returns bool;
}
```

The definition describes the RPC interface without requiring the developer to manually write the networking code.

The compiler parses this definition and generates C code for the client and server.

---

# Compiler Pipeline

The RPC generator follows a traditional compiler-like pipeline:

```text
RPC Definition
      |
      v
    Lexer
      |
      v
    Tokens
      |
      v
    Parser
      |
      v
     AST
      |
      v
 Validation
      |
      v
 Code Generator
      |
      ├───────────────┐
      v               v
Client Generator  Server Generator
      |               |
      v               v
rpc_client.c      rpc_server.c
```

## 1. Lexing

The lexer reads the `.rpc` source file and converts the input into tokens.

For example:

```text
service MathService {
    rpc isPrime(int32 num) returns bool;
}
```

is broken into tokens representing concepts such as:

```text
service
MathService
{
rpc
isPrime
(
int32
num
)
returns
bool
;
}
```

---

## 2. Parsing

The parser consumes the tokens and constructs an Abstract Syntax Tree.

The AST represents the structure of the RPC definition rather than the original source text.

The current AST contains concepts such as:

```c
typedef struct {
    Token type;
    Token name;
} Parameter;

typedef struct {
    Token name;
    vector parameters;
    Token return_type;
} Method;

typedef struct {
    Token name;
    vector methods;
} Service;
```

Conceptually:

```text
Service
└── MathService
    │
    └── Methods
        │
        └── isPrime
            ├── Parameter
            │   ├── type: int32
            │   └── name: num
            │
            └── Return type
                └── bool
```

---

# AST Validation

After parsing, the AST is validated before code generation.

This allows the generator to reject invalid RPC definitions before producing C source code.

For example, the compiler can validate the structure of:

```text
service MathService {
    rpc isPrime(int32 num) returns bool;
}
```

before passing it to the code generators.

The general pipeline is therefore:

```text
Parse
  |
  v
AST
  |
  v
Validate
  |
  v
Generate
```

---

# Code Generation

The project contains separate generators for the client and server.

Conceptually:

```text
                    Service AST
                        |
             ┌──────────┴──────────┐
             |                     |
             v                     v
      Client Generator       Server Generator
             |                     |
             v                     v
       rpc_client.c           rpc_server.c
```

The generators use an `Emitter` abstraction to construct the generated C source.

The emitter handles things such as:

* Writing source text
* Newlines
* Indentation
* Opening blocks
* Closing blocks
* Building generated source code

This keeps the code-generation logic separate from raw string manipulation.

---

# Generated Client

The client generator produces the networking and RPC stub code required to invoke a remote method.

Conceptually, generated code can expose something like:

```c
bool MathService_isPrime(RpcClient *client, int32_t num);
```

The generated function is responsible for:

```text
RPC method
    |
    v
Create request
    |
    v
Serialize arguments
    |
    v
Send method ID
    |
    v
Send request data
    |
    v
Receive response
    |
    v
Deserialize result
    |
    v
Return result
```

The application using the generated client does not need to manually construct the TCP request.

---

# Generated Server

The server generator produces the server-side RPC infrastructure.

The generated server is responsible for:

```text
TCP connection
      |
      v
Receive request
      |
      v
Read method ID
      |
      v
Deserialize arguments
      |
      v
Dispatch RPC method
      |
      v
Execute implementation
      |
      v
Serialize result
      |
      v
Send response
```

The actual application logic remains separate from the RPC transport layer.

For example:

```c
bool isPrime(int32_t num)
{
    // Application logic
}
```

The RPC layer is responsible for receiving the request and eventually invoking the implementation.

---

# RPC Method IDs

Each RPC method is assigned a method ID.

Currently, method IDs are derived from the method's position in the service's method collection.

Conceptually:

```text
Service
│
├── Method 0 → isPrime
├── Method 1 → add
└── Method 2 → subtract
```

The client can send the method ID over the network:

```text
Client

method ID
   |
   | htonl()
   v
Network byte order
   |
   | TCP
   v
Server
   |
   | ntohl()
   v
method ID
   |
   v
dispatch method
```

This forms the basis of the RPC dispatch mechanism.

---

# Serialization

RPC requires data to be converted into a representation that can be transmitted over the network.

For example, an integer:

```c
int32_t num = 17;
```

is converted to network byte order:

```c
uint32_t packed = htonl(num);
```

and transmitted through the TCP connection.

The server converts it back:

```c
uint32_t num = ntohl(packed);
```

The general flow is:

```text
C value
   |
   v
Serialization
   |
   v
Network representation
   |
   | TCP
   v
Network representation
   |
   v
Deserialization
   |
   v
C value
```

The project currently focuses on simple fixed-width values such as:

```text
int32
bool
```

with additional types such as:

```text
string
```

being incorporated into the RPC definition language and code-generation pipeline.

---

# Network Byte Order

One of the fundamental networking concepts demonstrated by the project is **network byte order**.

Different CPUs can represent multi-byte integers using different byte orders.

Network protocols conventionally use **big-endian byte order**.

Therefore, values are converted before transmission.

```text
Host Order
    |
    | htonl()
    v
Network Byte Order
    |
    | TCP
    v
Network Byte Order
    |
    | ntohl()
    v
Host Order
```

This project uses functions such as:

```c
htonl()
ntohl()
```

to explicitly handle this conversion.

---

# TCP Communication

The RPC implementation uses TCP sockets directly.

## Server Lifecycle

The generated server follows the standard TCP server lifecycle:

```text
getaddrinfo()
      |
      v
socket()
      |
      v
setsockopt()
      |
      v
bind()
      |
      v
listen()
      |
      v
accept()
      |
      v
recv()
      |
      v
process request
      |
      v
send()
      |
      v
close()
```

## Client Lifecycle

The generated client follows:

```text
getaddrinfo()
      |
      v
socket()
      |
      v
connect()
      |
      v
send()
      |
      v
recv()
      |
      v
close()
```

The goal is to expose the networking operations normally hidden by higher-level RPC frameworks.

---

# RPC Request Flow

A simplified request currently looks conceptually like:

```text
Client
  |
  | Method ID
  |
  | Arguments
  |
  v
TCP Socket
  |
  v
Server
  |
  | Read Method ID
  |
  | Deserialize Arguments
  |
  v
Dispatch
  |
  v
Application Function
```

For example:

```text
MathService_isPrime(17)
        |
        v
Method ID = 0
        |
        v
Serialize 17
        |
        v
TCP
        |
        v
Server
        |
        v
Method ID = 0
        |
        v
Deserialize 17
        |
        v
isPrime(17)
```

The response travels in the opposite direction:

```text
isPrime(17)
    |
    v
true
    |
    v
Serialize
    |
    v
TCP
    |
    v
Client
    |
    v
Deserialize
    |
    v
return true
```

---

# Command-Line Interface

The project also contains a command-line interface for the RPC generator.

The general usage is:

```bash
rpcgen [options] <input.rpc>
```

For example:

```bash
./rpcgen service.rpc
```

By default, the generator produces both client and server code.

An output directory can be specified:

```bash
./rpcgen -o generated service.rpc
```

or:

```bash
./rpcgen --output generated service.rpc
```

Client-only generation:

```bash
./rpcgen --client service.rpc
```

Server-only generation:

```bash
./rpcgen --server service.rpc
```

Help:

```bash
./rpcgen --help
```

The `--client` and `--server` options are mutually exclusive.

If neither is specified, both client and server code are generated.

---

# Example

Given:

```text
service MathService {
    rpc isPrime(int32 num) returns bool;
}
```

running:

```bash
./rpcgen -o generated math.rpc
```

will generate the corresponding client and server source files inside:

```text
generated/
```

Conceptually:

```text
math.rpc
   |
   v
  rpcgen
   |
   ├───────────────┐
   v               v
rpc_client.c   rpc_server.c
```

The generated code contains the networking and RPC infrastructure required to communicate between the client and server.

---

# Project Structure

The project is organized roughly around the following components:

```text
simple-rpc-from-scratch-c/
│
├── src/
│   ├── main.c
│   ├── lexer.c
│   ├── parser.c
│   ├── ast.c
│   ├── emitter.c
│   ├── code_gen.c
│   ├── code_gen_client.c
│   └── ...
│
├── include/
│   ├── lexer.h
│   ├── parser.h
│   ├── ast.h
│   ├── emitter.h
│   ├── code_gen.h
│   ├── code_gen_client.h
│   └── ...
│
├── build/
│
├── bin/
│
├── Makefile
│
└── README.md
```

The exact generated files may change as the compiler architecture evolves.

---

# Building

The project uses `make`.

Build the project with:

```bash
make
```

The executable is generated under:

```text
bin/
```

For example:

```bash
./bin/rpcgen --help
```

The project is developed with strict compiler warnings:

```text
-Wall
-Wextra
-Wpedantic
```

Debug builds also use:

```text
-g
```

The project can therefore be built with a command similar to:

```bash
gcc -Wall -Wextra -Wpedantic -std=c17 -g ...
```

---

# Error Handling

The generator performs validation at multiple stages.

Conceptually:

```text
Command Line
     |
     v
Argument Validation
     |
     v
Input File
     |
     v
Lexing
     |
     v
Parsing
     |
     v
AST Validation
     |
     v
Code Generation
     |
     v
Output Files
```

Failures at any stage should prevent invalid generated code from being produced.

The command-line interface also validates things such as:

* Missing input files
* Multiple input files
* Unknown options
* Missing parameters to options
* Invalid client/server option combinations
* Output file errors

---

# What This Project Does Not Currently Implement

This is intentionally a small educational implementation.

It does not currently attempt to provide:

* Authentication
* TLS/encryption
* Connection pooling
* Load balancing
* Retries
* Deadlines
* Streaming
* HTTP/2
* Production-grade concurrency
* Version negotiation
* Schema evolution
* Distributed service discovery
* Production-grade serialization
* Robust message framing
* Production-grade error propagation

These are possible areas for future development.

---

# Current Limitations

The project is still under active development.

Some important networking concerns are intentionally left for future iterations, including:

* Handling partial `send()` operations
* Handling partial `recv()` operations
* Explicit message framing
* Multiple concurrent clients
* Request IDs
* RPC error responses
* Connection reuse
* Timeouts
* Graceful shutdown

These limitations are useful learning opportunities because they expose problems that mature RPC frameworks must solve.

---

# Future Improvements

Possible future improvements include:

* [ ] Define a complete RPC request structure
* [ ] Define a complete RPC response structure
* [ ] Support multiple RPC methods
* [ ] Improve message framing
* [ ] Handle partial `send()` and `recv()` operations
* [ ] Add RPC error responses
* [ ] Add request IDs
* [ ] Add multiple-client support
* [ ] Add concurrent request handling
* [ ] Add timeouts
* [ ] Add graceful shutdown
* [ ] Improve serialization
* [ ] Add a method dispatch table
* [ ] Separate transport and serialization into dedicated modules
* [ ] Improve generated code
* [ ] Add more RPC data types
* [ ] Improve string serialization
* [ ] Add IPv4 and IPv6 support
* [ ] Add generated client/server headers
* [ ] Add automated integration tests
* [ ] Compare generated RPC behavior with gRPC
* [ ] Improve compiler diagnostics
* [ ] Add richer AST validation
* [ ] Add more compiler/CLI options

---

# Why Build RPC From Scratch?

Modern RPC frameworks hide a large amount of complexity.

A developer using a framework might write something conceptually simple such as:

```text
client.isPrime(17)
```

while the underlying system has to perform something closer to:

```text
Application Call
      |
      v
Generated Stub
      |
      v
Method Identification
      |
      v
Serialization
      |
      v
Connection Management
      |
      v
Transport Protocol
      |
      v
Network
      |
      v
Server Transport
      |
      v
Deserialization
      |
      v
Method Dispatch
      |
      v
Application Function
      |
      v
Response Serialization
      |
      v
Network
      |
      v
Client Deserialization
      |
      v
Return Value
```

Building a small version manually makes these layers visible.

The purpose of this project is therefore **not to replace gRPC**.

The purpose is to understand the fundamental systems concepts that make RPC frameworks possible.

---

# Learning Outcomes

By working through this project, the goal is to develop a practical understanding of:

### C

* Pointers
* Dynamic memory
* Structs
* Function interfaces
* Header/source organization
* Generic data structures
* String handling
* Error handling
* Resource ownership

### Networking

* TCP sockets
* Client/server architecture
* `socket()`
* `bind()`
* `listen()`
* `accept()`
* `connect()`
* `send()`
* `recv()`
* `getaddrinfo()`
* Network byte order
* `htonl()`
* `ntohl()`

### Compiler Fundamentals

* Lexing
* Tokens
* Parsing
* ASTs
* Validation
* Code generation
* Source-code emission

### RPC Design

* Method identification
* Serialization
* Deserialization
* Request/response protocols
* Client stubs
* Server dispatch
* Transport separation
* Interface definition languages

The project is intended to evolve alongside these concepts, gradually moving from a simple TCP experiment toward a small but complete RPC toolchain.

---

# License

This project is primarily an educational project for learning:

* C
* Systems programming
* TCP networking
* Serialization
* Compiler fundamentals
* Code generation
* RPC architecture

It is not intended to be used as a production RPC framework.
