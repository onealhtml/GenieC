#include "grafo.h"
#include "gemini.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

#ifdef _WIN32
#include <windows.h>
#include <time.h>
#else
#include <sys/time.h>
#endif

// Função auxiliar para obter tempo em microsegundos (alta precisão)
static double obter_tempo_ms() {
#ifdef _WIN32
    LARGE_INTEGER frequency;
    LARGE_INTEGER start;
    QueryPerformanceFrequency(&frequency);
    QueryPerformanceCounter(&start);
    return (double)start.QuadPart / (double)frequency.QuadPart * 1000.0;
#else
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return (double)(tv.tv_sec * 1000.0 + tv.tv_usec / 1000.0);
#endif
}
//
Grafo* criar_grafo() {
    Grafo* g = (Grafo*)malloc(sizeof(Grafo));
    if (!g) return NULL;

    g->num_cidades = 0;

    // Inicializa todas as adjacências como -1 (sem conexão)
    for (int i = 0; i < MAX_CIDADES; i++) {
        g->cidades[i].nome[0] = '\0';
        g->cidades[i].latitude = 0.0;
        g->cidades[i].longitude = 0.0;
        g->cidades[i].coords_validas = 0;
        for (int j = 0; j < MAX_CIDADES; j++) {
            g->cidades[i].adjacencias[j] = -1;
        }
    }
    return g;
}

int encontrar_cidade(Grafo* g, const char* nome) {
    if (!g || !nome) return -1;

    // Busca cidade pelo nome (case-insensitive)
    for (int i = 0; i < g->num_cidades; i++) {
        if (strcasecmp(g->cidades[i].nome, nome) == 0) {
            return i;
        }
    }
    return -1;
}

void adicionar_cidade(Grafo* g, const char* nome) {
    if (!g || !nome || g->num_cidades >= MAX_CIDADES) return;

    // Verifica se já existe
    if (encontrar_cidade(g, nome) != -1) return;

    strncpy(g->cidades[g->num_cidades].nome, nome, MAX_NOME_CIDADE - 1);
    g->cidades[g->num_cidades].nome[MAX_NOME_CIDADE - 1] = '\0';

    // Remove espaços extras no início e fim
    char* start = g->cidades[g->num_cidades].nome;
    while (*start == ' ') start++;
    if (start != g->cidades[g->num_cidades].nome) {
        memmove(g->cidades[g->num_cidades].nome, start, strlen(start) + 1);
    }

    char* end = g->cidades[g->num_cidades].nome + strlen(g->cidades[g->num_cidades].nome) - 1;
    while (end > g->cidades[g->num_cidades].nome && *end == ' ') {
        *end = '\0';
        end--;
    }

    g->num_cidades++;
}

// Adiciona aresta (conexão) entre duas cidades
void adicionar_aresta(Grafo* g, const char* cidade1, const char* cidade2, int distancia) {
    // Valida parâmetros
    if (!g || !cidade1 || !cidade2 || distancia <= 0) return;

    // Busca índices das cidades
    int idx1 = encontrar_cidade(g, cidade1);
    int idx2 = encontrar_cidade(g, cidade2);

    // Adiciona cidades se não existirem
    if (idx1 == -1) {
        adicionar_cidade(g, cidade1);
        idx1 = g->num_cidades - 1;
    }
    if (idx2 == -1) {
        adicionar_cidade(g, cidade2);
        idx2 = g->num_cidades - 1;
    }

    // Grafo não-direcionado (bidirecional)
    g->cidades[idx1].adjacencias[idx2] = distancia;
    g->cidades[idx2].adjacencias[idx1] = distancia;
}

