/* gemini.c - Integração com API Gemini
 * GenieC - Assistente Inteligente
 */

#include "gemini.h"
#include "http_utils.h"
#include "config.h"
#include "env_loader.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Cria o payload JSON para a API Gemini
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade) {
    cJSON *root = cJSON_CreateObject();

    // System instruction
    cJSON *system_instruction = cJSON_CreateObject();
    cJSON *system_parts = cJSON_CreateArray();
    cJSON *system_part = cJSON_CreateObject();

    char system_prompt_formatado[4096];
    snprintf(system_prompt_formatado, sizeof(system_prompt_formatado), SYSTEM_PROMPT, cidade);

    cJSON_AddItemToObject(system_part, "text", cJSON_CreateString(system_prompt_formatado));
    cJSON_AddItemToArray(system_parts, system_part);
    cJSON_AddItemToObject(system_instruction, "parts", system_parts);
    cJSON_AddItemToObject(root, "system_instruction", system_instruction);

    // Contents com histórico
    cJSON *contents_array = cJSON_CreateArray();

    if (historico != NULL && historico->contador > 1) {
        for (int i = 0; i < historico->contador - 1; i++) {
            cJSON *content_item = cJSON_CreateObject();
            cJSON *parts_array = cJSON_CreateArray();
            cJSON *part_item = cJSON_CreateObject();

            cJSON_AddItemToObject(part_item, "text", cJSON_CreateString(historico->turno[i].text));
            cJSON_AddItemToArray(parts_array, part_item);
            cJSON_AddItemToObject(content_item, "parts", parts_array);
            cJSON_AddItemToObject(content_item, "role", cJSON_CreateString(historico->turno[i].role));

            cJSON_AddItemToArray(contents_array, content_item);
        }
    }

    // Pergunta atual
    cJSON *user_content = cJSON_CreateObject();
    cJSON *user_parts = cJSON_CreateArray();
    cJSON *user_part = cJSON_CreateObject();

    cJSON_AddItemToObject(user_part, "text", cJSON_CreateString(prompt));
    cJSON_AddItemToArray(user_parts, user_part);
    cJSON_AddItemToObject(user_content, "parts", user_parts);
    cJSON_AddItemToObject(user_content, "role", cJSON_CreateString("user"));
    cJSON_AddItemToArray(contents_array, user_content);

    cJSON_AddItemToObject(root, "contents", contents_array);

    // Google Search tools
    cJSON *tools_array = cJSON_CreateArray();
    cJSON *tool_item = cJSON_CreateObject();
    cJSON *google_search = cJSON_CreateObject();

    cJSON_AddItemToObject(tool_item, "google_search", google_search);
    cJSON_AddItemToArray(tools_array, tool_item);
    cJSON_AddItemToObject(root, "tools", tools_array);

    char *json_string = cJSON_PrintUnformatted(root);
    cJSON_Delete(root);

    return json_string;
}

// Extrai o texto da resposta JSON
char* extrair_texto_da_resposta(const char* resposta_json) {
    char* texto_extraido = NULL;
    cJSON *root = cJSON_Parse(resposta_json);

    if (root == NULL) {
        fprintf(stderr, "Erro ao parsear o JSON da resposta.\n");
        return NULL;
    }

    cJSON *candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");
    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
        cJSON *content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");

        if (content) {
            cJSON *parts = cJSON_GetObjectItemCaseSensitive(content, "parts");

            if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                cJSON *first_part = cJSON_GetArrayItem(parts, 0);
                cJSON *text_node = cJSON_GetObjectItemCaseSensitive(first_part, "text");

                if (text_node && cJSON_IsString(text_node)) {
                    const char* texto = cJSON_GetStringValue(text_node);
                    if (texto) {
                        texto_extraido = strdup(texto);
                    }
                }
            }
        }
    }

    cJSON_Delete(root);
    return texto_extraido;
}

// Função principal para consultar o Gemini
char* consultar_gemini(const char* pergunta, HistoricoChat* historico, const char* cidade) {
    // Cria o payload
    char* payload = criar_payload_json_com_historico(pergunta, historico, cidade);
    if (payload == NULL) {
        fprintf(stderr, "Erro: Não foi possível criar o pacote JSON.\n");
        return NULL;
    }

    // Obtém a API key das variáveis de ambiente
    const char* api_key = obter_env("GEMINI_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Erro: API key do Gemini não encontrada.\n");
        free(payload);
        return NULL;
    }

    // Monta a URL
    char url_completa[512];
    snprintf(url_completa, sizeof(url_completa),
             "https://generativelanguage.googleapis.com/v1beta/models/%s:generateContent?key=%s",
             MODELO_GEMINI, api_key);

    // Faz a requisição com retry
    char* resposta_bruta = fazer_requisicao_http_com_retry(url_completa, payload, MAX_RETRIES);
    free(payload);

    if (resposta_bruta == NULL) {
        fprintf(stderr, "Erro: A comunicação com a API falhou após %d tentativas.\n", MAX_RETRIES);
        return NULL;
    }

    // Extrai o texto
    char* texto_final = extrair_texto_da_resposta(resposta_bruta);
    free(resposta_bruta);

    if (texto_final == NULL) {
        fprintf(stderr, "Erro: Não foi possível extrair o texto da resposta da API.\n");
    }

    return texto_final;
}
