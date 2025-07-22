#include <stdio.h>   // Standard input/output library
#include <stdlib.h>  // Standard memory allocation and utility functions library
#include <string.h>  // String manipulation library
#include <locale.h>  // Locale configuration library

#include <curl/curl.h>    // cURL library for HTTP requests
#include <cjson/cJSON.h>  // cJSON library for JSON manipulation

#include "api_key.h"      // Header file with API key
#include "limpar_tela.h"  // Function to clear screen
#include "dormir.h"       // Function to sleep (pause execution)

// --- Initial Settings ---
#define MODELO_GEMINI "gemini-2.5-flash" // Gemini model name
#define API_BASE_URL "https://generativelanguage.googleapis.com/v1beta/models/" MODELO_GEMINI ":generateContent?key=" // Gemini API base URL
#define MAX_PROMPT_SIZE 10000 // Maximum prompt size
#define MAX_HISTORY_SIZE 50   // Maximum turns in history
#define MAX_CITY_NAME 100     // Maximum city name size

// --- Weather data structure ---
typedef struct {
    char cidade[MAX_CITY_NAME]; // City name
    float temperatura;          // Temperature in Celsius
    char description[100];      // Weather description (e.g., "sunny", "rain")
    int valid;                  // Flag to indicate if data is valid
} DataClima;

// --- Structure to store conversation history ---
typedef struct {
    char* role;     // "user" or "model"
    char* text;     // Message content
} TurnoMensagem;

// --- Structure to store conversation turns ---
typedef struct {
    TurnoMensagem* turno; // Array of conversation turns
    int contador;         // Current number of turns
    int capacidade;       // Array capacity
} HistoricoChat;

// --- Structure to store HTTP request response (cURL standard) ---
struct MemoryStruct {
    char *memory; // Pointer to store received data
    size_t size;  // Current size of stored data
};

// --- System Base Prompt ---
#define SYSTEM_PROMPT "You are GenieC, a personal assistant to answer everyday questions. Follow these guidelines:\n\n" \
"COMMUNICATION:\n" \
"- Respond clearly, precisely and politely\n" \
"- Be VERY concise - maximum 2-3 sentences per response, as you run in a terminal/CLI\n" \
"- Use natural and accessible language\n" \
"- Avoid using special formatting markers (no *bold*, _italic_, etc.)\n" \
"- If you don't know something, admit it honestly and briefly\n\n" \
"SEARCH AND CONTEXT:\n" \
"- Use search tools when necessary for updated information\n" \
"- IMPORTANT: When the user doesn't mention a specific city, automatically use the city '%s' as context for weather, schedules, local events, etc.\n" \
"- Consider the location context when asked about schedules and related matters\n" \
"- For ambiguous questions, ask for specific clarifications briefly\n\n" \
"RESPONSE FORMAT:\n" \
"- CLI Interface: your responses should be direct and without special formatting\n" \
"- Avoid long lists, use only the essential\n" \
"- Provide practical and useful information in a summarized way"

// --- Function Declarations ---
void show_initial_art();                     // Function to show initial ASCII art
void menu_with_weather(DataClima clima);     // Function to display menu with weather information
void show_help();                            // Function to display help and tips
char* create_json_payload_with_history(const char* prompt, HistoricoChat* historico, const char* cidade); // Function that creates JSON payload with chat history
char* extract_text_from_response(const char* resposta_json); // Function that extracts text from JSON response
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp); // Callback to store response (cURL standard)
char* make_http_request(const char* url, const char* payload);                             // Function that makes HTTP request using cURL
void credits(); // Function to display project credits

// --- Weather functions ---
DataClima get_weather_data(const char* cidade);  // Function to get weather data from OpenWeather API
char* url_encode(const char* str);               // Function to encode URL (solves space problem)

// --- Chat History Functions ---
HistoricoChat* initialize_chat_history(); // Function to initialize chat history
void add_turn(HistoricoChat* historico, const char* role, const char* text); // Function to add a turn to chat history
void free_chat_history(HistoricoChat* historico); // Function to free chat history memory
void display_history(HistoricoChat* historico);   // Function to display chat history
void show_loading();

