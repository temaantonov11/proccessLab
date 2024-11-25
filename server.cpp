#include <iostream>
#include <sys/socket.h>
#include "information.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>

void handle_client_request(int sock) {
    char buffer[256];
    while (1) {
        ssize_t bytes_read = read(sock, buffer, sizeof(buffer) - 1);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        }
        if (strcmp(buffer, "ping") == 0){
            char message[256] = "pong";
            if (write(sock, message, strlen(message)) == -1){
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
}


int main()
{
    // создание сокета
    int connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket == -1){
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) == -1){
        perror("pton");
        exit(EXIT_FAILURE);
    }

    if (bind(connection_socket, (struct sockaddr*)& address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(connection_socket, 1);

    int sock;
    
    while (1) {
        sock = accept(connection_socket, NULL, NULL);
        if (sock < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        std::thread client_thread(handle_client_request, sock);
        client_thread.join();
        close(sock);
    }
    
    return 0;

}