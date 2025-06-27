#ifndef DORMIR_H
#define DORMIR_H

#ifdef _WIN32
    #include <Windows.h>
    #define dormir(ms) Sleep(ms)
#else
    #include <unistd.h>
    #define dormir(ms) usleep((ms) * 1000)
#endif

#endif // DORMIR_H
