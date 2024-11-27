#include <iostream>
#include <sys/socket.h>
#include "information.h"
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <cstring>
#include <arpa/inet.h>
#include <thread>
#include <unordered_map>
#include <string>
#include <mutex>
#include <fstream>

// Хэш-таблица, хранящая имя клиента (ключ) и его сокет (значение)
std::unordered_map<std::string, int> clients;
std::mutex clients_mutex;
std::mutex file_mutex;


void parser(const char buffer[], std::string& message, std::string& other_name) {
    bool reached_mark = false;
    message.clear();
    other_name.clear();
    for (int i = 0; i < strlen(buffer); ++i) {
        if (reached_mark) {
            message.push_back(buffer[i]);
        } else {
            if (buffer[i] == ':') {
                reached_mark = true;
            } else if (buffer[i] != '@') {
                other_name.push_back(buffer[i]);
            }
        }
    }
}


void log_message(const std::string& username, const std::string& direction, const std::string& message) {
    std::lock_guard<std::mutex> lock(file_mutex);
    std::ofstream log_file("history_" + username + ".txt", std::ios::app);
    if (log_file.is_open()) {
        log_file << direction << ": " << message << std::endl;
    }
}

void handle_client_request(int current_client_socket) {
    char buffer[1024];
    std::string other_name;
    std::string message;

    int byte_read = recv(current_client_socket, buffer, 1024, 0);
    if (byte_read <= 0) {
        perror("read name");
        close(current_client_socket);
        return;
    }

    buffer[byte_read] = '\0';
    std::string name(buffer);

    
    {
        std::lock_guard<std::mutex> lock(clients_mutex);
        
        clients[name] = current_client_socket; 
         
    }

    std::cout << "Client connected: " << name << " (" << current_client_socket << ")" << std::endl;

    
    memset(buffer, 0, sizeof(buffer));

    while (true) {
        ssize_t bytes_read = read(current_client_socket, buffer, sizeof(buffer) - 1);
        if (bytes_read <= 0) {
            std::cout << "Client disconnected: " << name << std::endl;
            std::ofstream log_file("history_" + name + ".txt", std::ios::app);
            if (log_file.is_open()) {
                log_file << std::endl;
            }
            close(current_client_socket);

            
            {
                std::lock_guard<std::mutex> lock(clients_mutex);
                clients.erase(name);
            }
            break;
        }

        buffer[bytes_read] = '\0';
        parser(buffer, message, other_name); 

        std::cout << "Message from " << name << " to " << other_name << ": " << message << std::endl;

        // Сохраняем сообщение в историю отправителя
        log_message(name, "Sent to " + other_name, message);

        // Ищем сокет адресата
        int other_socket = -1;
        {
            std::lock_guard<std::mutex> lock(clients_mutex);
            if (clients.find(other_name) != clients.end()) {
                other_socket = clients[other_name];
            }
        }
        std::string to_send = name + ":" + message;

        if (other_socket != -1) {
            // Отправляем сообщение адресату
            if (send(other_socket, to_send.c_str(), to_send.size(), 0) == -1) {
                perror("send message");
                exit(EXIT_FAILURE);
            }

            // Сохраняем сообщение в историю адресата
            log_message(other_name, "Received from " + name, message);
        } else {
            // Если адресат не подключен, выкидываем ошибку
            std::string error_msg = "User " + other_name + " is not connected.\n";
            send(current_client_socket, error_msg.c_str(), error_msg.size(), 0);

            // Логируем ошибку
            log_message(name, "Failed to send to " + other_name, message);
        }

        memset(buffer, 0, sizeof(buffer));
    }
}

int main() {
    int connection_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = htons(PORT);

    if (inet_pton(AF_INET, SERVER_IP, &address.sin_addr) == -1) {
        perror("pton");
        exit(EXIT_FAILURE);
    }

    if (bind(connection_socket, (struct sockaddr*)&address, sizeof(address)) < 0) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    listen(connection_socket, 1);

    while (true) {
        int new_socket = accept(connection_socket, NULL, NULL);
        if (new_socket < 0) {
            perror("accept");
            continue;
        }

        
        std::thread client_thread(handle_client_request, new_socket);
        client_thread.detach();
    }

    close(connection_socket);
    return 0;
}
