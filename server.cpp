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
#include <unordered_map>
#include <string>
#include <mutex>

// Хэш-таблица, хранящая имя клиента (ключ) и его сокет (значение)
std::unordered_map<std::string, int> clients;

std::mutex clients_mutex;

void parser(char buffer[], std::string& message, std::string& other_name){
    bool reached_mark = false;
    message.clear();
    other_name.clear();
    for (int i = 0; i < strlen(buffer); ++i) {
        if (reached_mark) {
            message.push_back(buffer[i]);
        } else {
            if (buffer[i] == ':'){
                reached_mark = true;
            } 
            else {
                other_name.push_back(buffer[i]);
            }
        }
        
    }
}

void handle_client_request(int current_client_socket) {
    char buffer[1024];
    std::string other_name;
    std::string message;

    int byte_read = read(current_client_socket, buffer, sizeof(buffer) - 1);
    if (byte_read <= 0) {
        perror("read name");
    } else {
        std::cout << "Successfull read name" << " " << current_client_socket << '\n';
    }

    std::string name(buffer);

    std::unique_lock<std::mutex> lock(clients_mutex);
    clients[name] = current_client_socket;
    lock.unlock();
    memset(buffer, 0, sizeof(buffer));

    while (1) {
        ssize_t bytes_read = read(current_client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read == -1) {
            perror("read");
            exit(EXIT_FAILURE);
        } else {
            std::cout << "Successfull read message" << " " << current_client_socket << '\n';
        }
        
        std::cout << "buffer: " << buffer << '\n';
        parser(buffer, message, other_name);
        std::cout << "Name: " << other_name << '\n';
        std::cout << "Message: " << message << '\n';

        std::unique_lock<std::mutex> lock(clients_mutex);

        int other_socket = clients[other_name];

        lock.unlock();

        if (other_socket != current_client_socket) {
            send(other_socket, message.c_str(), message.size(), 0);
        }

        memset(buffer, 0, sizeof(buffer));

    }

    close(current_client_socket);

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

    int new_socket;
    char buffer[1024];
  
    while (1) {
        new_socket = accept(connection_socket, NULL, NULL);
        if (new_socket < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        } else { 
            std::cout << "Successful connection" << '\n';
        }

        std::thread client_thread(handle_client_request, new_socket);
        client_thread.detach();
        
    }
    
    return 0;

}