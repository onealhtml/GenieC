/* GenieC - Utilidades para carregar recursos da interface */
#ifndef UI_LOADER_H
#define UI_LOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Forward declaration para evitar dependência circular
typedef void* webview_t;

/**
 * Carrega o conteúdo de um arquivo para uma string
 * @param filepath Caminho relativo ao executável
 * @return String alocada com o conteúdo ou NULL em caso de erro
 */
char* carregar_arquivo(const char* filepath);

/**
 * Gera o HTML completo com CSS e JS inline
 * @return String alocada com HTML completo ou NULL em caso de erro
 */
char* gerar_html_completo(void);

/**
 * Carrega a interface HTML na janela webview
 * @param w Ponteiro para webview
 */
void carregar_html_interface(webview_t w);

#endif // UI_LOADER_H
