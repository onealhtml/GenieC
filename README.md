# GenieC - Assistente Inteligente em C

**GenieC** é um assistente pessoal inteligente feito em C puro, criado como projeto acadêmico da disciplina de Programação para Resolução de Problemas, e Estrutura de Dados e Programação, da UNISC. 

O projeto integra o Google Gemini para conversas com IA e o OpenWeatherMap para informações de clima em tempo real.

## Funcionalidades

- **Conversas com IA**: Usa o Google Gemini para responder perguntas e manter conversas contextualizadas
- **Interface Gráfica Moderna**: Interface web com HTML/CSS/JavaScript integrada via Webview
- **Sistema de Grafos**: Visualização e manipulação de grafos com mapa interativo
- **Informações de Clima**: Mostra temperatura e condições climáticas em tempo real
- **Histórico de Conversas**: Guarda até 20 interações e permite visualizar em tela separada
- **Sistema de Retry**: Tenta novamente automaticamente se alguma requisição falhar

## Como Usar

### Executar

Rode o executável `GenieC.exe`.

No primeiro uso, digite o nome da sua cidade para buscar informações do clima.

### Interface Gráfica

- **Chat Inteligente**: Digite perguntas e converse com a IA
- **Painel de Clima**: Visualize temperatura e condições em tempo real
- **Sistema de Grafos**: Crie e visualize grafos em mapa interativo
  - Adicione nós (cidades) clicando no mapa
  - Conecte nós com arestas
  - Calcule menor caminho entre pontos
- **Histórico**: Acesse conversas anteriores
- **Limpar**: Reinicie a conversa

### Ícones de Clima

☀️ Céu limpo | 🌤️ Poucas nuvens | ☁️ Nublado | 🌧️ Chuva | ⛈️ Tempestade | ❄️ Neve | 🌫️ Neblina

## Estrutura do Projeto

O código está dividido em módulos:

- **main_gui.c** - Interface gráfica principal usando Webview
- **clima.c/h** - Busca informações do OpenWeatherMap
- **gemini.c/h** - Conversa com o Google Gemini
- **grafo.c/h** - Sistema de grafos e cálculos de menor caminho
- **historico.c/h** - Guarda as conversas
- **http_utils.c/h** - Faz as requisições HTTP
- **env_loader.c/h** - Lê o arquivo .env
- **ui_loader.c/h** - Carrega recursos da interface
- **ui/** - Arquivos HTML, CSS e JavaScript da interface

## Tecnologias

- C puro (C23)
- Webview para interface gráfica
- HTML/CSS/JavaScript para UI
- Leaflet.js para mapas interativos
- cURL para requisições HTTP
- cJSON para manipulação do JSON
- dotenv-c para ler variáveis de ambiente
- APIs do Google Gemini e OpenWeatherMap

## Equipe

Desenvolvido por:
- Lorenzo Farias
- Bernardo Soares Nunes  
- Pedro Cabral Buchaim

**Universidade de Santa Cruz do Sul (UNISC)**  
Disciplinas: Programação para Resolução de Problemas, Estrutura de Dados e Programação

---

## Configuração

### 1. Obter as chaves de API

**Google Gemini:**
- Entre em https://makersuite.google.com/app/apikey
- Crie uma API key gratuita

**OpenWeatherMap:**
- Entre em https://openweathermap.org/api
- Faça cadastro e pegue sua API key

### 2. Configurar o arquivo .env

Copie o arquivo `.env.example` para `.env` e coloque suas chaves:

```
GEMINI_API_KEY=sua_chave_aqui
OPENWEATHER_API_KEY=sua_chave_aqui
```

---

## Licença

MIT License - veja [LICENSE.txt](LICENSE.txt) para mais detalhes.