int main(){
    setlocale(LC_ALL, "Portuguese_Brazil.utf8"); // Set locale to Portuguese Brazilian
    system("chcp 65001"); // Configure console to UTF-8 (Windows)
    limpar_tela();        // Clear screen on startup

    // Show initial ASCII art
    show_initial_art();

    // Request user's city
    char cidade[MAX_CITY_NAME];           // Buffer to store city name
    printf("\n\033[1;36m🌍 Enter your city name to get weather information:\033[0m "); // Display message to user
    fgets(cidade, sizeof(cidade), stdin); // Read city name
    cidade[strcspn(cidade, "\n")] = 0;    // Remove line break

    // Get weather data
    printf("\n\033[33m🌤️ Getting weather information...\033[0m\n"); // Display loading message
    DataClima clima = get_weather_data(cidade);                     // Call function to get weather data

    limpar_tela();         // Clear screen

    menu_with_weather(clima); // Display menu with weather information

    // Initialize chat history
    HistoricoChat* chat_historico = initialize_chat_history(); // Function to initialize chat history
    if (chat_historico == NULL) {                              // If initialization fails
        fprintf(stderr, "Error initializing chat history.\n"); // Display error message
        return 1;                                              // End program with error
    }

    while (1) { // Infinite loop until user decides to exit
        char user_question[MAX_PROMPT_SIZE];                // Buffer to store user's question
        printf("You: ");                                    // Display prompt for user
        fgets(user_question, sizeof(user_question), stdin); // Read user's question
        user_question[strcspn(user_question, "\n")] = 0;    // Remove line break from end of string

        // Exit condition
        if (strcmp(user_question, "0") == 0)  // If user types "0"
            break;                            // Exit loop

        // Command to clear history
        if (strcmp(user_question, "clear") == 0) {                    // If user types "clear"
            limpar_tela();                                            // Clear screen
            free_chat_history(chat_historico);                        // Call function that frees current history
            chat_historico = initialize_chat_history();               // Call function that reinitializes chat history
            menu_with_weather(clima);                                 // Display menu again with weather information
            printf("History cleared! New conversation started.\n\n"); // Display confirmation message
            continue;                                                 // Return to beginning of loop for new question
        }

        // Command to display history
        if (strcmp(user_question, "history") == 0) {    // If user types "history"
            display_history(chat_historico);            // Call function that displays chat history
            continue;                                   // Return to beginning of loop for new question
        }

        // Command to show help
        if (strcmp(user_question, "help") == 0) {  // If user types "help"
            show_help();                           // Call function that displays help
            continue;                              // Return to beginning of loop for new question
        }

        // Add user's question to history
        add_turn(chat_historico, "user", user_question);

        char* payload = create_json_payload_with_history(user_question, chat_historico, cidade); // Create JSON payload with chat history and city
        if (payload == NULL) {                                                // If couldn't create payload
            fprintf(stderr, "Error: Could not create JSON package.\n");       // Display error message
            continue;                                                         // Return to beginning of loop
        }

        char complete_url[512];             // Buffer to store complete URL
        strcpy(complete_url, API_BASE_URL); // Copy API base URL
        strcat(complete_url, API_KEY);      // Concatenate API key

        char* raw_response = make_http_request(complete_url, payload); // Make HTTP request to Gemini API
        if (raw_response == NULL) {                                          // If request fails
            fprintf(stderr, "Error: Communication with API failed.\n");      // Display error message
            free(payload);                                                   // Free payload memory
            continue;                                                        // Return to beginning of loop
        }

        char* final_text = extract_text_from_response(raw_response);         // Extract text from JSON response
        if (final_text == NULL) {                                            // If couldn't extract text
            fprintf(stderr, "Error: Could not extract text from API response.\n"); // Display error message
            fprintf(stderr, "Raw response received: %s\n", raw_response);     // Display raw response for debugging
            free(payload);                                                    // Free payload memory
            free(raw_response);                                               // Free raw response memory
            continue;                                                         // Return to beginning of loop
        }

        printf("\r                         \r"); // Clear current line
        printf("\n\033[36mGenieC:\033[0m %s\n\n", final_text); // Display Gemini's response

        // Add Gemini's response to history
        add_turn(chat_historico, "model", final_text);

        // Free memory allocated within loop
        free(payload);
        free(raw_response);
        free(final_text);
    }

    // Free history before exiting
    free_chat_history(chat_historico);

    credits(); // Display project credits

    return 0;  // Return 0 to indicate program ended successfully
}

