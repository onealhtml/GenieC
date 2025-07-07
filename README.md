# GenieC - Cliente Gemini em C
Um assistente inteligente em linguagem C que integra a API do Google Gemini AI com funcionalidades avançadas de chat e informações climáticas em tempo real.

## 📋 Funcionalidades

### 🤖 Assistente AI
- Interface de linha de comando colorida e intuitiva
- Comunicação com a API do Google Gemini usando cURL
- **Histórico de conversa**: Mantém o contexto entre perguntas
- **Pesquisa em tempo real**: Integração com Google Search via API Gemini
- Prompt de sistema personalizado para respostas em português brasileiro

### 🌤️ Informações Climáticas
- **Dados meteorológicos em tempo real** via API OpenWeather
- Exibição de temperatura e condições climáticas da cidade do usuário
- Codificação automática de nomes de cidades para URLs

### 💬 Comandos Interativos
- `limpar` - Limpa o histórico da conversa e reinicia
- `historico` - Exibe todo o histórico da conversa atual
- `help` - Mostra ajuda detalhada e dicas de uso
- `0` - Sair do programa

### 🎨 Interface Avançada
- Arte ASCII colorida do GenieC
- Cores diferenciadas para melhor experiência do usuário
- Animação de loading durante consultas à AI
- Menu principal com informações do clima

## 🛠️ Pré-requisitos

- **Compilador C**: 
  - MinGW-w64
  - GCC
  - Visual Studio
- **CMake** (versão 3.21 ou superior)
- **Gerenciador de pacotes**:
  - MSYS2/pacman
  - vcpkg
- **Bibliotecas**:
  - cURL (para requisições HTTP)
  - cJSON (para parsing de JSON)

## 🔑 Configuração das API Keys

**IMPORTANTE**: Para usar este projeto, você precisa configurar duas chaves de API.

### 1. API Key do Google Gemini
1. Acesse o [Google AI Studio](https://aistudio.google.com/app/apikey)
2. Faça login com sua conta Google
3. Clique em "Create API Key"
4. Copie a chave gerada

### 2. API Key do OpenWeather
1. Acesse o [OpenWeatherMap](https://openweathermap.org/api)
2. Crie uma conta gratuita
3. Obtenha sua API key

### 3. Configurar as API Keys no projeto
Abra o arquivo `api_key.example.h`, renomeie para `api_key.h` e configure ambas as chaves:

```c
#ifndef API_KEY_H
#define API_KEY_H

// API Key do Google Gemini
#define API_KEY "SUA_API_KEY_GEMINI_AQUI"

// API Key do OpenWeather
#define API_KEY_WEATHER "SUA_API_KEY_OPENWEATHER_AQUI"

#endif
```

## 💻 Como Usar

### Primeira Execução
1. Execute o programa
2. Digite o nome da sua cidade para obter informações climáticas
3. O GenieC exibirá o menu principal com dados do clima

### Interagindo com o Assistente
- **Faça perguntas naturais**: "Como fazer um bolo de chocolate?"
- **Use comandos especiais**: Digite `help` para ver todas as opções
- **Mantenha contexto**: O GenieC lembra das conversas anteriores
- **Pesquise informações atuais**: "Qual é a notícia mais recente sobre tecnologia?"

### Exemplo de uso:
```
🌍 Digite o nome da sua cidade para obter informações do clima: Santa Cruz do Sul

╔═════════════════════════════════════════════════════════════════════════════╗
║                ██████╗ ███████╗███╗   ██╗██╗███████╗ ██████╗                ║
║               ██╔════╝ ██╔════╝████╗  ██║██║██╔════╝██╔════╝                ║
║               ██║  ███╗█████╗  ██╔██╗ ██║██║█████╗  ██║                     ║
║               ██║   ██║██╔══╝  ██║╚██╗██║██║██╔══╝  ██║                     ║
║               ╚██████╔╝███████╗██║ ╚████║██║███████╗╚██████╗                ║
║                ╚═════╝ ╚══════╝╚═╝  ╚═══╝╚═╝╚══════╝ ╚═════╝                ║
╚═════════════════════════════════════════════════════════════════════════════╝

🤖 Bem-vindo ao GenieC - Seu Assistente Inteligente Gemini! 🤖

🌤️ Clima atual em Santa Cruz do Sul: 22.5°C - parcialmente nublado

Você: Quem é o atual Papa?
GenieC: O atual Papa da Igreja Católica é Leão XIV. Ele foi eleito em 8 de maio de 2025, sucedendo o Papa Francisco.

Você: 0
Finalizando o programa...
```

## 📁 Estrutura do Projeto

```
GenieC/
├── GenieC.c          # Código principal
├── api_key.h         # Arquivo de configuração das API keys
├── limpar_tela.h     # Função para limpar a tela
├── dormir.h          # Função para pausas
├── CMakeLists.txt    # Configuração do CMake
├── build/            # Diretório de compilação
│   └── GenieC.exe    # Executável gerado
└── README.md         # Este arquivo
```

## 🔧 Principais Funções

### Core do Sistema
- `main()`: Loop principal do programa
- `mostrar_arte_inicial()`: Exibe a arte ASCII do GenieC
- `menu_com_clima()`: Mostra o menu principal com informações climáticas
- `mostrar_ajuda()`: Exibe ajuda detalhada e dicas

### Comunicação com APIs
- `fazer_requisicao_http()`: Realiza requisições HTTP usando cURL
- `criar_payload_json_com_historico()`: Cria payload JSON com histórico da conversa
- `extrair_texto_da_resposta()`: Extrai texto da resposta JSON do Gemini
- `obter_dados_clima()`: Obtém dados climáticos via API OpenWeather

### Gerenciamento de Histórico
- `inicializar_chat_historico()`: Inicializa o sistema de histórico
- `adicionar_turno()`: Adiciona nova mensagem ao histórico
- `exibir_historico()`: Mostra o histórico completo da conversa
- `liberar_chat_history()`: Libera memória do histórico

### Utilitários
- `url_encode()`: Codifica strings para URLs
- `mostrar_loading()`: Exibe animação de carregamento
- `WriteMemoryCallback()`: Callback para armazenar respostas HTTP

## 🌟 Recursos Avançados

### Sistema de Histórico Inteligente
- Mantém contexto completo da conversa
- Permite referências a mensagens anteriores
- Gerenciamento automático de memória
- Limite configurável de turnos (50 por padrão)

### Integração com Google Search
- Pesquisa em tempo real via API Gemini
- Respostas com informações atualizadas
- Suporte a consultas específicas de localização

### Prompt de Sistema Personalizado
- Respostas em português brasileiro
- Instruções específicas para o contexto brasileiro
- Diretrizes para respostas concisas e úteis

## 📄 Licença

Este projeto é para fins educacionais. Use responsavelmente e respeite os termos de uso das APIs do Google Gemini e OpenWeather.

## 👥 Créditos

Este projeto foi desenvolvido por:
- [Lorenzo Farias](https://github.com/onealhtml)
- [Bernardo Soares Nunes](https://github.com/besoaresn)
- [Pedro Cabral Buchaim](https://github.com/Tinpack)

**Universidade de Santa Cruz do Sul (UNISC)**  
**Disciplina**: Programação Para Resolução de Problemas  
**Professora Responsável**: Profa. Dra. Daniela Bagatini

## ⚠️ Aviso Legal

Este projeto utiliza as APIs do Google Gemini e OpenWeather. Certifique-se de cumprir os termos de uso de ambas as APIs e use-as de forma responsável. Os autores não se responsabilizam pelo uso inadequado das APIs ou por possíveis custos decorrentes do uso excessivo.