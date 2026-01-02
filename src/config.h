#ifndef CONFIG_H
#define CONFIG_H

// ============================================================================
// CONFIGURAÇÕES DE MODELOS DE IA
// ============================================================================

// Modelo para chat normal (rápido e leve)
#define MODELO_GEMINI_CHAT "gemini-2.5-flash-preview-09-2025"

// Modelo para grafos e análises complexas (mais poderoso)
#define MODELO_GEMINI_GRAFO "gemini-3-pro-preview"

// Modelo padrão (compatibilidade)
#define MODELO_GEMINI MODELO_GEMINI_CHAT

// ============================================================================
// CONFIGURAÇÕES DE LIMITES
// ============================================================================

#define MAX_PROMPT_SIZE 10000
#define MAX_HISTORY_SIZE 50
#define MAX_HISTORY_TURNS 20
#define MAX_CITY_NAME 100

// ============================================================================
// CONFIGURAÇÕES DE RETRY E TIMEOUT
// ============================================================================

#define MAX_RETRIES 3
#define INITIAL_RETRY_DELAY 1000  // 1 segundo em ms
#define HTTP_TIMEOUT 120L          // 30 segundos
#define HTTP_CONNECT_TIMEOUT 60L  // 10 segundos

// ============================================================================
// PROMPTS DO SISTEMA
// ============================================================================

// Prompt principal para chat conversacional
#define SYSTEM_PROMPT "Você é o GenieC, um assistente pessoal para responder dúvidas do dia a dia.\n\n" \
"CONTEXTO DO USUÁRIO:\n" \
"- O usuário está localizado em: %s\n" \
"- Sempre considere esta cidade como referência quando ele perguntar sobre 'minha cidade', 'aqui', clima local, horários, eventos, etc.\n\n" \
"COMUNICAÇÃO:\n" \
"- Responda de forma clara, precisa e educada\n" \
"- Seja MUITO conciso - máximo 2-3 frases por resposta\n" \
"- Use linguagem natural e acessível\n" \
"- Evite usar marcadores de formatação especial (sem *negrito*, _itálico_, etc.)\n\n" \
"PESQUISA:\n" \
"- Use ferramentas de pesquisa quando necessário para informações atualizadas\n" \
"- Para perguntas sobre o Brasil, considere o fuso horário e contexto brasileiro\n\n" \
"FORMATO:\n" \
"- Respostas diretas e sem formatação especial\n" \
"- Evite listas longas, use apenas o essencial"

// Prompt para obter distâncias entre cidades (usado em grafos)
#define PROMPT_DISTANCIAS_GRAFO \
"Liste distâncias rodoviárias REAIS (BR-XXX, rodovias principais) entre %s e %s.\n\n" \
"REGRAS OBRIGATÓRIAS:\n" \
"1. Use GOOGLE MAPS ou dados reais de rodovias brasileiras\n" \
"2. Inclua 10-15 cidades intermediárias IMPORTANTES na rota principal\n" \
"3. Adicione rotas alternativas com outras cidades\n" \
"4. Distâncias entre cidades VIZINHAS (adjacentes), não diretas\n" \
"5. Cada trecho deve ter 50-300 km (trechos curtos, realistas)\n" \
"6. Siga rodovias principais (BR-101, BR-116, BR-381, etc)\n\n" \
"NORMALIZAÇÃO DE NOMES (CRÍTICO):\n" \
"- SEMPRE use os nomes das cidades EXATAMENTE como informado pelo usuário\n" \
"- Se o usuário escreveu 'Sao Paulo' (sem acento), use 'Sao Paulo' na resposta\n" \
"- Se o usuário escreveu 'São Paulo' (com acento), use 'São Paulo' na resposta\n" \
"- NUNCA corrija ou altere a grafia das cidades de origem e destino\n" \
"- Para cidades intermediárias, prefira nomes SEM ACENTOS para compatibilidade\n\n" \
"FORMATO OBRIGATÓRIO (uma linha por conexão):\n" \
"CidadeA-CidadeB:XXX\n\n" \
"EXEMPLO DE MALHA REAL:\n" \
"Sao Paulo-Sao Jose dos Campos:85\n" \
"Sao Jose dos Campos-Taubate:45\n" \
"Taubate-Resende:115\n" \
"Resende-Volta Redonda:35\n" \
"Volta Redonda-Barra Mansa:15\n" \
"Barra Mansa-Rio de Janeiro:128\n\n" \
"IMPORTANTE: Use apenas distâncias VERIFICADAS. Não invente valores!\n" \
"RESPONDA APENAS COM AS LINHAS NO FORMATO, SEM TEXTO EXTRA."

// Prompt para obter coordenadas de uma única cidade
#define PROMPT_COORDENADAS_UNICA \
"Qual a coordenada geográfica exata de %s?\n\n" \
"INTERPRETAÇÃO DO NOME:\n" \
"- Aceite variações com ou sem acentos (ex: 'Sao Paulo' = 'São Paulo')\n" \
"- Identifique a cidade correta mesmo com pequenos erros de digitação\n\n" \
"RESPONDA APENAS NO FORMATO:\n" \
"LAT:valor_latitude\n" \
"LNG:valor_longitude\n\n" \
"EXEMPLO:\n" \
"LAT:-23.5505\n" \
"LNG:-46.6333\n\n" \
"NÃO ADICIONE TEXTO EXTRA. APENAS AS DUAS LINHAS."

// Prompt para obter coordenadas de múltiplas cidades em lote
#define PROMPT_COORDENADAS_MULTIPLAS \
"Forneça as coordenadas geográficas exatas das seguintes cidades:\n\n" \
"%s\n\n" \
"RESPONDA APENAS NO FORMATO (uma cidade por linha):\n" \
"CIDADE|LAT:valor|LNG:valor\n\n" \
"REGRAS CRÍTICAS:\n" \
"- Use o NOME EXATO da cidade como foi informado na lista acima\n" \
"- NÃO adicione ou remova acentos dos nomes\n" \
"- Se na lista está 'Sao Paulo', responda 'Sao Paulo', NÃO 'São Paulo'\n\n" \
"EXEMPLO:\n" \
"Sao Paulo|LAT:-23.5505|LNG:-46.6333\n" \
"Rio de Janeiro|LAT:-22.9068|LNG:-43.1729\n" \
"Curitiba|LAT:-25.4284|LNG:-49.2733\n\n" \
"IMPORTANTE:\n" \
"- Use coordenadas REAIS e PRECISAS\n" \
"- Uma linha por cidade\n" \
"- Formato exato: CIDADE|LAT:numero|LNG:numero\n" \
"- NÃO adicione texto extra, apenas as linhas no formato"

#endif // CONFIG_H
