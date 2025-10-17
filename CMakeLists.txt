cmake_minimum_required(VERSION 3.21)
project(GenieC C)

set(CMAKE_C_STANDARD 99)

# Configuração para link estático
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# Encontra as bibliotecas necessárias
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)

# Define os arquivos fonte da arquitetura modular
set(SOURCES
        main.c
        src/http_utils.c
        src/historico.c
        src/clima.c
        src/gemini.c
        src/ui.c
)

# Cria o executável
add_executable(GenieC ${SOURCES})

# Linka as bibliotecas
target_link_libraries(GenieC PRIVATE
        CURL::libcurl
        cjson
)

# Adiciona diretórios de include
target_include_directories(GenieC PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Mensagens de build
message(STATUS "====================================")
message(STATUS "GenieC - Versão 2.0 (Modular)")
message(STATUS "====================================")
message(STATUS "Configuração: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compilador C: ${CMAKE_C_COMPILER}")
message(STATUS "====================================")

