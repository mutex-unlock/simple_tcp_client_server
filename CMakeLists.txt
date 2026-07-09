# CMakeLists.txt главной директории проекта "simple_tcp_client_server".

cmake_minimum_required(VERSION 4.3)

project(simple_tcp_client_server)

# настраиваем стандарт языка C++20
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_executable(server
    src/server.cpp
)

add_executable(client
    src/client.cpp
)
