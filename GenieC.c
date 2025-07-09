/* GenieC - Assistente Inteligente
 * Lorenzo Farias, Bernardo Soares Nunes e Pedro Cabral Buchaim
 * Projeto de Programação para Resolução de Problemas
 * Programação para Resolução de Problemas
 * Profa. Dra. Daniela Bagatini
 * Universidade de Santa Cruz do Sul (UNISC).
 */

#include <stdio.h>   // Biblioteca padrão de entrada e saída
#include <stdlib.h>  // Biblioteca padrão de alocação de memória e funções utilitárias
#include <string.h>  // Biblioteca para manipulação de strings
#include <locale.h>  // Biblioteca para configuração de localidade

#include <curl/curl.h>    // Biblioteca cURL para requisições HTTP
#include <cjson/cJSON.h>  // Biblioteca cJSON para manipulação de JSON

#include "api_key.h"      // Arquivo de cabeçalho com a chave da API
#include "limpar_tela.h"  // Função para limpar a tela
#include "dormir.h"       // Função para dormir (pausar a execução)

// --- Configurações Iniciais ---
#define MODELO_GEMINI "gemini-2.5-flash" // Nome do modelo Gemini
#define API_BASE_URL "https://generativelanguage.googleapis.com/v1beta/models/" MODELO_GEMINI ":generateContent?key=" // URL base da API do Gemini
#define MAX_PROMPT_SIZE 10000 // Tamanho máximo do prompt
#define MAX_HISTORY_SIZE 50   // Máximo de turnos no histórico
#define MAX_CITY_NAME 100     // Tamanho máximo do nome da cidade

// --- Estrutura para dados do clima ---
typedef struct {
    char cidade[MAX_CITY_NAME]; // Nome da cidade
    float temperatura;          // Temperatura em Celsius
    char description[100];      // Descrição do clima (ex: "ensolarado", "chuva")
    int valid;                  // Flag para indicar se os dados são válidos
} DataClima;

// --- Estrutura para armazenar o histórico da conversa ---
typedef struct {
    char* role;     // "user" ou "model"
    char* text;     // Conteúdo da mensagem
} TurnoMensagem;

// --- Estrutura para armazenar os turnos da conversa ---
typedef struct {
    TurnoMensagem* turno; // Array de turnos da conversa
    int contador;         // Número atual de turnos
    int capacidade;       // Capacidade do array de turnos
} HistoricoChat;

// --- Estrutura para armazenar a resposta da requisição HTTP (padrão cURL) ---
struct MemoryStruct {
    char *memory; // Ponteiro para armazenar os dados recebidos
    size_t size;  // Tamanho atual dos dados armazenados
};

// --- Prompt Base do Sistema ---
#define SYSTEM_PROMPT "Você é o GenieC, um assistente pessoal para responder dúvidas do dia a dia. Siga estas diretrizes:\n\n" \
"COMUNICAÇÃO:\n" \
"- Responda de forma clara, precisa e educada\n" \
"- Seja MUITO conciso - máximo 2-3 frases por resposta, pois você roda em um terminal/CLI\n" \
"- Use linguagem natural e acessível\n" \
"- Evite usar marcadores de formatação especial (sem *negrito*, _itálico_, etc.)\n" \
"- Se não souber algo, admita honestamente de forma breve\n\n" \
"PESQUISA E CONTEXTO:\n" \
"- Use ferramentas de pesquisa quando necessário para informações atualizadas\n" \
"- IMPORTANTE: Quando o usuário não mencionar uma cidade específica, use automaticamente a cidade '%s' como contexto para perguntas sobre clima, horários, eventos locais, etc.\n" \
"- Leve em conta o Brasil quando perguntarem sobre horários e coisas afins\n" \
"- Para perguntas ambíguas, peça esclarecimentos específicos de forma breve\n\n" \
"FORMATO DAS RESPOSTAS:\n" \
"- Interface CLI: suas respostas devem ser diretas e sem formatação especial\n" \
"- Evite listas longas, use apenas o essencial\n" \
"- Forneça informações práticas e úteis de forma resumida"


