/* GenieC - Assistente Inteligente
 * Lorenzo Farias, Bernardo Soares Nunes e Pedro Cabral Buchaim
 * Projeto de Programação para Resolução de Problemas
 * Programação para Resolução de Problemas
 * Profa. Dra. Daniela Bagatini
 * Universidade de Santa Cruz do Sul (UNISC).
 */

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
#define MODELO_GEMINI "gemini-2.5-flash" // Nome do modelo Gemini
#define API_BASE_URL "https://generativelanguage.googleapis.com/v1beta/models/" MODELO_GEMINI ":generateContent?key=" //
#define MAX_PROMPT_SIZE 10000 // Tamanho máximo do prompt
#define MAX_HISTORY_SIZE 50   // Máximo de turnos no histórico
#define MAX_CITY_NAME 100     // Tamanho máximo do nome da cidade

// --- Estrutura para dados do clima ---
typedef struct {
    char city[MAX_CITY_NAME]; // Nome da cidade
    float temperature;        // Temperatura em Celsius
    char description[100];    // Descrição do clima (ex: "ensolarado", "chuva")
    int valid;                // Flag para indicar se os dados são válidos
} WeatherData;

// --- Estrutura para armazenar o histórico da conversa ---
typedef struct {
    char* role;     // "user" ou "model"
    char* text;     // Conteúdo da mensagem
} MessageTurn;

typedef struct {
    MessageTurn* turns;  // Array de turnos da conversa
    int count;           // Número atual de turnos
    int capacity;        // Capacidade máxima
} ChatHistory;

// --- Prompt Base do Sistema ---
#define SYSTEM_PROMPT "Você é o GenieC, um assistente pessoal para responder dúvidas do dia a dia. Siga estas diretrizes:\n\n" \
"COMUNICAÇÃO:\n" \
"- Responda de forma clara, precisa e educada\n" \
"- Seja conciso mas completo - evite respostas muito longas, no máximo um parágrafo de texto\n" \
"- Use linguagem natural e acessível\n" \
"- Evite usar marcadores de formatação especial\n" \
"- Se não souber algo, admita honestamente\n\n" \
"PESQUISA E CONTEXTO:\n" \
"- Use ferramentas de pesquisa quando necessário para informações atualizadas\n" \
"- Leve em conta o Brasil quando perguntarem sobre horários e coisas afins\n" \
"- Para perguntas sobre temperatura, clima, horários, eventos locais ou informações específicas de localização, pergunte a cidade/região antes de responder, mas somente quando o usuário não falar a localidade\n" \
"- Para perguntas ambíguas, peça esclarecimentos específicos\n\n" \
"IMPORTANTE:\n" \
"- Quando precisar de localização ou contexto adicional, peça ao usuário para reformular a pergunta com essas informações\n" \
"- Forneça respostas práticas e úteis sempre que possível"


// --- Declaração das Funções ---
void mostrar_arte_inicial();
WeatherData obter_dados_clima(const char* cidade);
char* url_encode(const char* str);
void menu_com_clima(WeatherData weather);
void mostrar_ajuda();
char* criar_payload_json_com_historico(const char* prompt, ChatHistory* history);
char* extrair_texto_da_resposta(const char* resposta_json);

