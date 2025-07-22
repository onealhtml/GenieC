cmake_minimum_required(VERSION 3.21)
project(GenieC C)

set(CMAKE_C_STANDARD 99)

# Garante que a runtime do C/C++ seja linkada estaticamente
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# O toolchain do vcpkg encontrará as versões estáticas corretas
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)


# Primeiro definimos o executável
add_executable(GenieC GenieC.c)

# Depois vinculamos as bibliotecas
target_link_libraries(GenieC PRIVATE CURL::libcurl)
target_link_libraries(GenieC PRIVATE cjson)