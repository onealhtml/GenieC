#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>

#include <curl/curl.h>
#include <cjson/cJSON.h>

#include "api_key.h"
#include "limpar_tela.h"
#include "dormir.h"

// --- Configurações Iniciais ---
#define API_BASE_URL "https://generativelanguage.googleapis.com/v1beta/models/gemini-1.5-flash:generateContent?key="
#define MAX_PROMPT_SIZE 10000

// --- Declaração das Funções ---
char* criar_payload_json(const char* prompt);
char* extrair_texto_da_resposta(const char* resposta_json);

// --- Requisição HTTP ---
struct MemoryStruct { // Estrutura para armazenar a resposta da requisição HTTP (padrão cURL)
    char *memory;
    size_t size;
};

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp); // Callback para armazenar a resposta (padrão cURL)
char* fazer_requisicao_http(const char* url, const char* payload); // Função que faz a requisição HTTP usando cURL

int main(){
    setlocale(LC_ALL, "Portuguese_Brazil.utf8");
    system("chcp 65001");

    while (1) { // Loop infinito
        limpar_tela(); // Limpa a tela do console
        char minha_pergunta[MAX_PROMPT_SIZE]; // Buffer para armazenar a pergunta do usuário

        printf("Digite sua pergunta para o Gemini (ou '0' para sair): ");
        fgets(minha_pergunta, sizeof(minha_pergunta), stdin);
        minha_pergunta[strcspn(minha_pergunta, "\n")] = 0;

        // Condição de saída
        if (strcmp(minha_pergunta, "0") == 0) {
            break;
        }

        char* payload = criar_payload_json(minha_pergunta);
        if (payload == NULL) {
            fprintf(stderr, "Erro: Não foi possível criar o pacote JSON.\n");
            printf("Pressione Enter para continuar...\n");
            getchar(); // Espera o usuário pressionar Enter
            continue; // Volta para o início do loop
        }

        char url_completa[512];
        strcpy(url_completa, API_BASE_URL);
        strcat(url_completa, API_KEY);

        char* resposta_bruta = fazer_requisicao_http(url_completa, payload);
        if (resposta_bruta == NULL) {
            fprintf(stderr, "Erro: A comunicação com a API falhou.\n");
            free(payload);
            printf("Pressione Enter para continuar...\n");
            getchar(); // Espera o usuário pressionar Enter
            continue; // Volta para o início do loop
        }

        char* texto_final = extrair_texto_da_resposta(resposta_bruta);
         if (texto_final == NULL) {
            fprintf(stderr, "Erro: Não foi possível extrair o texto da resposta da API.\n");
            fprintf(stderr, "Resposta bruta recebida: %s\n", resposta_bruta);
            free(payload);
            free(resposta_bruta);
            printf("Pressione Enter para continuar...\n");
            getchar(); // Espera o usuário pressionar Enter
            continue; // Volta para o início do loop
        }

        printf("\n--- Resposta do Gemini ---\n%s\n", texto_final);

        // Libera a memória alocada dentro do loop
        free(payload);
        free(resposta_bruta);
        free(texto_final);

        printf("\n");
        printf("Pressione Enter para continuar...\n");
        getchar(); // Espera o usuário pressionar Enter // Pausa para o usuário ler a resposta
    }

    printf("\nFinalizando o programa...\n");
    dormir(2000);
    return 0;
}

// ==============================================================================
// Funções
// ==============================================================================

// Cria o payload JSON usando a biblioteca cJSON.
char* criar_payload_json(const char* prompt) {
    // Passo 1: Criamos os objetos necessários para construir o JSON
    cJSON *root = cJSON_CreateObject();           // Objeto principal/raiz
    cJSON *contents_array = cJSON_CreateArray();  // Array de conteúdos
    cJSON *content_item = cJSON_CreateObject();   // Item de conteúdo
    cJSON *parts_array = cJSON_CreateArray();     // Array de partes
    cJSON *part_item = cJSON_CreateObject();      // Item de parte

    // Passo 2: Configuramos o texto da pergunta
    // Adiciona uma string ao objeto "part_item" com a chave "text"
    cJSON_AddItemToObject(part_item, "text", cJSON_CreateString(prompt));

    // Passo 3: Construímos a estrutura do JSON de dentro para fora
    // Primeiro adicionamos o objeto part_item ao array parts_array
    cJSON_AddItemToArray(parts_array, part_item);

    // Depois adicionamos o array parts_array ao objeto content_item com a chave "parts"
    cJSON_AddItemToObject(content_item, "parts", parts_array);

    // Em seguida adicionamos o objeto content_item ao array contents_array
    cJSON_AddItemToArray(contents_array, content_item);

    // Por fim, adicionamos o array contents_array ao objeto root com a chave "contents"
    cJSON_AddItemToObject(root, "contents", contents_array);

    // Passo 4: Convertemos o objeto JSON para uma string
    char *json_string_copy = cJSON_PrintUnformatted(root);

    // Passo 5: Liberamos a memória do objeto JSON
    cJSON_Delete(root);

    // Retornamos a string JSON que foi criada
    return json_string_copy;
}