// --- Funções de Histórico do Chat ---
ChatHistory* inicializar_chat_historico();
void adicionar_turno(ChatHistory* history, const char* role, const char* text);
void liberar_chat_history(ChatHistory* history);
void exibir_historico(ChatHistory* history);
void mostrar_loading();

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
    limpar_tela(); // Limpa a tela ao iniciar

    // Mostra a arte ASCII inicial
    mostrar_arte_inicial();

    // Solicita a cidade do usuário
    char cidade[MAX_CITY_NAME];
    printf("\n\033[1;36m🌍 Digite o nome da sua cidade para obter informações do clima:\033[0m ");
    fgets(cidade, sizeof(cidade), stdin);
    cidade[strcspn(cidade, "\n")] = 0; // Remove quebra de linha

    // Obtém dados do clima
    printf("\n\033[33m🌤️ Obtendo informações do clima...\033[0m\n");
    WeatherData clima = obter_dados_clima(cidade);

    limpar_tela();
    menu_com_clima(clima);

    // Inicializa o histórico do chat
    ChatHistory* chat_historico = inicializar_chat_historico();
    if (chat_historico == NULL) {
        fprintf(stderr, "Erro ao inicializar o histórico do chat.\n");
        return 1;
    }

    while (1) { // Loop infinito
        char minha_pergunta[MAX_PROMPT_SIZE]; // Buffer para armazenar a pergunta do usuário
        printf("Você: ");
        fgets(minha_pergunta, sizeof(minha_pergunta), stdin);
        minha_pergunta[strcspn(minha_pergunta, "\n")] = 0;

        // Condição de saída
        if (strcmp(minha_pergunta, "0") == 0)
            break;

        // Comando para limpar histórico
        if (strcmp(minha_pergunta, "limpar") == 0) {
            limpar_tela();
            liberar_chat_history(chat_historico);
            chat_historico = inicializar_chat_historico();
            menu_com_clima(clima);
            printf("Histórico limpo! Nova conversa iniciada.\n\n");
            continue;
        }

        // Comando para exibir histórico
        if (strcmp(minha_pergunta, "historico") == 0) {
            exibir_historico(chat_historico);
            continue;
        }

        // Comando para mostrar ajuda
        if (strcmp(minha_pergunta, "help") == 0) {
            mostrar_ajuda();
            continue;
        }

        // Adiciona a pergunta do usuário ao histórico
        adicionar_turno(chat_historico, "user", minha_pergunta);

        char* payload = criar_payload_json_com_historico(minha_pergunta, chat_historico);
        if (payload == NULL) {
            fprintf(stderr, "Erro: Não foi possível criar o pacote JSON.\n");
            continue; // Volta para o início do loop
        }

        char url_completa[512];
        strcpy(url_completa, API_BASE_URL);
        strcat(url_completa, API_KEY);

        char* resposta_bruta = fazer_requisicao_http(url_completa, payload);
        if (resposta_bruta == NULL) {
            fprintf(stderr, "Erro: A comunicação com a API falhou.\n");
            free(payload);
            continue; // Volta para o início do loop
        }

        char* texto_final = extrair_texto_da_resposta(resposta_bruta);
        if (texto_final == NULL) {
            fprintf(stderr, "Erro: Não foi possível extrair o texto da resposta da API.\n");
            fprintf(stderr, "Resposta bruta recebida: %s\n", resposta_bruta);
            free(payload);
            free(resposta_bruta);
            continue; // Volta para o início do loop
        }

        printf("\r                         \r"); // Limpa a linha atual
        printf("\nGenieC: %s\n\n", texto_final);

        // Adiciona a resposta do Gemini ao histórico
        adicionar_turno(chat_historico, "model", texto_final);

        // Libera a memória alocada dentro do loop
        free(payload);
        free(resposta_bruta);
        free(texto_final);
    }

    // Libera o histórico antes de sair
    liberar_chat_history(chat_historico);

    printf("\nFinalizando o programa...\n");
    dormir(2000);
    return 0;
}

// ==============================================================================
// Funções
// ==============================================================================

