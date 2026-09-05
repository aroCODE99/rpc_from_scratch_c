#include <stdio.h>
#include "is_prime.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

#define SERVER_PORT "9090"

uint32_t unpack(int num) {
    return ntohl(num);
}

int get_and_bind_socket()
{
    int sockfd;
    struct addrinfo hints, *server_info, *p;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;
    hints.ai_flags = AI_PASSIVE;  // Just use the server's IP.
    hints.ai_socktype = SOCK_STREAM;  // We want to use TCP to ensure it gets there
    int return_value = getaddrinfo(NULL, SERVER_PORT, &hints, &server_info);
    if (return_value != 0) {
        fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(return_value));
        exit(1);
    }

    for (p = server_info; p != NULL; p = p->ai_next) {
        // Try to make a socket with this one.
        if ((sockfd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
            // Something went wrong getting this socket, so we can try the next one.
            perror("server: socket");
            continue;
        }
        // We want to be able to reuse this, so we can set the socket option.
        int yes = 1;
        if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
            perror("setsockopt");
            close(sockfd);
            continue;
        }
        // Try to bind that socket.
        if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
            // If something went wrong binding this socket, we can close it and
            // move on to the next one.
            close(sockfd);
            perror("server: bind");
            continue;
        }

        // If we've made it this far, we have a valid socket and can stop iterating
        // through.
        break;
    }

    if (p == NULL) {
        fprintf(stderr, "server: failed to bind\n");
        exit(2);
    }

    freeaddrinfo(server_info);
    return sockfd;
}

// getting the socket is correct
int main()
{
    int sockfd = get_and_bind_socket();

    if (listen(sockfd, 5) == -1) {
        perror("listen");
        exit(1);
    }

    printf("server waiting for connections\n");

    struct sockaddr their_addr;  // Address information of the client
    socklen_t sin_size;
    int new_fd;
    for (;;) {
        sin_size = sizeof(their_addr);
        new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
        if (new_fd == -1) {
            perror("accept");
            continue;
        }
        int buffer;
        int bytes_received = recv(new_fd, &buffer, sizeof(int), 0);
        if (bytes_received == -1) {
            perror("recv");
            continue;
        }

        // just printing the normal buffer before unpacking it
        printf("recevied num: %d\n", buffer); // why is this still printing 0

        // We need to unpack the received data.
        uint32_t num = unpack(buffer);
        printf("Received a request: is %d prime?\n", num);

        bool num_is_prime = is_prime(num);
        printf("Sending response: %s\n", num_is_prime ? "true" : "false");

        if (send(new_fd, &num_is_prime, sizeof(num_is_prime), 0) == -1) {
            perror("send");
        }
        close(new_fd);
    }
}
