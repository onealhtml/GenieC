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

#include "src/ui.h"         // Funções de interface
#include "src/clima.h"      // Funções de clima
#include "src/gemini.h"     // Funções da API Gemini
#include "src/historico.h"  // Funções de histórico
#include "limpar_tela.h"    // Função para limpar a tela
#include "dormir.h"         // Função para dormir (pausar a execução)

#define MAX_PROMPT_SIZE 10000 // Tamanho máximo do prompt
#define MAX_CITY_NAME 100     // Tamanho máximo do nome da cidade

// --- Declaração de Funções ---
void creditos(); // Função para exibir os créditos do projeto

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
    DataClima clima = obter_dados_clima(cidade);                     // Chama a função para obter os dados do clima

    limpar_tela();         // Limpa a tela

    menu_com_clima(clima); // Exibe o menu com informações do clima

    // Inicializa o histórico do chat
    HistoricoChat* chat_historico = inicializar_chat_historico();      // Função para inicializar o histórico do chat
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
            liberar_historico_chat(chat_historico);                 // Chama a função que libera o histórico atual
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

        // Consulta o Gemini
        char* texto_final = consultar_gemini(minha_pergunta, chat_historico, cidade);
        if (texto_final == NULL) {                                            // Se não conseguiu obter resposta
            fprintf(stderr, "Erro: Não foi possível obter resposta do Gemini.\n"); // Exibe mensagem de erro
            continue;                                                         // Volta para o início do loop
        }

        printf("\r                         \r"); // Limpa a linha atual
        printf("\n\033[36mGenieC:\033[0m %s\n\n", texto_final); // Exibe a resposta do Gemini

        // Adiciona a resposta do Gemini ao histórico
        adicionar_turno(chat_historico, "model", texto_final);

        // Libera a memória alocada
        free(texto_final);
    }

    // Libera o histórico antes de sair
    liberar_historico_chat(chat_historico);

    creditos(); // Exibe os créditos do projeto

    return 0;   // Retorna 0 para indicar que o programa terminou com sucesso
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

