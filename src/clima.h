#ifndef CLIMA_H
#define CLIMA_H

// Estrutura para dados do clima
typedef struct {
    char cidade[100];
    float temperatura;
    char description[100];
    int valid;
} DataClima;

// Funções de clima
DataClima obter_dados_clima(const char* cidade);

#endif // CLIMA_H
