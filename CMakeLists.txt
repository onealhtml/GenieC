cmake_minimum_required(VERSION 3.21)
project(GenieC C)

set(CMAKE_C_STANDARD 23)

# Configuração para link estático
set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")

# Encontra as bibliotecas necessárias
find_package(CURL REQUIRED)
find_package(cJSON REQUIRED)


# Define os arquivos fonte da arquitetura modular
set(SOURCES
        main_gui.c
        src/http_utils.c
        src/historico.c
        src/clima.c
        src/gemini.c
        src/ui.c
        src/env_loader.c
        src/ui_loader.c
        src/grafo.c
        app.rc
)

# Cria o executável
add_executable(GenieC ${SOURCES}) #WIN32 para não mostrar a console

# Copia os arquivos da interface para a pasta de build
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_template.html ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_template.html COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_style.css ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_style.css COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_script.js ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_script.js COPYONLY)

# Define o caminho da biblioteca webview
set(WEBVIEW_LIB "C:/msys64/home/loren/vcpkg/installed/x64-mingw-static/lib/libwebview.a")

# Linka as bibliotecas
target_link_libraries(GenieC PRIVATE
        CURL::libcurl
        cjson
        dotenv-s
        ${WEBVIEW_LIB}
        stdc++
        shlwapi      # Para PathFindFileNameW e PathCombineW
        ole32        # APIs COM do Windows
        oleaut32     # Automação OLE
        uuid         # GUIDs do Windows
        gdi32        # Graphics Device Interface
        comctl32     # Controles comuns do Windows
        user32       # API de usuário do Windows
        version      # API de versão
)


# Adiciona diretórios de include
target_include_directories(GenieC PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)

# Mensagens de build
message(STATUS "====================================")
message(STATUS "GenieC - Versão 2.1 (Modular)")
message(STATUS "====================================")
message(STATUS "Configuração: ${CMAKE_BUILD_TYPE}")
message(STATUS "Compilador C: ${CMAKE_C_COMPILER}")
message(STATUS "====================================")
