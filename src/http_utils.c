/* http_utils.c - Utilitários para requisições HTTP
 * GenieC - Assistente Inteligente
 */

#include "http_utils.h"
#include "config.h"
#include "../dormir.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Callback para armazenar a resposta da requisição HTTP
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    size_t realsize = size * nmemb;
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    char *ptr = realloc(mem->memory, mem->size + realsize + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Erro: não foi possível alocar memória no callback!\n");
        return 0;
    }

    mem->memory = ptr;
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    mem->size += realsize;
    mem->memory[mem->size] = 0;

    return realsize;
}

// Função principal para fazer requisição HTTP
char* fazer_requisicao_http(const char* url, const char* payload) {
    CURL *curl_handle;
    CURLcode res;
    struct MemoryStruct chunk;
    long http_code = 0;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    if (!curl_handle) {
        fprintf(stderr, "Erro ao iniciar o cURL\n");
        free(chunk.memory);
        return NULL;
    }

    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configurações da requisição
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);
    curl_easy_setopt(curl_handle, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
    curl_easy_setopt(curl_handle, CURLOPT_CONNECTTIMEOUT, HTTP_CONNECT_TIMEOUT);

    // Executa a requisição
    res = curl_easy_perform(curl_handle);

    // Verifica erro de conexão
    if (res != CURLE_OK) {
        fprintf(stderr, "\n❌ Requisição falhou: %s\n", curl_easy_strerror(res));

        if (res == CURLE_OPERATION_TIMEDOUT) {
            fprintf(stderr, "   ⏱️  Timeout: A operação demorou muito.\n");
        } else if (res == CURLE_COULDNT_CONNECT) {
            fprintf(stderr, "   🌐 Não foi possível conectar ao servidor.\n");
        } else if (res == CURLE_COULDNT_RESOLVE_HOST) {
            fprintf(stderr, "   🔍 Não foi possível resolver o nome do host.\n");
        }

        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return NULL;
    }

    // Verifica código HTTP
    curl_easy_getinfo(curl_handle, CURLINFO_RESPONSE_CODE, &http_code);

    if (http_code != 200) {
        fprintf(stderr, "\n❌ Erro HTTP %ld\n", http_code);

        if (http_code == 400) {
            fprintf(stderr, "   📋 Requisição inválida. Verifique os dados enviados.\n");
        } else if (http_code == 401) {
            fprintf(stderr, "   🔑 Não autorizado. Verifique sua API key.\n");
        } else if (http_code == 403) {
            fprintf(stderr, "   🚫 Acesso negado.\n");
        } else if (http_code == 404) {
            fprintf(stderr, "   🔍 Recurso não encontrado.\n");
        } else if (http_code == 429) {
            fprintf(stderr, "   ⏸️  Limite de requisições atingido. Aguarde um momento.\n");
        } else if (http_code >= 500) {
            fprintf(stderr, "   🔧 Erro no servidor. Tente novamente mais tarde.\n");
        }

        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return NULL;
    }

    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    return chunk.memory;
}

// Função com retry e backoff exponencial
char* fazer_requisicao_http_com_retry(const char* url, const char* payload, int max_retries) {
    int retry_delay = 1000; // 1 segundo inicial

    for (int tentativa = 0; tentativa < max_retries; tentativa++) {
        if (tentativa > 0) {
            printf("\n⏳ Tentativa %d de %d (aguardando %d segundos)...\n",
                   tentativa + 1, max_retries, retry_delay / 1000);
            dormir(retry_delay);
            retry_delay *= 2; // Backoff exponencial: 1s, 2s, 4s, 8s...
        }

        char* resposta = fazer_requisicao_http(url, payload);

        if (resposta != NULL) {
            if (tentativa > 0) {
                printf("✅ Sucesso na tentativa %d!\n", tentativa + 1);
            }
            return resposta;
        }

        if (tentativa < max_retries - 1) {
            fprintf(stderr, "   🔄 Tentando novamente...\n");
        }
    }

    fprintf(stderr, "\n❌ Todas as %d tentativas falharam.\n", max_retries);
    return NULL;
}

// Função para codificar URL
char* url_encode(const char* str) {
    CURL *curl = curl_easy_init();
    if (!curl) return NULL;

    char *encoded = curl_easy_escape(curl, str, 0);
    curl_easy_cleanup(curl);
    return encoded;
}
