#ifndef UI_H
#define UI_H

#include "clima.h"

// Funções de interface
void mostrar_arte_inicial();
void menu_com_clima(DataClima clima);
void mostrar_ajuda();
void mostrar_loading();
const char* obter_icone_clima(const char* description); // Nova função para ícone dinâmico

#endif // UI_H
