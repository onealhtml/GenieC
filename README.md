<div align="center">

# 🤖 GenieC - Assistente Inteligente em C

![GenieC Banner](https://img.shields.io/badge/GenieC-v2.0-blue?style=for-the-badge)
![C](https://img.shields.io/badge/C-00599C?style=for-the-badge&logo=c&logoColor=white)
![License](https://img.shields.io/badge/License-MIT-green?style=for-the-badge)

**Um assistente inteligente desenvolvido em C puro, integrado com Google Gemini e OpenWeatherMap**

[Características](#-características) • [Instalação](#-instalação) • [Uso](#-uso) • [Arquitetura](#-arquitetura) • [Contribuir](#-contribuir)

</div>

---

## 📋 Sobre o Projeto

**GenieC** é um assistente pessoal inteligente desenvolvido inteiramente em linguagem C, criado como projeto acadêmico para a disciplina de Programação para Resolução de Problemas da UNISC. O projeto demonstra integração com APIs modernas, arquitetura modular e boas práticas de desenvolvimento em C.

### ✨ Versão 2.0 - Melhorias

- 🏗️ **Arquitetura Modular** - Código organizado em módulos independentes
- 🔐 **Variáveis de Ambiente** - Gerenciamento seguro de API keys com dotenv-c
- 🌦️ **Ícones Dinâmicos** - Ícones de clima que mudam de acordo com o tempo
- 📜 **Histórico Melhorado** - Visualização em tela separada com navegação intuitiva
- 🔄 **Sistema de Retry** - Maior confiabilidade nas requisições HTTP
- 🎨 **Interface Colorida** - Terminal com cores ANSI para melhor UX

---

## 🎯 Características

### 🤖 Integração com Google Gemini
- Conversas contextualizadas com IA
- Pesquisa em tempo real via Google Search
- Histórico de conversação mantido automaticamente
- Limite inteligente de turnos (até 20 interações)

### 🌤️ Informações Meteorológicas
- Clima em tempo real via OpenWeatherMap
- Ícones dinâmicos baseados nas condições climáticas:
  - ☀️ Céu limpo
  - 🌤️ Poucas nuvens
  - ⛅ Nuvens dispersas
  - ☁️ Nublado
  - 🌧️ Chuva
  - ⛈️ Tempestade
  - ❄️ Neve
  - 🌫️ Neblina

### 💬 Interface Interativa
- Menu principal intuitivo
- Comandos especiais:
  - `limpar` - Limpa o histórico e inicia nova conversa
  - `historico` - Exibe histórico em tela separada
  - `help` - Mostra ajuda e dicas
  - `0` - Sair do programa

### 🔧 Recursos Técnicos
- Parser de `.env` com dotenv-c
- Requisições HTTP com cURL e retry automático
- Parsing JSON com cJSON
- Gerenciamento de memória eficiente
- Tratamento robusto de erros

---

## 🚀 Uso

### Executar o Programa

```bash
# Windows
cmake-build-release\GenieC.exe

# Linux
./cmake-build-release/GenieC
```

### Fluxo de Uso

1. **Inicialização:**
   - Digite o nome da sua cidade
   - O programa carrega informações do clima

2. **Menu Principal:**
   ```
   ☀️ Clima atual em Santa Cruz do Sul: 22.5°C - céu limpo
   
   📋 MENU PRINCIPAL
   
   💬 Faça uma pergunta:
      Digite sua pergunta diretamente e pressione Enter
   
   🧹 Comandos especiais:
      🔸 limpar     - Limpa o histórico da conversa
      🔸 historico  - Mostra o histórico completo
      🔸 help       - Mostra ajuda e dicas
      🔸 0          - Sair do programa
   ```

3. **Fazer Perguntas:**
   ```
   Você: Como está o clima hoje?
   GenieC: Está ensolarado com 22°C em Santa Cruz do Sul...
   ```

4. **Ver Histórico:**
   ```
   Você: historico
   
   [Tela limpa e mostra todas as conversas]
   
   📜 HISTÓRICO DA CONVERSA
   
   👤 Você:
      Como está o clima hoje?
   
   🤖 GenieC:
      Está ensolarado com 22°C em Santa Cruz do Sul...
   
   ⏎ Pressione Enter para voltar ao chat...
   ```

---

## 🏗️ Arquitetura

### Estrutura de Diretórios

```
GenieC/
├── main.c                 # Ponto de entrada do programa
├── CMakeLists.txt         # Configuração do build
├── .env                   # Variáveis de ambiente (não commitado)
├── .env.example           # Template das variáveis
│
├── src/                   # Módulos do projeto
│   ├── clima.c/h          # Integração OpenWeatherMap
│   ├── gemini.c/h         # Integração Google Gemini
│   ├── historico.c/h      # Gerenciamento de histórico
│   ├── ui.c/h             # Interface do usuário
│   ├── http_utils.c/h     # Utilitários HTTP (cURL)
│   ├── env_loader.c/h     # Carregador de .env (dotenv-c)
│   └── config.h           # Configurações globais
│
├── limpar_tela.h          # Utilitário para limpar tela
├── dormir.h               # Utilitário para sleep
```

### Módulos Principais

#### 🌐 **clima.c**
- Requisições à API OpenWeatherMap
- Parse de dados meteorológicos
- Codificação de URLs

#### 🤖 **gemini.c**
- Integração com Google Gemini API
- Criação de payloads JSON
- Extração de respostas

#### 📚 **historico.c**
- Armazenamento de conversas
- Limite automático de turnos
- Exibição formatada

#### 🎨 **ui.c**
- Interface colorida com ANSI
- Ícones dinâmicos de clima
- Menus e arte ASCII

#### 🔧 **http_utils.c**
- Requisições HTTP com cURL
- Sistema de retry automático
- Timeouts configuráveis

#### 🔐 **env_loader.c**
- Wrapper para dotenv-c
- Busca em múltiplos diretórios
- Validação de chaves

### Fluxo de Dados

```
┌─────────────┐
│   main.c    │
└──────┬──────┘
       │
       ├─────► env_loader ──► Carrega .env
       │
       ├─────► clima ──► OpenWeatherMap API
       │         │
       │         └──► http_utils (cURL)
       │
       ├─────► ui ──► Exibe menu e clima
       │
       └─────► Loop principal
                 │
                 ├─► historico (gerencia conversas)
                 │
                 └─► gemini ──► Google Gemini API
                       │
                       └──► http_utils (cURL + retry)
```

---

## 🛠️ Tecnologias Utilizadas

| Tecnologia | Versão | Uso |
|------------|--------|-----|
| **C** | C99 | Linguagem principal |
| **CMake** | 3.21+ | Sistema de build |
| **cURL** | Latest | Requisições HTTP |
| **cJSON** | Latest | Parse de JSON |
| **dotenv-c** | Latest | Variáveis de ambiente |
| **Google Gemini** | API v1beta | IA conversacional |
| **OpenWeatherMap** | API 2.5 | Dados meteorológicos |

---

## 📖 Documentação Adicional

- 📘 [Configuração de API Keys](CONFIGURACAO_API.md)
- 📗 [Recursos do dotenv-c](DOTENV_FEATURES.md)
- 📙 [Arquitetura Modular](ARQUITETURA_MODULAR.md)
- 📕 [Análise de Código](ANALISE_CODIGO_COMPLETA.md)

---

## 🎓 Equipe de Desenvolvimento

<table>
  <tr>
    <td align="center">
      <b>Lorenzo Farias</b><br>
      <sub>Desenvolvedor</sub>
    </td>
    <td align="center">
      <b>Bernardo Soares Nunes</b><br>
      <sub>Desenvolvedor</sub>
    </td>
    <td align="center">
      <b>Pedro Cabral Buchaim</b><br>
      <sub>Desenvolvedor</sub>
    </td>
  </tr>
</table>

### 🏫 Instituição
**Universidade de Santa Cruz do Sul (UNISC)**

### 📚 Disciplina
Programação para Resolução de Problemas

### 👩‍🏫 Professora Orientadora
**Profa. Dra. Daniela Bagatini**

---

## 🤝 Contribuir

Contribuições são bem-vindas! Para contribuir:

1. Fork o projeto
2. Crie uma branch para sua feature (`git checkout -b feature/MinhaFeature`)
3. Commit suas mudanças (`git commit -m 'Adiciona MinhaFeature'`)
4. Push para a branch (`git push origin feature/MinhaFeature`)
5. Abra um Pull Request

### 📝 Diretrizes de Código

- Siga o padrão C99
- Comente código complexo
- Use nomes descritivos para variáveis
- Mantenha funções pequenas e focadas
- Teste suas alterações

---

## 🐛 Problemas Conhecidos

### Windows
- Codificação UTF-8 pode necessitar configuração do console
- Link estático pode aumentar o tamanho do executável

### Linux
- dotenv-c pode precisar ser compilado manualmente
- Algumas distribuições não têm cJSON nos repositórios

### Soluções
Consulte a [seção de Troubleshooting](CONFIGURACAO_API.md#-problemas-comuns) na documentação.

---

## 📜 Licença

Este projeto é licenciado sob a **MIT License** - veja o arquivo [LICENSE.txt](LICENSE.txt) para detalhes.

---

## 🙏 Agradecimentos

- **Google** - Pela API Gemini gratuita
- **OpenWeatherMap** - Pelos dados meteorológicos
- **Comunidade Open Source** - pelas bibliotecas cURL, cJSON e dotenv-c
- **UNISC** - Pelo suporte acadêmico

---

## 📞 Contato

Para dúvidas ou sugestões, entre em contato através dos canais da UNISC.

---

## 📦 Instalação

### Pré-requisitos

#### Windows (MSYS2/MinGW)
```bash
# Instalar ferramentas de build
pacman -S mingw-w64-x86_64-gcc
pacman -S mingw-w64-x86_64-cmake
pacman -S mingw-w64-x86_64-ninja

# Instalar bibliotecas necessárias
pacman -S mingw-w64-x86_64-curl
pacman -S mingw-w64-x86_64-cjson
pacman -S mingw-w64-x86_64-dotenv-c
```

#### Linux (Ubuntu/Debian)
```bash
# Instalar ferramentas de build
sudo apt update
sudo apt install build-essential cmake

# Instalar bibliotecas
sudo apt install libcurl4-openssl-dev
sudo apt install libcjson-dev

# Instalar dotenv-c (manual)
git clone https://github.com/Isty001/dotenv-c.git
cd dotenv-c
mkdir build && cd build
cmake ..
make
sudo make install
```

### 🔐 Configurar API Keys

1. **Copie o arquivo de exemplo:**
```bash
copy .env.example .env
```

2. **Obtenha suas chaves de API:**

   **Google Gemini API:**
    - Acesse: https://makersuite.google.com/app/apikey
    - Crie uma API key gratuita
    - Copie a chave gerada

   **OpenWeatherMap API:**
    - Acesse: https://openweathermap.org/api
    - Crie uma conta gratuita
    - Obtenha sua API key em "API Keys"

3. **Edite o arquivo `.env`:**
```env
GEMINI_API_KEY=sua_chave_gemini_aqui
OPENWEATHER_API_KEY=sua_chave_openweather_aqui
```

### 🏗️ Compilar o Projeto

```bash
# Configurar CMake
cmake -S . -B cmake-build-release -DCMAKE_BUILD_TYPE=Release

# Compilar
cmake --build cmake-build-release --config Release

# O executável estará em: cmake-build-release/GenieC.exe
```

---

<div align="center">

**Desenvolvido com ❤️ em C puro**

⭐ Se este projeto foi útil, considere dar uma estrela!

</div>
