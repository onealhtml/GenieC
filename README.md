# GenieC - Cliente Gemini AI em C

Um cliente simples em linguagem C para interagir com a API do Google Gemini AI. Este projeto permite fazer perguntas aos modelos Gemini através de uma interface de linha de comando.

## 📋 Funcionalidades

- Interface de linha de comando simples e intuitiva
- Comunicação com a API do Google Gemini usando cURL
- Parsing de JSON usando a biblioteca cJSON
- Loop interativo para múltiplas perguntas
- Suporte a codificação UTF-8 para caracteres especiais
- Gerenciamento automático de memória

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

## 🔑 Configuração da API Key

**IMPORTANTE**: Para usar este projeto, você precisa configurar sua chave da API do Google Gemini.

### 1. Obter a API Key

1. Acesse o [Google AI Studio](https://aistudio.google.com/app/apikey)
2. Faça login com sua conta Google
3. Clique em "Create API Key"
4. Copie a chave gerada

### 2. Configurar a API Key no projeto

Abra o arquivo `api_key.h` e substitua a chave existente pela sua:

```c
#ifndef API_KEY_H
#define API_KEY_H
#define API_KEY "SUA_API_KEY_AQUI"
#endif
```

## 💻 Como Usar

1. Execute o programa
2. Digite sua pergunta quando solicitado
3. Aguarde a resposta do Gemini
4. Digite '0' para sair do programa

### Exemplo de uso:

```
Digite sua pergunta para o Gemini (ou '0' para sair): Como funciona a inteligência artificial?

--- Resposta do Gemini ---
A inteligência artificial (IA) é um campo da ciência da computação que busca criar sistemas capazes de realizar tarefas que normalmente requerem inteligência humana...

Digite sua pergunta para o Gemini (ou '0' para sair): 0
Finalizando o programa...
```

## 📁 Estrutura do Projeto

```
GenieC/
├── GenieC.c          # Código principal
├── api_key.h         # Arquivo de configuração da API key
├── CMakeLists.txt    # Configuração do CMake
├── build/            # Diretório de compilação
│   └── PPRP.exe      # Executável gerado
└── README.md         # Este arquivo
```

## 🔧 Principais Funções

- `criar_payload_json()`: Cria o payload JSON para a API
- `extrair_texto_da_resposta()`: Extrai o texto da resposta JSON
- `fazer_requisicao_http()`: Realiza a requisição HTTP usando cURL

## 📄 Licença

Este projeto é para fins educacionais. Use responsavelmente e respeite os termos de uso da API do Google Gemini.

## 👥 Créditos

Este projeto foi desenvolvido por:
- **Lorenzo Farias**
- **Bernardo Soares Nunes**
- **Pedro Cabral Buchaim**

**Universidade de Santa Cruz do Sul (UNISC)**  
**Disciplina**: Programação Para Resolução de Problemas  
**Professora Responsável**: Profa. Dra. Daniela Bagatini

## ⚠️ Aviso Legal

Este projeto utiliza a API do Google Gemini. Certifique-se de cumprir os termos de uso do Google e use a API de forma responsável. Os autores não se responsabiliza pelo uso inadequado da API ou por possíveis custos decorrentes do uso excessivo.