// ==============================================================================
//                                   FUNCTIONS
// ==============================================================================

// Function to show initial ASCII art
void show_initial_art() {
    printf("\033[36m"); // Cyan for the ASCII title
    printf("╔═════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                             ║\n");
    printf("║                 ██████╗ ███████╗███╗   ██╗██╗███████╗ ██████╗               ║\n");
    printf("║                ██╔════╝ ██╔════╝████╗  ██║██║██╔════╝██╔════╝               ║\n");
    printf("║                ██║  ███╗█████╗  ██╔██╗ ██║██║█████╗  ██║                    ║\n");
    printf("║                ██║   ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║                    ║\n");
    printf("║                ╚██████╔╝███████╗██║ ╚████║██║███████╗╚██████╗               ║\n");
    printf("║                 ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝               ║\n");
    printf("║                                                                             ║\n");
    printf("╚═════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset color

    printf("\n");
    printf("\033[1;32m"); // Bold green for the title
    printf("🤖 Welcome to GenieC - Your Intelligent Assistant in C! 🤖\n");
    printf("\033[0m"); // Reset color
}

// Function to display menu with weather information
void menu_with_weather(DataClima clima) {
    show_initial_art();
    // Display weather information
    if(clima.valid) {
        printf("\n");
        printf("\033[1;34m"); // Strong blue for weather
        printf("🌤️  Current weather in %s: %.1f°C - %s\n", clima.cidade, clima.temperatura, clima.description); // Display city, temperature and weather description
        printf("\033[0m"); // Reset color
    } else {
        printf("\n");
        printf("\033[1;31m"); // Red for error
        printf("❌ Could not get weather information\n");
        printf("\033[0m"); // Reset color
    }

    printf("\n");
    printf("\033[33m"); // Yellow for options
    printf("┌─────────────────────────────────────────────────────────────────────────────┐\n");
    printf("│                                📋 MAIN MENU                                 │\n");
    printf("├─────────────────────────────────────────────────────────────────────────────┤\n");
    printf("│                                                                             │\n");
    printf("│  \033[1;37m💬 Ask a question:\033[0m\033[33m                                                         │\n");
    printf("│     Type your question directly and press Enter                             │\n");
    printf("│                                                                             │\n");
    printf("│  \033[1;37m🧹 Special commands:\033[0m\033[33m                                                       │\n");
    printf("│     🔸 \033[1;36mclear\033[0m\033[33m      - Clear conversation history                              │\n");
    printf("│     🔸 \033[1;36mhistory\033[0m\033[33m    - Show complete history                                   │\n");
    printf("│     🔸 \033[1;36mhelp\033[0m\033[33m       - Show help and tips                                      │\n");
    printf("│     🔸 \033[1;31m0\033[0m\033[33m          - Exit program                                            │\n");
    printf("│                                                                             │\n");
    printf("└─────────────────────────────────────────────────────────────────────────────┘\n");
    printf("\033[0m"); // Reset color

    printf("\n");
    printf("\033[32m"); // Green for tips
    printf("💡 \033[1mTips:\033[0m\033[32m Be specific in your questions to get better answers!\n");
    printf("🌟 \033[1mExample:\033[0m\033[32m \"What is the history of my city?\"\n");
    printf("\033[0m"); // Reset color
    printf("\n");
}

// Function to display help and tips
void show_help() {
    printf("\n");
    printf("\033[1;36m"); // Cyan bold
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                📚 HELP - GenieC                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset

    printf("\n\033[1;37m🎯 How to use GenieC:\033[0m\n");
    printf("   • Type your question directly and press Enter\n");
    printf("   • GenieC automatically maintains conversation context\n");
    printf("   • Use special commands for extra features\n\n");

    printf("\033[1;37m📝 Available Commands:\033[0m\n");
    printf("   \033[36m• clear\033[0m      - Clear all history and start new conversation\n");
    printf("   \033[36m• history\033[0m    - Show complete conversation history\n");
    printf("   \033[36m• help\033[0m       - Show this help screen\n");
    printf("   \033[31m• 0\033[0m          - Exit program\n\n");

    printf("\033[1;37m💡 Tips for better results:\033[0m\n");
    printf("   🔹 Be specific: \"Chocolate cake recipe\" is better than \"recipe\"\n");
    printf("   🔹 Include location: \"Weather in New York\" for local information\n");
    printf("   🔹 Ask follow-up questions: GenieC remembers previous conversation\n");
    printf("   🔹 Use context: \"What about London?\" after asking about Paris\n\n");

    printf("\033[1;37m🌟 Question examples:\033[0m\n");
    printf("   \033[32m• \"What's the weather forecast for today in Boston?\"\033[0m\n");
    printf("   \033[32m• \"How to write a professional resume?\"\033[0m\n");
    printf("   \033[32m• \"Simple lasagna recipe for 4 people\"\033[0m\n");
    printf("   \033[32m• \"Explain what artificial intelligence is\"\033[0m\n");
    printf("   \033[32m• \"Study tips for college exams\"\033[0m\n\n");

    printf("\033[1;37m⚙️ Features:\033[0m\n");
    printf("   ✅ Real-time search via Google\n");
    printf("   ✅ Preserved conversation context\n");
    printf("   ✅ Intelligent responses\n");
    printf("   ✅ Colorful and intuitive interface\n\n");

    printf("\033[1;33m💬 Now you can continue asking your questions!\033[0m\n");
    printf("────────────────────────────────────────────────────────────────────────────────\n");
}

// Create JSON payload using cJSON library.
char* create_json_payload_with_history(const char* prompt, HistoricoChat* historico, const char* cidade) {
    // Step 1: Create necessary objects to build JSON
    cJSON *root = cJSON_CreateObject();           // Main/root object

    // Step 2: Create system_instruction with formatted city
    cJSON *system_instruction = cJSON_CreateObject(); // Object for system instructions
    cJSON *system_parts = cJSON_CreateArray();        // Array for system parts
    cJSON *system_part = cJSON_CreateObject();        // Object for a system part

    // Create formatted system prompt with city
    char formatted_system_prompt[4096];
    snprintf(formatted_system_prompt, sizeof(formatted_system_prompt), SYSTEM_PROMPT, cidade);

    // Add formatted system prompt
    cJSON_AddItemToObject(system_part, "text", cJSON_CreateString(formatted_system_prompt)); // System prompt text with city
    cJSON_AddItemToArray(system_parts, system_part);                                         // Add part to system parts array
    cJSON_AddItemToObject(system_instruction, "parts", system_parts);                        // Add parts array to system instructions object

    // Add system_instruction to root object
    cJSON_AddItemToObject(root, "system_instruction", system_instruction);

    // Step 3: Create contents array with history
    cJSON *contents_array = cJSON_CreateArray();

    // If history exists, add all turns except the last one (which is current question)
    if (historico != NULL && historico->contador > 1) {      // Check if there's history and more than one turn
        for (int i = 0; i < historico->contador - 1; i++) {  // Go through all turns except the last
            cJSON *content_item = cJSON_CreateObject(); // Create object for turn content
            cJSON *parts_array = cJSON_CreateArray();   // Create array for turn parts
            cJSON *part_item = cJSON_CreateObject();    // Create object for a turn part

            // Add turn text
            cJSON_AddItemToObject(part_item, "text", cJSON_CreateString(historico->turno[i].text));    // Add turn text to part_item object
            cJSON_AddItemToArray(parts_array, part_item);                  // Add part to parts array
            cJSON_AddItemToObject(content_item, "parts", parts_array);     // Add parts array to content_item object
            cJSON_AddItemToObject(content_item, "role", cJSON_CreateString(historico->turno[i].role)); // Add turn role to content_item object

            cJSON_AddItemToArray(contents_array, content_item); // Add content_item object to contents_array
        }
    }

    // Add current user question
    cJSON *user_content = cJSON_CreateObject(); // Create object for user content
    cJSON *user_parts = cJSON_CreateArray();    // Create array for user parts
    cJSON *user_part = cJSON_CreateObject();    // Create object for a user part

    cJSON_AddItemToObject(user_part, "text", cJSON_CreateString(prompt));    // Add current question text to user_part object
    cJSON_AddItemToArray(user_parts, user_part);                             // Add part to user parts array
    cJSON_AddItemToObject(user_content, "parts", user_parts);                // Add parts array to user_content object
    cJSON_AddItemToObject(user_content, "role", cJSON_CreateString("user")); // Add user role to user_content object
    cJSON_AddItemToArray(contents_array, user_content);                      // Add user_content object to contents_array

    // Step 4: Add contents_array to root object
    cJSON_AddItemToObject(root, "contents", contents_array);

    // Step 5: Add Google Search tools
    // Create tools array
    cJSON *tools_array = cJSON_CreateArray();

    // Create tool object
    cJSON *tool_item = cJSON_CreateObject();

    // Create empty google_search object
    cJSON *google_search = cJSON_CreateObject();

    // Add google_search to tool_item
    cJSON_AddItemToObject(tool_item, "google_search", google_search);

    // Add tool_item to tools array
    cJSON_AddItemToArray(tools_array, tool_item);

    // Add tools array to root object
    cJSON_AddItemToObject(root, "tools", tools_array);

    // Step 6: Convert JSON object to string
    char *json_string_copy = cJSON_PrintUnformatted(root);

    // Step 7: Free JSON object memory
    cJSON_Delete(root);

    // Return created JSON string
    return json_string_copy;
}

// Extract text from JSON response using cJSON.
char* extract_text_from_response(const char* resposta_json) {
    // Variable to store final extracted text
    char* extracted_text = NULL;

    // Step 1: Convert JSON string to cJSON object
    cJSON *root = cJSON_Parse(resposta_json);

    // Check if parsing was successful
    if (root == NULL) {                                    // If parsing fails
        fprintf(stderr, "Error parsing JSON response.\n"); // Display error message
        return NULL;                                       // Return NULL indicating failure
    }

    // Step 2: Declare variables to navigate JSON structure
    cJSON *candidates, *first_candidate, *content, *parts, *first_part, *text_node;

    // Step 3: Navigate JSON structure to find text
    // Expected format: {"candidates": [{"content": {"parts": [{"text": "response here"}]}}]}

    // Check if "candidates" array exists and is really an array
    candidates = cJSON_GetObjectItemCaseSensitive(root, "candidates");
    if (cJSON_IsArray(candidates) && cJSON_GetArraySize(candidates) > 0) {
        // Get first element of array
        first_candidate = cJSON_GetArrayItem(candidates, 0);

        // Get "content" object inside first candidate
        content = cJSON_GetObjectItemCaseSensitive(first_candidate, "content");

        // Check if found "content" object
        if (content) {
            // Get "parts" array inside "content" object
            parts = cJSON_GetObjectItemCaseSensitive(content, "parts");

            // Check if found "parts" array and it's really an array
            if (cJSON_IsArray(parts) && cJSON_GetArraySize(parts) > 0) {
                // Get first element of array
                first_part = cJSON_GetArrayItem(parts, 0);

                // Get "text" object inside first element
                text_node = cJSON_GetObjectItemCaseSensitive(first_part, "text");

                // Check if found "text" object and it's a string
                if (text_node && cJSON_IsString(text_node)) {
                    // Get string value using helper function
                    const char* texto = cJSON_GetStringValue(text_node);

                    // Create copy of string to return
                    if (texto) {
                        extracted_text = strdup(texto);
                    }
                }
            }
        }
    }

    // Step 4: Free JSON object memory
    cJSON_Delete(root);

    // Step 5: Return extracted text (or NULL if not found)
    return extracted_text;
}

// --- cURL Functions ---
char* make_http_request(const char* url, const char* payload) {
    // Initialize necessary variables
    CURL *curl_handle;                 // cURL handler
    CURLcode res;                      // Operation result code
    struct MemoryStruct chunk;         // Structure to store response

    // Initialize memory structure
    chunk.memory = malloc(1);          // Allocate initial byte
    chunk.size = 0;                    // Initialize size as zero

    // Initialize cURL library
    curl_global_init(CURL_GLOBAL_ALL); // Initialize cURL library with all global options
    curl_handle = curl_easy_init();    // Create cURL handler

    // Check if initialization succeeded
    if (!curl_handle) {                           // If couldn't initialize cURL
        fprintf(stderr, "Error starting cURL\n"); // Display error message
        free(chunk.memory);                       // Free allocated memory
        return NULL;                              // Return NULL indicating failure
    }

    // Configure request headers
    struct curl_slist *headers = NULL; // Header list for request
    headers = curl_slist_append(headers, "Content-Type: application/json"); // Set content type as JSON

    // Configure request options
    curl_easy_setopt(curl_handle, CURLOPT_URL, url);                           // Set URL
    curl_easy_setopt(curl_handle, CURLOPT_HTTPHEADER, headers);                // Set headers
    curl_easy_setopt(curl_handle, CURLOPT_POSTFIELDS, payload);                // Set data to be sent
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Set callback function
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)&chunk);          // Set where to save data

    // Execute loading animation (simulates loading animation, since we can't perform multiple tasks simultaneously)
    show_loading();

    // Execute request
    res = curl_easy_perform(curl_handle);

    // Check if there was an error
    if (res != CURLE_OK) {                                                     // If request failed
        fprintf(stderr, "cURL request failed: %s\n", curl_easy_strerror(res)); // Display error message
        free(chunk.memory);                                                    // Free allocated memory
        curl_slist_free_all(headers);                                          // Free headers
        curl_easy_cleanup(curl_handle);                                        // Free cURL handler
        curl_global_cleanup();                                                 // Free global cURL resources
        return NULL;                                                           // Return NULL indicating failure
    }

    // Free used resources
    curl_slist_free_all(headers);   // Free header list
    curl_easy_cleanup(curl_handle); // Free cURL handler
    curl_global_cleanup();          // Free global cURL resources

    // Return obtained response
    return chunk.memory;
}

