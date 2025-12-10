cmake_minimum_required(VERSION 3.21)
project(GenieC C)

set(CMAKE_C_STANDARD 23)

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
        src/ui_cli.c
        src/env_loader.c
        src/ui_loader.c
        src/grafo.c
)

if(WIN32)
    # Configuração para link estático no Windows
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -static")
    list(APPEND SOURCES app.rc)
    add_executable(GenieC ${SOURCES})

    # Define o caminho da biblioteca webview para Windows
    set(WEBVIEW_LIB "C:/msys64/home/loren/vcpkg/installed/x64-mingw-static/lib/libwebview.a")

    # Linka as bibliotecas para Windows
    target_link_libraries(GenieC PRIVATE
            CURL::libcurl
            cjson
            ${WEBVIEW_LIB}
            dotenv-s
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
elseif(UNIX)
    # Configuração para Linux
    add_executable(GenieC ${SOURCES})

    # Adiciona diretórios de include para webview no Linux
    target_include_directories(GenieC PRIVATE /usr/local/include/webview)

    # Encontra GTK e WebKit2GTK usando pkg-config
    find_package(PkgConfig REQUIRED)
    pkg_check_modules(GTK REQUIRED gtk+-3.0)
    pkg_check_modules(WEBKIT REQUIRED webkit2gtk-4.1)

    # Linka as bibliotecas para Linux
    target_link_libraries(GenieC PRIVATE
            CURL::libcurl
            cjson
            dotenv-s
            /usr/local/lib64/libwebview.a
            ${GTK_LIBRARIES}
            ${WEBKIT_LIBRARIES}
            stdc++
            m
    )
endif()


# Copia os arquivos da interface para a pasta de build
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_template.html ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_template.html COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_style.css ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_style.css COPYONLY)
configure_file(${CMAKE_CURRENT_SOURCE_DIR}/ui/ui_script.js ${CMAKE_CURRENT_BINARY_DIR}/ui/ui_script.js COPYONLY)

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