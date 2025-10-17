#ifndef HTTP_UTILS_H
#define HTTP_UTILS_H

#include <curl/curl.h>
#include <stddef.h>

// Estrutura para armazenar a resposta da requisição HTTP
struct MemoryStruct {
    char *memory;
    size_t size;
};

// Callback para cURL (precisa ser declarado para uso em clima.c)
size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp);

// Funções HTTP
char* fazer_requisicao_http_com_retry(const char* url, const char* payload, int max_retries);
char* fazer_requisicao_http(const char* url, const char* payload);
char* url_encode(const char* str);

#endif // HTTP_UTILS_H
