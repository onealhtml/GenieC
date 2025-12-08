/* GenieC - JavaScript da Interface */

// Variável para controlar se a transição já foi iniciada
let transicaoIniciada = false;
let callbackClimaCarregado = null;
let painelGrafosAberto = false;

// ===== FUNÇÕES DO PAINEL DE GRAFOS =====

// Abre/fecha o painel lateral de grafos
function togglePainelGrafos() {
    const painel = document.getElementById('painel-grafos');
    painelGrafosAberto = !painelGrafosAberto;

    if (painelGrafosAberto) {
        painel.classList.add('aberto');
        // Atualiza estatísticas ao abrir
        atualizarEstatisticasGrafo();
    } else {
        painel.classList.remove('aberto');
    }
}

// Atualiza as estatísticas do grafo no painel
function atualizarEstatisticasGrafo() {
    console.log('Atualizando estatísticas do grafo...');
    window.rpc.call('grafo_estatisticas', {_method: 'grafo_estatisticas'});
}

// Callback chamado pelo backend com as estatísticas
function onEstatisticasGrafo(dados) {
    try {
        const stats = typeof dados === 'string' ? JSON.parse(dados) : dados;

        document.getElementById('stat-cidades').textContent = stats.cidades || 0;
        document.getElementById('stat-conexoes').textContent = stats.conexoes || 0;

        // Atualiza lista de cidades
        const listaCidades = document.getElementById('lista-cidades-grafo');

        if (stats.listaCidades && stats.listaCidades.length > 0) {
            listaCidades.innerHTML = stats.listaCidades.map(cidade => `
                <div class="cidade-item">
                    <span class="cidade-nome">${cidade.nome}</span>
                    <span class="cidade-conexoes">${cidade.conexoes} conexões</span>
                </div>
            `).join('');
        } else {
            listaCidades.innerHTML = '<p class="lista-vazia">Nenhuma cidade adicionada</p>';
        }
    } catch (e) {
        console.error('Erro ao processar estatísticas:', e);
    }
}

// Calcula a rota entre origem e destino
function calcularRotaGrafo() {
    const origem = document.getElementById('grafo-origem').value.trim();
    const destino = document.getElementById('grafo-destino').value.trim();

    if (!origem || !destino) {
        alert('Por favor, preencha origem e destino');
        return;
    }

    console.log('Calculando rota:', origem, '->', destino);

    // Fecha o painel para ver o resultado no chat
    togglePainelGrafos();

    // Mostra mensagem de processamento no chat
    adicionarMensagemHTML('Você', `🗺️ Calcular rota: <b>${origem}</b> → <b>${destino}</b>`, true);

    window.rpc.call('grafo_calcular_rota', {_method: 'grafo_calcular_rota', origem: origem, destino: destino});
}

// Visualiza o mapa completo do grafo
function visualizarMapaGrafo() {
    console.log('Visualizando mapa do grafo...');
    togglePainelGrafos();
    adicionarMensagemHTML('Você', '🗺️ Visualizar mapa do grafo', true);
    window.rpc.call('grafo_visualizar_mapa', {_method: 'grafo_visualizar_mapa'});
}

// Lista todas as cidades do grafo
function listarCidadesGrafo() {
    console.log('Listando cidades do grafo...');
    togglePainelGrafos();
    adicionarMensagemHTML('Você', '📋 Listar cidades do grafo', true);
    window.rpc.call('grafo_listar_cidades', {_method: 'grafo_listar_cidades'});
}

// Limpa o grafo
function limparGrafo() {
    if (confirm('⚠️ Tem certeza que deseja limpar todo o grafo?\n\nIsso removerá todas as cidades e conexões.')) {
        console.log('Limpando grafo...');
        window.rpc.call('grafo_limpar', {_method: 'grafo_limpar'});

        // Atualiza estatísticas
        setTimeout(() => {
            atualizarEstatisticasGrafo();
        }, 500);

        adicionarMensagemHTML('Sistema', '🗑️ Grafo limpo com sucesso!', false);
    }
}