// Function to show loading animation
void show_loading() {
    int dots = 0;                  // Dot counter for animation
    for (int i = 0; i < 4; i++) {  // 4 times of 0.5s = 2 seconds
        printf("\rConsulting AI%s", (dots % 4 == 0 ? "   " : dots % 4 == 1 ? "." : dots % 4 == 2 ? ".." : "..."));
        fflush(stdout);            // Update standard output immediately
        dots++;                    // Increment dot counter
        dormir(500);               // Wait 0.5 seconds
    }
    printf("\rProcessing response...");
    fflush(stdout); // Update standard output immediately
}

// Callback function to store HTTP request response
static size_t WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp) {
    // Calculate real size of received data
    size_t realsize = size * nmemb;

    // Convert userp pointer to correct type
    struct MemoryStruct *mem = (struct MemoryStruct *)userp;

    // Increase memory size to fit new data
    char *ptr = realloc(mem->memory, mem->size + realsize + 1);

    // Check if memory allocation succeeded
    if (ptr == NULL) {
        printf("Error: could not allocate memory in callback!\n");
        return 0; // Return 0 to indicate allocation failure
    }

    // Update pointer to new memory area
    mem->memory = ptr;

    // Copy new data to memory
    memcpy(&(mem->memory[mem->size]), contents, realsize);
    // Update memory size
    mem->size += realsize;
    // Add null terminator at end of string
    mem->memory[mem->size] = 0;

    // Return real size to inform operation was successful
    return realsize;
}

