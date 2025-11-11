#include "grafo.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>

Grafo* criar_grafo() {
    Grafo* g = (Grafo*)malloc(sizeof(Grafo));
    if (!g) return NULL;

    g->num_cidades = 0;

    // Inicializa todas as adjacências como -1 (sem conexão)
    for (int i = 0; i < MAX_CIDADES; i++) {
        g->cidades[i].nome[0] = '\0';
        for (int j = 0; j < MAX_CIDADES; j++) {
            g->cidades[i].adjacencias[j] = -1;
        }
    }
    return g;
}

int encontrar_cidade(Grafo* g, const char* nome) {
    if (!g || !nome) return -1;

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

void adicionar_aresta(Grafo* g, const char* cidade1, const char* cidade2, int distancia) {
    if (!g || !cidade1 || !cidade2 || distancia <= 0) return;

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

    // Verifica se encontrou caminho
    if (dist[idx_destino] == INT_MAX) {
        return strdup("❌ <b>Não há caminho entre as cidades</b><br>"
                     "As cidades não estão conectadas no grafo.");
    }

    // Reconstrói o caminho
    int path[MAX_CIDADES];
    int path_size = 0;

    for (int v = idx_destino; v != -1; v = anterior[v]) {
        path[path_size++] = v;
    }

    // Monta a string do caminho com detalhes de cada trecho
    char caminho_visual[2048] = "";
    char detalhes_trechos[2048] = "";

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

    // Calcula centro do mapa (média das posições - Brasil central)
    const char* centro_lat = "-15.7939";
    const char* centro_lng = "-47.8828";

    // Usa um ID único baseado em timestamp
    static int mapa_counter = 0;
    mapa_counter++;

    snprintf(resultado, 32768,
        "🗺️ <b>Mapa Interativo das Rotas</b><br><br>"
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
        "    window.mapaGrafo_%d = L.map('mapa-container-%d').setView([%s, %s], 5);"
        "    L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {"
        "      attribution: '© OpenStreetMap',"
        "      maxZoom: 18"
        "    }).addTo(window.mapaGrafo_%d);"
        "    console.log('Mapa do grafo criado com sucesso!');"
        , mapa_counter, mapa_counter, mapa_counter, mapa_counter, mapa_counter, mapa_counter, centro_lat, centro_lng, mapa_counter);

    // Adiciona marcadores para cada cidade (usando coordenadas aproximadas do Brasil)
    // Coordenadas aproximadas de cidades brasileiras principais
    const char* cidades_coords[][3] = {
        {"São Paulo", "-23.5505", "-46.6333"},
        {"Rio de Janeiro", "-22.9068", "-43.1729"},
        {"Curitiba", "-25.4284", "-49.2733"},
        {"Porto Alegre", "-30.0346", "-51.2177"},
        {"Florianópolis", "-27.5954", "-48.5480"},
        {"Belo Horizonte", "-19.9167", "-43.9345"},
        {"Brasília", "-15.7939", "-47.8828"},
        {"Salvador", "-12.9714", "-38.5014"},
        {"Recife", "-8.0476", "-34.8770"},
        {"Fortaleza", "-3.7319", "-38.5267"},
        {"Campinas", "-22.9099", "-47.0626"},
        {"Santos", "-23.9608", "-46.3336"},
        {"Joinville", "-26.3045", "-48.8487"},
        {"Blumenau", "-26.9194", "-49.0661"},
        {"Caxias do Sul", "-29.1634", "-51.1797"},
        {"Pelotas", "-31.7654", "-52.3376"},
        {"Londrina", "-23.3045", "-51.1696"},
        {"Maringá", "-23.4205", "-51.9333"},
        {"Foz do Iguaçu", "-25.5478", "-54.5882"},
        {"Vitória", "-20.3155", "-40.3128"},
        {"Goiânia", "-16.6869", "-49.2648"},
        {"Sorocaba", "-23.5015", "-47.4526"},
        {"Ribeirão Preto", "-21.1767", "-47.8208"},
        {"São José dos Campos", "-23.2237", "-45.9009"},
        {"Uberlândia", "-18.9188", "-48.2768"},
        {"Taubaté", "-23.0264", "-45.5555"},
        {"Resende", "-22.4688", "-44.4465"},
        {"Volta Redonda", "-22.5231", "-44.1044"},
        {"Barra Mansa", "-22.5444", "-44.1728"},
        {"Campos dos Goytacazes", "-21.7622", "-41.3181"},
        {"Juiz de Fora", "-21.7642", "-43.3502"},
        {"Angra dos Reis", "-23.0067", "-44.3181"},
        {"Paraty", "-23.2236", "-44.7161"},
        {"Guarulhos", "-23.4538", "-46.5333"},
        {"Osasco", "-23.5329", "-46.7919"},
        {"São Bernardo do Campo", "-23.6914", "-46.5650"},
        {"Santo André", "-23.6636", "-46.5341"},
        {"Mogi das Cruzes", "-23.5229", "-46.1883"},
        {"Jundiaí", "-23.1864", "-46.8978"},
        {"Piracicaba", "-22.7253", "-47.6492"},
        {"Bauru", "-22.3147", "-49.0608"},
        {"Presidente Prudente", "-22.1256", "-51.3889"},
        {"Registro", "-24.4878", "-47.8433"},
        {"Itanhaém", "-24.1831", "-46.7889"},
        {"Guaratinguetá", "-22.8161", "-45.1928"},
        {"Lorena", "-22.7311", "-45.1248"},
        {"Criciúma", "-28.6778", "-49.3697"},
        {"Lages", "-27.8167", "-50.3264"},
        {"Chapecó", "-27.0964", "-52.6181"},
        {"Passo Fundo", "-28.2625", "-52.4083"},
        {"Canoas", "-29.9175", "-51.1836"},
        {"Novo Hamburgo", "-29.6783", "-51.1306"},
        {"São Leopoldo", "-29.7603", "-51.1472"},
        {NULL, NULL, NULL}
    };

    // Adiciona marcadores das cidades do grafo
    for (int i = 0; i < g->num_cidades; i++) {
        const char* lat = NULL;
        const char* lng = NULL;

        // Procura coordenadas conhecidas
        for (int j = 0; cidades_coords[j][0] != NULL; j++) {
            if (strcasecmp(g->cidades[i].nome, cidades_coords[j][0]) == 0) {
                lat = cidades_coords[j][1];
                lng = cidades_coords[j][2];
                break;
            }
        }

        // Se encontrou coordenadas, adiciona marcador
        if (lat && lng) {
            char marker_code[1024];

            // Conta conexões
            int num_conexoes = 0;
            for (int j = 0; j < g->num_cidades; j++) {
                if (g->cidades[i].adjacencias[j] != -1) num_conexoes++;
            }

            snprintf(marker_code, sizeof(marker_code),
                "    L.marker([%s, %s]).addTo(window.mapaGrafo_%d)"
                ".bindPopup('<b>%s</b><br>%d conexões');"
                , lat, lng, mapa_counter, g->cidades[i].nome, num_conexoes);
            strcat(resultado, marker_code);
        }
    }

    // Adiciona linhas conectando as cidades
    for (int i = 0; i < g->num_cidades; i++) {
        const char* lat1 = NULL;
        const char* lng1 = NULL;

        for (int k = 0; cidades_coords[k][0] != NULL; k++) {
            if (strcasecmp(g->cidades[i].nome, cidades_coords[k][0]) == 0) {
                lat1 = cidades_coords[k][1];
                lng1 = cidades_coords[k][2];
                break;
            }
        }

        if (lat1 && lng1) {
            for (int j = i + 1; j < g->num_cidades; j++) {
                if (g->cidades[i].adjacencias[j] != -1) {
                    const char* lat2 = NULL;
                    const char* lng2 = NULL;

                    for (int k = 0; cidades_coords[k][0] != NULL; k++) {
                        if (strcasecmp(g->cidades[j].nome, cidades_coords[k][0]) == 0) {
                            lat2 = cidades_coords[k][1];
                            lng2 = cidades_coords[k][2];
                            break;
                        }
                    }

                    if (lat2 && lng2) {
                        char line_code[512];
                        snprintf(line_code, sizeof(line_code),
                            "    L.polyline([[%s,%s],[%s,%s]], {color: '#2196F3', weight: 2, opacity: 0.7})"
                            ".addTo(window.mapaGrafo_%d).bindPopup('%s ↔ %s: %d km');"
                            , lat1, lng1, lat2, lng2, mapa_counter,
                            g->cidades[i].nome, g->cidades[j].nome,
                            g->cidades[i].adjacencias[j]);
                        strcat(resultado, line_code);
                    }
                }
            }
        }
    }

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

    if (dist[idx_destino] == INT_MAX) {
        return strdup("❌ <b>Não há caminho entre as cidades</b>");
    }

    // Reconstrói o caminho
    int path[MAX_CIDADES];
    int path_size = 0;
    for (int v = idx_destino; v != -1; v = anterior[v]) {
        path[path_size++] = v;
    }

    // Coordenadas das cidades (mesma lista da função acima)
    const char* cidades_coords[][3] = {
        {"São Paulo", "-23.5505", "-46.6333"},
        {"Rio de Janeiro", "-22.9068", "-43.1729"},
        {"Curitiba", "-25.4284", "-49.2733"},
        {"Porto Alegre", "-30.0346", "-51.2177"},
        {"Florianópolis", "-27.5954", "-48.5480"},
        {"Belo Horizonte", "-19.9167", "-43.9345"},
        {"Brasília", "-15.7939", "-47.8828"},
        {"Salvador", "-12.9714", "-38.5014"},
        {"Joinville", "-26.3045", "-48.8487"},
        {"Blumenau", "-26.9194", "-49.0661"},
        {"Caxias do Sul", "-29.1634", "-51.1797"},
        {"Campinas", "-22.9099", "-47.0626"},
        {"Santos", "-23.9608", "-46.3336"},
        {"São José dos Campos", "-23.2237", "-45.9009"},
        {"Sorocaba", "-23.5015", "-47.4526"},
        {"Ribeirão Preto", "-21.1767", "-47.8208"},
        {"Taubaté", "-23.0264", "-45.5555"},
        {"Resende", "-22.4688", "-44.4465"},
        {"Volta Redonda", "-22.5231", "-44.1044"},
        {"Barra Mansa", "-22.5444", "-44.1728"},
        {"Campos dos Goytacazes", "-21.7622", "-41.3181"},
        {"Juiz de Fora", "-21.7642", "-43.3502"},
        {"Guarulhos", "-23.4538", "-46.5333"},
        {"Osasco", "-23.5329", "-46.7919"},
        {"Mogi das Cruzes", "-23.5229", "-46.1883"},
        {"Jundiaí", "-23.1864", "-46.8978"},
        {"Registro", "-24.4878", "-47.8433"},
        {"Guaratinguetá", "-22.8161", "-45.1928"},
        {"Lorena", "-22.7311", "-45.1248"},
        {"Criciúma", "-28.6778", "-49.3697"},
        {"Lages", "-27.8167", "-50.3264"},
        {"Canoas", "-29.9175", "-51.1836"},
        {"Novo Hamburgo", "-29.6783", "-51.1306"},
        {NULL, NULL, NULL}
    };

    // Monta resultado com mapa
    char* resultado = (char*)malloc(32768);

    // Calcula centro do mapa (média entre origem e destino)
    const char *lat_orig = NULL, *lng_orig = NULL;
    const char *lat_dest = NULL, *lng_dest = NULL;

    for (int i = 0; cidades_coords[i][0] != NULL; i++) {
        if (strcasecmp(origem, cidades_coords[i][0]) == 0) {
            lat_orig = cidades_coords[i][1];
            lng_orig = cidades_coords[i][2];
        }
        if (strcasecmp(destino, cidades_coords[i][0]) == 0) {
            lat_dest = cidades_coords[i][1];
            lng_dest = cidades_coords[i][2];
        }
    }

    const char* centro_lat = lat_orig ? lat_orig : "-15.7939";
    const char* centro_lng = lng_orig ? lng_orig : "-47.8828";

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
        "<div id='mapa-rota-%d' style='width: 100%%; height: 400px; border: 2px solid #4CAF50; border-radius: 8px; margin: 10px 0;'></div>"
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

    // Adiciona marcadores e linha da rota
    char polyline_points[4096] = "[";

    for (int i = path_size - 1; i >= 0; i--) {
        const char *lat = NULL, *lng = NULL;

        for (int j = 0; cidades_coords[j][0] != NULL; j++) {
            if (strcasecmp(g->cidades[path[i]].nome, cidades_coords[j][0]) == 0) {
                lat = cidades_coords[j][1];
                lng = cidades_coords[j][2];
                break;
            }
        }

        if (lat && lng) {
            // Adiciona ao polyline
            char point[128];
            snprintf(point, sizeof(point), "%s[%s,%s]", (i < path_size - 1) ? "," : "", lat, lng);
            strcat(polyline_points, point);

            // Adiciona marcador
            char marker[512];
            const char* icon_color = (i == path_size - 1) ? "green" : (i == 0) ? "red" : "blue";
            const char* label = (i == path_size - 1) ? "🚩 Origem" : (i == 0) ? "🎯 Destino" : "📍";

            snprintf(marker, sizeof(marker),
                "    L.marker([%s,%s]).addTo(window.mapaRota_%d).bindPopup('<b>%s</b><br>%s');"
                , lat, lng, rota_counter, g->cidades[path[i]].nome, label);
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

