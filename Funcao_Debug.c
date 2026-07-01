// gcc simple_simulator_Template.c -O3 -march=native -o simulador -Wall -lm
// -lm is option to execute math.h library file.

#define TAMANHO_PALAVRA 16
#define TAMANHO_MEMORIA 32768
#define MAX_VAL 65535

// Estados do Processador
#define STATE_RESET 0
#define STATE_FETCH 1
#define STATE_DECODE 2
#define STATE_EXECUTE 3
#define STATE_EXECUTE2 4
#define STATE_HALTED 5
//----------------

// Selecao do Mux1
#define sPC 0
#define sMAR 1
#define sM4 2
#define sSP 3

// Selecao do Mux2
#define sULA 0
#define sDATA_OUT 1
#define sTECLADO 4

// Opcodes das Instrucoes:
#define LOAD 48       
#define STORE 49      
#define LOADN 56   
#define LOADI 60  
#define STOREI 61 
#define MOV	51        

// I/O Instructions:
#define OUTCHAR	50  
#define INCHAR 53   

// Aritmethic Instructions:
#define ARITH 2
#define ADD 32      
#define SUB 33      
#define MULT 34     
#define DIV	35      
#define INC	36      
#define LMOD 37     

// Logic Instructions:
#define LOGIC 1
#define LAND 18     
#define LOR 19      
#define LXOR 20     
#define LNOT 21     
#define SHIFT 16    
#define CMP 22      

// FLOW CONTROL Instructions:
#define JMP 2       
#define CALL 3      
#define RTS	4       
#define PUSH 5      
#define POP	6       

// Control Instructions:
#define NOP	0       
#define HALT 15     
#define SETC 8      
#define BREAKP 14   

// Flag register
#define NEGATIVE 9
#define STACK_UNDERFLOW 8
#define STACK_OVERFLOW 7
#define DIV_BY_ZERO 6
#define ARITHMETIC_OVERFLOW 5
#define CARRY 4
#define ZERO 3
#define EQUAL 2
#define LESSER 1
#define GREATER 0

#include <stdlib.h>     
#include <stdio.h>      
#include <fcntl.h>      
#include <math.h>
#include <termios.h>
#include <unistd.h>

unsigned int MEMORY[TAMANHO_MEMORIA]; 
int reg[8]; 

typedef struct _resultadoUla{
	unsigned int result;
	unsigned int auxFR;
} ResultadoUla;

void le_arquivo(void);
int processa_linha(char* linha); 
int pega_pedaco(int ir, int a, int b); 
unsigned int _rotl(const unsigned int value, int shift);
unsigned int _rotr(const unsigned int value, int shift);
ResultadoUla ULA(unsigned int x, unsigned int y, unsigned int OP, int carry);
int kbhit(void);

int FR[16] = {0};
  
// Sistema de Debug do Simulador do ICMC
// Retorna qual comando em Assembly estava sendo executado e causou o erro.
const char* dedo_duro(int op) {
	switch(op) {
		case LOAD:    return "LOAD";
		case STORE:   return "STORE";
		case LOADN:   return "LOADN";
		case LOADI:   return "LOADI";
		case STOREI:  return "STOREI";
		case MOV:     return "MOV";
		case OUTCHAR: return "OUTCHAR";
		case INCHAR:  return "INCHAR";
		case ADD:     return "ADD";
		case SUB:     return "SUB";
		case MULT:    return "MULT";
		case DIV:     return "DIV";
		case INC:     return "INC";
		case LMOD:    return "LMOD";
		case LAND:    return "LAND";
		case LOR:     return "LOR";
		case LXOR:    return "LXOR";
		case LNOT:    return "LNOT";
		case SHIFT:   return "SHIFT";
		case CMP:     return "CMP";
		case JMP:     return "JMP";
		case CALL:    return "CALL";
		case RTS:     return "RTS";
		case PUSH:    return "PUSH";
		case POP:     return "POP";
		case NOP:     return "NOP";
		case HALT:    return "HALT";
		case SETC:    return "SETC";
		case BREAKP:  return "BREAKP";
		default:      return "OPCODE_INVALIDO_OU_LIXO";
	}
}

