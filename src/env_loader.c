/* env_loader.c - Wrapper para dotenv-c
 * GenieC - Assistente Inteligente
 * Usa a biblioteca dotenv-c instalada
 */

#include "env_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Include da biblioteca dotenv-c
#include <dotenv.h>

// Carrega as variáveis de ambiente do arquivo .env usando dotenv-c
int carregar_env(const char* filename) {
    // Tenta carregar do diretório atual primeiro
    int result = env_load((char*)filename, 0);

    if (result == 0) {
        return 1; // Sucesso!
    }

    // Se falhar, tenta caminhos alternativos (pasta acima)
    const char* paths[] = {
        "../.env",
        "../../.env",
        NULL
    };

    for (int i = 0; paths[i] != NULL; i++) {
        result = env_load((char*)paths[i], 0);
        if (result == 0) {
            return 1; // Sucesso!
        }
    }

    fprintf(stderr, "Aviso: Arquivo %s não encontrado ou erro ao carregar.\n", filename);
    return 0;
}

// Obtém o valor de uma variável de ambiente
const char* obter_env(const char* key) {
    // Valida parâmetro
    if (!key) return NULL;

    // Usa getenv() padrão do sistema, pois dotenv-c carrega para o ambiente
    return getenv(key);
}

// Libera a memória alocada (dotenv-c gerencia internamente)
void limpar_env() {
    // dotenv-c não requer limpeza explícita
    // As variáveis ficam no ambiente do processo
}

// Função adicional: lista todas as variáveis carregadas (útil para debug)
void listar_env() {
    printf("\n=== Variáveis de Ambiente Carregadas ===\n");

    // Lista as principais variáveis do GenieC
    const char* keys[] = {
        "GEMINI_API_KEY",
        "OPENWEATHER_API_KEY",
        "APP_NAME",
        "APP_VERSION",
        "DEBUG_MODE",
        NULL
    };

    // Conta e exibe as variáveis encontradas
    int count = 0;
    for (int i = 0; keys[i] != NULL; i++) {
        const char* value = getenv(keys[i]);
        if (value) {
            count++;
            // Mascara valores sensíveis (API keys)
            if (strstr(keys[i], "KEY") || strstr(keys[i], "SECRET")) {
                printf("%s=***HIDDEN***\n", keys[i]);
            } else {
                printf("%s=%s\n", keys[i], value);
            }
        }
    }

    printf("Total: %d variáveis\n", count);
    printf("=========================================\n\n");
}