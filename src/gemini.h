#ifndef GEMINI_H
#define GEMINI_H

#include "historico.h"

// Funções da API Gemini
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade);
char* extrair_texto_da_resposta(const char* resposta_json);
char* consultar_gemini(const char* pergunta, HistoricoChat* historico, const char* cidade);

#endif // GEMINI_H