// Emite um relatório de Crash de Hardware completo interrompendo o fluxo
void pane(const char* motivo, int pc_erro, int ir_erro, int *registradores, int *flags) {
	int op = (ir_erro >> 10) & 0x3F; // Pega os 6 bits superiores (Opcode)
	printf("ERRO\n");
	printf("Motivo do Erro:      %s\n", motivo);
	printf("Linha do Código (PC): %d (0x%04X)\n", pc_erro, pc_erro);
	printf("Comando Causador:    %s (Opcode: %d, IR: 0x%04X)\n", dedo_duro(op), op, ir_erro);
	printf("Estado dos Registradores:\n");
	for(int i = 0; i < 8; i++) {
		printf("  R%d: %5d (0x%04X)", i, registradores[i], registradores[i]);
		if(i % 2 != 0) printf("\n");
	}
	printf("Estado das Flags Principais:\n");
	printf("  ZERO:%d | CARRY:%d | EQUAL:%d | LESSER:%d | GREATER:%d\n", 
	       flags[ZERO], flags[CARRY], flags[EQUAL], flags[LESSER], flags[GREATER]);
	printf("  DIV_BY_ZERO:%d | NEGATIVE:%d | OVERFLOW:%d\n", 
	       flags[DIV_BY_ZERO], flags[NEGATIVE], flags[ARITHMETIC_OVERFLOW]);
	exit(1);
}

