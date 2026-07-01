
# FOUR NIGHTS AT ICMC

Nomes:
Gabriel Delatore de Brito &
Giovanni Mazzon Sacheto


Jogo de sobrevivência estilo *Five Nights at Freddy's*, escrito inteiramente em assembly (rodando numa VM/simulador próprio, sem bibliotecas externas).

## Sobre

Você é um segurança noturno trancado numa pizzaria com animatrônicos "meio vivos". O objetivo de cada noite é sobreviver até as 6h controlando portas, luzes e câmeras — sem deixar a energia acabar antes disso.

## Controles

| Tecla | Ação |
|---|---|
| `A` | Alterna porta esquerda |
| `D` | Alterna porta direita |
| `Q` | Luz esquerda |
| `E` | Luz direita |
| `1`–`4` | Câmeras 1 a 4 |
| `5` | Escritório |
| `R` | Confirmar / continuar |

## Inimigos

| Animatrônico | Ativo a partir de |
|---|---|
| Freddy | Noite 1 |
| Bonnie | Noite 2 |
| Chica | Noite 3 |
| Foxy | Noite 4 |

## Mecânica de energia

Cada ciclo do jogo soma quantas portas estão fechadas e quantas luzes estão acesas (0 a 4) para decidir a velocidade de consumo:

- **0** → consumo normal
- **1** → consumo rápido
- **2 ou mais** → consumo muito rápido (usando uma flag de controle para não descontar energia rápido demais de uma vez)

Se a energia chega a zero, é game over. Se o relógio chega às 6h, a noite é vencida e a próxima começa.

## Progressão

4 noites ao todo. Vencer a noite 4 encerra o jogo com vitória; a energia zerada ou ser pego por um animatrônico leva à tela de derrota, com opção de tentar de novo.

## Estrutura do código

Todo o jogo roda em um loop principal (`game_loop`) que alterna entre:
- `update` — atualiza IA dos animatrônicos, energia e relógio
- `render` — redesenha HUD, câmera atual e status das portas/luzes

Foi utilizada a ferramenta em python do gustavo 
link: https://github.com/GustavoSelhorstMarconi/Create-Screens-in-Assembly-with-python

## Atenção!
Para rodar o jogo, é necessario alterar o charmap do simulador, pelo charmap da PROJETO.zip. Sem isso, o jogo não ira rodar adequadamente.


# Função Debug para o Simulador do ICMC

## Lógica geral do Simulador:

O código funciona como um emulador de hardware em software baseado em uma Máquina de Estados. Ele imita o comportamento físico de um processador real (registradores, memória RAM, barramentos e unidade aritmética) utilizando variáveis e vetores em C. O coração do simulador roda dentro de um laço infinito (loop:), onde cada iteração representa um pulso de clock do sistema.

## As Etapas do Ciclo de Instrução (A Máquina de Estados):
O processador executa os programas sequencialmente alternando entre estados lógicos controlados pela variável state:

### 1. Inicialização (STATE_RESET): * É o ponto de partida (ou reinício) do processador.

Limpa todos os 8 registradores gerais (reg[0] a reg[7]) e o registrador de flags (FR).

Zera o contador de programa (PC = 0) e joga o ponteiro de pilha para o topo da memória (SP = 32767), deixando a RAM pronta para uso.

### 2. Busca (STATE_FETCH):

O processador usa o endereço guardado no PC para ler a instrução binária atual de 16 bits diretamente do vetor MEMORY.

Essa instrução é guardada no Registrador de Instrução (IR).

O PC é incrementado (PC++) para apontar para a próxima instrução do programa antes mesmo da atual ser executada.

### 3. Decodificação (STATE_DECODE):

O simulador extrai os 6 bits mais significativos do IR usando a função pega_pedaco. Esse pedaço é o Opcode (o código da operação, ex: 32 para ADD).

Um comando switch(opcode) gigante analisa o que a instrução pede.

Nesta etapa, o código descobre quais registradores serão afetados (extraindo os parâmetros de bits rx, ry e rz) e prepara os sinais de controle e multiplexadores. Se a instrução for simples (como operações lógicas/aritméticas ou MOV), ela é finalizada aqui e o estado volta para o STATE_FETCH.

### 4. Execução Avançada (STATE_EXECUTE e STATE_EXECUTE2):

Instruções complexas que exigem múltiplos ciclos de clock (como ler/escrever na memória em LOAD/STORE, manipulação da pilha em POP/RTS ou desvios de sub-rotinas em CALL) passam por este estado para completar o ciclo de dados antes de retornar à busca.

## Componentes Lógicos Centrais:

### A ULA (Unidade Lógica e Aritmética):

É a função responsável por todo o processamento matemático e lógico do simulador.

Ela recebe dois valores dos registradores (x e y), verifica a operação desejada (OP) e calcula o resultado bruto (soma, subtração, multiplicação, divisões, operações AND, OR, XOR, etc.).

A ULA também gera um número inteiro contendo as novas flags resultantes daquela conta (como indicar se o resultado deu zero, se houve sinal negativo ou estouro matemático).

### Multiplexadores Virtuais (selM1 a selM6):

Ao final de cada ciclo, o código simula circuitos multiplexadores físicos de hardware. Eles determinam, de acordo com a instrução, quais dados vão trafegar pelos barramentos (por exemplo, decidir se a memória RAM vai ler o endereço vindo do PC, do MAR ou do SP).

## O Sistema de Proteção e Diagnóstico (A Função Debug propriamente dita):
Logo no início de cada ciclo do clock (loop:), o código executa uma rotina contínua de monitoramento e auditoria do hardware virtual para capturar erros de software (do programa rodando em Assembly):

### Sensores de Falha Ativa:

Verifica se a ULA ativou a flag de divisão por zero.

Verifica se o ponteiro de pilha SP estourou os limites inferiores (SP < 0, Stack Overflow) ou superiores da memória devido a desempilhamentos inválidos (Stack Underflow).

Verifica se o endereço final de barramento M1 tenta ler ou escrever além do espaço físico da memória de 32KB (simulando um Segmentation Fault).

### Interrupção por pane:

Se qualquer um dos problemas acima acontecer, o simulador interrompe a execução e chama a função "pane".

Ela congela o sistema, usa a função "dedo_duro" para traduzir o Opcode numérico em uma string amigável (como "DIV" ou "PUSH") e joga na tela um relatório detalhado com a causa do crash, a linha exata em que o erro aconteceu (PC) e o conteúdo de todos os registradores para que o programador saiba exatamente onde corrigir o seu código em Assembly.

### O Breakpoint Manual (BREAKP):

O mapeamento do Opcode 14 funciona como uma parada programada. 

Sempre que o processador encontra essa instrução no meio do código, ele pausa o loop, exibe o PC atual e aguarda que o usuário pressione ENTER para prosseguir, permitindo analisar a execução passo a passo.