// Algoritmo de Dijkstra para encontrar o menor caminho
char* calcular_menor_caminho(Grafo* g, const char* origem, const char* destino) {
    time_t inicio, fim;

    if (!g || !origem || !destino) {
        return strdup("❌ Erro: parâmetros inválidos");
    }

    int idx_origem = encontrar_cidade(g, origem);
    int idx_destino = encontrar_cidade(g, destino);

    if (idx_origem == -1 || idx_destino == -1) {
        char* msg = (char*)malloc(512);
        snprintf(msg, 512,
            "❌ <b>Cidade não encontrada no grafo</b><br><br>"
            "Cidades disponíveis no grafo: use o comando <b>grafocidades</b>");
        return msg;
    }

    // Arrays para o algoritmo de Dijkstra
    int dist[MAX_CIDADES];
    int anterior[MAX_CIDADES];
    int visitado[MAX_CIDADES] = {0};

    // Inicializa distâncias
    for (int i = 0; i < g->num_cidades; i++) {
        dist[i] = INT_MAX;
        anterior[i] = -1;
    }
    dist[idx_origem] = 0;

    // Inicia medição de tempo do Dijkstra (Matriz de Adjacência) - Alta Precisão
    // double tempo_inicio = obter_tempo_ms();
    time(&inicio);

    // Algoritmo de Dijkstra
    for (int count = 0; count < g->num_cidades - 1; count++) {
        int min_dist = INT_MAX;
        int u = -1;

        // Encontra o vértice não visitado com menor distância
        for (int v = 0; v < g->num_cidades; v++) {
            if (!visitado[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }

        if (u == -1) break;
        visitado[u] = 1;

        // Atualiza distâncias dos vizinhos
        for (int v = 0; v < g->num_cidades; v++) {
            if (!visitado[v] && g->cidades[u].adjacencias[v] != -1) {
                int nova_dist = dist[u] + g->cidades[u].adjacencias[v];
                if (nova_dist < dist[v]) {
                    dist[v] = nova_dist;
                    anterior[v] = u;
                }
            }
        }
    }

    // Finaliza medição de tempo do Dijkstra
    //double tempo_fim = obter_tempo_ms();

    time(&fim);

    // Calcula tempo de execução
    long double tempo_execucao = difftime(fim,inicio);
    fprintf(stderr, "[PERFORMANCE] Dijkstra (Matriz de Adjacência) - Tempo: %.6Lf ms | Cidades: %d\n",
            tempo_execucao, g->num_cidades);

    // Verifica se encontrou caminho
    if (dist[idx_destino] == INT_MAX) {
        return strdup("❌ <b>Não há caminho entre as cidades</b><br>"
                     "As cidades não estão conectadas no grafo.");
    }

    // Reconstrói o caminho
    int path[MAX_CIDADES];
    int path_size = 0;

    // Percorre do destino até a origem
    for (int v = idx_destino; v != -1; v = anterior[v]) {
        path[path_size++] = v;
    }

    // Monta a string do caminho com detalhes de cada trecho
    char caminho_visual[2048] = "";
    char detalhes_trechos[2048] = "";

    // Percorre caminho de trás para frente (origem -> destino)
    for (int i = path_size - 1; i >= 0; i--) {
        strcat(caminho_visual, g->cidades[path[i]].nome);

        if (i > 0) {
            strcat(caminho_visual, " → ");

            // Adiciona detalhes do trecho
            int cidade_atual = path[i];
            int proxima_cidade = path[i-1];
            int dist_trecho = g->cidades[cidade_atual].adjacencias[proxima_cidade];

            char linha_trecho[256];
            snprintf(linha_trecho, sizeof(linha_trecho),
                "  • %s → %s: <b>%d km</b><br>",
                g->cidades[cidade_atual].nome,
                g->cidades[proxima_cidade].nome,
                dist_trecho);
            strcat(detalhes_trechos, linha_trecho);
        }
    }

    // Monta o resultado formatado
    char* resultado = (char*)malloc(6144);
    snprintf(resultado, 6144,
        "🗺️ <b>Menor Caminho Encontrado (Dijkstra):</b><br><br>"
        "📍 <b>Origem:</b> %s<br>"
        "🎯 <b>Destino:</b> %s<br>"
        "🏙️ <b>Cidades no percurso:</b> %d<br><br>"
        "🛣️ <b>Rota Visual:</b><br>"
        "<div style='background: #f5f5f5; padding: 10px; border-radius: 5px; margin: 10px 0;'>"
        "%s"
        "</div><br>"
        "📊 <b>Detalhes dos Trechos:</b><br>"
        "<div style='background: #fff3cd; padding: 10px; border-radius: 5px; margin: 10px 0;'>"
        "%s"
        "</div>"
        "📏 <b>Distância Total:</b> <span style='color: #4CAF50; font-size: 1.3em;'><b>%d km</b></span><br><br>"
        "💡 <i>Calculado usando o algoritmo de Dijkstra (menor caminho garantido)</i>",
        g->cidades[idx_origem].nome,
        g->cidades[idx_destino].nome,
        path_size,
        caminho_visual,
        detalhes_trechos,
        dist[idx_destino]);

    return resultado;
}

char* listar_cidades_grafo(Grafo* g) {
    if (!g || g->num_cidades == 0) {
        return strdup("📭 <b>Grafo vazio</b><br>"
                     "Nenhuma cidade foi adicionada ainda.<br><br>"
                     "Use: <b>grafo Cidade1-Cidade2</b> para começar!");
    }

    char* resultado = (char*)malloc(16384);

    // Conta o total de conexões
    int total_conexoes = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        for (int j = i + 1; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                total_conexoes++;
            }
        }
    }

    snprintf(resultado, 16384,
        "🗺️ <b>Malha de Rotas (Grafo):</b><br><br>"
        "📊 <b>Estatísticas:</b><br>"
        "🏙️ Cidades: <b>%d</b><br>"
        "🛣️ Conexões: <b>%d</b><br><br>"
        "<div style='background: #e3f2fd; padding: 15px; border-radius: 8px; border-left: 4px solid #2196F3;'>"
        "<b>🏙️ Cidades e suas Conexões:</b><br><br>",
        g->num_cidades, total_conexoes);

    for (int i = 0; i < g->num_cidades; i++) {
        char linha[2048];

        // Conta conexões desta cidade
        int num_conexoes = 0;
        for (int j = 0; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                num_conexoes++;
            }
        }

        snprintf(linha, sizeof(linha),
            "<b>%d. %s</b> <span style='color: #666;'>(%d conexões)</span><br>",
            i + 1, g->cidades[i].nome, num_conexoes);
        strcat(resultado, linha);

        // Lista conexões com formatação melhor
        if (num_conexoes > 0) {
            strcat(resultado, "<div style='margin-left: 20px; color: #555;'>");
            int conexoes_listadas = 0;
            for (int j = 0; j < g->num_cidades; j++) {
                if (g->cidades[i].adjacencias[j] != -1) {
                    char temp[256];
                    snprintf(temp, sizeof(temp),
                        "  → %s <span style='color: #4CAF50;'><b>%d km</b></span><br>",
                        g->cidades[j].nome, g->cidades[i].adjacencias[j]);
                    strcat(resultado, temp);
                    conexoes_listadas++;
                }
            }
            strcat(resultado, "</div>");
        }
        strcat(resultado, "<br>");
    }

    strcat(resultado,
        "</div><br>"
        "💡 <b>Dica:</b> Use <b>grafo origem-destino</b> para calcular o menor caminho<br>"
        "📖 <b>Exemplo:</b> grafo São Paulo-Rio de Janeiro");

    return resultado;
}