// Chat History Functions
HistoricoChat* initialize_chat_history() {
    // Allocate memory for chat history
    HistoricoChat* history = (HistoricoChat*)malloc(sizeof(HistoricoChat));
    if (history == NULL) {                                              // If allocation fails
        fprintf(stderr, "Error allocating memory for chat history.\n"); // Display error message
        return NULL;                                                    // Return NULL indicating failure
    }

    // Initialize history fields
    history->turno = NULL;
    history->contador = 0;
    history->capacidade = 0;

    // Return pointer to initialized history
    return history;
}

// Function to add a new turn to chat history
void add_turn(HistoricoChat* historico, const char* role, const char* text) {
    // Check if history needs to be expanded
    if (historico->contador >= historico->capacidade) {
        // Increase history capacity
        int new_capacity;

        if (historico->capacidade == 0) // If current capacity is zero
            new_capacity = 2;           // Define new initial capacity as 2
        else                            // If already has capacity
            new_capacity = historico->capacidade * 2; // Double current capacity

        TurnoMensagem* new_turns = (TurnoMensagem*)realloc(historico->turno, new_capacity * sizeof(TurnoMensagem)); // Allocate memory for new turns with new capacity
        if (new_turns == NULL) { // If allocation fails
            fprintf(stderr, "Error allocating memory for history turns.\n"); // Display error message
            return;              // Return without adding turn
        }
        historico->turno = new_turns;         // Update pointer to turns with new allocated memory
        historico->capacidade = new_capacity; // Update history capacity
    }

    // Add new turn to history
    TurnoMensagem* current_turn = &historico->turno[historico->contador++]; // Increment turn counter and get pointer to current turn
    current_turn->role = strdup(role);                                      // Duplicate turn role string
    current_turn->text = strdup(text);                                      // Duplicate turn text string
}

