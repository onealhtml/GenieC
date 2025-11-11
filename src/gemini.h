#ifndef GEMINI_H
#define GEMINI_H

#include "historico.h"
#include "grafo.h"

// Funções da API Gemini
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade);
char* extrair_texto_da_resposta(const char* resposta_json);
char* consultar_gemini(const char* pergunta, HistoricoChat* historico, const char* cidade);

// Função para integração com grafos
int obter_distancias_ia_e_preencher_grafo(const char* cidade1, const char* cidade2, Grafo* grafo);

#endif // GEMINI_H