// Função para mostrar a arte ASCII inicial
void mostrar_arte_inicial() {
    printf("\033[36m"); // Cyan para o título ASCII
    printf("╔═════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                             ║\n");
    printf("║                ██████╗ ███████╗███╗   ██╗██╗███████╗ ██████╗                ║\n");
    printf("║               ██╔════╝ ██╔════╝████╗  ██║██║██╔════╝██╔════╝                ║\n");
    printf("║               ██║  ███╗█████╗  ██╔██╗ ██║██║█████╗  ██║                     ║\n");
    printf("║               ██║   ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║                     ║\n");
    printf("║               ╚██████╔╝███████╗██║ ╚████║██║███████╗╚██████╗                ║\n");
    printf("║                ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝                ║\n");
    printf("║                                                                             ║\n");
    printf("╚═════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset cor

    printf("\n");
    printf("\033[1;32m"); // Verde bold para o título
    printf("🤖 Bem-vindo ao GenieC - Seu Assistente Inteligente Gemini! 🤖\n");
    printf("\033[0m"); // Reset cor
}

// Função para exibir o menu com informações do clima
void menu_com_clima(WeatherData weather) {
    printf("\033[36m"); // Cyan para o título ASCII
    printf("╔═════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                             ║\n");
    printf("║                ██████╗ ███████╗███╗   ██╗██╗███████╗ ██████╗                ║\n");
    printf("║               ██╔════╝ ██╔════╝████╗  ██║██║██╔════╝██╔════╝                ║\n");
    printf("║               ██║  ███╗█████╗  ██╔██╗ ██║██║█████╗  ██║                     ║\n");
    printf("║               ██║   ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║                     ║\n");
    printf("║               ╚██████╔╝███████╗██║ ╚████║██║███████╗╚██████╗                ║\n");
    printf("║                ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝                ║\n");
    printf("║                                                                             ║\n");
    printf("╚═════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset cor

    printf("\n");
    printf("\033[1;32m"); // Verde bold para o título
    printf("🤖 Bem-vindo ao GenieC - Seu Assistente Inteligente Gemini! 🤖\n");
    printf("\033[0m"); // Reset cor

    // Exibe informações do clima
    if(weather.valid) {
        printf("\n");
        printf("\033[1;34m"); // Azul bold para clima
        printf("🌤️  Clima atual em %s: %.1f°C - %s\n", weather.city, weather.temperature, weather.description);
        printf("\033[0m"); // Reset cor
    } else {
        printf("\n");
        printf("\033[1;31m"); // Vermelho para erro
        printf("❌ Não foi possível obter informações do clima\n");
        printf("\033[0m"); // Reset cor
    }

    printf("\n");
    printf("\033[33m"); // Amarelo para as opções
    printf("┌─────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│                              📋 MENU PRINCIPAL                              │\n");
    printf("├─────────────────────────────────────────────────────────────────────────────┤\n");
    printf("│                                                                             │\n");
    printf("│  \033[1;37m💬 Faça uma pergunta:\033[0m\033[33m                                                      │\n");
    printf("│     Digite sua pergunta diretamente e pressione Enter                       │\n");
    printf("│                                                                             │\n");
    printf("│  \033[1;37m🧹 Comandos especiais:\033[0m\033[33m                                                     │\n");
    printf("│     🔸 \033[1;36mlimpar\033[0m\033[33m     - Limpa o histórico da conversa                           │\n");
    printf("│     🔸 \033[1;36mhistorico\033[0m\033[33m  - Mostra o histórico completo                             │\n");
    printf("│     🔸 \033[1;36mhelp\033[0m\033[33m       - Mostra ajuda e dicas                                    │\n");
    printf("│     🔸 \033[1;31m0\033[0m\033[33m          - Sair do programa                                        │\n");
    printf("│                                                                             │\n");
    printf("└─────────────────────────────────────────────────────────────────────────────┘\n");
    printf("\033[0m"); // Reset cor

    printf("\n");
    printf("\033[32m"); // Verde para dicas
    printf("💡 \033[1mDicas:\033[0m\033[32m Seja específico em suas perguntas para obter melhores respostas!\n");
    printf("🌟 \033[1mExemplo:\033[0m\033[32m \"Qual é a previsão do tempo para São Paulo hoje?\"\n");
    printf("\033[0m"); // Reset cor

    printf("\n");
}

// Função para exibir ajuda e dicas
void mostrar_ajuda() {
    printf("\n");
    printf("\033[1;36m"); // Cyan bold
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                📚 AJUDA - GenieC                              ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset

    printf("\n\033[1;37m🎯 Como usar o GenieC:\033[0m\n");
    printf("   • Digite sua pergunta diretamente e pressione Enter\n");
    printf("   • O GenieC mantém o contexto da conversa automaticamente\n");
    printf("   • Use comandos especiais para funcionalidades extras\n\n");

    printf("\033[1;37m📝 Comandos Disponíveis:\033[0m\n");
    printf("   \033[36m• limpar\033[0m     - Limpa todo o histórico e inicia nova conversa\n");
    printf("   \033[36m• historico\033[0m  - Exibe todo o histórico da conversa atual\n");
    printf("   \033[36m• help\033[0m       - Mostra esta tela de ajuda\n");
    printf("   \033[31m• 0\033[0m          - Encerra o programa\n\n");

    printf("\033[1;37m💡 Dicas para melhores resultados:\033[0m\n");
    printf("   🔹 Seja específico: \"Receita de bolo de chocolate\" é melhor que \"receita\"\n");
    printf("   🔹 Inclua localização: \"Tempo em São Paulo\" para informações locais\n");
    printf("   🔹 Faça perguntas de follow-up: O GenieC lembra da conversa anterior\n");
    printf("   🔹 Use contexto: \"E sobre o Rio de Janeiro?\" após perguntar sobre SP\n\n");

    printf("\033[1;37m🌟 Exemplos de perguntas:\033[0m\n");
    printf("   \033[32m• \"Qual é a previsão do tempo para hoje em Brasília?\"\033[0m\n");
    printf("   \033[32m• \"Como fazer um currículo profissional?\"\033[0m\n");
    printf("   \033[32m• \"Receita simples de lasanha para 4 pessoas\"\033[0m\n");
    printf("   \033[32m• \"Explique o que é inteligência artificial\"\033[0m\n");
    printf("   \033[32m• \"Dicas de estudos para concursos públicos\"\033[0m\n\n");

    printf("\033[1;37m⚙️ Funcionalidades:\033[0m\n");
    printf("   ✅ Pesquisa em tempo real via Google\n");
    printf("   ✅ Contexto de conversa preservado\n");
    printf("   ✅ Respostas em português brasileiro\n");
    printf("   ✅ Interface colorida e intuitiva\n\n");

    printf("\033[1;33m💬 Agora você pode continuar fazendo suas perguntas!\033[0m\n");
    printf("────────────────────────────────────────────────────────────────────────────────\n");
}

// Cria o payload JSON usando a biblioteca cJSON.
char* criar_payload_json_com_historico(const char* prompt, ChatHistory* history) {
    // Passo 1: Criamos os objetos necessários para construir o JSON
    cJSON *root = cJSON_CreateObject();           // Objeto principal/raiz

    // Passo 2: Criamos o system_instruction
    cJSON *system_instruction = cJSON_CreateObject();
    cJSON *system_parts = cJSON_CreateArray();
    cJSON *system_part = cJSON_CreateObject();

    // Adicionamos o prompt do sistema
    cJSON_AddItemToObject(system_part, "text", cJSON_CreateString(SYSTEM_PROMPT));
    cJSON_AddItemToArray(system_parts, system_part);
    cJSON_AddItemToObject(system_instruction, "parts", system_parts);

    // Adicionamos o system_instruction ao objeto root
    cJSON_AddItemToObject(root, "system_instruction", system_instruction);

    // Passo 3: Criamos o array contents com todo o histórico
    cJSON *contents_array = cJSON_CreateArray();

    // Se existe histórico, adiciona todos os turnos exceto o último (que é a pergunta atual)
    if (history != NULL && history->count > 1) {
        for (int i = 0; i < history->count - 1; i++) {
            cJSON *content_item = cJSON_CreateObject();
            cJSON *parts_array = cJSON_CreateArray();
            cJSON *part_item = cJSON_CreateObject();

            // Adiciona o texto do turno
            cJSON_AddItemToObject(part_item, "text", cJSON_CreateString(history->turns[i].text));
            cJSON_AddItemToArray(parts_array, part_item);
            cJSON_AddItemToObject(content_item, "parts", parts_array);
            cJSON_AddItemToObject(content_item, "role", cJSON_CreateString(history->turns[i].role));

            cJSON_AddItemToArray(contents_array, content_item);
        }
    }

    // Adiciona a pergunta atual do usuário
    cJSON *user_content = cJSON_CreateObject();
    cJSON *user_parts = cJSON_CreateArray();
    cJSON *user_part = cJSON_CreateObject();

    cJSON_AddItemToObject(user_part, "text", cJSON_CreateString(prompt));
    cJSON_AddItemToArray(user_parts, user_part);
    cJSON_AddItemToObject(user_content, "parts", user_parts);
    cJSON_AddItemToObject(user_content, "role", cJSON_CreateString("user"));
    cJSON_AddItemToArray(contents_array, user_content);

    // Passo 4: Adicionamos o array contents_array ao objeto root
    cJSON_AddItemToObject(root, "contents", contents_array);

    // Passo 5: Adicionamos o Google Search tools
    // Criamos o array de tools
    cJSON *tools_array = cJSON_CreateArray();

    // Criamos o objeto tool
    cJSON *tool_item = cJSON_CreateObject();

    // Criamos o objeto google_search vazio
    cJSON *google_search = cJSON_CreateObject();

    // Adicionamos o google_search ao tool_item
    cJSON_AddItemToObject(tool_item, "google_search", google_search);

    // Adicionamos o tool_item ao array de tools
    cJSON_AddItemToArray(tools_array, tool_item);

    // Adicionamos o array de tools ao objeto root
    cJSON_AddItemToObject(root, "tools", tools_array);

    // Passo 6: Convertemos o objeto JSON para uma string
    char *json_string_copy = cJSON_PrintUnformatted(root);

    // Passo 7: Liberamos a memória do objeto JSON
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

    // Executa o Loading
    mostrar_loading();

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

void mostrar_loading() {
    int dots = 0;
    for (int i = 0; i < 6; i++) {  // 6 vezes de 0.5s = 3 segundos
        printf("\rConsultando IA%s", (dots % 4 == 0 ? "   " : dots % 4 == 1 ? "." : dots % 4 == 2 ? ".." : "..."));
        fflush(stdout);
        dots++;
        dormir(500);  // espera 0.5 segundos
    }
    printf("\rProcessando resposta...");
    fflush(stdout);
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

// Funções de Histórico do Chat
ChatHistory* inicializar_chat_historico() {
    // Aloca memória para o histórico do chat
    ChatHistory* history = (ChatHistory*)malloc(sizeof(ChatHistory));
    if (history == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o histórico do chat.\n");
        return NULL;
    }

    // Inicializa os campos do histórico
    history->turns = NULL;
    history->count = 0;
    history->capacity = 0;

    return history;
}

void adicionar_turno(ChatHistory* history, const char* role, const char* text) {
    // Verifica se o histórico precisa ser expandido
    if (history->count >= history->capacity) {
        // Aumenta a capacidade do histórico
        int nova_capacidade = (history->capacity == 0) ? 2 : history->capacity * 2;
        MessageTurn* novos_turnos = (MessageTurn*)realloc(history->turns, nova_capacidade * sizeof(MessageTurn));
        if (novos_turnos == NULL) {
            fprintf(stderr, "Erro ao alocar memória para os turnos do histórico.\n");
            return;
        }
        history->turns = novos_turnos;
        history->capacity = nova_capacidade;
    }

    // Adiciona o novo turno ao histórico
    MessageTurn* turno_atual = &history->turns[history->count++];
    turno_atual->role = strdup(role);
    turno_atual->text = strdup(text);
}

void liberar_chat_history(ChatHistory* history) {
    if (history != NULL) {
        // Libera a memória de cada turno
        for (int i = 0; i < history->count; i++) {
            free(history->turns[i].role);
            free(history->turns[i].text);
        }
        // Libera a memória do array de turnos
        free(history->turns);
        // Libera a memória do histórico em si
        free(history);
    }
}

void exibir_historico(ChatHistory* history) {
    if (history != NULL && history->count > 0) {
        printf("\n----- Histórico da Conversa -----\n");
        for (int i = 0; i < history->count; i++) {
            printf("%s: %s\n", history->turns[i].role, history->turns[i].text);
        }
        printf("---------------------------------\n");
    }
}

// Função para obter dados do clima da API OpenWeather
WeatherData obter_dados_clima(const char* cidade) {
    WeatherData weather = {0};
    weather.valid = 0;

    // Codifica a cidade para URL (resolve problema com espaços)
    char* cidade_encoded = url_encode(cidade);
    if (!cidade_encoded) {
        return weather;
    }

    // Monta a URL da API OpenWeather
    char url[512];
    snprintf(url, sizeof(url), "http://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=pt_br",
             cidade_encoded, API_KEY_WEATHER);

    // Faz a requisição HTTP
    CURL *curl;
    CURLcode res;
    struct MemoryStruct chunk;

    chunk.memory = malloc(1);
    chunk.size = 0;

    curl = curl_easy_init();
    if(curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        res = curl_easy_perform(curl);

        if(res == CURLE_OK) {
            // Parse do JSON da resposta
            cJSON *json = cJSON_Parse(chunk.memory);
            if(json) {
                cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");
                cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather");
                cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");

                if(main && weather_array && name) {
                    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp");
                    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);

                    if(temp && weather_item) {
                        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item, "description");

                        weather.temperature = (float)temp->valuedouble;
                        strncpy(weather.city, name->valuestring, MAX_CITY_NAME - 1);
                        if(description) {
                            strncpy(weather.description, description->valuestring, 99);
                        }
                        weather.valid = 1;
                    }
                }
                cJSON_Delete(json);
            }
        }
        curl_easy_cleanup(curl);
    }

    // Libera a memória da cidade codificada
    curl_free(cidade_encoded);
    free(chunk.memory);
    return weather;
}

// Função para codificar URL
char* url_encode(const char* str) {
    CURL *curl = curl_easy_init();
    char *encoded = curl_easy_escape(curl, str, 0);
    curl_easy_cleanup(curl);
    return encoded;
}
