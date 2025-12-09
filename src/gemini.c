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
    // Parse do JSON
    cJSON *root = cJSON_Parse(resposta_json);

    if (root == NULL) {
        fprintf(stderr, "Erro ao parsear o JSON da resposta.\n");
        return NULL;
    }

    // Extrai array de candidatos
    cJSON *candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");
    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        // Pega primeiro candidato
        cJSON *first_candidate = cJSON_GetArrayItem(candidates, 0);
        cJSON *content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");

        if (content) {
            // Extrai partes do conteúdo
            cJSON *parts = cJSON_GetObjectItemCaseSensitive(content, "parts");

            if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                // Pega primeira parte
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

// Função para consultar o Gemini com modelo específico
char* consultar_gemini_com_modelo(const char* pergunta, HistoricoChat* historico, const char* cidade, const char* modelo) {
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

    // Monta a URL com o modelo especificado
    char url_completa[512];
    snprintf(url_completa, sizeof(url_completa),
             "https://generativelanguage.googleapis.com/v1beta/models/%s:generateContent?key=%s",
             modelo, api_key);

    fprintf(stderr, "[DEBUG GEMINI] Usando modelo: %s\n", modelo);
    fflush(stderr);

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

// Função principal para consultar o Gemini (usa modelo padrão para chat)
char* consultar_gemini(const char* pergunta, HistoricoChat* historico, const char* cidade) {
    return consultar_gemini_com_modelo(pergunta, historico, cidade, MODELO_GEMINI_CHAT);
}

// Função para obter distâncias entre cidades usando IA e preencher o grafo
int obter_distancias_ia_e_preencher_grafo(const char* cidade1, const char* cidade2, Grafo* grafo) {
    if (!cidade1 || !cidade2 || !grafo) return 0;

    // Monta prompt usando template do config.h
    char prompt[2048];
    snprintf(prompt, sizeof(prompt), PROMPT_DISTANCIAS_GRAFO, cidade1, cidade2);

    fprintf(stderr, "[DEBUG GRAFO] Consultando IA (modelo: %s) para distâncias entre %s e %s\n",
            MODELO_GEMINI_GRAFO, cidade1, cidade2);
    fflush(stderr);

    // Consulta a IA usando modelo específico para grafos (sem histórico)
    char* resposta = consultar_gemini_com_modelo(prompt, NULL, "", MODELO_GEMINI_GRAFO);

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

// Função para obter coordenadas geográficas de uma cidade via IA
int obter_coordenadas_cidade(const char* cidade, double* latitude, double* longitude) {
    if (!cidade || !latitude || !longitude) return 0;

    // Monta prompt usando template do config.h
    char prompt[1024];
    snprintf(prompt, sizeof(prompt), PROMPT_COORDENADAS_UNICA, cidade);

    fprintf(stderr, "[DEBUG COORDS] Consultando IA (modelo: %s) para coordenadas de %s\n",
            MODELO_GEMINI_GRAFO, cidade);
    fflush(stderr);

    // Consulta a IA usando modelo específico para grafos (sem histórico)
    char* resposta = consultar_gemini_com_modelo(prompt, NULL, "", MODELO_GEMINI_GRAFO);

    if (!resposta) {
        fprintf(stderr, "[ERRO COORDS] IA não retornou resposta\n");
        return 0;
    }

    fprintf(stderr, "[DEBUG COORDS] Resposta da IA:\n%s\n", resposta);
    fflush(stderr);

    // Parse da resposta
    double lat = 0.0, lng = 0.0;
    int lat_found = 0, lng_found = 0;

    char* linha = strtok(resposta, "\n\r");
    while (linha) {
        // Remove espaços em branco no início
        while (*linha && isspace(*linha)) linha++;

        // Procura por LAT:
        if (strncasecmp(linha, "LAT:", 4) == 0) {
            char* valor = linha + 4;
            while (*valor && isspace(*valor)) valor++;
            lat = atof(valor);
            if (lat != 0.0 || *valor == '0') {
                lat_found = 1;
                fprintf(stderr, "[DEBUG COORDS] Latitude encontrada: %.4f\n", lat);
            }
        }
        // Procura por LNG: ou LON:
        else if (strncasecmp(linha, "LNG:", 4) == 0 || strncasecmp(linha, "LON:", 4) == 0) {
            char* valor = linha + 4;
            while (*valor && isspace(*valor)) valor++;
            lng = atof(valor);
            if (lng != 0.0 || *valor == '0') {
                lng_found = 1;
                fprintf(stderr, "[DEBUG COORDS] Longitude encontrada: %.4f\n", lng);
            }
        }

        linha = strtok(NULL, "\n\r");
    }

    free(resposta);

    // Valida se encontrou ambas as coordenadas
    if (lat_found && lng_found) {
        *latitude = lat;
        *longitude = lng;
        fprintf(stderr, "[DEBUG COORDS] Coordenadas de %s: %.4f, %.4f\n", cidade, lat, lng);
        return 1;
    }

    fprintf(stderr, "[ERRO COORDS] Não foi possível extrair coordenadas completas\n");
    return 0;
}

// Função OTIMIZADA para obter coordenadas de múltiplas cidades em UMA ÚNICA requisição
int obter_coordenadas_multiplas(char cidades[][100], int num_cidades, double latitudes[], double longitudes[]) {
    if (!cidades || num_cidades <= 0 || !latitudes || !longitudes) return 0;

    fprintf(stderr, "[DEBUG COORDS BATCH] Buscando coordenadas de %d cidades em uma única requisição\n", num_cidades);

    // Monta lista de cidades para o prompt
    char lista_cidades[4096] = "";
    for (int i = 0; i < num_cidades; i++) {
        strcat(lista_cidades, cidades[i]);
        if (i < num_cidades - 1) strcat(lista_cidades, ", ");
    }

    // Monta prompt usando template do config.h
    char prompt[8192];
    snprintf(prompt, sizeof(prompt), PROMPT_COORDENADAS_MULTIPLAS, lista_cidades);

    fprintf(stderr, "[DEBUG COORDS BATCH] Consultando IA (modelo: %s) para %d cidades\n",
            MODELO_GEMINI_GRAFO, num_cidades);
    fflush(stderr);

    // Consulta a IA usando modelo específico para grafos (sem histórico)
    char* resposta = consultar_gemini_com_modelo(prompt, NULL, "", MODELO_GEMINI_GRAFO);

    if (!resposta) {
        fprintf(stderr, "[ERRO COORDS BATCH] IA não retornou resposta\n");
        return 0;
    }

    fprintf(stderr, "[DEBUG COORDS BATCH] Resposta da IA:\n%s\n", resposta);
    fflush(stderr);

    // Parse da resposta - processa linha por linha
    int coords_encontradas = 0;
    char* resposta_copia = strdup(resposta); // Cópia para não modificar original
    char* linha = strtok(resposta_copia, "\n\r");

    while (linha && coords_encontradas < num_cidades) {
        // Remove espaços em branco no início
        while (*linha && isspace(*linha)) linha++;

        // Ignora linhas vazias ou que começam com caracteres especiais
        if (*linha == '\0' || *linha == '#' || *linha == '*' || *linha == '-' || *linha == '=') {
            linha = strtok(NULL, "\n\r");
            continue;
        }

        // Procura pelo padrão: CIDADE|LAT:valor|LNG:valor
        char* sep1 = strchr(linha, '|');
        if (!sep1) {
            linha = strtok(NULL, "\n\r");
            continue;
        }

        // Extrai nome da cidade
        char nome_cidade[100] = {0};
        size_t len_nome = sep1 - linha;
        if (len_nome > 0 && len_nome < 100) {
            strncpy(nome_cidade, linha, len_nome);
            nome_cidade[len_nome] = '\0';

            // Remove espaços extras
            char* end = nome_cidade + strlen(nome_cidade) - 1;
            while (end > nome_cidade && isspace(*end)) *end-- = '\0';
            char* start = nome_cidade;
            while (*start && isspace(*start)) start++;
            if (start != nome_cidade) memmove(nome_cidade, start, strlen(start) + 1);
        }

        // Encontra LAT:
        char* lat_pos = strstr(sep1, "LAT:");
        char* lng_pos = strstr(sep1, "LNG:");

        if (lat_pos && lng_pos) {
            // Extrai latitude
            double lat = 0.0;
            char* lat_val = lat_pos + 4;
            while (*lat_val && isspace(*lat_val)) lat_val++;

            char lat_str[32] = {0};
            int idx = 0;
            while (*lat_val && (isdigit(*lat_val) || *lat_val == '.' || *lat_val == '-') && idx < 31) {
                lat_str[idx++] = *lat_val++;
            }
            lat_str[idx] = '\0';
            lat = atof(lat_str);

            // Extrai longitude
            double lng = 0.0;
            char* lng_val = lng_pos + 4;
            while (*lng_val && isspace(*lng_val)) lng_val++;

            char lng_str[32] = {0};
            idx = 0;
            while (*lng_val && (isdigit(*lng_val) || *lng_val == '.' || *lng_val == '-') && idx < 31) {
                lng_str[idx++] = *lng_val++;
            }
            lng_str[idx] = '\0';
            lng = atof(lng_str);

            // Encontra qual cidade corresponde
            for (int i = 0; i < num_cidades; i++) {
                if (strcasecmp(nome_cidade, cidades[i]) == 0) {
                    latitudes[i] = lat;
                    longitudes[i] = lng;
                    coords_encontradas++;
                    fprintf(stderr, "[DEBUG COORDS BATCH] ✓ %s: %.4f, %.4f\n", cidades[i], lat, lng);
                    break;
                }
            }
        }

        linha = strtok(NULL, "\n\r");
    }

    free(resposta_copia);
    free(resposta);

    fprintf(stderr, "[DEBUG COORDS BATCH] Total de coordenadas encontradas: %d de %d\n",
            coords_encontradas, num_cidades);

    return coords_encontradas;
}