// Function to free chat history
void free_chat_history(HistoricoChat* historico) {
    if (historico != NULL) { // Check if history is not null
        for (int i = 0; i < historico->contador; i++) { // Go through all turns
            free(historico->turno[i].role);             // Free turn role memory
            free(historico->turno[i].text);             // Free turn text memory
        }
        // Free turns array memory
        free(historico->turno);
        // Free history itself memory
        free(historico);
    }
}

// Function to display conversation history
void display_history(HistoricoChat* historico) {
    if (historico != NULL && historico->contador > 0) {                             // Check if history is not null and has turns
        printf("\n----- Conversation History -----\n");
        for (int i = 0; i < historico->contador; i++) {                             // Go through all history turns
            printf("%s: %s\n", historico->turno[i].role, historico->turno[i].text); // Display turn role and text
        }
        printf("---------------------------------\n");
    } else                                        // If history is null or has no turns
        printf("\nNo history available.\n"); // Display message informing there's no history
}

// Function to get weather data from OpenWeather API
DataClima get_weather_data(const char* cidade) {
    DataClima clima = {0};   // Initialize weather data structure
    clima.valid = 0;         // Mark as invalid initially

    // Encode city for URL (solves space problem)
    char* cidade_encoded = url_encode(cidade); // Function that encodes city for URL use
    if (!cidade_encoded) {                     // If encoding fails
        return clima;                          // Return invalid weather structure
    }

    // Build OpenWeather API URL
    char url[512];                             // Buffer to store complete URL
    snprintf(url, sizeof(url), "https://api.openweathermap.org/data/2.5/weather?q=%s&appid=%s&units=metric&lang=en",
             cidade_encoded, API_KEY_WEATHER); // Concatenate encoded city and API key in URL

    // Make HTTP request
    CURL *curl;                // cURL handler
    CURLcode res;              // Operation result code
    struct MemoryStruct chunk; // Structure to store response

    chunk.memory = malloc(1);  // Allocate initial byte for memory
    chunk.size = 0;            // Initialize size as zero

    curl = curl_easy_init();   // Initialize cURL
    if(curl) {                                                              // Check if initialization was successful
        curl_easy_setopt(curl, CURLOPT_URL, url);                           // Set request URL
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteMemoryCallback); // Set callback function to write received data
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);          // Set where received data will be stored

        res = curl_easy_perform(curl); // Execute HTTP request

        if (res == CURLE_OK) { // If request was successful
            // Parse JSON response
            cJSON *json = cJSON_Parse(chunk.memory); // Convert JSON response to cJSON object
            if (json) { // If parsing was successful
                cJSON *main = cJSON_GetObjectItemCaseSensitive(json, "main");             // Get "main" object from JSON
                cJSON *weather_array = cJSON_GetObjectItemCaseSensitive(json, "weather"); // Get "weather" array from JSON
                cJSON *name = cJSON_GetObjectItemCaseSensitive(json, "name");             // Get city name from JSON

                // Check if necessary objects were found and their types
                if (main && weather_array && cJSON_IsArray(weather_array) &&
                    name && cJSON_IsString(name)) {
                    cJSON *temp = cJSON_GetObjectItemCaseSensitive(main, "temp"); // Get temperature from "main" object
                    cJSON *weather_item = cJSON_GetArrayItem(weather_array, 0);   // Get first item from "weather" array

                    // Check if temperature is a number and weather item exists
                    if (temp && cJSON_IsNumber(temp) && weather_item) {
                        cJSON *description = cJSON_GetObjectItemCaseSensitive(weather_item, "description"); // Get weather description

                        clima.temperatura = (float)cJSON_GetNumberValue(temp);                  // Convert temperature to float
                        strncpy(clima.cidade, cJSON_GetStringValue(name), MAX_CITY_NAME - 1);   // Copy city name
                        clima.cidade[MAX_CITY_NAME - 1] = '\0';                                 // Ensure null termination

                        if (description && cJSON_IsString(description)) { // If description was found and is string
                            strncpy(clima.description, cJSON_GetStringValue(description), 99); // Copy description
                            clima.description[99] = '\0';                                      // Ensure null termination
                        }
                        clima.valid = 1; // Mark weather data as valid
                    }
                }
                cJSON_Delete(json); // Free cJSON object memory
            }
        }
        curl_easy_cleanup(curl); // Free cURL handler
    }

    // Free encoded city memory
    curl_free(cidade_encoded);
    free(chunk.memory);

    return clima; // Return weather data (may be invalid if couldn't get data)
}

