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

## 📦 Instalação das Dependências

### vcpkg no MSYS2 (Recomendado)

Esta é a abordagem recomendada pela Microsoft para usar vcpkg em ambientes Windows com MinGW.

#### 1. Instalar MSYS2

Baixe e instale o MSYS2 do site oficial: https://www.msys2.org/

#### 2. Instalar ferramentas básicas no MSYS2

Abra o terminal MSYS2 e execute:

```bash
# Atualizar o sistema
pacman -Syu

# Instalar MinGW-w64 toolchain
pacman -S mingw-w64-x86_64-toolchain

# Instalar CMake
pacman -S mingw-w64-x86_64-cmake

# Instalar Git (necessário para o vcpkg)
pacman -S git

# Instalar ninja (opcional, mas recomendado para builds mais rápidos)
pacman -S mingw-w64-x86_64-ninja
```

#### 3. Instalar e configurar vcpkg

No terminal MSYS2, execute:

```bash
# Clonar o vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg

# Executar o bootstrap
./bootstrap-vcpkg.sh

# Integrar com o sistema
./vcpkg integrate install
```

#### 4. Instalar as bibliotecas necessárias

```bash
# Instalar cURL e cJSON
./vcpkg install curl:x64-mingw-static
./vcpkg install cjson:x64-mingw-static
```

#### 5. Configurar o PATH

Adicione ao PATH do Windows: `C:\msys64\mingw64\bin`

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

**⚠️ ATENÇÃO DE SEGURANÇA**: 
- Nunca compartilhe sua API key publicamente
- Não faça commit da sua API key real no GitHub
- Considere usar variáveis de ambiente em projetos de produção

## 🚀 Compilação e Execução

### Usando CMake com vcpkg no MSYS2 (Recomendado)

1. **Abrir terminal MSYS2 MinGW 64-bit**

2. **Navegar até o diretório do projeto**:
```bash
cd /c/Users/seu_usuario/caminho/para/GenieC
```

3. **Criar diretório de build**:
```bash
mkdir build
cd build
```

4. **Configurar o projeto com vcpkg**:
```bash
cmake .. -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=/caminho/para/vcpkg/scripts/buildsystems/vcpkg.cmake
```

5. **Compilar**:
```bash
cmake --build .
```

6. **Executar**:
```bash
./PPRP.exe
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

## 🐛 Solução de Problemas

### Problemas com MinGW/MSYS2
- Certifique-se de usar o terminal "MSYS2 MinGW 64-bit"
- Verifique se o PATH do MinGW está configurado corretamente
- Use `pacman -S mingw-w64-x86_64-pkg-config` se houver problemas de linking

### Erro de compilação
- **MSYS2**: Verifique se todas as dependências foram instaladas via pacman
- **vcpkg**: Verifique se o vcpkg está corretamente configurado
- Certifique-se de que as bibliotecas foram instaladas corretamente

### Erro de API
- Verifique se sua API key está correta
- Confirme se você tem acesso à API do Gemini
- Verifique sua conexão com a internet

### Problemas de codificação
- O programa está configurado para UTF-8
- Certifique-se de que seu terminal suporta caracteres especiais
- No Windows, o programa executa `chcp 65001` automaticamente

### Problemas de linking
- Se usar MSYS2, certifique-se de estar no ambiente MinGW correto
- Para vcpkg, verifique se o toolchain file está correto

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