int main()
{
	int i=0;
	int key=0;    
	int PC=0, IR=0, SP=0, MAR=0, rx=0, ry=0, rz=0, COND=0, RW=0, DATA_OUT=0;
	int LoadPC=0, IncPC=0, LoadIR=0, LoadSP=0, IncSP=0, DecSP=0, LoadMAR=0, LoadFR=0;
	int M1=0, M2=0, M3=0, M4=0, M5=0, M6=0;
	int selM1=0, selM2=0, selM3=0, selM4=0, selM5=0, selM6=0;
	int LoadReg[8] = {0};
	int carry=0;
	int opcode=0;
	int temp=0;
	unsigned char state=0; 
	int OP=0;  
	int TECLADO;
	ResultadoUla resultadoUla;

	// Variável de debug para registrar a linha da instrução sob processamento
	int PC_atual = 0;

	le_arquivo();

inicio:
	printf("PROCESSADOR ICMC  - Menu:\n");
	printf("                          'r' goto inicio...\n");
	printf("                          'q' goto fim...\n\n");
	printf("Rodando...\n");

	state = STATE_RESET;

loop:
	// Sincronismo dos registradores (Borda de Clock)
	if(LoadIR) IR = DATA_OUT;
	if(LoadPC) PC = DATA_OUT;
	if(IncPC) PC++;
	if(LoadMAR) MAR = DATA_OUT;
	if(LoadSP) SP = M4;
	if(IncSP) SP++;
	if(DecSP) SP--;

	if(LoadFR)
		for(i=16; i--; )              
			FR[i] = pega_pedaco(M6,i,i); 

	rx = pega_pedaco(IR,9,7);
	ry = pega_pedaco(IR,6,4);
	rz = pega_pedaco(IR,3,1);
	
	if(LoadReg[rx]) reg[rx] = M2;
	if (RW == 1) MEMORY[M1] = M5;

	// Construindo os sensores de erro de Hardware (DEBUGGER CONTÍNUO)
	if (FR[DIV_BY_ZERO] == 1) {
		pane("Divisão por zero detectada em operação aritmética/módulo.", PC_atual, IR, reg, FR);
	}
	if (SP < 0) {
		FR[STACK_OVERFLOW] = 1;
		pane("Stack Overflow! A pilha ultrapassou os limites inferiores seguros.", PC_atual, IR, reg, FR);
	}
	if (SP >= TAMANHO_MEMORIA) {
		FR[STACK_UNDERFLOW] = 1;
		pane("Stack Underflow! Tentativa de desempilhar dados de uma pilha vazia.", PC_atual, IR, reg, FR);
	}

	// Reinicializa os Loads
	for(i=0;i<8;i++) LoadReg[i] = 0;
	RW = 0;
	LoadIR  = 0; LoadMAR = 0; LoadPC  = 0; IncPC   = 0;
	LoadSP  = 0; IncSP   = 0; DecSP   = 0; LoadFR  = 0;

	// Máquina de Estados de Controle
	switch(state)
	{
		case STATE_RESET:
			for(i=0;i<8;i++) { reg[i] = 0; LoadReg[i] = 0; }
			for(i=0;i<16;i++) FR[i] = 0;

			PC = 0;  
			IR = 0;
			MAR = 0;
			SP = TAMANHO_MEMORIA -1;
			RW = 0; DATA_OUT = 0;
			selM1 = sPC; selM2 = sDATA_OUT; selM3 = 0; selM4 = 0; selM5 = sM3; selM6 = sULA;

			state=STATE_FETCH;
			break;

		case STATE_FETCH:
			PC_atual = PC; // Salva o endereço original do comando antes de incrementar o PC
			selM1 = sPC;
			RW = 0;
			LoadIR = 1;
			IncPC = 1;
			state=STATE_DECODE;
			break;

		case STATE_DECODE:
			opcode = pega_pedaco(IR,15,10);

			switch(opcode){
				case INCHAR:
					if(kbhit()) TECLADO = getchar();
					else TECLADO = 255;

					TECLADO = pega_pedaco(TECLADO,7,0);
					selM2 = sTECLADO;
					LoadReg[rx] = 1;
					state=STATE_FETCH;
					break;

				case OUTCHAR:
					printf("%c", reg[rx]);
					state=STATE_FETCH;
					break;

				case LOADN:
					selM1 = sPC;
					RW = 0;
					selM2 = sDATA_OUT;
					LoadReg[rx] = 1;
					IncPC = 1;
					state=STATE_FETCH;
					break;

				case LOAD:
					selM1 = sPC;
					RW = 0;
					LoadMAR = 1;
					IncPC = 1;
					state=STATE_EXECUTE;
					break;

				case STORE:
					selM1 = sPC;
					RW = 0;
					LoadMAR = 1;
					IncPC = 1;
					state=STATE_EXECUTE;
					break;

				case LOADI:
					selM4 = ry;
					selM1 = sM4;
					RW = 0;
					selM2 = sDATA_OUT;
					LoadReg[rx] = 1;
					state=STATE_FETCH;
					break;

				case STOREI:
					selM4 = rx;
					selM1 = sM4;
					selM3 = ry;
					selM5 = sM3;
					RW = 1;
					state=STATE_FETCH;
					break;

				case MOV:
					selM4 = ry;
					selM2 = sM4;
					LoadReg[rx] = 1;
					state=STATE_FETCH;
					break;

				case ADD:
				case SUB:
				case MULT:
				case DIV:
				case LMOD:
				case LAND:
				case LOR:
				case LXOR:
				case LNOT:
					selM3 = ry;
					selM4 = rz;
					OP = opcode;
					carry = pega_pedaco(IR, 0, 0);
					selM2 = sULA;
					LoadReg[rx] = 1;
					selM6 = sULA;
					LoadFR = 1;
					state=STATE_FETCH;
					break;

				case INC:
					selM3 = rx;
					selM4 = 8; // Mux4 seleciona constante '1'
					if(pega_pedaco(IR, 6, 6) == 0) OP = ADD;
					else OP = SUB;
					carry = 0;
					selM2 = sULA;
					LoadReg[rx] = 1;
					selM6 = sULA;
					LoadFR = 1;
					state=STATE_FETCH;
					break;

				case CMP:   
					selM3 = rx;
					selM4 = ry;
					OP = CMP;
					carry = 0;
					selM6 = sULA;
					LoadFR = 1;
					state=STATE_FETCH;
					break;

				case SHIFT:
					switch(pega_pedaco(IR,6,4))
					{   case 0: reg[rx] = reg[rx] << pega_pedaco(IR,3,0);           break;
						case 1: reg[rx] = ~((~(reg[rx]) << pega_pedaco(IR,3,0)));   break;
						case 2: reg[rx] = reg[rx] >> pega_pedaco(IR,3,0);           break;
						case 3: reg[rx] = ~((~(reg[rx]) >> pega_pedaco(IR,3,0)));   break;
						default:
								if(pega_pedaco(IR,6,5)==2) 
									reg[rx] = _rotl(reg[rx],pega_pedaco(IR,3,0));
								else
									reg[rx] = _rotr(reg[rx],pega_pedaco(IR,3,0)); 
								break;
					}
					FR[3] = 0; 
					if(reg[rx] == 0) FR[3] = 1;  
					state=STATE_FETCH;
					break;

				case JMP:
					COND = pega_pedaco(IR,9,6);
					if((COND == 0)                       	                      
							|| (FR[0]==1 && (COND==7))                            
							|| ((FR[2]==1 || FR[0]==1) && (COND==9))              
							|| (FR[1]==1 && (COND==8))                            
							|| ((FR[2]==1 || FR[1]==1) && (COND==10))             
							|| (FR[2]==1 && (COND==1))                            
							|| (FR[2]==0 && (COND==2))                            
							|| (FR[3]==1 && (COND==3))                            
							|| (FR[3]==0 && (COND==4))                            
							|| (FR[4]==1 && (COND==5))                            
							|| (FR[4]==0 && (COND==6))                            
							|| (FR[5]==1 && (COND==11))                           
							|| (FR[5]==0 && (COND==12))                           
							|| (FR[6]==1 && (COND==14))                           
							|| (FR[9]==1 && (COND==13)))                          
					{ 
						selM1 = sPC;
						RW = 0;
						LoadPC = 1;
					}
					else IncPC = 1;
					state=STATE_FETCH;
					break;

				case CALL:
					selM1 = sSP;
					selM5 = sPC;
					RW = 1;
					DecSP = 1;
					state=STATE_EXECUTE;
					break;

				case PUSH:
					selM1 = sSP;
					if(pega_pedaco(IR, 6, 6) == 0) selM3 = rx;
					else selM3 = 8; // Passa FR convertido no Mux3
					selM5 = sM3;
					RW = 1;
					DecSP = 1;
					state=STATE_FETCH;
					break;

				case POP:
					IncSP = 1;
					state=STATE_EXECUTE;
					break;

				case RTS:
					IncSP = 1;
					state=STATE_EXECUTE;
					break;

				case SETC:
					FR[4] = pega_pedaco(IR,9,9);
					state=STATE_FETCH;
					break;

				case HALT:        
					state=STATE_HALTED;
					break;

				case NOP:         
					state=STATE_FETCH;
					break;

				case BREAKP: 
					printf("\n [BREAKPOINT] PC: %d | Apertando ENTER continua...\n", PC_atual);
					key = getchar(); 
					state=STATE_FETCH;
					break;

				default:
					// Dispara caso seja lido um opcode não mapeado
					pane("Opcode desconhecido ou instrução inválida. Possível execução de lixo na memória.", PC_atual, IR, reg, FR);
					state=STATE_FETCH;
					break;
			}
			break;

		case STATE_EXECUTE:
			switch(opcode){
				case LOAD:
					selM1 = sMAR;
					RW = 0;
					selM2 = sDATA_OUT;
					LoadReg[rx] = 1;
					state=STATE_FETCH;
					break;

				case STORE:
					selM1 = sMAR;
					selM3 = rx;
					selM5 = sM3;
					RW = 1;
					state=STATE_FETCH;
					break; 

				case CALL:
					selM1 = sPC;
					RW = 0;
					LoadPC = 1;
					state=STATE_FETCH;
					break; 

				case POP:
					selM1 = sSP;
					RW = 0;
					if(pega_pedaco(IR, 6, 6) == 0) {
						selM2 = sDATA_OUT;
						LoadReg[rx] = 1;
					} else {
						selM6 = sDATA_OUT;
						LoadFR = 1;
					}
					state=STATE_FETCH;
					break; 

				case RTS:
					selM1 = sSP;
					RW = 0;
					LoadPC = 1;
					state=STATE_EXECUTE2;
					break;

				case PUSH:
					state=STATE_FETCH;
					break;
			}
			break;

		case STATE_EXECUTE2:
			state=STATE_FETCH;
			break;

		case STATE_HALTED:
			printf("\n[Processador ICMC Parado (HALT)]\n");
			key = getchar();
			if (key == 'r') goto inicio;
			if (key == 'q') goto fim;
			break;

		default:
			break;
	}

	// Seleção dos Multiplexadores (HARDWARE COMBINATÓRIO)
	if(selM4 == 8) M4 = 1;  
	else M4 = reg[selM4]; 

	if      (selM1 == sPC)  M1 = PC;
	else if (selM1 == sMAR) M1 = MAR;
	else if (selM1 == sM4)  M1 = M4;
	else if (selM1 == sSP)  M1 = SP;

	// Sensor de Acesso Indevido de Memória (SEGMENTATION FAULT)
	if(M1 >= TAMANHO_MEMORIA || M1 < 0) {
		pane("Acesso indevido de leitura/escrita fora dos limites físicos da memória RAM.", PC_atual, IR, reg, FR);
	}

	if (RW == 0) DATA_OUT = MEMORY[M1];  

	temp = 0;
	for(i=16; i--; )        
		temp = temp + (int) (FR[i] * (pow(2.0,i))); 

	if(selM3 == 8) M3 = temp;  
	else M3 = reg[selM3]; 

	resultadoUla = ULA(M3, M4, OP, carry);

	if      (selM2 == sULA) M2 = resultadoUla.result;
	else if (selM2 == sDATA_OUT) M2 = DATA_OUT;
	else if (selM2 == sM4)  M2 = M4;
	else if (selM2 == sSP)  M2 = SP; 

	if (selM5 == sPC) M5 = PC;
	else if (selM5 == sM3) M5 = M3;

	if (selM6 == sULA) M6 = resultadoUla.auxFR;
	else if (selM6 == sDATA_OUT) M6 = DATA_OUT; 

	goto loop;

fim:
	return 0;
}

