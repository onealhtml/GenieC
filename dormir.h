#ifndef DORMIR_H
#define DORMIR_H

#ifdef _WIN32
    #include <Windows.h>         // Biblioteca para Windows
    #define dormir(ms) Sleep(ms) // função Sleep do Windows
#else
    #include <unistd.h>                    // Biblioteca para sistemas POSIX (Linux, macOS)
    #define dormir(ms) usleep((ms) * 1000) // Função usleep para pausar em milissegundos
#endif

#endif // DORMIR_H
