#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdlib.h>
#include <error.h>
#include "is_prime_rpc_client.h"

#define SERVER_PORT "9090"
#define SERVER "127.0.0.1"

// rpc client side
uint32_t pack(int num) {
    return htonl(num);
}

// how is this even compiling it doesn't have any return value
int get_socket()
{
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    memset(&hints, 0, sizeof hints);
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    int return_value = getaddrinfo(SERVER, SERVER_PORT, &hints, &server_info);
    if (return_value != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
        exit(1);
    }

    for (p = server_info; p != NULL; p = p->ai_next) {
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            perror("client: socket");
            continue;
        }
        if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            perror("client: connect");
            continue;
        }

        break;
    }

    if (p == NULL) {
        fprintf(stderr, "client: failed to connect\n");
        exit(2);
    }
    
    return sock_fd;
}

bool is_prime_rpc(int num)
{
    uint32_t packed_number = pack(num);
    int sockfd = get_socket(); // getting the connected socket
    if (send(sockfd, &packed_number, sizeof(packed_number), 0) == -1) {
        close(sockfd);
        exit(0);
    }

    int buf[1];
    int bytes_received = recv(sockfd, &buf, 1, 0);
    if (bytes_received == -1) {
        perror("recv");
        exit(1);
    }

    bool result = buf[0];
    printf("Result is: %d\n", result);
    // All done! Close the socket and return the result.
    close(sockfd);
    return result;
}
