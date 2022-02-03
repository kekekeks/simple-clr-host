cmake_minimum_required(VERSION 3.17)
project(SimpleClrHost)

set(CMAKE_CXX_STANDARD 11)

add_executable(SimpleClrHost main.cpp)
target_link_libraries(SimpleClrHost dl)