// Gera um mapa interativo com OpenStreetMap/Leaflet
char* gerar_mapa_grafo(Grafo* g) {
    if (!g || g->num_cidades == 0) {
        return strdup("📭 <b>Grafo vazio</b><br>Adicione cidades primeiro!");
    }

    fprintf(stderr, "[DEBUG MAPA] Gerando mapa do grafo com %d cidades\n", g->num_cidades);

    // Gera HTML com mapa Leaflet
    char* resultado = (char*)malloc(32768);

    // Usa um ID único baseado em timestamp
    static int mapa_counter = 0;
    mapa_counter++;

    // Conta estatísticas do grafo
    int total_conexoes = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        for (int j = i + 1; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                total_conexoes++;
            }
        }
    }

    snprintf(resultado, 32768,
        "🗺️ <b>Mapa Interativo das Rotas</b><br><br>"
        "📊 <b>Estatísticas da Malha Rodoviária:</b><br>"
        "🏙️ <b>Total de Cidades:</b> %d<br>"
        "🛣️ <b>Total de Conexões:</b> %d<br>"
        "📍 <b>Visualização:</b> Todas as rotas e conexões entre cidades<br><br>"
        "🗺️ <b>Mapa Completo da Rede:</b><br>"
        "<div id='mapa-container-%d' style='width: 100%%; height: 500px; border: 2px solid #2196F3; border-radius: 8px; margin: 10px 0;'></div>"
        "<script>"
        "(function() {"
        "  console.log('Inicializando mapa do grafo...');"
        "  if (typeof L === 'undefined') {"
        "    console.error('ERRO: Leaflet não está carregado!');"
        "    document.getElementById('mapa-container-%d').innerHTML = '<div style=\"padding: 20px; color: red; text-align: center;\">❌ Erro: Biblioteca de mapas não carregada. Recarregue a página.</div>';"
        "    return;"
        "  }"
        "  try {"
        "    if (typeof window.mapaGrafo_%d !== 'undefined') {"
        "      window.mapaGrafo_%d.remove();"
        "    }"
        "    window.mapaGrafo_%d = L.map('mapa-container-%d').setView([-15.7939, -47.8828], 4);"
        "    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {"
        "      attribution: '© OpenStreetMap',"
        "      maxZoom: 18"
        "    }).addTo(window.mapaGrafo_%d);"
        "    console.log('Mapa do grafo criado com sucesso!');"
        "    var allMarkers = [];"
        , g->num_cidades, total_conexoes, mapa_counter, mapa_counter, mapa_counter, mapa_counter, mapa_counter, mapa_counter, mapa_counter);

    // OTIMIZAÇÃO: Obtém todas as coordenadas em UMA ÚNICA requisição
    // Nota: Coordenadas já foram carregadas no início do programa
    fprintf(stderr, "[DEBUG MAPA] Verificando coordenadas para %d cidades\n", g->num_cidades);

    // Identifica quais cidades precisam de coordenadas
    char cidades_sem_coords[MAX_CIDADES][100];
    int indices_sem_coords[MAX_CIDADES];
    int num_sem_coords = 0;

    for (int i = 0; i < g->num_cidades; i++) {
        if (!g->cidades[i].coords_validas) {
            strncpy(cidades_sem_coords[num_sem_coords], g->cidades[i].nome, 99);
            cidades_sem_coords[num_sem_coords][99] = '\0';
            indices_sem_coords[num_sem_coords] = i;
            num_sem_coords++;
        }
    }

    // Se há cidades sem coordenadas, busca TODAS de uma vez
    if (num_sem_coords > 0) {
        fprintf(stderr, "[DEBUG MAPA] Buscando coordenadas de %d cidades em LOTE\n", num_sem_coords);

        double latitudes[MAX_CIDADES];
        double longitudes[MAX_CIDADES];

        int encontradas = obter_coordenadas_multiplas(cidades_sem_coords, num_sem_coords,
                                                       latitudes, longitudes);

        if (encontradas > 0) {
            // Atualiza coordenadas no grafo
            for (int i = 0; i < num_sem_coords; i++) {
                if (latitudes[i] != 0.0 || longitudes[i] != 0.0) {
                    int idx = indices_sem_coords[i];
                    g->cidades[idx].latitude = latitudes[i];
                    g->cidades[idx].longitude = longitudes[i];
                    g->cidades[idx].coords_validas = 1;
                }
            }
            fprintf(stderr, "[DEBUG MAPA] ✓ Coordenadas de %d cidades obtidas com sucesso!\n", encontradas);

            // Salva as novas coordenadas no arquivo
            salvar_coordenadas_grafo(g, "coordenadas_grafo.txt");
        }
    } else {
        fprintf(stderr, "[DEBUG MAPA] Todas as cidades já têm coordenadas em cache\n");
    }

    // Adiciona marcadores para cada cidade
    for (int i = 0; i < g->num_cidades; i++) {
        if (g->cidades[i].coords_validas) {
            char marker_code[1024];

            // Conta conexões
            int num_conexoes = 0;
            for (int j = 0; j < g->num_cidades; j++) {
                if (g->cidades[i].adjacencias[j] != -1) num_conexoes++;
            }

            snprintf(marker_code, sizeof(marker_code),
                "    var marker_%d = L.marker([%.4f, %.4f]).addTo(window.mapaGrafo_%d)"
                ".bindPopup('<b>%s</b><br>%d conexões');"
                "    allMarkers.push(marker_%d);"
                , i, g->cidades[i].latitude, g->cidades[i].longitude, mapa_counter,
                g->cidades[i].nome, num_conexoes, i);
            strcat(resultado, marker_code);
        }
    }

    // Adiciona linhas conectando as cidades (usando coordenadas dinâmicas)
    for (int i = 0; i < g->num_cidades; i++) {
        if (g->cidades[i].coords_validas) {
            for (int j = i + 1; j < g->num_cidades; j++) {
                if (g->cidades[i].adjacencias[j] != -1 && g->cidades[j].coords_validas) {
                    char line_code[512];
                    snprintf(line_code, sizeof(line_code),
                        "    L.polyline([[%.4f,%.4f],[%.4f,%.4f]], {color: '#2196F3', weight: 2, opacity: 0.7})"
                        ".addTo(window.mapaGrafo_%d).bindPopup('%s ↔ %s: %d km');"
                        , g->cidades[i].latitude, g->cidades[i].longitude,
                        g->cidades[j].latitude, g->cidades[j].longitude,
                        mapa_counter,
                        g->cidades[i].nome, g->cidades[j].nome,
                        g->cidades[i].adjacencias[j]);
                    strcat(resultado, line_code);
                }
            }
        }
    }

    strcat(resultado, "    if (allMarkers.length > 0) {");
    strcat(resultado, "      var group = new L.featureGroup(allMarkers);");
    strcat(resultado, "      window.mapaGrafo_" );
    char temp_counter[32];
    snprintf(temp_counter, sizeof(temp_counter), "%d", mapa_counter);
    strcat(resultado, temp_counter);
    strcat(resultado, ".fitBounds(group.getBounds().pad(0.1));");
    strcat(resultado, "    }");
    strcat(resultado, "  } catch(e) { console.error('Erro ao criar mapa:', e); }");
    strcat(resultado, "})();");
    strcat(resultado, "</script>");
    strcat(resultado, "<br>🗺️ <i>Mapa interativo com OpenStreetMap</i><br>");
    strcat(resultado, "💡 Clique nos marcadores para ver detalhes");

    fprintf(stderr, "[DEBUG MAPA] Mapa do grafo gerado com sucesso\n");

    return resultado;
}

