/* ui.c - Interface do usuário
 * GenieC - Assistente Inteligente
 */

#include "ui.h"
#include "clima.h"
#include "../old/dormir.h"
#include <stdio.h>
#include <string.h>

// Função para obter o ícone apropriado baseado na descrição do clima
const char* obter_icone_clima(const char* description) {
    if (!description) return "🌡️";

    // Converte para lowercase para comparação (simplificada)
    char desc_lower[100];
    strncpy(desc_lower, description, sizeof(desc_lower) - 1);
    desc_lower[sizeof(desc_lower) - 1] = '\0';

    // Mapeia descrições para ícones
    if (strstr(desc_lower, "limpo") || strstr(desc_lower, "céu limpo")) return "☀️";
    if (strstr(desc_lower, "ensolarado")) return "☀️";
    if (strstr(desc_lower, "poucas nuvens")) return "🌤️";
    if (strstr(desc_lower, "nuvens dispersas")) return "⛅";
    if (strstr(desc_lower, "nublado")) return "☁️";
    if (strstr(desc_lower, "nuvens")) return "☁️";
    if (strstr(desc_lower, "chuva")) return "🌧️";
    if (strstr(desc_lower, "chuvisco")) return "🌦️";
    if (strstr(desc_lower, "trovoada") || strstr(desc_lower, "tempestade")) return "⛈️";
    if (strstr(desc_lower, "neve")) return "❄️";
    if (strstr(desc_lower, "neblina") || strstr(desc_lower, "névoa")) return "🌫️";
    if (strstr(desc_lower, "nevoeiro")) return "🌫️";

    return "🌡️"; // Ícone padrão
}

// Mostra a arte ASCII inicial
void mostrar_arte_inicial() {
    printf("\033[36m");
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
    printf("\033[0m");

    printf("\n");
    printf("\033[1;32m");
    printf("🤖 Bem-vindo ao GenieC - Seu Assistente Inteligente em C! 🤖\n");
    printf("\033[0m");
}

// Exibe o menu com informações do clima
void menu_com_clima(DataClima clima) {
    mostrar_arte_inicial();

    if (clima.valid) {
        const char* icone = obter_icone_clima(clima.description);
        printf("\n\033[1;34m");
        printf("%s Clima atual em %s: %.1f°C - %s\n",
               icone, clima.cidade, clima.temperatura, clima.description);
        printf("\033[0m");
    } else {
        printf("\n\033[1;31m");
        printf("❌ Não foi possível obter informações do clima\n");
        printf("\033[0m");
    }

    printf("\n\033[33m");
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
    printf("\033[0m");

    printf("\n\033[32m");
    printf("💡 \033[1mDicas:\033[0m\033[32m Seja específico em suas perguntas para obter melhores respostas!\n");
    printf("🌟 \033[1mExemplo:\033[0m\033[32m \"Qual a história de minha cidade?\"\n");
    printf("\033[0m\n");
}

// Exibe ajuda
void mostrar_ajuda() {
    printf("\n\033[1;36m");
    printf("╔═══════════════════════════════════════════════════════════════════════════════╗\n");
    printf("║                                📚 AJUDA - GenieC                              ║\n");
    printf("╚═══════════════════════════════════════════════════════════════════════════════╝\n");
    printf("\033[0m");

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
    printf("   🔹 Seja específico nas perguntas\n");
    printf("   🔹 Faça perguntas de follow-up\n");
    printf("   🔹 Use contexto da conversa anterior\n\n");

    printf("\033[1;37m🌟 Exemplos de perguntas:\033[0m\n");
    printf("   \033[32m• \"Qual é a previsão do tempo para hoje em Brasília?\"\033[0m\n");
    printf("   \033[32m• \"Como fazer um currículo profissional?\"\033[0m\n");
    printf("   \033[32m• \"Explique o que é inteligência artificial\"\033[0m\n\n");

    printf("\033[1;37m⚙️ Funcionalidades:\033[0m\n");
    printf("   ✅ Pesquisa em tempo real via Google\n");
    printf("   ✅ Contexto de conversa preservado (até 20 turnos)\n");
    printf("   ✅ Retry automático em caso de falha de rede\n");
    printf("   ✅ Respostas em português brasileiro\n\n");

    printf("\033[1;33m💬 Agora você pode continuar fazendo suas perguntas!\033[0m\n");
    printf("────────────────────────────────────────────────────────────────────────────────\n");
}

// Animação de loading
void mostrar_loading() {
    int dots = 0;
    for (int i = 0; i < 4; i++) {
        const char* loading_dots;
        if (dots % 4 == 0) {
            loading_dots = "   ";
        } else if (dots % 4 == 1) {
            loading_dots = ".";
        } else if (dots % 4 == 2) {
            loading_dots = "..";
        } else {
            loading_dots = "...";
        }
        printf("\rConsultando IA%s", loading_dots);
        fflush(stdout);
        dots++;
        dormir(500);
    }
    printf("\rProcessando resposta...");
    fflush(stdout);
}