// Funções Auxiliares mantidas sem alteração do comportamento lógico básico:

void le_arquivo(void){
	FILE *stream;   
	int i, j;
	int processando = 0; 

	if ( (stream = fopen("cpuram.mif","r")) == NULL)  
	{
		printf("Erro: Arquivo cpuram.mif nao encontrado no diretorio atual!\n");
		exit(1);
	}

	char linha[110];
	j = 0;

	while (fscanf(stream,"%s", linha)!=EOF)   
	{
		char letra[2] = "00";
		if (!processando) {
			i = 0;
			do  {   
				letra[0] = letra[1];
				letra[1] = linha[i];
				if ((letra[0]=='0') && (letra[1]==':') )  
				{
					processando = 1;
					j = 0;
				}
				i++;
			} while (linha[i] != '\0');
		}

		if (processando && (j < TAMANHO_MEMORIA)) {
			MEMORY[j] = processa_linha(linha);
			if (MEMORY[j] == -1) {
				printf("Linha invalida (%d): '%s'\n", j, linha);
			}
			j++;
		}
	} 
	fclose(stream);  
}

int processa_linha(char* linha) {
	int i=0;
	int j=0;
	int valor=0;
	while (linha[i] != ':') {
		if (linha[i] == 0) return -1;
		i++;
	}
	valor = 0;
	for (j=0;j<16;j++) { 
		valor <<= 1; 
		valor += linha[i+j+1] - '0'; 
	}
	return valor;
}

