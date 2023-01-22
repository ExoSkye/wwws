cmake_minimum_required(VERSION 3.24)
project(wwws C)

set(CMAKE_C_STANDARD 11)

add_executable(wwws src/main.c src/net.c src/net.h)
target_link_libraries(wwws PRIVATE pthread)
