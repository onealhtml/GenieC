#ifndef GRAFO_H
#define GRAFO_H

#define MAX_CIDADES 50
#define MAX_NOME_CIDADE 100

typedef struct {
    char nome[MAX_NOME_CIDADE];
    int adjacencias[MAX_CIDADES];  // Distâncias em km (-1 = sem conexão)
    double latitude;                // Coordenada geográfica
    double longitude;               // Coordenada geográfica
    int coords_validas;            // Flag: 1 se coordenadas foram carregadas
} Cidade;

typedef struct {
    Cidade cidades[MAX_CIDADES];
    int num_cidades;
} Grafo;

// Funções do grafo
Grafo* criar_grafo();
void adicionar_cidade(Grafo* g, const char* nome);
void adicionar_aresta(Grafo* g, const char* cidade1, const char* cidade2, int distancia);
int encontrar_cidade(Grafo* g, const char* nome);
char* calcular_menor_caminho(Grafo* g, const char* origem, const char* destino);
char* calcular_menor_caminho_com_mapa(Grafo* g, const char* origem, const char* destino);
char* listar_cidades_grafo(Grafo* g);
char* gerar_mapa_grafo(Grafo* g);
void liberar_grafo(Grafo* g);

// Funções para persistência de coordenadas
int salvar_coordenadas_grafo(Grafo* g, const char* arquivo);
int carregar_coordenadas_grafo(Grafo* g, const char* arquivo);

#endif // GRAFO_H