// Extrai o texto da resposta JSON usando cJSON.
char* extrair_texto_da_resposta(const char* resposta_json) {
    // Variável que irá armazenar o texto final extraído
    char* texto_extraido = NULL;

    // Passo 1: Converter a string JSON em um objeto cJSON
    cJSON *root = cJSON_Parse(resposta_json);

    // Verificar se o parsing foi bem-sucedido
    if (root == NULL) {
        fprintf(stderr, "Erro ao parsear o JSON da resposta.\n");
        return NULL;
    }

    // Passo 2: Declarar variáveis para navegar na estrutura do JSON
    cJSON *candidates, *first_candidate, *content, *parts, *first_part, *text_node;

    // Passo 3: Navegar na estrutura JSON para encontrar o texto
    // Formato esperado: {"candidates": [{"content": {"parts": [{"text": "resposta aqui"}]}}]}

    // Verificar se existe o array "candidates"
    if (cJSON_GetObjectItemCaseSensitive(root, "candidates") &&
        cJSON_GetObjectItemCaseSensitive(root, "candidates")->type == cJSON_Array) {

        // Pegar o array "candidates"
        candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");

        // Verificar se o array tem pelo menos um elemento
        if (cJSON_GetArraySize(candidates) > 0) {
            // Pegar o primeiro elemento do array
            first_candidate = cJSON_GetArrayItem(candidates, 0);

            // Pegar o objeto "content" dentro do primeiro candidato
            content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");

            // Verificar se encontrou o objeto "content"
            if (content) {
                // Pegar o array "parts" dentro do objeto "content"
                parts = cJSON_GetObjectItemCaseSensitive(content, "parts");

                // Verificar se encontrou o array "parts"
                if (parts && parts->type == cJSON_Array) {
                    // Verificar se o array tem pelo menos um elemento
                    if (cJSON_GetArraySize(parts) > 0) {
                        // Pegar o primeiro elemento do array
                        first_part = cJSON_GetArrayItem(parts, 0);

                        // Pegar o objeto "text" dentro do primeiro elemento
                        text_node = cJSON_GetObjectItemCaseSensitive(first_part, "text");

                        // Verificar se encontrou o objeto "text" e se ele é uma string
                        if (text_node && text_node->type == cJSON_String) {
                            // Pegar o valor da string
                            const char* texto = text_node->valuestring;

                            // Criar uma cópia da string para retornar
                            texto_extraido = strdup(texto);
                        }
                    }
                }
            }
        }
    }

    // Passo 4: Liberar a memória do objeto JSON
    cJSON_Delete(root);

    // Passo 5: Retornar o texto extraído (ou NULL se não foi encontrado)
    return texto_extraido;
}


// --- Funções do cURL ---
char* fazer_requisicao_http(const char* url, const char* payload) {
    // Inicializa as variáveis necessárias
    CURL *curl_handle;                // Manipulador do cURL
    CURLcode res;                     // Código de resultado da operação
    struct MemoryStruct chunk;        // Estrutura para armazenar a resposta

    // Inicializa a estrutura de memória
    chunk.memory = malloc(1);         // Aloca um byte inicial
    chunk.size = 0;                   // Inicializa o tamanho como zero

    // Inicializa a biblioteca cURL
    curl_global_init(CURL_GLOBAL_ALL);
    curl_handle = curl_easy_init();

    // Verifica se a inicialização deu certo
    if (!curl_handle) {
        fprintf(stderr, "Erro ao iniciar o cURL\n");
        free(chunk.memory);
        return NULL;
    }

    // Configura os cabeçalhos da requisição
    struct curl_slist *headers = NULL;
    headers = curl_slist_append(headers, "Content-Type: application/json");

    // Configura as opções da requisição
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);                           // Define a URL
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);                // Define os cabeçalhos
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload);                // Define os dados a serem enviados
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Define a função de callback
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);          // Define onde salvar os dados

    // Executa a requisição
    res = curl_easy_perform(curl_handle);

    // Verifica se houve erro
    if (res != CURLE_OK) {
        fprintf(stderr, "A requisição cURL falhou: %s\n", curl_easy_strerror(res));
        free(chunk.memory);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl_handle);
        curl_global_cleanup();
        return NULL;
    }

    // Libera os recursos utilizados
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl_handle);
    curl_global_cleanup();

    // Retorna a resposta obtida
    return chunk.memory;
}

static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Calcula o tamanho real dos dados recebidos
    size_t realsize = size * nmemb;

    // Converte o ponteiro userp para o tipo correto
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Aumenta o tamanho da memória para caber os novos dados
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);

    // Verifica se conseguiu alocar memória
    if (ptr == NULL) {
        printf("Erro: não foi possível alocar memória no callback!\n");
        return 0;
    }

    // Atualiza o ponteiro para a nova área de memória
    mem->memory = ptr;

    // Copia os novos dados para a memória
    memcpy(&(mem->memory[mem->size]), contents, realsize);

    // Atualiza o tamanho da memória
    mem->size += realsize;
    // Adiciona um terminador nulo ao final da string
    mem->memory[mem->size] = 0;

    // Retorna o tamanho real para informar que a operação foi bem-sucedida
    return realsize;
}