#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <json-c/json.h>
#include <locale.h>

// Estrutura simples para armazenar a resposta da API
// Pense nela como um vetor de caracteres que cresce conforme necessário
typedef struct {
    char* texto;  // Ponteiro para o texto da resposta
    size_t tamanho;  // Tamanho atual do texto
} DadosResposta;

// Esta função é chamada pelo curl cada vez que recebe dados da internet
// Funciona assim: o curl baixa os dados em pedaços e chama esta função para cada pedaço
size_t salvarResposta(void* conteudo, size_t tamanhoItem, size_t numItens, DadosResposta* dados) {
    // Calcula o tamanho real dos dados recebidos
    size_t tamanhoReal = tamanhoItem * numItens;

    // Aumenta o espaço na memória para caber os novos dados
    // É como se estivéssemos aumentando o tamanho de um vetor
    char* novoEspaco = realloc(dados->texto, dados->tamanho + tamanhoReal + 1);

    // Verifica se conseguiu aumentar o espaço
    if (novoEspaco == NULL) {
        printf("Erro: Não foi possível alocar memória para a resposta.\n");
        return 0;  // Retorna erro
    }

    // Atualiza o ponteiro com o novo espaço alocado
    dados->texto = novoEspaco;

    // Copia os novos dados para o final do texto existente
    // É como adicionar novos elementos no final de um vetor
    memcpy(&(dados->texto[dados->tamanho]), conteudo, tamanhoReal);

    // Atualiza o tamanho total
    dados->tamanho += tamanhoReal;

    // Adiciona o caractere nulo no final (para que seja uma string válida)
    dados->texto[dados->tamanho] = '\0';

    // Retorna o tamanho processado com sucesso
    return tamanhoReal;
}

// Função para substituir espaços por %20 nas URLs
void codificarURL(const char* original, char* resultado, size_t tamanhoMaximo) {
    size_t posResultado = 0;

    // Percorre cada caractere da string original
    for (size_t i = 0; original[i] != '\0' && posResultado < tamanhoMaximo - 1; i++) {
        // Se encontrar um espaço, substitui por %20
        if (original[i] == ' ' && posResultado < tamanhoMaximo - 3) {
            resultado[posResultado++] = '%'; // Adiciona o primeiro caractere de %20
            resultado[posResultado++] = '2'; // Adiciona o segundo caractere de %20
            resultado[posResultado++] = '0'; // Adiciona o terceiro caractere de %20
        } else {
            // Caso contrário, mantém o caractere original
            resultado[posResultado++] = original[i];
        }
    }

    // Finaliza a string com caractere nulo
    resultado[posResultado] = '\0';
}

// Função para buscar as coordenadas de uma cidade
int buscarCoordenadas(const char* cidade, double* latitude, double* longitude) {
    // Inicializa a estrutura para armazenar a resposta da API
    DadosResposta resposta;
    resposta.texto = malloc(1);  // Aloca um espaço mínimo inicial
    resposta.tamanho = 0;

    // Prepara a URL com o nome da cidade (substituindo espaços)
    char cidadeCodificada[256];
    codificarURL(cidade, cidadeCodificada, sizeof(cidadeCodificada));

    // Monta a URL completa para a API de geocodificação
    char url[1024];
    snprintf(url, sizeof(url),
             "https://geocoding-api.open-meteo.com/v1/search?name=%s&count=1&language=pt&format=json",
             cidadeCodificada);

    // Inicializa o curl (biblioteca para fazer requisições HTTP)
    CURL* curl = curl_easy_init();
    if (!curl) {
        printf("Erro: Não foi possível inicializar o curl.\n");
        free(resposta.texto);
        return 0;
    }

    // Configura as opções do curl
    curl_easy_setopt(curl, CURLOPT_URL, url);  // Define a URL
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, salvarResposta);  // Define a função para salvar a resposta
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&resposta);  // Passa a estrutura para salvar os dados

    // Executa a requisição
    CURLcode resultado = curl_easy_perform(curl);
    if (resultado != CURLE_OK) {
        printf("Erro na requisição: %s\n", curl_easy_strerror(resultado));
        curl_easy_cleanup(curl);
        free(resposta.texto);
        return 0;
    }

    // Analisa a resposta JSON
    struct json_object* json = json_tokener_parse(resposta.texto);
    if (!json) {
        printf("Erro: Não foi possível interpretar a resposta JSON.\n");
        curl_easy_cleanup(curl);
        free(resposta.texto);
        return 0;
    }

    // Procura pelo campo "results" no JSON
    struct json_object* resultados = NULL;
    if (json_object_object_get_ex(json, "results", &resultados) &&
        json_object_get_type(resultados) == json_type_array &&
        json_object_array_length(resultados) > 0) {

        // Pega o primeiro resultado
        struct json_object* primeiro = json_object_array_get_idx(resultados, 0);

        // Extrai latitude e longitude
        struct json_object* latObj = NULL;
        struct json_object* lonObj = NULL;

        if (json_object_object_get_ex(primeiro, "latitude", &latObj) &&
            json_object_object_get_ex(primeiro, "longitude", &lonObj)) {

            // Salva os valores nos ponteiros fornecidos
            *latitude = json_object_get_double(latObj);
            *longitude = json_object_get_double(lonObj);

            // Limpa os recursos
            json_object_put(json);
            curl_easy_cleanup(curl);
            free(resposta.texto);

            return 1;  // Sucesso
        }
    }

    // Se chegou aqui, não encontrou a cidade
    printf("Cidade não encontrada. Verifique o nome e tente novamente.\n");

    // Limpa os recursos
    json_object_put(json);
    curl_easy_cleanup(curl);
    free(resposta.texto);

    return 0;  // Falha
}

