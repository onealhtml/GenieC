#ifndef DORMIR_H
#define DORMIR_H

#ifdef _WIN32 // Se o sistema for Windows
    #include <Windows.h>         // Biblioteca para Windows
    #define dormir(ms) Sleep(ms) // função Sleep do Windows
#else // Se o sistema for POSIX (Linux, macOS, etc.)
    #include <unistd.h>                    // Biblioteca para sistemas POSIX (Linux, macOS)
    #define dormir(ms) usleep((ms) * 1000) // Função usleep para pausar em milissegundos
#endif

#endif // DORMIR_H
