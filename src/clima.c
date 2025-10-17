/* clima.c - Integração com API OpenWeather
 * GenieC - Assistente Inteligente
 */

#include "clima.h"
#include "http_utils.h"
#include "config.h"
#include "../api_key.h"
#include <curl/curl.h>
#include <cjson/cJSON.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Obtém dados do clima da API OpenWeather
DataClima obter_dados_clima(const char* cidade) {
    DataClima clima = {0};
    clima.valid = 0;

    // Codifica a cidade para URL
    char* cidade_encoded = url_encode(cidade);
    if (!cidade_encoded) {
        return clima;
    }

    // Monta a URL da API
    char url[512];
    snprintf(url, sizeof(url),
             "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=pt_br",
             cidade_encoded, API_KEY_WEATHER);

    // Faz a requisição HTTP
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, HTTP_TIMEOUT);
        curl_easy_setopt(curl, CURLOPT_CONNECTTIMEOUT, HTTP_CONNECT_TIMEOUT);

        res = curl_easy_perform(curl);

        if (res == CURLE_OK) {
            cJSON *json = cJSON_Parse(chunk.memory);
            if (json) {
                cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");
                cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather");
                cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");

                if (main && weather_array && cJSON_IsArray(weather_array) &&
                    name && cJSON_IsString(name)) {

                    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
                    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);

                    if (temp && cJSON_IsNumber(temp) && weather_item) {
                        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item, "description");

                        clima.temperatura = (float)cJSON_GetNumberValue(temp);
                        strncpy(clima.cidade, cJSON_GetStringValue(name), sizeof(clima.cidade) - 1);
                        clima.cidade[sizeof(clima.cidade) - 1] = '\0';

                        if (description && cJSON_IsString(description)) {
                            strncpy(clima.description, cJSON_GetStringValue(description),
                                   sizeof(clima.description) - 1);
                            clima.description[sizeof(clima.description) - 1] = '\0';
                        }

                        clima.valid = 1;
                    }
                }
                cJSON_Delete(json);
            }
        }
        curl_easy_cleanup(curl);
    }

    curl_free(cidade_encoded);
    free(chunk.memory);

    return clima;
}
