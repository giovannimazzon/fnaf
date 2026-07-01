
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

O código funciona basicamente como um emulador do hardware do processador do ICMC em software. A lógica é integralmente controlada por uma máquina de estados em um laço infinito (loop:), onde cada iteração replica um ciclo de clock.

## As Etapas do Ciclo de Instrução:
:
O processador executa os programas sequencialmente alternando entre estados lógicos controlados pela variável state:

### 1. Inicialização (STATE_RESET): 

Limpa os registradores gerais (reg[0] a reg[7]) e o registrador de flags (FR). O PC reinicia (ou seja, volta para 0) e o ponteiro da pilha (SP) é inicializado no topo da memória (32767).

### 2. Busca (STATE_FETCH):

Busca a instrução de 16 bits na memória (MEMORY) usando a posição atual do PC, depois a armazena no IR e atualiza o PC (faz o PC++).

### 3. Decodificação (STATE_DECODE):

Essa função isola os 6 bits superiores do IR usando a função "pega_pedaco" para achar o Opcode. Depois disso, o switch(opcode) identifica quais registradores serão lidos ou escritos (rx, ry, rz). Caso seja uma instrução simples (tipo MOV ou lógica), ela já é executada nesta etapa e o estado volta pro FETCH.

### 4. Execução Avançada (STATE_EXECUTE e STATE_EXECUTE2):

Usados para comandos mais complexos, que gastam mais ciclos de clock para manipular dados ou memória (como LOAD, STORE, CALL e RTS).

## Componentes Principais:

### A ULA (Unidade Lógica e Aritmética):

Faz a parte matemática e lógica (soma, sub, and, or, cmp, etc.), ou seja, ela recebe os valores de entrada e devolve o resultado bruto junto com os bits empacotados que atualizam o registrador de flags (FR).

### Os Muxes Virtuais:

Controlam os barramentos (M1 a M6) e, a depender da instrução do switch, os seletores ativam o dado que deve passar (um exemplo disso seria decidir se o endereço da memória vem do PC ou do SP).

## O Sistema de Proteção e Diagnóstico (A Função Debug propriamente dita):

Logo no início de cada ciclo do clock (loop:), o código executa uma rotina contínua de monitoramento do hardware virtual para capturar possíveis erros de software do programa rodando em Assembly:

### Sensores de Falha:

Checa se a ULA ativou a flag de divisão por zero.

Verifica se o ponteiro de pilha SP estourou os limites inferiores (SP < 0, Stack Overflow) ou superiores da memória devido a desempilhamentos inválidos (Stack Underflow).

Confere se o endereço final de barramento M1 tenta ler ou escrever além do espaço físico da memória de 32KB (simulando um Segmentation Fault).

### Interrupção por pane:

Se qualquer um dos sensores for ativado, o simulador interrompe a execução do programa e chama a função "pane", que congela o sistema.

Após isso, a função "dedo_duro" é utilizada para traduzir o Opcode numérico no comando que causou o problema (como "DIV" ou "PUSH") e exibe na tela um relatório detalhado com a causa da falha, a linha exata em que o erro aconteceu (PC) e o conteúdo de todos os registradores para que o programador saiba exatamente o que e onde corrigir o seu código em Assembly.

### O Breakpoint Manual (BREAKP):

O mapeamento do Opcode 14 funciona como uma parada programada, ou seja, sempre que o processador encontra essa instrução no meio do código, ele pausa o loop, exibe o PC atual e aguarda que o usuário pressione ENTER para prosseguir, permitindo analisar a execução passo a passo.

