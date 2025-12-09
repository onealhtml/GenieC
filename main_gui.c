/* GenieC GUI - Interface Gráfica usando Webview

   Descrição:
   Este arquivo implementa a interface gráfica do GenieC usando a biblioteca Webview.
   Ele gerencia a comunicação entre o frontend (HTML/JavaScript) e o backend (
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <locale.h>
#ifdef _WIN32
#include <windows.h>
#endif
#include <cjson/cJSON.h>
#include "webview/webview.h"

#ifdef _WIN32
#define IDI_ICON1 101
#endif

#include "src/clima.h"
#include "src/env_loader.h"
#include "src/gemini.h"
#include "src/historico.h"
#include "src/ui_cli.h"
#include "src/ui_loader.h"
#include "src/grafo.h"

// Variáveis globais para comunicação
static HistoricoChat* g_historico = NULL;
static char g_cidade[100] = "";
static Grafo* g_grafo = NULL;

// Callback quando JavaScript chama funções C
void handle_rpc(const char *seq, const char *req, void *arg) {
    webview_t w = (webview_t)arg;

    // DEBUG: Log para ver o que está chegando
    const char* seq_str;
    if (seq != NULL) {
        seq_str = seq;
    } else {
        seq_str = "(null)";
    }
    const char* req_str;
    if (req != NULL) {
        req_str = req;
    } else {
        req_str = "(null)";
    }
    fprintf(stderr, "[DEBUG handle_rpc] seq=%s\n", seq_str);
    fprintf(stderr, "[DEBUG handle_rpc] req=%s\n", req_str);
    fflush(stderr);

    // Parse robusto com cJSON
    cJSON *root = cJSON_Parse(req);
    if (!root) {
        fprintf(stderr, "[ERRO] JSON inválido recebido\n");
        webview_return(w, seq, 1, "{\"error\":\"invalid_json\"}");
        return;
    }

    // Extrai o método de várias formas possíveis
    const char *method = NULL;
    const char *texto = NULL;

    // CASO 1: Array de parâmetros [{"text":"..."}] ou [{}]
    if (cJSON_IsArray(root)) {
        fprintf(stderr, "[DEBUG] Request é um array\n");
        cJSON *first_item = cJSON_GetArrayItem(root, 0);
        if (first_item && cJSON_IsObject(first_item)) {
            // Primeiro verifica se tem o campo "_method" (usado pelas funções do grafo)
            cJSON *_method_item = cJSON_GetObjectItemCaseSensitive(first_item, "_method");
            if (_method_item && cJSON_IsString(_method_item)) {
                method = _method_item->valuestring;
                fprintf(stderr, "[DEBUG] Detectado _method: %s\n", method);
            }
            // Verifica se tem o campo "text"
            else {
                cJSON *text_item = cJSON_GetObjectItemCaseSensitive(first_item, "text");
                if (text_item && cJSON_IsString(text_item)) {
                    method = "pergunta";
                    texto = text_item->valuestring;
                    fprintf(stderr, "[DEBUG] Detectado array com text, assumindo método 'pergunta'\n");
                }
                // Verifica se tem origem/destino (grafo)
                else {
                    cJSON *origem_item = cJSON_GetObjectItemCaseSensitive(first_item, "origem");
                    cJSON *destino_item = cJSON_GetObjectItemCaseSensitive(first_item, "destino");
                    if (origem_item && destino_item && cJSON_IsString(origem_item) && cJSON_IsString(destino_item)) {
                        method = "grafo_calcular_rota";
                        fprintf(stderr, "[DEBUG] Detectado array com origem/destino para grafo\n");
                    }
                    // Verifica se tem o campo "cidade" (atualizar clima)
                    else {
                        cJSON *cidade_item = cJSON_GetObjectItemCaseSensitive(first_item, "cidade");
                        if (cidade_item && cJSON_IsString(cidade_item)) {
                            method = "atualizar_clima";
                            texto = cidade_item->valuestring;
                            fprintf(stderr, "[DEBUG] Detectado array com cidade, assumindo método 'atualizar_clima'\n");
                        }
                        // Verifica se tem o campo "method" dentro do objeto (fallback RPC)
                        else {
                            cJSON *method_item = cJSON_GetObjectItemCaseSensitive(first_item, "method");
                            if (method_item && cJSON_IsString(method_item)) {
                                method = method_item->valuestring;
                                fprintf(stderr, "[DEBUG] Detectado método dentro do array: %s\n", method);
                            } else {
                                // Verifica se o objeto tem algum campo - se não tiver, é um objeto vazio
                                if (first_item->child == NULL) {
                                    // Objeto realmente vazio e não é do grafo - assume limpar
                                    method = "limpar";
                                    fprintf(stderr, "[DEBUG] Detectado array com objeto vazio, assumindo método 'limpar'\n");
                                } else {
                                    fprintf(stderr, "[DEBUG] Objeto tem campos mas não reconhecidos\n");
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    // CASO 2: Objeto normal {"method":"...", "params":{...}}
    else if (cJSON_IsObject(root)) {
        cJSON *method_item = cJSON_GetObjectItemCaseSensitive(root, "method");
        if (cJSON_IsString(method_item)) {
            method = method_item->valuestring;
        }
    }

    const char* method_str;
    if (method != NULL) {
        method_str = method;
    } else {
        method_str = "(nenhum)";
    }
    fprintf(stderr, "[DEBUG] Método identificado: %s\n", method_str);
    fflush(stderr);

    if (method && strcmp(method, "pergunta") == 0) {
        const char* texto_str;
        if (texto != NULL) {
            texto_str = texto;
        } else {
            texto_str = "(vazio)";
        }
        fprintf(stderr, "[DEBUG] Texto extraído: %s\n", texto_str);
        fflush(stderr);

        if (texto && texto[0] != '\0') {
            // Verifica comandos especiais
            if (strcmp(texto, "ajuda") == 0 || strcmp(texto, "help") == 0) {
                char ajuda[3072];
                const char* cidade_exemplo;
                if (g_cidade[0] != '\0') {
                    cidade_exemplo = g_cidade;
                } else {
                    cidade_exemplo = "minha cidade";
                }
                snprintf(ajuda, sizeof(ajuda),
                    "📚 <b>AJUDA - GenieC</b><br><br>"
                    "🎯 <b>Como usar:</b><br>"
                    "• Digite sua pergunta e pressione Enter<br>"
                    "• O GenieC mantém o contexto da conversa<br><br>"
                    "📝 <b>Comandos Especiais:</b><br>"
                    "• <b>ajuda</b> ou <b>help</b> - Mostra esta ajuda<br>"
                    "• <b>historico</b> - Mostra histórico da conversa<br><br>"
                    "🗺️ <b>Comandos de Grafo (Algoritmo de Dijkstra):</b><br>"
                    "• <b>grafo Cidade1-Cidade2</b> - Calcula menor caminho<br>"
                    "  Exemplo: <b>grafo São Paulo-Rio de Janeiro</b><br>"
                    "  → A IA busca distâncias reais + mostra no mapa!<br>"
                    "• <b>grafocidades</b> - Lista todas as cidades no grafo<br>"
                    "• <b>grafomapa</b> - Visualiza o grafo no mapa interativo<br><br>"
                    "💡 <b>Dicas:</b><br>"
                    "🔹 Seja específico nas perguntas<br>"
                    "🔹 Faça perguntas de follow-up<br>"
                    "🔹 Use contexto da conversa anterior<br>"
                    "🔹 O grafo usa dados reais da IA!<br><br>"
                    "🌟 <b>Exemplos:</b><br>"
                    "• \"Qual é a história de %s?\"<br>"
                    "• \"Como fazer um currículo profissional?\"<br>"
                    "• \"grafo Curitiba-Florianópolis\"",
                    cidade_exemplo);

                char js_code[4096];
                snprintf(js_code, sizeof(js_code),
                    "adicionarMensagemHTML('Sistema', `%s`, false);", ajuda);
                webview_eval(w, js_code);
                webview_return(w, seq, 0, "{}");
                cJSON_Delete(root);
                return;
            }

            if (strcmp(texto, "historico") == 0) {
                char historico_html[8192] = "📜 <b>Histórico da Conversa:</b><br><br>";

                if (g_historico && g_historico->contador > 0) {
                    int pos = strlen(historico_html);
                    for (int i = 0; i < g_historico->contador && pos < 7500; i++) {
                        const char* icone;
                        const char* nome;
                        if (strcmp(g_historico->turno[i].role, "user") == 0) {
                            icone = "👤";
                            nome = "Você";
                        } else {
                            icone = "🤖";
                            nome = "GenieC";
                        }

                        char linha[512];
                        char msg_preview[200];
                        size_t texto_len = strlen(g_historico->turno[i].text);
                        strncpy(msg_preview, g_historico->turno[i].text, 150);
                        msg_preview[150] = '\0';

                        const char* ellipsis;
                        if (texto_len > 150) {
                            ellipsis = "...";
                        } else {
                            ellipsis = "";
                        }

                        snprintf(linha, sizeof(linha), "%s <b>%s:</b> %s%s<br><br>",
                            icone, nome, msg_preview, ellipsis);

                        if (pos + strlen(linha) < sizeof(historico_html) - 100) {
                            strcat(historico_html, linha);
                            pos += strlen(linha);
                        } else {
                            break;
                        }
                    }
                } else {
                    strcat(historico_html, "<i>Nenhuma conversa ainda.</i>");
                }

                char js_code[10000];
                snprintf(js_code, sizeof(js_code),
                    "adicionarMensagemHTML('Sistema', `%s`, false);", historico_html);
                webview_eval(w, js_code);
                webview_return(w, seq, 0, "{}");
                cJSON_Delete(root);
                return;
            }

            // Comando para listar cidades no grafo
            if (strcmp(texto, "grafocidades") == 0) {
                char* resultado = listar_cidades_grafo(g_grafo);

                // Usa buffer maior para evitar truncamento
                size_t js_size = strlen(resultado) + 256;
                char* js_code = (char*)malloc(js_size);
                snprintf(js_code, js_size,
                    "adicionarMensagemHTML('Sistema', `%s`, false);", resultado);
                webview_eval(w, js_code);

                free(js_code);
                free(resultado);
                webview_return(w, seq, 0, "{}");
                cJSON_Delete(root);
                return;
            }

            // Comando para ver mapa do grafo
            if (strcmp(texto, "grafomapa") == 0) {
                char* resultado = gerar_mapa_grafo(g_grafo);

                size_t js_size = strlen(resultado) + 256;
                char* js_code = (char*)malloc(js_size);
                snprintf(js_code, js_size,
                    "adicionarMensagemHTML('Sistema', `%s`, false);", resultado);
                webview_eval(w, js_code);

                free(js_code);
                free(resultado);
                webview_return(w, seq, 0, "{}");
                cJSON_Delete(root);
                return;
            }

            // Comando para calcular menor caminho entre cidades
            if (strncmp(texto, "grafo ", 6) == 0) {
                char origem[MAX_NOME_CIDADE] = {0};
                char destino[MAX_NOME_CIDADE] = {0};

                // Parse: "grafo São Paulo-Rio de Janeiro"
                const char* input = texto + 6;
                char* separador = strchr(input, '-');

                if (separador) {
                    size_t len_origem = separador - input;
                    if (len_origem > 0 && len_origem < MAX_NOME_CIDADE) {
                        strncpy(origem, input, len_origem);
                        origem[len_origem] = '\0';

                        // Remove espaços no final da origem
                        char* end = origem + strlen(origem) - 1;
                        while (end > origem && *end == ' ') *end-- = '\0';
                    }

                    // Extrai destino
                    const char* dest_start = separador + 1;
                    while (*dest_start == ' ') dest_start++;
                    strncpy(destino, dest_start, MAX_NOME_CIDADE - 1);
                    destino[MAX_NOME_CIDADE - 1] = '\0';

                    if (strlen(origem) > 0 && strlen(destino) > 0) {
                        // Mostra mensagem de processamento
                        webview_eval(w, "adicionarMensagemHTML('Sistema', "
                            "'🔄 <b>Consultando IA para obter distâncias...</b><br>"
                            "⏳ Isso pode levar alguns minutos...', false);");

                        fprintf(stderr, "[INFO GRAFO] Processando rota: %s -> %s\n", origem, destino);
                        fflush(stderr);

                        // Consulta a IA para preencher o grafo
                        int conexoes = obter_distancias_ia_e_preencher_grafo(origem, destino, g_grafo);

                        if (conexoes > 0) {
                            char msg_sucesso[768];
                            snprintf(msg_sucesso, sizeof(msg_sucesso),
                                "✅ <b>Malha de rotas criada!</b><br>"
                                "🏙️ <b>%d cidades</b> mapeadas<br>"
                                "🛣️ <b>%d conexões</b> adicionadas pela IA<br>"
                                "🔍 Buscando coordenadas e calculando menor caminho com Dijkstra...<br><br>",
                                g_grafo->num_cidades, conexoes);

                            char* js_code = (char*)malloc(2048);
                            snprintf(js_code, 2048,
                                "adicionarMensagemHTML('Sistema', `%s`, false);", msg_sucesso);
                            webview_eval(w, js_code);
                            free(js_code);

                            // Salva o grafo atualizado com coordenadas E conexões
                            salvar_coordenadas_grafo(g_grafo, "coordenadas_grafo.txt");

                            // Calcula o menor caminho usando Dijkstra COM MAPA
                            char* resultado = calcular_menor_caminho_com_mapa(g_grafo, origem, destino);

                            // Usa buffer dinâmico para suportar o HTML do mapa
                            size_t resultado_size = strlen(resultado) + 256;
                            js_code = (char*)malloc(resultado_size);
                            snprintf(js_code, resultado_size,
                                "adicionarMensagemHTML('GenieC', `%s`, false);", resultado);
                            webview_eval(w, js_code);

                            free(js_code);
                            free(resultado);

                            // Atualiza estatísticas no painel (se estiver aberto)
                            char* stats = obter_estatisticas_grafo(g_grafo);
                            js_code = (char*)malloc(strlen(stats) + 256);
                            snprintf(js_code, strlen(stats) + 256,
                                "if(typeof onEstatisticasGrafo === 'function') onEstatisticasGrafo(%s);", stats);
                            webview_eval(w, js_code);
                            free(js_code);
                            free(stats);
                        } else {
                            webview_eval(w, "adicionarMensagemHTML('Sistema', "
                                "'❌ Não foi possível obter distâncias da IA.<br>"
                                "Verifique se as cidades são válidas.', false);");
                        }
                    } else {
                        webview_eval(w, "adicionarMensagemHTML('Sistema', "
                            "'❌ Formato inválido. Use: <b>grafo Cidade1-Cidade2</b>', false);");
                    }
                } else {
                    webview_eval(w, "adicionarMensagemHTML('Sistema', "
                        "'❌ Formato inválido. Use: <b>grafo Cidade1-Cidade2</b><br>"
                        "Exemplo: <b>grafo São Paulo-Rio de Janeiro</b>', false);");
                }

                webview_return(w, seq, 0, "{}");
                cJSON_Delete(root);
                return;
            }

            // Adiciona ao histórico
            adicionar_turno(g_historico, "user", texto);

            fprintf(stderr, "[DEBUG] Consultando Gemini com cidade: %s\n", g_cidade);
            fflush(stderr);

            // Consulta o Gemini
            char* resposta = consultar_gemini(texto, g_historico, g_cidade);

            if (resposta) {
                fprintf(stderr, "[DEBUG] Resposta recebida: %.100s...\n", resposta);
                fflush(stderr);

                adicionar_turno(g_historico, "model", resposta);

                // Escapa a resposta usando cJSON
                cJSON *tmp = cJSON_CreateString(resposta);
                char *quoted = cJSON_PrintUnformatted(tmp);
                cJSON_Delete(tmp);

                if (quoted) {
                    char js_code[8192];
                    snprintf(js_code, sizeof(js_code),
                        "adicionarMensagem('GenieC', %s, false);", quoted);
                    webview_eval(w, js_code);
                    free(quoted);
                } else {
                    webview_eval(w, "adicionarMensagem('Sistema', 'Erro ao formatar resposta', false);");
                }

                free(resposta);
            } else {
                fprintf(stderr, "[ERRO] consultar_gemini retornou NULL\n");
                fflush(stderr);
                webview_eval(w, "adicionarMensagem('Sistema', 'Erro ao consultar IA', false);");
            }
        } else {
            webview_eval(w, "adicionarMensagem('Sistema', 'Pergunta vazia', false);");
        }

        webview_return(w, seq, 0, "{}");
    }
    else if (method && strcmp(method, "atualizar_clima") == 0) {
        if (texto && texto[0] != '\0') {
            fprintf(stderr, "[DEBUG] Atualizando clima para: %s\n", texto);
            fflush(stderr);

            DataClima clima = obter_dados_clima(texto);

            if (clima.valid) {
                // Usa o nome da cidade retornado pela API (padronizado)
                strncpy(g_cidade, clima.cidade, sizeof(g_cidade) - 1);
                g_cidade[sizeof(g_cidade) - 1] = '\0';

                fprintf(stderr, "[DEBUG] Cidade global atualizada para: %s (da API)\n", g_cidade);
                fflush(stderr);

                const char* icone = obter_icone_clima(clima.description);
                char js_clima[512];
                snprintf(js_clima, sizeof(js_clima),
                    "document.getElementById('clima-info').innerHTML = "
                    "'%s <b>%s:</b> %.1f°C - %s';"
                    "document.getElementById('cidade-input').value = '';",
                    icone, clima.cidade, clima.temperatura, clima.description);
                webview_eval(w, js_clima);

                // Notifica o JavaScript que o clima foi carregado com sucesso
                webview_eval(w, "if(typeof onClimaAtualizado === 'function') onClimaAtualizado(true, 'Clima carregado');");

                char msg[512];
                snprintf(msg, sizeof(msg),
                    "✅ <b>Contexto atualizado!</b><br>"
                    "📍 Cidade: <b>%s</b><br>"
                    "🌡️ Clima: %.1f°C - %s<br><br>"
                    "💡 Agora quando você perguntar sobre clima, horários ou eventos sem especificar cidade, "
                    "usarei automaticamente <b>%s</b> como referência.",
                    clima.cidade, clima.temperatura, clima.description, clima.cidade);

                char js_code[1024];
                snprintf(js_code, sizeof(js_code),
                    "adicionarMensagemHTML('Sistema', `%s`, false);", msg);
                webview_eval(w, js_code);
            } else {
                // Notifica o JavaScript que houve erro ao carregar o clima
                webview_eval(w, "if(typeof onClimaAtualizado === 'function') onClimaAtualizado(false, 'Cidade não encontrada');");
                webview_eval(w, "document.getElementById('clima-info').innerHTML = '❌ Não foi possível obter dados do clima';");
            }
        }
        webview_return(w, seq, 0, "{}");
    }
    // Método limpar histórico
    else if (method && strcmp(method, "limpar") == 0) {
        fprintf(stderr, "[DEBUG] Limpando histórico\n");
        fflush(stderr);

        // Libera histórico atual e cria novo
        liberar_historico_chat(g_historico);
        g_historico = inicializar_chat_historico();
        // Limpa interface e mostra mensagem inicial
        webview_eval(w, "document.getElementById('chat-messages').innerHTML = '';"
                        "adicionarMensagem('GenieC', 'Olá! Sou o GenieC. Como posso ajudar?', false);");
        webview_return(w, seq, 0, "{}");
    }
    // ===== HANDLERS DO PAINEL DE GRAFOS =====
    // Método para obter estatísticas do grafo
    else if (method && strcmp(method, "grafo_estatisticas") == 0) {
        fprintf(stderr, "[DEBUG] Obtendo estatísticas do grafo\n");
        fflush(stderr);

        // Gera JSON com estatísticas
        char* stats = obter_estatisticas_grafo(g_grafo);

        // Envia estatísticas para JavaScript
        char* js_code = (char*)malloc(strlen(stats) + 256);
        snprintf(js_code, strlen(stats) + 256,
            "if(typeof onEstatisticasGrafo === 'function') onEstatisticasGrafo(%s);", stats);
        webview_eval(w, js_code);

        // Libera memória
        free(js_code);
        free(stats);
        webview_return(w, seq, 0, "{}");
    }
    // Método para calcular rota do grafo
    else if (method && strcmp(method, "grafo_calcular_rota") == 0) {
        fprintf(stderr, "[DEBUG] Calculando rota via painel de grafos\n");
        fflush(stderr);

        // Extrai origem e destino dos parâmetros
        cJSON *first_item = cJSON_GetArrayItem(root, 0);
        cJSON *origem_item = cJSON_GetObjectItemCaseSensitive(first_item, "origem");
        cJSON *destino_item = cJSON_GetObjectItemCaseSensitive(first_item, "destino");

        if (origem_item && destino_item &&
            cJSON_IsString(origem_item) && cJSON_IsString(destino_item)) {

            const char* origem = origem_item->valuestring;
            const char* destino = destino_item->valuestring;

            fprintf(stderr, "[INFO GRAFO] Processando rota via painel: %s -> %s\n", origem, destino);
            fflush(stderr);

            // Mostra mensagem de processamento
            webview_eval(w, "adicionarMensagemHTML('Sistema', "
                "'🔄 <b>Consultando IA para obter distâncias...</b><br>"
                "⏳ Isso pode levar alguns minutos...', false);");

            // Consulta a IA para preencher o grafo
            int conexoes = obter_distancias_ia_e_preencher_grafo(origem, destino, g_grafo);

            if (conexoes > 0) {
                char msg_sucesso[768];
                snprintf(msg_sucesso, sizeof(msg_sucesso),
                    "✅ <b>Malha de rotas criada!</b><br>"
                    "🏙️ <b>%d cidades</b> mapeadas<br>"
                    "🛣️ <b>%d conexões</b> adicionadas pela IA<br>"
                    "🔍 Calculando menor caminho com Dijkstra...<br><br>",
                    g_grafo->num_cidades, conexoes);

                char* js_code = (char*)malloc(2048);
                snprintf(js_code, 2048,
                    "adicionarMensagemHTML('Sistema', `%s`, false);", msg_sucesso);
                webview_eval(w, js_code);
                free(js_code);

                // Salva o grafo atualizado
                salvar_coordenadas_grafo(g_grafo, "coordenadas_grafo.txt");

                // Calcula o menor caminho usando Dijkstra COM MAPA
                char* resultado = calcular_menor_caminho_com_mapa(g_grafo, origem, destino);

                size_t resultado_size = strlen(resultado) + 256;
                js_code = (char*)malloc(resultado_size);
                snprintf(js_code, resultado_size,
                    "adicionarMensagemHTML('GenieC', `%s`, false);", resultado);
                webview_eval(w, js_code);

                free(js_code);
                free(resultado);

                // Atualiza estatísticas no painel (se estiver aberto)
                char* stats = obter_estatisticas_grafo(g_grafo);
                js_code = (char*)malloc(strlen(stats) + 256);
                snprintf(js_code, strlen(stats) + 256,
                    "if(typeof onEstatisticasGrafo === 'function') onEstatisticasGrafo(%s);", stats);
                webview_eval(w, js_code);
                free(js_code);
                free(stats);
            } else {
                webview_eval(w, "adicionarMensagemHTML('Sistema', "
                    "'❌ Não foi possível obter distâncias da IA.<br>"
                    "Verifique se as cidades são válidas.', false);");
            }
        } else {
            webview_eval(w, "adicionarMensagemHTML('Sistema', "
                "'❌ Parâmetros inválidos. Informe origem e destino.', false);");
        }

        webview_return(w, seq, 0, "{}");
    }
    else if (method && strcmp(method, "grafo_visualizar_mapa") == 0) {
        fprintf(stderr, "[DEBUG] Visualizando mapa do grafo via painel\n");
        fflush(stderr);

        char* resultado = gerar_mapa_grafo(g_grafo);

        size_t js_size = strlen(resultado) + 256;
        char* js_code = (char*)malloc(js_size);
        snprintf(js_code, js_size,
            "adicionarMensagemHTML('Sistema', `%s`, false);", resultado);
        webview_eval(w, js_code);

        free(js_code);
        free(resultado);
        webview_return(w, seq, 0, "{}");
    }
    else if (method && strcmp(method, "grafo_listar_cidades") == 0) {
        fprintf(stderr, "[DEBUG] Listando cidades do grafo via painel\n");
        fflush(stderr);

        char* resultado = listar_cidades_grafo(g_grafo);

        size_t js_size = strlen(resultado) + 256;
        char* js_code = (char*)malloc(js_size);
        snprintf(js_code, js_size,
            "adicionarMensagemHTML('Sistema', `%s`, false);", resultado);
        webview_eval(w, js_code);

        free(js_code);
        free(resultado);
        webview_return(w, seq, 0, "{}");
    }
    else if (method && strcmp(method, "grafo_limpar") == 0) {
        fprintf(stderr, "[DEBUG] Limpando grafo via painel\n");
        fflush(stderr);

        limpar_grafo(g_grafo);

        // Remove o arquivo de coordenadas também
        remove("coordenadas_grafo.txt");

        // Envia estatísticas zeradas para o painel
        webview_eval(w, "if(typeof onEstatisticasGrafo === 'function') onEstatisticasGrafo({cidades: 0, conexoes: 0, listaCidades: []});");

        webview_return(w, seq, 0, "{}");
    }
    else if (method && strcmp(method, "grafo_salvar") == 0) {
        fprintf(stderr, "[DEBUG] Salvando grafo via painel\n");
        fflush(stderr);

        int salvos = salvar_coordenadas_grafo(g_grafo, "coordenadas_grafo.txt");

        char msg[256];
        snprintf(msg, sizeof(msg),
            "💾 Grafo salvo com sucesso!<br>📊 %d cidades salvas.", salvos);

        char js_code[512];
        snprintf(js_code, sizeof(js_code),
            "adicionarMensagemHTML('Sistema', '%s', false);", msg);
        webview_eval(w, js_code);

        webview_return(w, seq, 0, "{}");
    }
    else {
        fprintf(stderr, "[AVISO] Método não reconhecido: %s\n", method ? method : "(null)");
        fflush(stderr);
        webview_return(w, seq, 0, "{}");
    }

    cJSON_Delete(root);
}

int main() {
    // Configura localidade para português brasileiro e UTF-8
    setlocale(LC_ALL, "Portuguese_Brazil.utf8");
    #ifdef _WIN32
    system("chcp 65001 > nul"); // Configura o console Windows para UTF-8
    #endif

    // Carrega variáveis de ambiente
    if (!carregar_env(".env")) {
        fprintf(stderr, "Erro ao carregar .env\n");
        return 1;
    }

    // Inicializa histórico
    g_historico = inicializar_chat_historico();
    g_cidade[0] = '\0'; // Inicia sem cidade - usuário vai definir na tela de boas-vindas

    // Inicializa o grafo
    g_grafo = criar_grafo();
    if (!g_grafo) {
        fprintf(stderr, "Erro ao criar grafo\n");
        liberar_historico_chat(g_historico);
        limpar_env();
        return 1;
    }

    // Carrega coordenadas salvas anteriormente (se existir)
    // IMPORTANTE: Isso carrega as coordenadas das cidades que já foram usadas antes
    int coords_carregadas = carregar_coordenadas_grafo(g_grafo, "coordenadas_grafo.txt");
    if (coords_carregadas > 0) {
        fprintf(stderr, "[INFO] %d coordenadas carregadas do cache ao iniciar\n", coords_carregadas);
    }

    // Cria a janela
    webview_t w = webview_create(0, NULL);
    webview_set_title(w, "GenieC - Assistente Inteligente");
    webview_set_size(w, 1000, 700, WEBVIEW_HINT_NONE);

#ifdef _WIN32
    // Obter o handle da janela nativa
    HWND hwnd = (HWND)webview_get_window(w);

    // Carregar e definir o ícone
    HICON hIcon = LoadIcon(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_ICON1));
    if (hIcon) {
        SendMessage(hwnd, WM_SETICON, ICON_SMALL, (LPARAM)hIcon);
        SendMessage(hwnd, WM_SETICON, ICON_BIG, (LPARAM)hIcon);
    } else {
        fprintf(stderr, "[AVISO] Não foi possível carregar o ícone do recurso\n");
    }
#endif

    // **IMPORTANTE**: Registra callback ANTES de carregar o HTML
    fprintf(stderr, "[INFO] Registrando callback RPC...\n");
    fflush(stderr);
    webview_bind(w, "rpc", handle_rpc, w);

    // Carrega e define o HTML da interface
    fprintf(stderr, "[INFO] Carregando HTML da interface...\n");
    fflush(stderr);
    carregar_html_interface(w);

    // Mensagem inicial com mais detalhes
    webview_eval(w,
        "adicionarMensagemHTML('GenieC', "
        "'Olá! Sou o <b>GenieC</b>, seu assistente inteligente. 🤖<br><br>"
        "💡 <b>Dicas rápidas:</b><br>"
        "• Faça perguntas naturalmente<br>"
        "• Digite <b>ajuda</b> para ver todos os comandos<br>"
        "• Digite <b>historico</b> para revisar a conversa<br><br>"
        "Como posso ajudar você hoje?', false);");

    fprintf(stderr, "[INFO] Iniciando loop da janela...\n");
    fflush(stderr);

    // Roda a interface
    webview_run(w);

    // Cleanup
    webview_destroy(w);
    liberar_historico_chat(g_historico);
    liberar_grafo(g_grafo);
    limpar_env();

    return 0;
}