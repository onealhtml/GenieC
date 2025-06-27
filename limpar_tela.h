#ifndef LIMPAR_TELA_H
#define LIMPAR_TELA_H

#ifdef _WIN32
    #include <stdlib.h>
    #define limpar_tela() system("cls")
#else
    #include <stdlib.h>
    #define limpar_tela() system("clear")
#endif

#endif // LIMPAR_TELA_H