// --- Declaração das Funções ---
void mostrar_arte_inicial();                     // Função para mostrar a arte ASCII inicial
DataClima obter_dados_clima(const char* cidade); // Função para obter dados do clima da API OpenWeather
char* url_encode(const char* str);               // Função para codificar a URL (resolve problema com espaços)
void menu_com_clima(DataClima clima);            // Função para exibir o menu com informações do clima
void mostrar_ajuda();                            // Função para exibir ajuda e dicas
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade); // Função que cria o payload JSON com o histórico do chat
char* extrair_texto_da_resposta(const char* resposta_json); // Função que extrai o texto da resposta JSON
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp); // Callback para armazenar a resposta (padrão cURL)
char* fazer_requisicao_http(const char* url, const char* payload);                         // Função que faz a requisição HTTP usando cURL
void creditos(); // Função para exibir os créditos do projeto

// --- Funções de Histórico do Chat ---
HistoricoChat* inicializar_chat_historico(); // Função para inicializar o histórico do chat
void adicionar_turno(HistoricoChat* historico, const char* role, const char* text); // Função para adicionar um turno ao histórico do chat
void liberar_historico_chat(HistoricoChat* historico); // Função para liberar a memória do histórico do chat
void exibir_historico(HistoricoChat* historico);     // Função para exibir o histórico do chat
void mostrar_loading();

int main(){
    setlocale(LC_ALL, "Portuguese_Brazil.utf8"); // Configura a localidade para português brasileiro
    system("chcp 65001"); // Configura o console para UTF-8 (Windows)
    limpar_tela();        // Limpa a tela ao iniciar

    // Mostra a arte ASCII inicial
    mostrar_arte_inicial();

    // Solicita a cidade do usuário
    char cidade[MAX_CITY_NAME];           // Buffer para armazenar o nome da cidade
    printf("\n\033[1;36m🌍 Digite o nome da sua cidade para obter informações do clima:\033[0m "); // Exibe mensagem para o usuário
    fgets(cidade, sizeof(cidade), stdin); // Lê o nome da cidade
    cidade[strcspn(cidade, "\n")] = 0;    // Remove quebra de linha

    // Obtém dados do clima
    printf("\n\033[33m🌤️ Obtendo informações do clima...\033[0m\n"); // Exibe mensagem de carregamento
    DataClima clima = obter_dados_clima(cidade);                   // Chama a função para obter os dados do clima

    limpar_tela();         // Limpa a tela
    menu_com_clima(clima); // Exibe o menu com informações do clima

    // Inicializa o histórico do chat
    HistoricoChat* chat_historico = inicializar_chat_historico();        // Função para inicializar o histórico do chat
    if (chat_historico == NULL) {                                      // Se a inicialização falhar
        fprintf(stderr, "Erro ao inicializar o histórico do chat.\n"); // Exibe mensagem de erro
        return 1;                                                      // Encerra o programa com erro
    }

    while (1) { // Loop infinito até o usuário decidir sair
        char minha_pergunta[MAX_PROMPT_SIZE];                 // Buffer para armazenar a pergunta do usuário
        printf("Você: ");                                     // Exibe prompt para o usuário
        fgets(minha_pergunta, sizeof(minha_pergunta), stdin); // Lê a pergunta do usuário
        minha_pergunta[strcspn(minha_pergunta, "\n")] = 0;    // Remove a quebra de linha do final da string

        // Condição de saída
        if (strcmp(minha_pergunta, "0") == 0) // Se o usuário digitar "0"
            break;                            // Saí do loop

        // Comando para limpar histórico
        if (strcmp(minha_pergunta, "limpar") == 0) {                // Se o usuário digitar "limpar"
            limpar_tela();                                          // Limpa a tela
            liberar_historico_chat(chat_historico);                   // Chama a função que libera o histórico atual
            chat_historico = inicializar_chat_historico();          // Chama a função que reinicializa o histórico do chat
            menu_com_clima(clima);                                  // Exibe o menu novamente com as informações do clima
            printf("Histórico limpo! Nova conversa iniciada.\n\n"); // Exibe mensagem de confirmação
            continue;                                               // Volta para o início do loop para nova pergunta
        }

        // Comando para exibir histórico
        if (strcmp(minha_pergunta, "historico") == 0) { // Se o usuário digitar "historico"
            exibir_historico(chat_historico);           // Chama a função que exibe o histórico do chat
            continue;                                   // Volta para o início do loop para nova pergunta
        }

        // Comando para mostrar ajuda
        if (strcmp(minha_pergunta, "help") == 0) { // Se o usuário digitar "help"
            mostrar_ajuda();                       // Chama a função que exibe a ajuda
            continue;                              // Volta para o início do loop para nova pergunta
        }

        // Adiciona a pergunta do usuário ao histórico
        adicionar_turno(chat_historico, "user", minha_pergunta);

        char* payload = criar_payload_json_com_historico(minha_pergunta, chat_historico, cidade); // Cria o payload JSON com o histórico do chat e a cidade
        if (payload == NULL) {                                                // Se não conseguiu criar o payload
            fprintf(stderr, "Erro: Não foi possível criar o pacote JSON.\n"); // Exibe mensagem de erro
            continue;                                                         // Volta para o início do loop
        }

        char url_completa[512];             // Buffer para armazenar a URL completa
        strcpy(url_completa, API_BASE_URL); // Copia a URL base da API
        strcat(url_completa, API_KEY);      // Concatena a chave da API

        char* resposta_bruta = fazer_requisicao_http(url_completa, payload); // Faz a requisição HTTP para a API do Gemini
        if (resposta_bruta == NULL) {                                        // Se a requisição falhar
            fprintf(stderr, "Erro: A comunicação com a API falhou.\n");      // Exibe mensagem de erro
            free(payload);                                                   // Libera a memória do payload
            continue;                                                        // Volta para o início do loop
        }

        char* texto_final = extrair_texto_da_resposta(resposta_bruta);        // Extrai o texto da resposta JSON
        if (texto_final == NULL) {                                            // Se não conseguiu extrair o texto
            fprintf(stderr, "Erro: Não foi possível extrair o texto da resposta da API.\n"); // Exibe mensagem de erro
            fprintf(stderr, "Resposta bruta recebida: %s\n", resposta_bruta); // Exibe a resposta bruta para depuração
            free(payload);                                                    // Libera a memória do payload
            free(resposta_bruta);                                             // Libera a memória da resposta bruta
            continue;                                                         // Volta para o início do loop
        }

        printf("\r                         \r"); // Limpa a linha atual
        printf("\n\033[36mGenieC:\033[0m %s\n\n", texto_final); // Exibe a resposta do Gemini

        // Adiciona a resposta do Gemini ao histórico
        adicionar_turno(chat_historico, "model", texto_final);

        // Libera a memória alocada dentro do loop
        free(payload);
        free(resposta_bruta);
        free(texto_final);
    }

    // Libera o histórico antes de sair
    liberar_historico_chat(chat_historico);

    creditos(); // Exibe os créditos do projeto

    return 0;   // Retorna 0 para indicar que o programa terminou com sucesso
}

