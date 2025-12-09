/* GenieC - Utilidades para carregar recursos da interface */
#include "ui_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <windows.h>
#endif

// Incluir webview.h para ter acesso a webview_set_html
#include "webview/webview.h"

/**
 * Carrega o conteúdo de um arquivo para uma string
 */
char* carregar_arquivo(const char* filepath) {
    FILE* file = fopen(filepath, "rb");
    if (!file) {
        fprintf(stderr, "[ERRO] Não foi possível abrir: %s\n", filepath);
        return NULL;
    }

    // Obtém o tamanho do arquivo
    fseek(file, 0, SEEK_END);
    long size = ftell(file);
    fseek(file, 0, SEEK_SET);

    // Aloca memória
    char* content = (char*)malloc(size + 1);
    if (!content) {
        fclose(file);
        return NULL;
    }

    // Lê o conteúdo
    size_t read = fread(content, 1, size, file);
    content[read] = '\0';
    fclose(file);

    return content;
}

/**
 * Gera o HTML completo com CSS e JS inline
 */
char* gerar_html_completo(void) {
    // Tenta múltiplos caminhos (para funcionar em diferentes contextos)
    const char* paths[][3] = {
        {"ui/ui_template.html", "ui/ui_style.css", "ui/ui_script.js"},
        {"../ui/ui_template.html", "../ui/ui_style.css", "../ui/ui_script.js"},
        {"../../ui/ui_template.html", "../../ui/ui_style.css", "../../ui/ui_script.js"}
    };

    char* html = NULL;
    char* css = NULL;
    char* js = NULL;

    // Tenta carregar os arquivos de diferentes locais
    for (int i = 0; i < 3 && !html; i++) {
        html = carregar_arquivo(paths[i][0]);
        if (html) {
            css = carregar_arquivo(paths[i][1]);
            js = carregar_arquivo(paths[i][2]);

            if (!css || !js) {
                free(html);
                html = NULL;
                free(css);
                free(js);
                css = NULL;
                js = NULL;
            }
        }
    }

    if (!html || !css || !js) {
        fprintf(stderr, "[ERRO] Não foi possível carregar arquivos da interface\n");
        fprintf(stderr, "[INFO] Certifique-se que os arquivos estão em ui/\n");
        free(html);
        free(css);
        free(js);
        return NULL;
    }

    // Calcula tamanho necessário com margem de segurança
    size_t total_size = strlen(html) + strlen(css) + strlen(js) + 2048;
    char* resultado = (char*)malloc(total_size);

    if (!resultado) {
        free(html);
        free(css);
        free(js);
        return NULL;
    }

    resultado[0] = '\0';

    // Procura posições dos marcadores
    char* link_pos = strstr(html, "<link rel='stylesheet' href='ui_style.css'>");
    char* script_pos = strstr(html, "<script src='ui_script.js'></script>");

    if (!link_pos || !script_pos) {
        fprintf(stderr, "[ERRO] Template HTML inválido\n");
        free(resultado);
        free(html);
        free(css);
        free(js);
        return NULL;
    }

    // Parte 1: Do início até antes do <link>
    size_t len1 = link_pos - html;
    strncat(resultado, html, len1);

    // Parte 2: Insere CSS inline
    strcat(resultado, "<style>");
    strcat(resultado, css);
    strcat(resultado, "</style>");

    // Parte 3: Do fim do <link> até antes do <script>
    char* after_link = link_pos + strlen("<link rel='stylesheet' href='ui_style.css'>");
    size_t len2 = script_pos - after_link;
    strncat(resultado, after_link, len2);

    // Parte 4: Insere JS inline
    strcat(resultado, "<script>");
    strcat(resultado, js);
    strcat(resultado, "</script>");

    // Parte 5: Do fim do <script> até o final
    char* after_script = script_pos + strlen("<script src='ui_script.js'></script>");
    strcat(resultado, after_script);

    free(html);
    free(css);
    free(js);

    return resultado;
}

/**
 * Carrega a interface HTML na janela webview
 */
void carregar_html_interface(webview_t w) {
    // Gera HTML completo com CSS e JS inline
    char* html_completo = gerar_html_completo();

    if (html_completo) {
        // Carrega HTML na webview
        fprintf(stderr, "[INFO] HTML carregado com sucesso (%zu bytes)\n", strlen(html_completo));
        webview_set_html(w, html_completo);
        free(html_completo);
    } else {
        fprintf(stderr, "[ERRO] Falha ao carregar HTML, usando fallback\n");

        // HTML de fallback simples em caso de erro
        const char* fallback =
            "<!DOCTYPE html><html><head><meta charset='UTF-8'></head><body>"
            "<h1 style='color:red'>ERRO: Não foi possível carregar a interface</h1>"
            "<p>Certifique-se que os arquivos estão em:</p>"
            "<ul><li>ui/ui_template.html</li><li>ui/ui_style.css</li><li>ui/ui_script.js</li></ul>"
            "<p>Execute o programa no diretório correto (onde está a pasta ui/)</p>"
            "</body></html>";

        webview_set_html(w, fallback);
    }
}