int pega_pedaco(int ir, int a, int b) {
	int pedaco=0;
	pedaco = ((int)(pow(2.0, (a-b+1))) - 1);
	pedaco = pedaco & (ir >> b);
	return pedaco;
}

unsigned int _rotl(const unsigned int value, int shift) {
	if ((shift &= 16*8 - 1) == 0) return value;
	return (value << shift) | (value >> (16*8 - shift));
}

unsigned int _rotr(const unsigned int value, int shift) {
	if ((shift &= 16*8 - 1) == 0) return value;
	return (value >> shift) | (value << (16*8 - shift));
}

ResultadoUla ULA(unsigned int x, unsigned int y, unsigned int OP, int carry) {
	unsigned int auxFRbits[16]={0};
	unsigned int result = 0;

	switch(pega_pedaco(OP, 5, 4)) {
		case ARITH:
			switch(OP) {
				case ADD:
					if(carry==1) result = x+y+FR[CARRY];
					else result = x+y;
					if(result > MAX_VAL){
						auxFRbits[CARRY] = 1;
						result -= MAX_VAL;
					}else auxFRbits[CARRY] = 0;
					break;	
				case SUB:
					result = x-y;
					if((int)result < 0) auxFRbits[NEGATIVE] = 1;
					else auxFRbits[NEGATIVE] = 0;
					break;	
				case MULT:
					result = x*y;
					if(result > MAX_VAL) auxFRbits[ARITHMETIC_OVERFLOW] = 1;
					else auxFRbits[ARITHMETIC_OVERFLOW] = 0;
					break;	
				case DIV:
					if(y==0) {
						result = 0;
						auxFRbits[DIV_BY_ZERO] = 1;
					}else {
						result = x/y;
						auxFRbits[DIV_BY_ZERO] = 0;
					}
					break;	
				case LMOD:
					if(y==0) {
						result = 0;
						auxFRbits[DIV_BY_ZERO] = 1;
					}else {
						result = x%y;
						auxFRbits[DIV_BY_ZERO] = 0;
					}
					break;	
				default:
					result = x;
			}
			if(result==0) auxFRbits[ZERO] = 1;
			else auxFRbits[ZERO] = 0;
			break;

		case LOGIC:
			if(OP==CMP)
			{
				result = x;
				if(x>y){
					auxFRbits[GREATER] = 1; auxFRbits[LESSER] = 0; auxFRbits[EQUAL] = 0;
				}else if(x<y){
					auxFRbits[GREATER] = 0; auxFRbits[LESSER] = 1; auxFRbits[EQUAL] = 0;
				}else if(x==y){
					auxFRbits[GREATER] = 0; auxFRbits[LESSER] = 0; auxFRbits[EQUAL] = 1;
				}
			}else{
				switch(OP) {
					case LAND: result = x & y; break;
					case LXOR: result = x ^ y; break;
					case LOR:  result = x | y; break;
					case LNOT: result = ~x & MAX_VAL; break;
					default:   result = x;
				}
				if(result==0) auxFRbits[ZERO] = 1;
				else auxFRbits[ZERO] = 0;
			}
			break;
	}

	unsigned int auxFR = 0;
	for(int i=16; i--; )        
		auxFR = auxFR + (int) (auxFRbits[i] * (pow(2.0,i))); 

	ResultadoUla resultadoUla;
	resultadoUla.result = result;
	resultadoUla.auxFR = auxFR;
	return resultadoUla;
}

int kbhit(void) {
	struct termios oldt, newt;
	int ch, oldf;
	tcgetattr(STDIN_FILENO, &oldt);
	newt = oldt;
	newt.c_lflag &= ~(ICANON | ECHO);
	tcsetattr(STDIN_FILENO, TCSANOW, &newt);
	oldf = fcntl(STDIN_FILENO, F_GETFL, 0);
	fcntl(STDIN_FILENO, F_SETFL, oldf | O_NONBLOCK);
	ch = getchar();
	tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
	fcntl(STDIN_FILENO, F_SETFL, oldf);
	if(ch != EOF) {
		ungetc(ch, stdin);
		return 1;
	}
	return 0;
}