// ==============================================================================
//                                   FUNÇÕES
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
    printf("🤖 Bem-vindo ao GenieC - Seu Assistente Inteligente em C! 🤖\n");
    printf("\033[0m"); // Reset cor
}

// Função para exibir o menu com informações do clima
void menu_com_clima(DataClima clima) {
    mostrar_arte_inicial();
    // Exibe informações do clima
    if(clima.valid) {
        printf("\n");
        printf("\033[1;34m"); // Azul forte para clima
        printf("🌤️  Clima atual em %s: %.1f°C - %s\n", clima.cidade, clima.temperatura, clima.description); // Exibe a cidade, temperatura e descrição do clima
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
    printf("🌟 \033[1mExemplo:\033[0m\033[32m \"Qual a história de minha cidade?\"\n");
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
char* criar_payload_json_com_historico(const char* prompt, HistoricoChat* historico, const char* cidade) {
    // Passo 1: Criamos os objetos necessários para construir o JSON
    cJSON *root = cJSON_CreateObject();           // Objeto principal/raiz

    // Passo 2: Criamos o system_instruction com a cidade formatada
    cJSON *system_instruction = cJSON_CreateObject(); // Objeto para instruções do sistema
    cJSON *system_parts = cJSON_CreateArray();        // Array para partes do sistema
    cJSON *system_part = cJSON_CreateObject();        // Objeto para uma parte do sistema

    // Cria o prompt do sistema formatado com a cidade
    char system_prompt_formatado[4096];
    snprintf(system_prompt_formatado, sizeof(system_prompt_formatado), SYSTEM_PROMPT, cidade);

    // Adicionamos o prompt do sistema formatado
    cJSON_AddItemToObject(system_part, "text", cJSON_CreateString(system_prompt_formatado)); // Texto do prompt do sistema com a cidade
    cJSON_AddItemToArray(system_parts, system_part);                                         // Adiciona a parte ao array de partes do sistema
    cJSON_AddItemToObject(system_instruction, "parts", system_parts);                        // Adiciona o array de partes ao objeto de instruções do sistema

    // Adicionamos o system_instruction ao objeto root
    cJSON_AddItemToObject(root, "system_instruction", system_instruction);

    // Passo 3: Criamos o array contents com o histórico
    cJSON *contents_array = cJSON_CreateArray();

    // Se existe histórico, adiciona todos os turnos exceto o último (que é a pergunta atual)
    if (historico != NULL && historico->contador > 1) {        // Verifica se há histórico e se tem mais de um turno
        for (int i = 0; i < historico->contador - 1; i++) {  // Percorre todos os turnos, exceto o último
            cJSON *content_item = cJSON_CreateObject(); // Cria um objeto para o conteúdo do turno
            cJSON *parts_array = cJSON_CreateArray();   // Cria um array para as partes do turno
            cJSON *part_item = cJSON_CreateObject();    // Cria um objeto para uma parte do turno

            // Adiciona o texto do turno
            cJSON_AddItemToObject(part_item, "text", cJSON_CreateString(historico->turno[i].text));    // Adiciona o texto do turno ao objeto part_item
            cJSON_AddItemToArray(parts_array, part_item);                  // Adiciona a parte ao array de partes
            cJSON_AddItemToObject(content_item, "parts", parts_array);     // Adiciona o array de partes ao objeto content_item
            cJSON_AddItemToObject(content_item, "role", cJSON_CreateString(historico->turno[i].role)); // Adiciona o papel (role) do turno ao objeto content_item

            cJSON_AddItemToArray(contents_array, content_item); // Adiciona o objeto content_item ao array contents_array
        }
    }

    // Adiciona a pergunta atual do usuário
    cJSON *user_content = cJSON_CreateObject(); // Cria um objeto para o conteúdo do usuário
    cJSON *user_parts = cJSON_CreateArray();    // Cria um array para as partes do usuário
    cJSON *user_part = cJSON_CreateObject();    // Cria um objeto para uma parte do usuário

    cJSON_AddItemToObject(user_part, "text", cJSON_CreateString(prompt));    // Adiciona o texto da pergunta atual ao objeto user_part
    cJSON_AddItemToArray(user_parts, user_part);                             // Adiciona a parte ao array de partes do usuário
    cJSON_AddItemToObject(user_content, "parts", user_parts);                // Adiciona o array de partes ao objeto user_content
    cJSON_AddItemToObject(user_content, "role", cJSON_CreateString("user")); // Adiciona o papel (role) do usuário ao objeto user_content
    cJSON_AddItemToArray(contents_array, user_content);                      // Adiciona o objeto user_content ao array contents_array

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
    if (root == NULL) {                                             // Se o parsing falhar
        fprintf(stderr, "Erro ao parsear o JSON da resposta.\n");   // Exibe mensagem de erro
        return NULL;                                                // Retorna NULL indicando falha
    }

    // Passo 2: Declarar variáveis para navegar na estrutura do JSON
    cJSON *candidates, *first_candidate, *content, *parts, *first_part, *text_node;

    // Passo 3: Navegar na estrutura JSON para encontrar o texto
    // Formato esperado: {"candidates": [{"content": {"parts": [{"text": "resposta aqui"}]}}]}

    // Verificar se existe o array "candidates" e se é realmente um array
    candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");
    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        // Pegar o primeiro elemento do array
        first_candidate = cJSON_GetArrayItem(candidates, 0);

        // Pegar o objeto "content" dentro do primeiro candidato
        content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");

        // Verificar se encontrou o objeto "content"
        if (content) {
            // Pegar o array "parts" dentro do objeto "content"
            parts = cJSON_GetObjectItemCaseSensitive(content, "parts");

            // Verificar se encontrou o array "parts" e se é realmente um array
            if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                // Pegar o primeiro elemento do array
                first_part = cJSON_GetArrayItem(parts, 0);

                // Pegar o objeto "text" dentro do primeiro elemento
                text_node = cJSON_GetObjectItemCaseSensitive(first_part, "text");

                // Verificar se encontrou o objeto "text" e se ele é uma string
                if (text_node && cJSON_IsString(text_node)) {
                    // Pegar o valor da string usando a função helper
                    const char* texto = cJSON_GetStringValue(text_node);

                    // Criar uma cópia da string para retornar
                    if (texto) {
                        texto_extraido = strdup(texto);
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
    CURL *curl_handle;                 // Manipulador do cURL
    CURLcode res;                      // Código de resultado da operação
    struct MemoryStruct chunk;         // Estrutura para armazenar a resposta

    // Inicializa a estrutura de memória
    chunk.memory = malloc(1);          // Aloca um byte inicial
    chunk.size = 0;                    // Inicializa o tamanho como zero

    // Inicializa a biblioteca cURL
    curl_global_init(CURL_GLOBAL_ALL); // Inicializa a biblioteca cURL com todas as opções globais
    curl_handle = curl_easy_init();    // Cria um manipulador cURL

    // Verifica se a inicialização deu certo
    if (!curl_handle) {                              // Se não conseguiu inicializar o cURL
        fprintf(stderr, "Erro ao iniciar o cURL\n"); // Exibe mensagem de erro
        free(chunk.memory);                          // Libera a memória alocada
        return NULL;                                 // Retorna NULL indicando falha
    }

    // Configura os cabeçalhos da requisição
    struct curl_slist *headers = NULL; // Lista de cabeçalhos para a requisição
    headers = curl_slist_append(headers, "Content-Type: application/json"); // Define o tipo de conteúdo como JSON

    // Configura as opções da requisição
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);                           // Define a URL
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);                // Define os cabeçalhos
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload);                // Define os dados a serem enviados
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Define a função de callback
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);          // Define onde salvar os dados

    // Executa o Loading (simula uma animação de carregamento, pois não conseguimos realizar mais de uma tarefa ao mesmo tempo)
    mostrar_loading();

    // Executa a requisição
    res = curl_easy_perform(curl_handle);

    // Verifica se houve erro
    if (res != CURLE_OK) {                                                          // Se a requisição falhou
        fprintf(stderr, "A requisição cURL falhou: %s\n", curl_easy_strerror(res)); // Exibe mensagem de erro
        free(chunk.memory);                                                         // Libera a memória alocada
        curl_slist_free_all(headers);                                               // Libera os cabeçalhos
        curl_easy_cleanup(curl_handle);                                             // Libera o manipulador cURL
        curl_global_cleanup();                                                      // Libera os recursos globais do cURL
        return NULL;                                                                // Retorna NULL indicando falha
    }

    // Libera os recursos utilizados
    curl_slist_free_all(headers);   // Libera a lista de cabeçalhos
    curl_easy_cleanup(curl_handle); // Libera o manipulador cURL
    curl_global_cleanup();          // Libera os recursos globais do cURL

    // Retorna a resposta obtida
    return chunk.memory;
}
// Função para codificar a URL (resolve problema com espaços)
void mostrar_loading() {
    int dots = 0;                  // Contador de pontos para animação
    for (int i = 0; i < 4; i++) {  // 6 vezes de 0.5s = 3 segundos
        printf("\rConsultando IA%s", (dots % 4 == 0 ? "   " : dots % 4 == 1 ? "." : dots % 4 == 2 ? ".." : "..."));
        fflush(stdout);            // Atualiza a saída padrão imediatamente
        dots++;                    // Incrementa o contador de pontos
        dormir(500);               // espera 0.5 segundos
    }
    printf("\rProcessando resposta...");
    fflush(stdout); // Atualiza a saída padrão imediatamente
}

// Função de callback para armazenar a resposta da requisição HTTP
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
        return 0; // Retorna 0 para indicar falha na alocação
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
HistoricoChat* inicializar_chat_historico() {
    // Aloca memória para o histórico do chat
    HistoricoChat* history = (HistoricoChat*)malloc(sizeof(HistoricoChat));
    if (history == NULL) {                                                     // Se a alocação falhar
        fprintf(stderr, "Erro ao alocar memória para o histórico do chat.\n"); // Exibe mensagem de erro
        return NULL;                                                           // Retorna NULL indicando falha
    }

    // Inicializa os campos do histórico
    history->turno = NULL;
    history->contador = 0;
    history->capacidade = 0;

    // Retorna o ponteiro para o histórico inicializado
    return history;
}

// Função para adicionar um novo turno ao histórico do chat
void adicionar_turno(HistoricoChat* historico, const char* role, const char* text) {
    // Verifica se o histórico precisa ser expandido
    if (historico->contador >= historico->capacidade) {
        // Aumenta a capacidade do histórico
        int nova_capacidade;

        if (historico->capacidade == 0) // Se a capacidade atual é zero
            nova_capacidade = 2;        // Define nova capacidade inicial como 2
        else                            // Se já tem capacidade
            nova_capacidade = historico->capacidade * 2; // Dobra a capacidade atual

        TurnoMensagem* novos_turnos = (TurnoMensagem*)realloc(historico->turno, nova_capacidade * sizeof(TurnoMensagem)); // Aloca memória para os novos turnos com a nova capacidade
        if (novos_turnos == NULL) { // Se a alocação falhar
            fprintf(stderr, "Erro ao alocar memória para os turnos do histórico.\n"); // Exibe mensagem de erro
            return;                 // Retorna sem adicionar o turno
        }
        historico->turno = novos_turnos;         // Atualiza o ponteiro para os turnos com a nova memória alocada
        historico->capacidade = nova_capacidade; // Atualiza a capacidade do histórico
    }

    // Adiciona o novo turno ao histórico
    TurnoMensagem* turno_atual = &historico->turno[historico->contador++]; // Incrementa o contador de turnos e obtém o ponteiro para o turno atual
    turno_atual->role = strdup(role);                                      // Duplica a string do papel (role) do turno
    turno_atual->text = strdup(text);                                      // Duplica a string do texto do turno
}

// Função para limpar o histórico do chat
void liberar_historico_chat(HistoricoChat* historico) {
    if (historico != NULL) { // Verifica se o histórico não é nulo
        for (int i = 0; i < historico->contador; i++) { // Percorre todos os turnos
            free(historico->turno[i].role);             // Libera a memória do papel (role) do turno
            free(historico->turno[i].text);             // Libera a memória do texto do turno
        }
        // Libera a memória do array de turnos
        free(historico->turno);
        // Libera a memória do histórico em si
        free(historico);
    }
}

// Função para exibir o histórico da conversa
void exibir_historico(HistoricoChat* historico) {
    if (historico != NULL && historico->contador > 0) {                             // Verifica se o histórico não é nulo e tem turnos
        printf("\n----- Histórico da Conversa -----\n");
        for (int i = 0; i < historico->contador; i++) {                             // Percorre todos os turnos do histórico
            printf("%s: %s\n", historico->turno[i].role, historico->turno[i].text); // Exibe o papel (role) e o texto do turno
        }
        printf("---------------------------------\n");
    } else                                        // Se o histórico é nulo ou não tem turnos
        printf("\nNenhum histórico disponível.\n"); // Exibe mensagem informando que não há histórico
}

// Função para obter dados do clima da API OpenWeather
DataClima obter_dados_clima(const char* cidade) {
    DataClima clima = {0};   // Inicializa a estrutura de dados do clima
    clima.valid = 0;         // Marca como inválido inicialmente

    // Codifica a cidade para URL (resolve problema com espaços)
    char* cidade_encoded = url_encode(cidade); // Função que codifica a cidade para uso em URL
    if (!cidade_encoded) {                     // Se a codificação falhar
        return clima;                          // Retorna a estrutura de clima inválida
    }

    // Monta a URL da API OpenWeather
    char url[512];                             // Buffer para armazenar a URL completa
    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=pt_br",
             cidade_encoded, API_KEY_WEATHER); // Concatena a cidade codificada e a chave da API na URL

    // Faz a requisição HTTP
    CURL *curl;                // Manipulador do cURL
    CURLcode res;              // Código de resultado da operação
    struct MemoryStruct chunk; // Estrutura para armazenar a resposta

    chunk.memory = malloc(1);  // Aloca um byte inicial para a memória
    chunk.size = 0;            // Inicializa o tamanho como zero

    curl = curl_easy_init();   // Inicializa o cURL
    if(curl) {                                                              // Verifica se a inicialização foi bem-sucedida
        curl_easy_setopt(curl, CURLOPT_URL, url);                           // Define a URL da requisição
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Define a função de callback para escrever os dados recebidos
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);          // Define onde os dados recebidos serão armazenados

        res = curl_easy_perform(curl); // Executa a requisição HTTP

        if (res == CURLE_OK) { // Se a requisição foi bem-sucedida
            // Parse do JSON da resposta
            cJSON *json = cJSON_Parse(chunk.memory); // Converte a resposta JSON em um objeto cJSON
            if (json) { // Se o parsing foi bem-sucedido
                cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");             // Obtém o objeto "main" do JSON
                cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather"); // Obtém o array "weather" do JSON
                cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");             // Obtém o nome da cidade do JSON

                // Verifica se os objetos necessários foram encontrados e seus tipos
                if (main && weather_array && cJSON_IsArray(weather_array) &&
                    name && cJSON_IsString(name)) {
                    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp"); // Obtém a temperatura do objeto "main"
                    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);   // Obtém o primeiro item do array "weather"

                    // Verifica se a temperatura é um número e se o item do clima existe
                    if (temp && cJSON_IsNumber(temp) && weather_item) {
                        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item, "description"); // Obtém a descrição do clima

                        clima.temperatura = (float)cJSON_GetNumberValue(temp);                  // Converte a temperatura para float
                        strncpy(clima.cidade, cJSON_GetStringValue(name), MAX_CITY_NAME - 1);   // Copia o nome da cidade
                        clima.cidade[MAX_CITY_NAME - 1] = '\0';                                 // Garante terminação nula

                        if (description && cJSON_IsString(description)) { // Se a descrição foi encontrada e é string
                            strncpy(clima.description, cJSON_GetStringValue(description), 99); // Copia a descrição
                            clima.description[99] = '\0';                                      // Garante terminação nula
                        }
                        clima.valid = 1; // Marca os dados do clima como válidos
                    }
                }
                cJSON_Delete(json); // Libera a memória do objeto cJSON
            }
        }
        curl_easy_cleanup(curl); // Libera o manipulador cURL
    }

    // Libera a memória da cidade codificada
    curl_free(cidade_encoded);
    free(chunk.memory);

    return clima; // Retorna os dados do clima (pode ser inválido se não conseguiu obter os dados)
}

