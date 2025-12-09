#ifndef LIMPAR_TELA_H
#define LIMPAR_TELA_H

#ifdef _WIN32 // Se o sistema for Windows
    #include <stdlib.h>                 // Biblioteca padrão de alocação de memória e funções utilitárias
    #define limpar_tela() system("cls") // Função para limpar a tela no Windows
#else // Se o sistema for POSIX (Linux, macOS, etc.)
    #include <stdlib.h>                   // Biblioteca padrão de alocação de memória e funções utilitárias
    #define limpar_tela() system("clear") // Função para limpar a tela em sistemas POSIX
#endif

#endif // LIMPAR_TELA_H
