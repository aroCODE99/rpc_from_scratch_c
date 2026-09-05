# Simple RPC From Scratch in C

A minimal **Remote Procedure Call (RPC)** implementation built from scratch in C using **TCP sockets**.

The goal of this project is to understand what happens underneath higher-level RPC frameworks by implementing a simple client-server RPC system without relying on frameworks such as gRPC.

## What is RPC?

RPC (Remote Procedure Call) allows a program to call a function that actually executes on another process or machine as if it were a local function.

For example, the client can call:

```c
bool is_prime_rpc(17);
```

while the actual `is_prime()` function executes on the server.

Conceptually:

```text
Client                              Server

is_prime_rpc(17)
      |
      | serialize request
      |
      | -------- TCP -------->
      |                       |
      |                       | deserialize request
      |                       |
      |                       | is_prime(17)
      |                       |
      |                       | result = true
      |                       |
      | <------- TCP -------- |
      |
deserialize response
      |
return true
```

The client doesn't directly execute `is_prime()`. It sends a request to the server, and the server performs the operation and sends the result back.

---

## Project Goals

This project is primarily for understanding the fundamentals behind RPC and network communication.

It covers:

* TCP client/server communication
* Socket creation
* `getaddrinfo()`
* `bind()`
* `listen()`
* `accept()`
* `connect()`
* `send()`
* `recv()`
* Network byte order
* `htonl()` / `ntohl()`
* Basic serialization and deserialization
* Request/response protocols
* Client-side RPC stubs
* Server-side request handling
* Separation between RPC transport and business logic

---

## Architecture

The project follows a simple RPC architecture:

```text
                    TCP Connection
              ┌──────────────────────┐
              │                      │
              ▼                      ▼

        ┌────────────┐        ┌────────────┐
        │   Client   │        │   Server   │
        │            │        │            │
        │ RPC Stub   │        │ RPC Server │
        │            │        │            │
        └─────┬──────┘        └─────▲──────┘
              │                     │
              │ Request             │
              └─────────────────────┘
              
              ┌─────────────────────┐
              │    TCP Sockets      │
              └─────────────────────┘
```

The client exposes a local-looking function:

```c
bool is_prime_rpc(int num);
```

Internally, this function:

1. Creates a socket.
2. Connects to the RPC server.
3. Serializes the integer.
4. Sends the request.
5. Waits for the response.
6. Deserializes the response.
7. Returns the result to the caller.

---

## Example RPC

The project currently implements a simple remote `is_prime` operation.

The client can conceptually do:

```c
bool result = is_prime_rpc(17);
```

The RPC client converts the request into bytes:

```text
17
 ↓
htonl()
 ↓
Network representation
 ↓
TCP
```

The server receives the bytes:

```text
TCP
 ↓
recv()
 ↓
Network representation
 ↓
ntohl()
 ↓
17
```

It then executes:

```c
bool result = is_prime(17);
```

and sends the result back to the client.

---

## Network Byte Order

One of the important concepts demonstrated by this project is **network byte order**.

Different CPUs can represent multi-byte integers differently because of endianness.

Network protocols conventionally use **big-endian byte order**.

The client therefore uses:

```c
uint32_t packed_number = htonl(num);
```

before sending an integer.

The server converts it back:

```c
uint32_t num = ntohl(buffer);
```

The basic flow is:

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

This project therefore provides a practical demonstration of why functions such as `htonl()` and `ntohl()` exist.

---

## Socket Lifecycle

### Server

The server follows the normal TCP server lifecycle:

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

### Client

The client follows:

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

---

## Building

Make sure you have a C compiler installed.

For example:

```bash
gcc --version
```

Compile the server:

```bash
gcc server.c is_prime.c -o server
```

Compile the client:

```bash
gcc client.c is_prime.c -o client
```

For development, it is recommended to enable warnings:

```bash
gcc -Wall -Wextra -Wpedantic server.c is_prime.c -o server
```

and:

```bash
gcc -Wall -Wextra -Wpedantic client.c is_prime.c -o client
```

---

## Running

Start the server first:

```bash
./server
```

The server listens on:

```text
127.0.0.1:9090
```