// Calcula menor caminho e mostra no mapa
char* calcular_menor_caminho_com_mapa(Grafo* g, const char* origem, const char* destino) {
    if (!g || !origem || !destino) {
        return strdup("❌ Erro: parâmetros inválidos");
    }

    int idx_origem = encontrar_cidade(g, origem);
    int idx_destino = encontrar_cidade(g, destino);

    if (idx_origem == -1 || idx_destino == -1) {
        return strdup("❌ <b>Cidade não encontrada no grafo</b>");
    }

    // Primeiro calcula o caminho (Dijkstra)
    int dist[MAX_CIDADES];
    int anterior[MAX_CIDADES];
    int visitado[MAX_CIDADES] = {0};

    for (int i = 0; i < g->num_cidades; i++) {
        dist[i] = INT_MAX;
        anterior[i] = -1;
    }
    dist[idx_origem] = 0;

    // Inicia medição de tempo do Dijkstra (Matriz de Adjacência) - Alta Precisão
    double tempo_inicio = obter_tempo_ms();

    // Dijkstra
    for (int count = 0; count < g->num_cidades - 1; count++) {
        int min_dist = INT_MAX;
        int u = -1;

        for (int v = 0; v < g->num_cidades; v++) {
            if (!visitado[v] && dist[v] < min_dist) {
                min_dist = dist[v];
                u = v;
            }
        }

        if (u == -1) break;
        visitado[u] = 1;

        for (int v = 0; v < g->num_cidades; v++) {
            if (!visitado[v] && g->cidades[u].adjacencias[v] != -1) {
                int nova_dist = dist[u] + g->cidades[u].adjacencias[v];
                if (nova_dist < dist[v]) {
                    dist[v] = nova_dist;
                    anterior[v] = u;
                }
            }
        }
    }

    // Finaliza medição de tempo do Dijkstra
    double tempo_fim = obter_tempo_ms();
    double tempo_execucao = tempo_fim - tempo_inicio; // em milissegundos

    fprintf(stderr, "[PERFORMANCE] Dijkstra com Mapa (Matriz de Adjacência) - Tempo: %.6f ms | Cidades: %d\n",
            tempo_execucao, g->num_cidades);

    if (dist[idx_destino] == INT_MAX) {
        return strdup("❌ <b>Não há caminho entre as cidades</b>");
    }

    // Reconstrói o caminho
    int path[MAX_CIDADES];
    int path_size = 0;
    for (int v = idx_destino; v != -1; v = anterior[v]) {
        path[path_size++] = v;
    }

    // OTIMIZAÇÃO: Obtém coordenadas das cidades da rota em UMA ÚNICA requisição
    // Nota: Coordenadas já foram carregadas no início do programa
    char cidades_sem_coords[MAX_CIDADES][100];
    int indices_sem_coords[MAX_CIDADES];
    int num_sem_coords = 0;

    for (int i = 0; i < path_size; i++) {
        int idx = path[i];
        if (!g->cidades[idx].coords_validas) {
            strncpy(cidades_sem_coords[num_sem_coords], g->cidades[idx].nome, 99);
            cidades_sem_coords[num_sem_coords][99] = '\0';
            indices_sem_coords[num_sem_coords] = idx;
            num_sem_coords++;
        }
    }

    if (num_sem_coords > 0) {
        fprintf(stderr, "[DEBUG MAPA ROTA] Buscando coordenadas de %d cidades em LOTE\n", num_sem_coords);

        double latitudes[MAX_CIDADES];
        double longitudes[MAX_CIDADES];

        int encontradas = obter_coordenadas_multiplas(cidades_sem_coords, num_sem_coords,
                                                       latitudes, longitudes);

        if (encontradas > 0) {
            for (int i = 0; i < num_sem_coords; i++) {
                if (latitudes[i] != 0.0 || longitudes[i] != 0.0) {
                    int idx = indices_sem_coords[i];
                    g->cidades[idx].latitude = latitudes[i];
                    g->cidades[idx].longitude = longitudes[i];
                    g->cidades[idx].coords_validas = 1;
                }
            }
            fprintf(stderr, "[DEBUG MAPA ROTA] ✓ Coordenadas obtidas com sucesso!\n");

            // Salva as novas coordenadas no arquivo
            salvar_coordenadas_grafo(g, "coordenadas_grafo.txt");
        }
    }

    // Monta resultado com mapa
    char* resultado = (char*)malloc(32768);

    // Calcula centro do mapa (usa coordenadas da origem ou Brasil central)
    char centro_lat[32] = "-15.7939";
    char centro_lng[32] = "-47.8828";

    if (g->cidades[idx_origem].coords_validas) {
        snprintf(centro_lat, sizeof(centro_lat), "%.4f", g->cidades[idx_origem].latitude);
        snprintf(centro_lng, sizeof(centro_lng), "%.4f", g->cidades[idx_origem].longitude);
    }

    // Monta caminho visual
    char caminho_visual[2048] = "";
    char detalhes_trechos[2048] = "";

    for (int i = path_size - 1; i >= 0; i--) {
        strcat(caminho_visual, g->cidades[path[i]].nome);
        if (i > 0) {
            strcat(caminho_visual, " → ");
            int cidade_atual = path[i];
            int proxima_cidade = path[i-1];
            int dist_trecho = g->cidades[cidade_atual].adjacencias[proxima_cidade];

            char linha_trecho[256];
            snprintf(linha_trecho, sizeof(linha_trecho),
                "  • %s → %s: <b>%d km</b><br>",
                g->cidades[cidade_atual].nome,
                g->cidades[proxima_cidade].nome,
                dist_trecho);
            strcat(detalhes_trechos, linha_trecho);
        }
    }

    // ID único para o mapa
    static int rota_counter = 0;
    rota_counter++;

    fprintf(stderr, "[DEBUG MAPA] Gerando mapa de rota de %s para %s\n", origem, destino);

    snprintf(resultado, 32768,
        "🗺️ <b>Menor Caminho Encontrado (Dijkstra):</b><br><br>"
        "📍 <b>Origem:</b> %s<br>"
        "🎯 <b>Destino:</b> %s<br>"
        "🏙️ <b>Cidades no percurso:</b> %d<br>"
        "📏 <b>Distância Total:</b> <span style='color: #4CAF50; font-size: 1.3em;'><b>%d km</b></span><br><br>"

        "🗺️ <b>Mapa da Rota:</b><br>"
        "<div id='mapa-rota-%d' style='width: 100%%; height: 500px; border: 2px solid #4CAF50; border-radius: 8px; margin: 10px 0;'></div>"
        "<script>"
        "(function() {"
        "  console.log('Inicializando mapa de rota...');"
        "  if (typeof L === 'undefined') {"
        "    console.error('ERRO: Leaflet não está carregado!');"
        "    document.getElementById('mapa-rota-%d').innerHTML = '<div style=\"padding: 20px; color: red; text-align: center;\">❌ Erro: Biblioteca de mapas não carregada. Recarregue a página.</div>';"
        "    return;"
        "  }"
        "  try {"
        "    if (typeof window.mapaRota_%d !== 'undefined') { window.mapaRota_%d.remove(); }"
        "    window.mapaRota_%d = L.map('mapa-rota-%d').setView([%s, %s], 6);"
        "    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {"
        "      attribution: '© OpenStreetMap',"
        "      maxZoom: 18"
        "    }).addTo(window.mapaRota_%d);"
        "    console.log('Mapa de rota criado com sucesso!');"
        , origem, destino, path_size, dist[idx_destino], rota_counter, rota_counter,
          rota_counter, rota_counter, rota_counter, rota_counter, centro_lat, centro_lng, rota_counter);

    // Adiciona marcadores e linha da rota (usando coordenadas dinâmicas)
    char polyline_points[4096] = "[";

    for (int i = path_size - 1; i >= 0; i--) {
        int idx = path[i];

        if (g->cidades[idx].coords_validas) {
            // Adiciona ao polyline
            char point[128];
            const char* separator;
            if (i < path_size - 1) {
                separator = ",";
            } else {
                separator = "";
            }
            snprintf(point, sizeof(point), "%s[%.4f,%.4f]",
                separator,
                g->cidades[idx].latitude,
                g->cidades[idx].longitude);
            strcat(polyline_points, point);

            // Adiciona marcador
            char marker[512];
            const char* label;
            if (i == path_size - 1) {
                label = "🚩 Origem";
            } else if (i == 0) {
                label = "🎯 Destino";
            } else {
                label = "📍";
            }

            snprintf(marker, sizeof(marker),
                "    L.marker([%.4f,%.4f]).addTo(window.mapaRota_%d).bindPopup('<b>%s</b><br>%s');"
                , g->cidades[idx].latitude, g->cidades[idx].longitude,
                rota_counter, g->cidades[idx].nome, label);
            strcat(resultado, marker);
        }
    }

    strcat(polyline_points, "]");

    // Adiciona linha da rota
    char route_line[512];
    snprintf(route_line, sizeof(route_line),
        "    var routeLine = L.polyline(%s, {color: '#4CAF50', weight: 4, opacity: 0.8})"
        ".addTo(window.mapaRota_%d);"
        "    window.mapaRota_%d.fitBounds(routeLine.getBounds());"
        , polyline_points, rota_counter, rota_counter);
    strcat(resultado, route_line);

    strcat(resultado, "  } catch(e) { console.error('Erro ao criar mapa de rota:', e); }");
    strcat(resultado, "})();");
    strcat(resultado, "</script><br>");

    fprintf(stderr, "[DEBUG MAPA] Mapa de rota gerado com sucesso\n");

    // Adiciona detalhes textuais
    char detalhes[2048];
    snprintf(detalhes, sizeof(detalhes),
        "🛣️ <b>Rota Visual:</b><br>"
        "<div style='background: #f5f5f5; padding: 10px; border-radius: 5px; margin: 10px 0;'>"
        "%s</div><br>"
        "📊 <b>Detalhes dos Trechos:</b><br>"
        "<div style='background: #fff3cd; padding: 10px; border-radius: 5px; margin: 10px 0;'>"
        "%s</div>"
        "💡 <i>Calculado com Dijkstra + Visualizado no OpenStreetMap</i>"
        , caminho_visual, detalhes_trechos);
    strcat(resultado, detalhes);

    return resultado;
}