// Função para codificar URL
char* url_encode(const char* str) {
    CURL *curl = curl_easy_init();                  // Inicializa o cURL
    char *encoded = curl_easy_escape(curl, str, 0); // Codifica a string para URL
    curl_easy_cleanup(curl);                        // Libera o manipulador cURL
    return encoded;                                 // Retorna a string codificada (ou NULL se falhar)
}

// Função para exibir os créditos do programa
void creditos() {
    limpar_tela(); // Limpa a tela antes de mostrar os créditos

    printf("\033[36m"); // Cyan para as bordas
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                               ║\n");
    printf("║                            🏆 GenieC - Créditos 🏆                            ║\n");
    printf("║                                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset cor

    printf("\n");
    printf("\033[1;32m💻 Desenvolvido por:\033[0m\n");
    printf("   \033[37m• Lorenzo Farias\033[0m\n");
    printf("   \033[37m• Bernardo Soares Nunes\033[0m\n");
    printf("   \033[37m• Pedro Cabral Buchaim\033[0m\n\n");

    printf("\033[1;34m🎓 Instituição:\033[0m\n");
    printf("   \033[37mUniversidade de Santa Cruz do Sul (UNISC)\033[0m\n\n");

    printf("\033[1;33m📚 Disciplina:\033[0m\n");
    printf("   \033[37mProgramação para Resolução de Problemas\033[0m\n\n");

    printf("\033[1;35m👩‍🏫 Professora:\033[0m\n");
    printf("   \033[37mProfa. Dra. Daniela Bagatini\033[0m\n\n");

    printf("\033[36m"); // Cyan para a borda inferior
    printf("────────────────────────────────────────────────────────────────────────────────\n");
    printf("\033[0m"); // Reset cor

    printf("\n\033[1;32m🤖 Obrigado por usar o GenieC! 🤖\033[0m\n");
    printf("\033[33mPressione Enter para sair...\033[0m");

    getchar(); // Pausa para o usuário ler a mensagem
}