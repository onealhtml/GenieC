/* gemini.c - Integração com API Gemini
 * GenieC - Assistente Inteligente
 */

#include "gemini.h"
#include "http_utils.h"
#include "config.h"
#include "env_loader.h"
#include "grafo.h"
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Cria o payload JSON para a API Gemini
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade) {
    cJSON *root = cJSON_CreateObject();

    // System instruction
    cJSON *system_instruction = cJSON_CreateObject();
    cJSON *system_parts = cJSON_CreateArray();
    cJSON *system_part = cJSON_CreateObject();

    char system_prompt_formatado[4096];
    snprintf(system_prompt_formatado, sizeof(system_prompt_formatado), SYSTEM_PROMPT, cidade);

    fprintf(stderr, "[DEBUG GEMINI] Criando payload com cidade: %s\n", cidade);
    fflush(stderr);

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

// Função para obter distâncias entre cidades usando IA e preencher o grafo
int obter_distancias_ia_e_preencher_grafo(const char* cidade1, const char* cidade2, Grafo* grafo) {
    if (!cidade1 || !cidade2 || !grafo) return 0;

    // Monta um prompt específico para extrair distâncias
    char prompt[2048];
    snprintf(prompt, sizeof(prompt),
        "Liste distâncias rodoviárias REAIS (BR-XXX, rodovias principais) entre %s e %s.\n\n"
        "REGRAS OBRIGATÓRIAS:\n"
        "1. Use GOOGLE MAPS ou dados reais de rodovias brasileiras\n"
        "2. Inclua 10-15 cidades intermediárias IMPORTANTES na rota principal\n"
        "3. Adicione rotas alternativas com outras cidades\n"
        "4. Distâncias entre cidades VIZINHAS (adjacentes), não diretas\n"
        "5. Cada trecho deve ter 50-300 km (trechos curtos, realistas)\n"
        "6. Siga rodovias principais (BR-101, BR-116, BR-381, etc)\n\n"
        "FORMATO OBRIGATÓRIO (uma linha por conexão):\n"
        "CidadeA-CidadeB:XXX\n\n"
        "EXEMPLO DE MALHA REAL:\n"
        "São Paulo-São José dos Campos:85\n"
        "São José dos Campos-Taubaté:45\n"
        "Taubaté-Resende:115\n"
        "Resende-Volta Redonda:35\n"
        "Volta Redonda-Barra Mansa:15\n"
        "Barra Mansa-Rio de Janeiro:128\n\n"
        "IMPORTANTE: Use apenas distâncias VERIFICADAS. Não invente valores!\n"
        "RESPONDA APENAS COM AS LINHAS NO FORMATO, SEM TEXTO EXTRA.",
        cidade1, cidade2);

    fprintf(stderr, "[DEBUG GRAFO] Consultando IA para distâncias entre %s e %s\n", cidade1, cidade2);
    fflush(stderr);

    // Consulta a IA (sem histórico para resposta objetiva)
    char* resposta = consultar_gemini(prompt, NULL, "");

    if (!resposta) {
        fprintf(stderr, "[ERRO GRAFO] IA não retornou resposta\n");
        return 0;
    }

    fprintf(stderr, "[DEBUG GRAFO] Resposta da IA:\n%s\n", resposta);
    fflush(stderr);

    // Parse da resposta linha por linha
    int conexoes_adicionadas = 0;
    char* linha = strtok(resposta, "\n\r");

    while (linha) {
        // Remove espaços em branco no início
        while (*linha && isspace(*linha)) linha++;

        // Ignora linhas vazias ou que começam com caracteres especiais
        if (*linha == '\0' || *linha == '#' || *linha == '*' || *linha == '-' || *linha == '=') {
            linha = strtok(NULL, "\n\r");
            continue;
        }

        // Procura pelo padrão: Cidade1-Cidade2:distancia
        char c1[MAX_NOME_CIDADE] = {0};
        char c2[MAX_NOME_CIDADE] = {0};
        int distancia = 0;

        // Tenta fazer parse de várias formas
        char* sep1 = strchr(linha, '-');
        char* sep2 = strchr(linha, ':');

        if (sep1 && sep2 && sep1 < sep2) {
            // Extrai cidade1
            size_t len1 = sep1 - linha;
            if (len1 > 0 && len1 < MAX_NOME_CIDADE) {
                strncpy(c1, linha, len1);
                c1[len1] = '\0';

                // Remove espaços e caracteres especiais no final de c1
                char* end = c1 + strlen(c1) - 1;
                while (end > c1 && (isspace(*end) || *end == '*' || *end == '.')) *end-- = '\0';

                // Remove caracteres especiais no início
                char* start = c1;
                while (*start && (*start == '*' || *start == '-' || *start == '.' || isspace(*start))) start++;
                if (start != c1) {
                    memmove(c1, start, strlen(start) + 1);
                }
            }

            // Extrai cidade2
            char* start2 = sep1 + 1;
            while (*start2 && isspace(*start2)) start2++;
            size_t len2 = sep2 - start2;
            if (len2 > 0 && len2 < MAX_NOME_CIDADE) {
                strncpy(c2, start2, len2);
                c2[len2] = '\0';

                // Remove espaços e caracteres especiais no final de c2
                char* end = c2 + strlen(c2) - 1;
                while (end > c2 && (isspace(*end) || *end == '*' || *end == '.')) *end-- = '\0';
            }

            // Extrai distância (remove texto adicional como "km")
            char* dist_str = sep2 + 1;
            while (*dist_str && isspace(*dist_str)) dist_str++;

            // Extrai apenas os dígitos
            char dist_num[20] = {0};
            int idx = 0;
            while (*dist_str && isdigit(*dist_str) && idx < 19) {
                dist_num[idx++] = *dist_str++;
            }
            dist_num[idx] = '\0';
            distancia = atoi(dist_num);

            // Valida e adiciona ao grafo
            if (strlen(c1) > 2 && strlen(c2) > 2 && distancia > 0 && distancia < 10000) {
                fprintf(stderr, "[DEBUG GRAFO] Adicionando: %s - %s: %d km\n", c1, c2, distancia);
                adicionar_aresta(grafo, c1, c2, distancia);
                conexoes_adicionadas++;
            } else {
                fprintf(stderr, "[DEBUG GRAFO] Linha ignorada: '%s' (c1='%s', c2='%s', dist=%d)\n",
                    linha, c1, c2, distancia);
            }
        }

        linha = strtok(NULL, "\n\r");
    }

    free(resposta);

    fprintf(stderr, "[DEBUG GRAFO] Total de conexões adicionadas: %d\n", conexoes_adicionadas);
    fprintf(stderr, "[DEBUG GRAFO] Total de cidades no grafo: %d\n", grafo->num_cidades);
    fflush(stderr);

    return conexoes_adicionadas;
}

