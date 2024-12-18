#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <arpa/inet.h>
#include "information.h"

void receiveMessage(int client_socket) {
    char buffer[256];
    while (true) {
        int len = recv(client_socket, buffer, sizeof(buffer) - 1,0);
        if (len <= 0) {
            perror("read");
            close(client_socket);
            exit(EXIT_FAILURE);
        }
        buffer[len] = '\0';
        std::cout << buffer << std::endl;
        
    }


}

int main() {

    // Создаем сокет
    int client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == -1) {
        perror("socket error");
        exit(EXIT_FAILURE);
    }

    // Настройка адресса сервера
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));  // Обнуляем структуру
    serverAddr.sin_family = AF_INET;  // Используем IPv4
    serverAddr.sin_port = htons(PORT);  // Порт сервера

    // Преобразуем строковый IP в формат числового адреса
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        perror("inet_pton");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    // Подключаемся к серверу
    if (connect(client_socket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connection");
        close(client_socket);
        exit(EXIT_FAILURE);
    }

    std::cout << "Welcome to the chat!\n";
    std::string name;
    std::cout << "Enter your name: ";
    std::cin >> name;

    if (send(client_socket,name.c_str(),name.size(),0) == -1) {
        perror("send name");
        exit(EXIT_FAILURE);
    }
    
    
    
    std::thread(receiveMessage,client_socket).detach();

    std::string my_message;
    std::cout << "For exit write: exit" << '\n';
    std::cout << "Enter message using format: @username: adress" << '\n';

    
    while (true) {
        std::cin >> my_message;
        if (my_message == "exit") {
            break;
        }
        if (send(client_socket,my_message.c_str(),my_message.size(),0) == -1){
            perror("send message");
            exit(EXIT_FAILURE);
        };
    }
   
    return 0;
} 