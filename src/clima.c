/* clima.c - Integração com API OpenWeather
 * GenieC - Assistente Inteligente
 */

#include "clima.h"
#include "http_utils.h"
#include "config.h"
#include "env_loader.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Obtém dados do clima da API OpenWeather
DataClima obter_dados_clima(const char* cidade) {
    // Inicializa estrutura de dados do clima
    DataClima clima = {0};
    clima.valid = 0;

    // Obtém a API key das variáveis de ambiente
    const char* api_key = obter_env("OPENWEATHER_API_KEY");
    if (!api_key) {
        fprintf(stderr, "Erro: API key do OpenWeather não encontrada.\n");
        return clima;
    }

    // Codifica a cidade para URL
    char* cidade_encoded = url_encode(cidade);
    if (!cidade_encoded) {
        return clima;
    }

    // Monta a URL da API
    char url[512];
    snprintf(url, sizeof(url),
             "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=pt_br",
             cidade_encoded, api_key);

    // Faz a requisição HTTP
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    // Aloca memória inicial para resposta
    chunk.memory = malloc(1);
    chunk.size = 0;

    // Inicializa cURL
    curl = curl_easy_init();
    if (curl) {
        // Configura opções do cURL
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_CONNECT_TIMEOUT);

        // Executa a requisição
        res = curl_easy_perform(curl);

        // Processa resposta se bem-sucedida
        if (res == CURLE_OK) {
            // Parse do JSON retornado
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json) {
                // Extrai campos do JSON
                cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");
                cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather");
                cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");

                // Valida estrutura do JSON
                if (main && weather_array && cJSON_IsArray(weather_array) &&
                    name && cJSON_IsString(name)) {

                    // Extrai temperatura e descrição
                    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
                    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);

                    if (temp && cJSON_IsNumber(temp) && weather_item) {
                        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item, "description");

                        // Preenche estrutura de dados
                        clima.temperatura = (float)cJSON_GetNumberValue(temp);
                        strncpy(clima.cidade, cJSON_GetStringValue(name), sizeof(clima.cidade) - 1);
                        clima.cidade[sizeof(clima.cidade) - 1] = '\0';

                        if (description && cJSON_IsString(description)) {
                            strncpy(clima.description, cJSON_GetStringValue(description),
                                   sizeof(clima.description) - 1);
                            clima.description[sizeof(clima.description) - 1] = '\0';
                        }

                        // Marca como válido
                        clima.valid = 1;
                    }
                }
                // Libera JSON
                cJSON_Delete(json);
            }
        }
        // Limpa cURL
        curl_easy_cleanup(curl);
    }

    // Libera memória alocada
    curl_free(cidade_encoded);
    free(chunk.memory);

    return clima;
}