// Salva o grafo em arquivo
function salvarGrafo() {
    console.log('Salvando grafo...');
    window.rpc.call('grafo_salvar', {_method: 'grafo_salvar'});
    adicionarMensagemHTML('Sistema', '💾 Grafo salvo com sucesso!', false);
}

// Função chamada pelo backend quando o clima é atualizado
function onClimaAtualizado(sucesso, mensagem) {
    console.log('Clima atualizado:', sucesso, mensagem);
    if (callbackClimaCarregado) {
        callbackClimaCarregado(sucesso, mensagem);
        callbackClimaCarregado = null;
    }
}

// Função para mostrar créditos
function mostrarCreditos() {
    const modal = document.getElementById('modal-creditos');
    modal.classList.add('show');
}

// Função para fechar créditos
function fecharCreditos() {
    const modal = document.getElementById('modal-creditos');
    const content = modal.querySelector('.modal-content');

    // Adiciona animação de saída
    modal.classList.add('hide');
    content.classList.add('slide-down');

    // Remove o modal após a animação
    setTimeout(() => {
        modal.classList.remove('show', 'hide');
        content.classList.remove('slide-down');
    }, 300);
}

// Fecha modal ao pressionar ESC
document.addEventListener('keydown', function(event) {
    if (event.key === 'Escape') {
        fecharCreditos();
    }
});

// Função para iniciar o app após inserir a cidade
function iniciarApp() {
    const input = document.getElementById('welcome-cidade-input');
    const cidade = input.value.trim();
    
    if (!cidade) {
        alert('Por favor, digite o nome de uma cidade');
        input.focus();
        return;
    }
    
    // Evita múltiplos cliques
    if (transicaoIniciada) {
        return;
    }
    
    console.log('Iniciando app com cidade:', cidade);
    
    // Desabilita o botão para evitar múltiplos cliques
    const btn = document.querySelector('.welcome-btn');
    btn.disabled = true;
    btn.textContent = 'Carregando...';
    transicaoIniciada = true;
    
    // Define callback para quando o clima for carregado
    callbackClimaCarregado = function(sucesso, mensagem) {
        if (sucesso) {
            // Clima carregado com sucesso - inicia transição
            console.log('Cidade válida, iniciando transição...');
            fazerTransicao();
        } else {
            // Erro ao carregar clima - cidade inválida
            console.log('Cidade inválida:', mensagem);
            btn.disabled = false;
            btn.textContent = 'Começar';
            transicaoIniciada = false;
            alert('❌ Cidade não encontrada. Por favor, verifique o nome e tente novamente.');
        }
    };

    // Timeout de segurança (10 segundos)
    setTimeout(() => {
        if (transicaoIniciada && callbackClimaCarregado) {
            callbackClimaCarregado = null;
            btn.disabled = false;
            btn.textContent = 'Começar';
            transicaoIniciada = false;
            alert('⏱️ Tempo esgotado. Verifique sua conexão com a internet e tente novamente.');
        }
    }, 10000);

    // Chama o backend para buscar o clima
    window.rpc.call('atualizar_clima', {cidade: cidade});
}

function fazerTransicao() {
    const welcomeScreen = document.getElementById('welcome-screen');
    const mainApp = document.getElementById('main-app');
    
    // Fade out da tela de boas-vindas
    welcomeScreen.classList.add('fade-out');
    
    // Após a animação, mostra a tela principal
    setTimeout(() => {
        welcomeScreen.style.display = 'none';
        mainApp.style.display = 'flex';
        
        // Fade in da tela principal
        setTimeout(() => {
            mainApp.classList.add('fade-in');
            mainApp.style.opacity = '1';
            
            // Deixa o campo de cidade VAZIO para o usuário saber onde trocar
            document.getElementById('cidade-input').value = '';
            document.getElementById('cidade-input').placeholder = 'Digite sua cidade...';
        }, 50);
    }, 500);
}

