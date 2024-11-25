#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include "information.h"


int main() {
    // Создаем сокет
    int client_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (client_fd == -1) {
        std::cout << "socket error";
        exit(EXIT_FAILURE);
    }

    // Настроим адрес сервера
    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));  // Обнуляем структуру
    serverAddr.sin_family = AF_INET;  // Используем IPv4
    serverAddr.sin_port = htons(PORT);  // Порт сервера

    // Преобразуем строковый IP в формат числового адреса
    if (inet_pton(AF_INET, SERVER_IP, &serverAddr.sin_addr) <= 0) {
        std::cout << "Error with inet_pton";
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Подключаемся к серверу
    if (connect(client_fd, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        std::cout << "Error with connection";
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    const char* message = "ping";
    if (write(client_fd, message, strlen(message)) == -1) {
        std::cout << "Error write response";
        close(client_fd);
        exit(EXIT_FAILURE);
    }

    // Чтение ответа от сервера
    char buffer[256];
    ssize_t len = read(client_fd, buffer, sizeof(buffer) - 1);
    if (len == -1) {
        std::cout << "Error read";
        close(client_fd);
        exit(EXIT_FAILURE);
    }
    buffer[len] = '\0';  // Завершаем строку

    std::cout << "Server response: " << buffer << std::endl;

    close(client_fd);
    return 0;
} 