void liberar_grafo(Grafo* g) {
    if (g) {
        free(g);
    }
}

// Limpa o grafo sem liberar a memória (reseta para estado inicial)
void limpar_grafo(Grafo* g) {
    if (!g) return;

    g->num_cidades = 0;

    // Reinicializa todas as cidades
    for (int i = 0; i < MAX_CIDADES; i++) {
        g->cidades[i].nome[0] = '\0';
        g->cidades[i].latitude = 0.0;
        g->cidades[i].longitude = 0.0;
        g->cidades[i].coords_validas = 0;
        for (int j = 0; j < MAX_CIDADES; j++) {
            g->cidades[i].adjacencias[j] = -1;
        }
    }

    fprintf(stderr, "[INFO GRAFO] Grafo limpo com sucesso\n");
}

// Retorna estatísticas do grafo em formato HTML
char* obter_estatisticas_grafo(Grafo* g) {
    if (!g) {
        return strdup("{\"cidades\": 0, \"conexoes\": 0}");
    }

    // Conta conexões
    int total_conexoes = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        for (int j = i + 1; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                total_conexoes++;
            }
        }
    }

    // Monta JSON com estatísticas
    char* resultado = (char*)malloc(4096);
    snprintf(resultado, 4096,
        "{\"cidades\": %d, \"conexoes\": %d, \"listaCidades\": [",
        g->num_cidades, total_conexoes);

    for (int i = 0; i < g->num_cidades; i++) {
        char cidade_json[256];
        int num_conexoes = 0;
        for (int j = 0; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) num_conexoes++;
        }

        snprintf(cidade_json, sizeof(cidade_json),
            "%s{\"nome\": \"%s\", \"conexoes\": %d}",
            i > 0 ? ", " : "",
            g->cidades[i].nome,
            num_conexoes);
        strcat(resultado, cidade_json);
    }

    strcat(resultado, "]}");
    return resultado;
}

