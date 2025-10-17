/* env_loader.h - Carregador de variáveis de ambiente
 * GenieC - Assistente Inteligente
 */

#ifndef ENV_LOADER_H
#define ENV_LOADER_H

// Carrega as variáveis de ambiente do arquivo .env
int carregar_env(const char* filename);

// Obtém o valor de uma variável de ambiente
const char* obter_env(const char* key);

// Libera a memória alocada
void limpar_env();

// Lista todas as variáveis carregadas (para debug)
void listar_env();

#endif // ENV_LOADER_H
