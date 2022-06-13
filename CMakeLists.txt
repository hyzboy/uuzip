cmake_minimum_required(VERSION 3.10.0)

project(uuzip)

find_package(libzip CONFIG REQUIRED)

add_executable(uuzip uuzip.cpp)
target_link_libraries(uuzip PRIVATE libzip::zip)
