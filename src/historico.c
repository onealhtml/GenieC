/* historico.c - Gerenciamento do histórico de conversação
 * GenieC - Assistente Inteligente
 */

#include "historico.h"
#include "config.h"
#include "../limpar_tela.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Inicializa o histórico do chat
HistoricoChat* inicializar_chat_historico() {
    HistoricoChat* history = (HistoricoChat*)malloc(sizeof(HistoricoChat));
    if (history == NULL) {
        fprintf(stderr, "Erro ao alocar memória para o histórico do chat.\n");
        return NULL;
    }

    history->turno = NULL;
    history->contador = 0;
    history->capacidade = 0;

    return history;
}

// Adiciona um turno ao histórico com limite automático
void adicionar_turno(HistoricoChat* historico, const char* role, const char* text) {
    // Verifica se precisa expandir o array
    if (historico->contador >= historico->capacidade) {
        int nova_capacidade;
        if (historico->capacidade == 0) {
            nova_capacidade = 2;
        } else {
            nova_capacidade = historico->capacidade * 2;
        }

        TurnoMensagem* novos_turnos = (TurnoMensagem*)realloc(
            historico->turno,
            nova_capacidade * sizeof(TurnoMensagem)
        );

        if (novos_turnos == NULL) {
            fprintf(stderr, "Erro ao alocar memória para os turnos do histórico.\n");
            return;
        }

        historico->turno = novos_turnos;
        historico->capacidade = nova_capacidade;
    }

    // Adiciona o novo turno
    TurnoMensagem* turno_atual = &historico->turno[historico->contador++];
    turno_atual->role = strdup(role);
    turno_atual->text = strdup(text);

    // Limita o histórico ao máximo definido
    if (historico->contador > MAX_HISTORY_TURNS) {
        // Remove o turno mais antigo
        TurnoMensagem* turno_antigo = &historico->turno[0];
        free(turno_antigo->role);
        free(turno_antigo->text);

        // Move todos os turnos uma posição para frente
        memmove(
            turno_antigo,
            turno_antigo + 1,
            (historico->contador - 1) * sizeof(TurnoMensagem)
        );

        historico->contador--;
    }
}

// Libera a memória do histórico
void liberar_historico_chat(HistoricoChat* historico) {
    if (historico != NULL) {
        for (int i = 0; i < historico->contador; i++) {
            free(historico->turno[i].role);
            free(historico->turno[i].text);
        }
        free(historico->turno);
        free(historico);
    }
}

// Exibe o histórico completo em tela separada
void exibir_historico(HistoricoChat* historico) {
    // Limpa a tela para mostrar apenas o histórico
    limpar_tela();

    if (historico != NULL && historico->contador > 0) {
        printf("\n");
        printf("\033[36m╔═══════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[36m║                           📜 HISTÓRICO DA CONVERSA                            ║\033[0m\n");
        printf("\033[36m╚═══════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");

        // Itera sobre todos os turnos
        for (int i = 0; i < historico->contador; i++) {
            // Exibe mensagem do usuário
            if (strcmp(historico->turno[i].role, "user") == 0) {
                printf("\033[1;32m👤 Você:\033[0m\n");
                printf("   %s\n\n", historico->turno[i].text);
            } else {
                // Exibe mensagem do assistente
                printf("\033[1;36m🤖 GenieC:\033[0m\n");
                printf("   %s\n\n", historico->turno[i].text);
            }
        }

        printf("\033[33m═══════════════════════════════════════════════════════════════════════════════\033[0m\n");
        printf("\033[33mTotal de interações: %d (máximo: %d)\033[0m\n\n",
               historico->contador, MAX_HISTORY_TURNS);
    } else {
        printf("\n");
        printf("\033[36m╔═══════════════════════════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[36m║                          📜 HISTÓRICO DA CONVERSA                             ║\033[0m\n");
        printf("\033[36m╚═══════════════════════════════════════════════════════════════════════════════╝\033[0m\n\n");
        printf("\033[33m📭 Nenhum histórico disponível ainda.\033[0m\n\n");
        printf("\033[33m═══════════════════════════════════════════════════════════════════════════════\033[0m\n\n");
    }

    // Aguarda o usuário pressionar Enter para voltar
    printf("\033[1;37m⏎ Pressione Enter para voltar ao chat...\033[0m");
    getchar(); // Consome o Enter
}