You should see:

```text
server waiting for connections
```

Then run the client:

```bash
./client
```

The client sends a number to the server.

For example:

```text
Sending number: 17
Result is: 1
```

The server processes the request:

```text
Received a request: is 17 prime?
Sending response: true
```

---

## Project Structure

A simple version of the project can be organized as:

```text
simple-rpc-from-scratch-c/
│
├── client.c
├── server.c
├── is_prime.c
├── is_prime.h
├── is_prime_rpc_client.h
└── README.md
```

### `client.c`

Contains the RPC client implementation.

Responsible for:

* Creating the connection
* Connecting to the server
* Packing request data
* Sending requests
* Receiving responses
* Returning the RPC result

### `server.c`

Contains the RPC server.

Responsible for:

* Creating the listening socket
* Binding to the server address
* Listening for connections
* Accepting clients
* Receiving requests
* Executing the requested operation
* Sending responses

### `is_prime.c`

Contains the actual business logic:

```c
bool is_prime(int num);
```

The important idea is that the server executes this function, while the client accesses it remotely through:

```c
is_prime_rpc(int num);
```

---

## RPC vs Normal Function Call

A normal function call:

```c
bool result = is_prime(17);
```

roughly involves:

```text
Caller
  |
  v
Function
  |
  v
Return value
```

The RPC version involves significantly more work:

```text
Client

is_prime_rpc(17)
      |
      v
Serialize
      |
      v
send()
      |
      | TCP
      v
recv()
      |
      v
Deserialize
      |
      v
is_prime(17)
      |
      v
Serialize result
      |
      v
send()
      |
      | TCP
      v
recv()
      |
      v
Deserialize
      |
      v
return result
```

This illustrates why RPC frameworks need to handle things such as serialization, transport, protocols, errors, and connection management.

---

## What This Project Does Not Try to Implement

This is intentionally a minimal educational implementation.

It does not currently attempt to provide:

* Authentication
* Encryption/TLS
* Multiple RPC methods
* Automatic code generation
* Message schemas
* Connection pooling
* Load balancing
* Retries
* Deadlines/timeouts
* Streaming
* Concurrent request handling
* Production-grade error handling
* Version negotiation

These are areas that can be explored later.

---

## Learning Outcomes

After completing this project, you should have a better understanding of what happens underneath an RPC framework.

In particular:

```text
Application Function
        ↓
RPC Stub
        ↓
Serialization
        ↓
Transport
        ↓
TCP Socket
        ↓
Network
        ↓
TCP Socket
        ↓
Deserialization
        ↓
RPC Server
        ↓
Application Function
```

This provides the foundation for understanding higher-level technologies such as:

* gRPC
* Protocol Buffers
* Thrift
* JSON-RPC
* REST
* Message brokers

---

## Future Improvements

Possible improvements include:

* [ ] Define a proper RPC request structure
* [ ] Define a proper RPC response structure
* [ ] Support multiple RPC methods
* [ ] Implement message framing
* [ ] Handle partial `send()` and `recv()` operations
* [ ] Add client/server error responses
* [ ] Add request IDs
* [ ] Support multiple clients
* [ ] Add concurrency using threads
* [ ] Add timeouts
* [ ] Add graceful shutdown
* [ ] Implement a simple serialization format
* [ ] Add a dispatch table for RPC methods
* [ ] Separate transport, serialization, and RPC layers
* [ ] Experiment with IPv4 and IPv6
* [ ] Compare the implementation with gRPC

---

## Why Build RPC From Scratch?

Modern frameworks hide most of the networking complexity.

For example, with gRPC, a developer can write something that looks roughly like:

```text
client.call(request)
```

and the framework handles:

```text
serialization
      ↓
connection management
      ↓
HTTP/2
      ↓
network transmission
      ↓
deserialization
      ↓
server dispatch
      ↓
response serialization
```

Building a tiny RPC system manually makes these layers visible.

The purpose of this project is therefore **not to replace gRPC**, but to understand the concepts that make systems like gRPC possible.

---

## License

This project is intended primarily as an educational project for learning C networking, TCP sockets, serialization, and RPC fundamentals.