function adicionarMensagem(sender, text, isUser) {
    const container = document.getElementById('chat-messages');
    const msgDiv = document.createElement('div');
    msgDiv.className = 'message ' + (isUser ? 'user' : 'assistant');
    const bubble = document.createElement('div');
    bubble.className = 'message-bubble';
    const senderEl = document.createElement('div');
    senderEl.className = 'message-sender';
    senderEl.textContent = sender;
    const textEl = document.createElement('div');
    textEl.textContent = text;
    bubble.appendChild(senderEl);
    bubble.appendChild(textEl);
    msgDiv.appendChild(bubble);
    container.appendChild(msgDiv);
    container.parentElement.scrollTop = container.parentElement.scrollHeight;
}

function adicionarMensagemHTML(sender, html, isUser) {
    const container = document.getElementById('chat-messages');
    const msgDiv = document.createElement('div');
    msgDiv.className = 'message ' + (isUser ? 'user' : 'assistant');
    const bubble = document.createElement('div');
    bubble.className = 'message-bubble';

    // Detecta se a mensagem contém um mapa (id começando com 'mapa-')
    if (html.includes("id='mapa-") || html.includes('id="mapa-') || html.includes("id='mapa_") || html.includes('id="mapa_')) {
        bubble.classList.add('has-map');
    }

    const senderEl = document.createElement('div');
    senderEl.className = 'message-sender';
    senderEl.textContent = sender;
    const textEl = document.createElement('div');
    textEl.innerHTML = html;
    bubble.appendChild(senderEl);
    bubble.appendChild(textEl);
    msgDiv.appendChild(bubble);
    container.appendChild(msgDiv);
    
    // Aguarda um frame para garantir que o DOM foi atualizado
    requestAnimationFrame(() => {
        container.parentElement.scrollTop = container.parentElement.scrollHeight;
        
        // Processa scripts dentro do HTML (necessário para inicializar mapas)
        const scripts = textEl.querySelectorAll('script');
        scripts.forEach(script => {
            try {
                // Usa eval para executar o script no contexto global
                // Isso é necessário para que o Leaflet consiga acessar os elementos do DOM
                const scriptCode = script.textContent;
                console.log('Executando script do mapa...');
                eval(scriptCode);
                script.remove();
            } catch (e) {
                console.error('Erro ao executar script do mapa:', e);
            }
        });
    });
}

function enviarPergunta() {
    const input = document.getElementById('input-text');
    const text = input.value.trim();
    if (!text) return;
    console.log('Enviando pergunta:', text);
    adicionarMensagem('Você', text, true);
    window.rpc.call('pergunta', {text: text});
    input.value = '';
}

function atualizarClima() {
    const input = document.getElementById('cidade-input');
    const cidade = input.value.trim();
    if (!cidade) {
        alert('Por favor, digite o nome de uma cidade');
        return;
    }
    console.log('Atualizando clima para:', cidade);
    window.rpc.call('atualizar_clima', {cidade: cidade});
}

function limparChat() {
    if (confirm('Limpar histórico da conversa?')) {
        console.log('Limpando chat');
        window.rpc.call('limpar', {});
    }
}

// Verifica se o Leaflet está disponível
if (typeof L === 'undefined') {
    console.error('ERRO: Leaflet (L) não está carregado! Os mapas não funcionarão.');
} else {
    console.log('✓ Leaflet carregado com sucesso, versão:', L.version);
}

// Shim para garantir compatibilidade com diferentes webviews
console.log('Verificando window.rpc...');
if (!window.rpc || !window.rpc.call) {
    console.warn('window.rpc.call não disponível, criando fallback');
    window.rpc = window.rpc || {};
    window.rpc.call = function(method, params) {
        console.log('RPC fallback chamado:', method, params);
        var payload = JSON.stringify({method: method, params: params});
        if (window.external && window.external.invoke) {
            console.log('Usando window.external.invoke');
            window.external.invoke(payload);
        } else if (typeof window.rpc === 'function') {
            console.log('Usando window.rpc como função');
            window.rpc(payload);
        } else {
            console.error('Nenhum método RPC disponível!');
            alert('Erro: comunicação com backend não está funcionando');
        }
    };
} else {
    console.log('window.rpc.call está disponível');
}