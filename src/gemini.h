#ifndef GEMINI_H
#define GEMINI_H

#include "historico.h"
#include "grafo.h"

// Funções da API Gemini
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade);
char* extrair_texto_da_resposta(const char* resposta_json);
char* consultar_gemini(const char* pergunta, HistoricoChat* historico, const char* cidade);
char* consultar_gemini_com_modelo(const char* pergunta, HistoricoChat* historico, const char* cidade, const char* modelo);

// Função para integração com grafos
int obter_distancias_ia_e_preencher_grafo(const char* cidade1, const char* cidade2, Grafo* grafo);

// Função para obter coordenadas geográficas via IA
int obter_coordenadas_cidade(const char* cidade, double* latitude, double* longitude);

// Função para obter coordenadas de múltiplas cidades em uma única requisição (OTIMIZADO)
int obter_coordenadas_multiplas(char cidades[][100], int num_cidades, double latitudes[], double longitudes[]);

#endif // GEMINI_H

