/* historico.c - Gerenciamento do histórico de conversação
 * GenieC - Assistente Inteligente
 */

#include "historico.h"
#include "config.h"
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
        int nova_capacidade = (historico->capacidade == 0) ? 2 : historico->capacidade * 2;

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

// Exibe o histórico completo
void exibir_historico(HistoricoChat* historico) {
    if (historico != NULL && historico->contador > 0) {
        printf("\n");
        printf("\033[36m╔═══════════════════════════════════════════════════════════╗\033[0m\n");
        printf("\033[36m║           📜 HISTÓRICO DA CONVERSA                        ║\033[0m\n");
        printf("\033[36m╚═══════════════════════════════════════════════════════════╝\033[0m\n\n");

        for (int i = 0; i < historico->contador; i++) {
            if (strcmp(historico->turno[i].role, "user") == 0) {
                printf("\033[1;32m👤 Você:\033[0m %s\n", historico->turno[i].text);
            } else {
                printf("\033[1;36m🤖 GenieC:\033[0m %s\n", historico->turno[i].text);
            }
            printf("\n");
        }

        printf("\033[33m─────────────────────────────────────────────────────────────\033[0m\n");
        printf("\033[33mTotal de interações: %d (máximo: %d)\033[0m\n",
               historico->contador, MAX_HISTORY_TURNS);
    } else {
        printf("\n\033[33m📭 Nenhum histórico disponível.\033[0m\n");
    }
}

