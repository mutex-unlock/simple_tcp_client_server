# CMakeLists.txt главной директории проекта "simple_tcp_client_server".

cmake_minimum_required(VERSION 4.3)

project(simple_tcp_client_server)

# настраиваем стандарт языка C++20
set(CMAKE_CXX_STANDART 20)
set(CMAKE_CXX_STANDART_REQUIRED ON)

find_package(Boost 1.91.0 REQUIRED COMPONENTS)


add_executable(server
    src/server.cpp
)
# target_link_libraries(server PRIVATE Boost::system)
# target_include_directories(server PRIVATE include)
target_include_directories(server PRIVATE ${Boost_INCLUDE_DIRS})


add_executable(client
    src/client.cpp
)
#target_link_libraries(client PRIVATE Boost::system)
# target_include_directories(client PRIVATE include)
target_include_directories(client PRIVATE ${Boost_INCLUDE_DIRS})
