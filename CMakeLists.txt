cmake_minimum_required(VERSION 3.22)
project(proccessLab)

set(CMAKE_CXX_STANDARD 20)

add_executable(client client.cpp)
add_executable(server server.cpp)