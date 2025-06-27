cmake_minimum_required(VERSION 3.21)
project(PPRP C)

set(CMAKE_C_STANDARD 99)

# Garante que a runtime do C/C++ seja linkada estaticamente
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# O toolchain do vcpkg encontrará as versões estáticas corretas
find_package(CURL REQUIRED)
find_package(cJSON CONFIG REQUIRED)


# Primeiro definimos o executável
add_executable(PPRP GenieC.c)

# Depois vinculamos as bibliotecas
target_link_libraries(PPRP PRIVATE CURL::libcurl)
target_link_libraries(PPRP PRIVATE cjson)