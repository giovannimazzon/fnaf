# FOUR NIGHTS AT ICMC
Nomes: Gabriel Delatore de Brito & Giovanni Mazzon Sacheto

Jogo de sobrevivência estilo Five Nights at Freddy's, escrito inteiramente em assembly (rodando numa VM/simulador próprio, sem bibliotecas externas).

Sobre
Você é um segurança noturno trancado numa pizzaria com animatrônicos "meio vivos". O objetivo de cada noite é sobreviver até as 6h controlando portas, luzes e câmeras — sem deixar a energia acabar antes disso.

Controles
Tecla	Ação
A	Alterna porta esquerda
D	Alterna porta direita
Q	Luz esquerda
E	Luz direita
1–4	Câmeras 1 a 4
5	Escritório
R	Confirmar / continuar
Inimigos
Animatrônico	Ativo a partir de
Freddy	Noite 1
Bonnie	Noite 2
Chica	Noite 3
Foxy	Noite 4
Mecânica de energia
Cada ciclo do jogo soma quantas portas estão fechadas e quantas luzes estão acesas (0 a 4) para decidir a velocidade de consumo:

0 → consumo normal
1 → consumo rápido
2 ou mais → consumo muito rápido (usando uma flag de controle para não descontar energia rápido demais de uma vez)
Se a energia chega a zero, é game over. Se o relógio chega às 6h, a noite é vencida e a próxima começa.

Progressão
4 noites ao todo. Vencer a noite 4 encerra o jogo com vitória; a energia zerada ou ser pego por um animatrônico leva à tela de derrota, com opção de tentar de novo.

Estrutura do código
Todo o jogo roda em um loop principal (game_loop) que alterna entre:

update — atualiza IA dos animatrônicos, energia e relógio
render — redesenha HUD, câmera atual e status das portas/luzes
Foi utilizada a ferramenta em python do gustavo link: https://github.com/GustavoSelhorstMarconi/Create-Screens-in-Assembly-with-python

!atenção Para rodar o jogo, é necessario alterar o charmap do simulador, pelo charmap da PROJETO.zip. Sem isso, o jogo não ira rodar adequadamente.
