cmake_minimum_required(VERSION 3.10.0)

project(uuzip)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/CMCMakeModule)

include(use_cm_module)
use_cm_module(Core)
use_cm_module(Platform)

find_package(libzip CONFIG REQUIRED)

add_executable(uuzip uuzip.cpp)
target_link_libraries(uuzip PRIVATE CMCore CMPlatform libzip::zip)