// Função para obter a previsão do tempo usando as coordenadas
void buscarPrevisaoTempo(double latitude, double longitude) {
    // Inicializa a estrutura para armazenar a resposta da API
    DadosResposta resposta;
    resposta.texto = malloc(1);  // Aloca um espaço mínimo inicial
    resposta.tamanho = 0;

    // Monta a URL para a API de previsão do tempo
    char url[1024];
    sprintf(url, "https://api.open-meteo.com/v1/forecast?latitude=%.4f&longitude=%.4f"
                 "&current=temperature_2m,relative_humidity_2m,weather_code,wind_speed_10m&timezone=auto",
                 latitude, longitude);

    // Inicializa o curl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    CURL* curl = curl_easy_init();

    if (curl) {
        // Configura as opções do curl
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, salvarResposta);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void*)&resposta);

        // Executa a requisição
        CURLcode resultado = curl_easy_perform(curl);

        if (resultado != CURLE_OK) {
            printf("Erro na requisição: %s\n", curl_easy_strerror(resultado));
        } else {
            // Analisa a resposta JSON
            struct json_object* json = json_tokener_parse(resposta.texto);

            if (json == NULL) {
                printf("Erro: Não foi possível interpretar a resposta JSON.\n");
            } else {
                // Extrai os dados de previsão do tempo
                struct json_object* current = NULL;

                if (json_object_object_get_ex(json, "current", &current)) {
                    // Variáveis para armazenar os dados extraídos
                    double temperatura = 0;
                    double umidade = 0;
                    double vento = 0;
                    int codigoClima = 0;
                    int dadosValidos = 1; // Flag para verificar se temos dados válidos

                    // Extrai a temperatura
                    struct json_object* tempObj = NULL;
                    if (json_object_object_get_ex(current, "temperature_2m", &tempObj)) {
                        temperatura = json_object_get_double(tempObj);
                    } else {
                        dadosValidos = 0;
                    }

                    // Extrai a umidade
                    struct json_object* umidadeObj = NULL;
                    if (json_object_object_get_ex(current, "relative_humidity_2m", &umidadeObj)) {
                        umidade = json_object_get_double(umidadeObj);
                    } else {
                        dadosValidos = 0;
                    }

                    // Extrai a velocidade do vento
                    struct json_object* ventoObj = NULL;
                    if (json_object_object_get_ex(current, "wind_speed_10m", &ventoObj)) {
                        vento = json_object_get_double(ventoObj);
                    } else {
                        dadosValidos = 0;
                    }

                    // Extrai o código do clima
                    struct json_object* climaObj = NULL;
                    if (json_object_object_get_ex(current, "weather_code", &climaObj)) {
                        codigoClima = json_object_get_int(climaObj);
                    } else {
                        dadosValidos = 0;
                    }

                    // Se tiver dados válidos, exibe o relatório
                    if (dadosValidos) {
                        printf("Temperatura atual: %.1f°C\n", temperatura);
                        printf("Umidade relativa: %.1f%%\n", umidade);
                        printf("Velocidade do vento: %.1f km/h\n", vento);

                        printf("Condição: ");
                        // Converte o código do clima para texto
                        if (codigoClima == 0) printf("Céu limpo\n");
                        else if (codigoClima == 1) printf("Principalmente limpo\n");
                        else if (codigoClima == 2) printf("Parcialmente nublado\n");
                        else if (codigoClima == 3) printf("Nublado\n");
                        else if (codigoClima >= 45 && codigoClima <= 48) printf("Nevoeiro\n");
                        else if (codigoClima >= 51 && codigoClima <= 55) printf("Garoa\n");
                        else if (codigoClima >= 61 && codigoClima <= 65) printf("Chuva\n");
                        else if (codigoClima >= 71 && codigoClima <= 77) printf("Neve\n");
                        else if (codigoClima >= 80 && codigoClima <= 82) printf("Pancadas de chuva\n");
                        else if (codigoClima >= 95 && codigoClima <= 99) printf("Tempestade\n");
                        else printf("Código %d (desconhecido)\n", codigoClima);
                    } else {
                        printf("Não foi possível obter todos os dados meteorológicos.\n");
                    }
                } else {
                    printf("Dados de previsão do tempo incompletos ou inválidos.\n");
                }

                // Libera o objeto JSON
                json_object_put(json);
            }
        }
        // Limpa os recursos do curl
        curl_easy_cleanup(curl);
    }

    // Finaliza o curl e libera a memória alocada
    curl_global_cleanup();
    free(resposta.texto);
}

int main() {
    // Configura o programa para usar caracteres especiais em português
    setlocale(LC_ALL, "Portuguese_Brazil.utf8");
    system("chcp 65001");

    system("cls");  // Limpa a tela do console

    // Variável para armazenar o nome da cidade
    char cidade[100];

    // Solicita o nome da cidade ao usuário
    printf("Digite o nome da cidade: ");
    fgets(cidade, sizeof(cidade), stdin);

    // Remove o caractere de nova linha do final
    cidade[strcspn(cidade, "\n")] = 0;

    // Variáveis para armazenar as coordenadas da cidade
    double latitude, longitude;

    // Busca as coordenadas da cidade
    if (buscarCoordenadas(cidade, &latitude, &longitude)) {
        // Se encontrou a cidade, exibe os dados meteorológicos
        printf("\n==== PREVISÃO DO TEMPO PARA %s ====\n", cidade);
        printf("Coordenadas: %.4f, %.4f\n", latitude, longitude);
        buscarPrevisaoTempo(latitude, longitude);
        printf("====================================\n");
    }

    // Pausa antes de encerrar o programa
    system("pause");
    return 0;
}