// Salva as coordenadas e conexões do grafo em arquivo
int salvar_coordenadas_grafo(Grafo* g, const char* arquivo) {
    if (!g || !arquivo) return 0;

    FILE* f = fopen(arquivo, "w");
    if (!f) {
        fprintf(stderr, "[ERRO COORDS] Não foi possível abrir arquivo para salvar: %s\n", arquivo);
        return 0;
    }

    // Conta conexões
    int total_conexoes = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        for (int j = i + 1; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                total_conexoes++;
            }
        }
    }

    fprintf(f, "# Grafo GenieC - Coordenadas e Conexões\n");
    fprintf(f, "# Formato Cidade: CIDADE|LATITUDE|LONGITUDE\n");
    fprintf(f, "# Formato Conexão: CONEXAO|CIDADE1|CIDADE2|DISTANCIA_KM\n");
    fprintf(f, "# Total de cidades: %d\n", g->num_cidades);
    fprintf(f, "# Total de conexões: %d\n\n", total_conexoes);

    // Salva coordenadas das cidades
    fprintf(f, "# === CIDADES ===\n");
    int salvos = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        if (g->cidades[i].coords_validas) {
            fprintf(f, "%s|%.6f|%.6f\n",
                g->cidades[i].nome,
                g->cidades[i].latitude,
                g->cidades[i].longitude);
            salvos++;
        } else {
            // Salva cidade sem coordenadas (lat/lng = 0)
            fprintf(f, "%s|0.000000|0.000000\n", g->cidades[i].nome);
            salvos++;
        }
    }

    // Salva conexões (arestas)
    fprintf(f, "\n# === CONEXÕES ===\n");
    int conexoes_salvas = 0;
    for (int i = 0; i < g->num_cidades; i++) {
        for (int j = i + 1; j < g->num_cidades; j++) {
            if (g->cidades[i].adjacencias[j] != -1) {
                fprintf(f, "CONEXAO|%s|%s|%d\n",
                    g->cidades[i].nome,
                    g->cidades[j].nome,
                    g->cidades[i].adjacencias[j]);
                conexoes_salvas++;
            }
        }
    }

    fclose(f);
    fprintf(stderr, "[INFO GRAFO] Salvo: %d cidades, %d conexões em: %s\n", salvos, conexoes_salvas, arquivo);
    return salvos;
}

