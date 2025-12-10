# GenieC - Assistente Inteligente em C

**GenieC** é um assistente pessoal inteligente feito em C puro, criado como projeto acadêmico da disciplina de Programação para Resolução de Problemas, e Estrutura de Dados e Programação, da UNISC. 

O projeto integra o Google Gemini para conversas com IA e o OpenWeatherMap para informações de clima em tempo real.

## Funcionalidades

- **Conversas com IA**: Usa o Google Gemini para responder perguntas e manter conversas contextualizadas
- **Informações de Clima**: Mostra temperatura e condições climáticas com ícones animados
- **Histórico de Conversas**: Guarda até 20 interações e permite visualizar tudo em uma tela separada
- **Interface Colorida**: Terminal com cores e ícones para melhor experiência
- **Sistema de Retry**: Tenta novamente automaticamente se alguma requisição falhar

## Como Usar

### Comandos Disponíveis

- **Digite qualquer pergunta** - O assistente responde usando o Google Gemini
- `limpar` - Apaga o histórico e começa uma conversa nova
- `historico` - Mostra todas as conversas anteriores
- `help` - Exibe ajuda
- `0` - Fecha o programa

### Ícones de Clima

O programa mostra ícones diferentes dependendo do clima:
- ☀️ Céu limpo
- 🌤️ Poucas nuvens  
- ☁️ Nublado
- 🌧️ Chuva
- ⛈️ Tempestade
- ❄️ Neve
- 🌫️ Neblina


## Executar

Rode o executável `GenieC.exe`.

No primeiro uso, o programa vai pedir o nome da sua cidade para buscar informações do clima.

## Estrutura do Projeto

O código está dividido em módulos:

- **clima.c/h** - Busca informações do OpenWeatherMap
- **gemini.c/h** - Conversa com o Google Gemini
- **historico.c/h** - Guarda as conversas
- **ui_cli.c/h** - Interface no terminal
- **http_utils.c/h** - Faz as requisições HTTP
- **env_loader.c/h** - Lê o arquivo .env

## Tecnologias

- C puro (C99)
- cURL para requisições HTTP
- cJSON para ler JSON
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
