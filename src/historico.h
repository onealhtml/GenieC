#ifndef HISTORICO_H
#define HISTORICO_H

// Estrutura para mensagem individual
typedef struct {
    char* role;
    char* text;
} TurnoMensagem;

// Estrutura para histórico completo
typedef struct {
    TurnoMensagem* turno;
    int contador;
    int capacidade;
} HistoricoChat;

// Funções de gerenciamento do histórico
HistoricoChat* inicializar_chat_historico();
void adicionar_turno(HistoricoChat* historico, const char* role, const char* text);
void liberar_historico_chat(HistoricoChat* historico);
void exibir_historico(HistoricoChat* historico);

#endif // HISTORICO_H