// Carrega as coordenadas e conexões do grafo de arquivo
int carregar_coordenadas_grafo(Grafo* g, const char* arquivo) {
    if (!g || !arquivo) return 0;

    FILE* f = fopen(arquivo, "r");
    if (!f) {
        fprintf(stderr, "[INFO GRAFO] Arquivo não encontrado: %s (será criado ao salvar)\n", arquivo);
        return 0;
    }

    char linha[512];
    int cidades_carregadas = 0;
    int conexoes_carregadas = 0;

    while (fgets(linha, sizeof(linha), f)) {
        // Ignora comentários e linhas vazias
        if (linha[0] == '#' || linha[0] == '\n' || linha[0] == '\r') continue;

        // Remove quebra de linha
        linha[strcspn(linha, "\r\n")] = 0;

        // Verifica se é uma linha de CONEXÃO
        if (strncmp(linha, "CONEXAO|", 8) == 0) {
            // Parse: CONEXAO|CIDADE1|CIDADE2|DISTANCIA
            char* ptr = linha + 8; // Pula "CONEXAO|"

            char* pipe1 = strchr(ptr, '|');
            if (!pipe1) continue;

            char* pipe2 = strchr(pipe1 + 1, '|');
            if (!pipe2) continue;

            // Extrai cidade1
            char cidade1[MAX_NOME_CIDADE];
            size_t len1 = pipe1 - ptr;
            if (len1 >= MAX_NOME_CIDADE) len1 = MAX_NOME_CIDADE - 1;
            strncpy(cidade1, ptr, len1);
            cidade1[len1] = '\0';

            // Extrai cidade2
            char cidade2[MAX_NOME_CIDADE];
            size_t len2 = pipe2 - (pipe1 + 1);
            if (len2 >= MAX_NOME_CIDADE) len2 = MAX_NOME_CIDADE - 1;
            strncpy(cidade2, pipe1 + 1, len2);
            cidade2[len2] = '\0';

            // Extrai distância
            int distancia = atoi(pipe2 + 1);

            if (distancia > 0) {
                adicionar_aresta(g, cidade1, cidade2, distancia);
                conexoes_carregadas++;
                fprintf(stderr, "[DEBUG GRAFO] Conexão carregada: %s - %s: %d km\n", cidade1, cidade2, distancia);
            }
        } else {
            // Parse de coordenada: CIDADE|LATITUDE|LONGITUDE
            char nome[MAX_NOME_CIDADE];
            double lat, lng;

            char* pipe1 = strchr(linha, '|');
            if (!pipe1) continue;

            char* pipe2 = strchr(pipe1 + 1, '|');
            if (!pipe2) continue;

            // Extrai nome da cidade
            size_t len = pipe1 - linha;
            if (len >= MAX_NOME_CIDADE) len = MAX_NOME_CIDADE - 1;
            strncpy(nome, linha, len);
            nome[len] = '\0';

            // Extrai coordenadas
            lat = atof(pipe1 + 1);
            lng = atof(pipe2 + 1);

            // Procura a cidade no grafo e atualiza coordenadas
            int idx = encontrar_cidade(g, nome);
            if (idx != -1) {
                // Cidade já existe no grafo - apenas atualiza coordenadas
                if (lat != 0.0 || lng != 0.0) {
                    g->cidades[idx].latitude = lat;
                    g->cidades[idx].longitude = lng;
                    g->cidades[idx].coords_validas = 1;
                }
                cidades_carregadas++;
                fprintf(stderr, "[DEBUG GRAFO] Coordenadas atualizadas: %s (%.4f, %.4f)\n", nome, lat, lng);
            } else if (g->num_cidades < MAX_CIDADES) {
                // Cidade não existe - adiciona com coordenadas
                adicionar_cidade(g, nome);
                idx = g->num_cidades - 1;
                if (lat != 0.0 || lng != 0.0) {
                    g->cidades[idx].latitude = lat;
                    g->cidades[idx].longitude = lng;
                    g->cidades[idx].coords_validas = 1;
                }
                cidades_carregadas++;
                fprintf(stderr, "[DEBUG GRAFO] Cidade adicionada: %s (%.4f, %.4f)\n", nome, lat, lng);
            }
        }
    }

    fclose(f);
    fprintf(stderr, "[INFO GRAFO] Carregado: %d cidades, %d conexões de: %s\n",
            cidades_carregadas, conexoes_carregadas, arquivo);
    return cidades_carregadas;
}