// Function to encode URL
char* url_encode(const char* str) {
    CURL *curl = curl_easy_init();                  // Initialize cURL
    char *encoded = curl_easy_escape(curl, str, 0); // Encode string for URL
    curl_easy_cleanup(curl);                        // Free cURL handler
    return encoded;                                 // Return encoded string (or NULL if fails)
}

// Function to display program credits
void credits() {
    limpar_tela(); // Clear screen before showing credits

    printf("\033[36m"); // Cyan for borders
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                                                               ║\n");
    printf("║                            🏆 GenieC - Credits 🏆                             ║\n");
    printf("║                                                                               ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m"); // Reset color

    printf("\n");
    printf("\033[1;32m💻 Developed by:\033[0m\n");
    printf("   \033[37m• Lorenzo Farias\033[0m\n\n");

    printf("\033[1;34m🎓 HackClub\033[0m\n");
    printf("   \033[37mSummer of Code 2025\033[0m\n\n");

    printf("\033[36m"); // Cyan for bottom border
    printf("─────────────────────────────────────────────────────────────────────────────────\n");
    printf("\033[0m"); // Reset color

    printf("\n\033[1;32m🤖 Thank you for using GenieC! 🤖\033[0m\n");
    printf("\033[33mPress Enter to exit...\033[0m");

    getchar(); // Pause for user to read message
}