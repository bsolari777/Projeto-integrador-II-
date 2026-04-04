#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Struct para armazenar instruções em binário (16 bits + '\0')
struct memoria_instrucao{
    char bits[17];
};

// Struct para armazenar memoria de dados em binário (16 bits + '\0')
struct memoria_dados{
    char dados[17];
};

// Struct para armazenar o estado da CPU/memória em cada passo
struct estado{
    int reg[8];        
    int memdados[256]; 
    int pc;            
};


// assinatura das funções a serem utilizadas no código
void executarinstrucao(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc);
void executarstep(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc, struct estado historico[], int *topo);
void executarback(int reg[], int memdados[], int *pc, struct estado historico[], int *topo);
int binario_para_int(char *bits);
int ULA(int op, int a, int b);
void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr);
void carregar_mem_dados(struct memoria_dados data[], int *qtd_dados);
void imprimir_memorias(struct memoria_instrucao mem[], struct memoria_dados data[], int qtd_instr, int qtd_dados);
void imprimir_tudo(struct memoria_instrucao mem[], int qtd_instr, int memdados[], int reg[], int pc);
void salvar_dat(int memdados[], int tamanho);
void salvar_asm(struct memoria_instrucao mem[], int qtd_instr);

int main(){

    struct memoria_instrucao mem[256]; // Memória de instruções
    struct memoria_dados data[256];    // Memória de dados

    int reg[8] = {10,10,10,10,10,10,10,10}; 
    int memdados[256] = {0};                

    struct estado historico[1000]; 
    int topo = -1;                  

    int pc = 0;          // Program counter inicial
    int qtd_instr = 0;   // Quantidade de instruções carregadas
    int qtd_dados = 0;   // Quantidade de dados carregados
    int opcao;           // Opção do menu

    do{
        printf("\n---- MENU ----\n");
        printf("1 - Carregar memoria de instrucoes\n");
        printf("2 - Carregar memoria de dados\n");
        printf("3 - Imprimir memorias\n");
        printf("4 - Imprimir registradores\n");
        printf("5 - Imprimir todo simulador\n");
        printf("6 - Salvar .asm\n");
        printf("7 - Salvar .dat\n");
        printf("8 - Executar programa (run)\n");
        printf("9 - Step\n");
        printf("10 - Back\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);

        switch(opcao){

            case 1: 
                carregar_mem_instr(mem, &qtd_instr); // Carrega instruções
                break;
            
            case 2: 
                carregar_mem_dados(data, &qtd_dados); // Carrega dados
                break;
            
            case 3:
                imprimir_memorias(mem, data, qtd_instr, qtd_dados); // Mostra memórias
                break;

            case 4:
                printf("\n---- REGISTRADORES ----\n");
                for(int i=0;i<8;i++)
                    printf("R%d = %d\n", i, reg[i]); // Mostra registradores
                break;
			case 5:
			imprimir_tudo(mem, qtd_instr, memdados, reg, pc);
			break;
			
			
			case 6:
				salvar_asm(mem, qtd_instr);
				break;
			
			case 7:
			salvar_dat(memdados,256);
			break;

            case 8:
                executarinstrucao(mem, qtd_instr, reg, memdados, &pc); // Executa programa todo
                break;

            case 9:
                executarstep(mem, qtd_instr, reg, memdados, &pc, historico, &topo); // Executa uma instrução (step)
                break;

            case 10:
                executarback(reg, memdados, &pc, historico, &topo); // Volta instrução (back)
                break;

            case 0:
                printf("Saindo...\n"); // Encerra programa
                break;

            default:
                printf("Opcao nao implementada\n"); // Opção inválida
        }

    }while(opcao != 0);

    return 0;
}

void executarinstrucao(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc){

    while(*pc < qtd_instr){ 

        char *instr = mem[*pc].bits;  
        char opcode1[5];
        
        strncpy(opcode1, instr, 4); 
        opcode1[4] = '\0';
        
        int opcode2 = binario_para_int(opcode1); 

        printf("\ninstrucao do pc = %d | %s\n", *pc, instr);

        // -------- TIPO R --------
        if(opcode2 == 0){
            char rs1[4], rt1[4], rd1[4], funct1[4];

           
            strncpy(rs1, instr+4, 3); rs1[3]='\0';
            strncpy(rt1, instr+7, 3); rt1[3]='\0';
            strncpy(rd1, instr+10, 3); rd1[3]='\0';
            strncpy(funct1, instr+13, 3); funct1[3]='\0';

            int rs2 = binario_para_int(rs1);
            int rt2 = binario_para_int(rt1);
            int rd2 = binario_para_int(rd1);
            int funct2 = binario_para_int(funct1);

            reg[rd2] = ULA(funct2, reg[rs2], reg[rt2]); 
        }

        // -------- TIPO I --------
        else if(opcode2 == 1 || opcode2 == 2 || opcode2 == 3 || opcode2 == 4 || opcode2 == 5){
            char rs1[4], rt1[4], imediato1[7];

            strncpy(rs1, instr+4, 3); rs1[3]='\0';
            strncpy(rt1, instr+7, 3); rt1[3]='\0';
            strncpy(imediato1, instr+10, 6); imediato1[6]='\0';

            int rs2 = binario_para_int(rs1);
            int rt2 = binario_para_int(rt1);
            int imediato2 = binario_para_int(imediato1);

            if(opcode2 == 1) // addi
                reg[rt2] = reg[rs2] + imediato2;
            else if(opcode2 == 2) // subi
                reg[rt2] = reg[rs2] - imediato2;
            else if(opcode2 == 3) // lw
                reg[rt2] = memdados[reg[rs2] + imediato2];
            else if(opcode2 == 4) // sw
                memdados[reg[rs2] + imediato2] = reg[rt2];
            else if(opcode2 == 5){ // BEQ
                if(reg[rs2] == reg[rt2]){
                    *pc = *pc + imediato2; // Salta se iguais
                }
            }
        }

        // -------- TIPO J --------
        else if(opcode2 == 8){
            char addr_s[13];
            strncpy(addr_s, instr+4, 12); // Endereço do jump
            addr_s[12] = '\0';

            int addr = binario_para_int(addr_s);

            *pc = addr - 1; // Ajusta PC
        }

        else {
            printf("instrucao invalida !"); 
        }

        (*pc)++; 
    }
}

// ----------- executar instrucao uma de cada vez -----------
void executarstep(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc, struct estado historico[], int *topo){

    if(*pc >= qtd_instr){ 
        printf("Fim do programa\n");
        return;
    }

    if(*topo >= 999){ // Limite do histórico
        printf("Limite do historico atingido\n");
        return;
    }

    (*topo)++; // Avança topo do histórico

    historico[*topo].pc = *pc; // Salva PC atual

    for(int i=0;i<8;i++)
        historico[*topo].reg[i] = reg[i]; // Salva registradores

    for(int i=0;i<256;i++)
        historico[*topo].memdados[i] = memdados[i]; // Salva memória de dados

    executarinstrucao(mem, *pc + 1, reg, memdados, pc); // Executa próxima instrução
}

// 			voltar instrucao executada 
void executarback(int reg[], int memdados[], int *pc, struct estado historico[], int *topo){

    if(*topo < 0){ 
        printf("Não tem ninuhma instrucao para voltar\n");
        return;
    }

    *pc = historico[*topo].pc; // Recupera PC do histórico

    for(int i=0;i<8;i++)
        reg[i] = historico[*topo].reg[i]; // Recupera registradores

    for(int i=0;i<256;i++)
        memdados[i] = historico[*topo].memdados[i]; // Recupera memória de dados

    (*topo)--; // Diminui topo do histórico

    printf("Voltou para PC = %d\n", *pc);
}

// ----------- ULA -----------
int ULA(int op, int a, int b){
    switch(op){
        case 0: return a + b; // soma
        case 1: return a - b; // subtração
        case 2: return a & b; // AND
        case 3: return a | b; // OR
        default: return 0;    // default
    }
}

int binario_para_int(char *bits){

    int valor = 0;
    int tamanho = strlen(bits);
    int negativo = 0;
    
    if(bits[0] == '1'){ // Se o bit mais significativo for 1, considera negativo
        negativo = 1;
    }

    for(int i=0;i<tamanho;i++)
        valor = valor * 2 + (bits[i]-'0'); // Converte binário para inteiro

    if(negativo==1){
        valor = valor - (1 << tamanho); // Ajusta valor negativo (2's complement)
    }
    
    return valor;
}

void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr){

    FILE *f = fopen("instrucoes.mem", "r"); 
    char linha[20];

    if(f==NULL){ 
        printf("Erro ao abrir arquivo\n");
        return;
    }

    int i=0;
    while(fgets(linha, sizeof(linha), f) && i<256){ // Lê linha por linha
        linha[strcspn(linha,"\n")] = '\0'; // Remove '\n'
        strcpy(mem[i].bits, linha);        // Salva na memória
        i++;
    }

    *qtd_instr = i; // Atualiza quantidade de instruções
    fclose(f);      // Fecha arquivo
}

// ----------- CARREGAR DADOS -----------
void carregar_mem_dados(struct memoria_dados data[], int *qtd_dados){

    FILE *f = fopen("dados.dat", "r"); // Abre arquivo de dados
    char linha[20];

    if(f==NULL){ // Verifica erro
        printf("Erro ao abrir arquivo\n");
        return;
    }

    int i=0;
    while(fgets(linha, sizeof(linha), f) && i<256){ // Lê linha por linha
        linha[strcspn(linha,"\n")] = '\0'; // Remove '\n'
        strcpy(data[i].dados, linha);      // Salva na memória
        i++;
    }

    *qtd_dados = i; // Atualiza quantidade de dados
    fclose(f);      // Fecha arquivo
}

// ----------- PRINT MEMORIAS -----------
void imprimir_memorias(struct memoria_instrucao mem[], struct memoria_dados data[], int qtd_instr, int qtd_dados){

    printf("\n---- INSTRUCOES ----\n");
    for(int i=0;i<qtd_instr;i++)
        printf("%d: %s\n", i, mem[i].bits); // Mostra instruções

    printf("\n---- DADOS ----\n");
    for(int i=0;i<qtd_dados;i++)
        printf("%d: %s\n", i, data[i].dados); // Mostra dados
}






void salvar_asm(struct memoria_instrucao mem[], int qtd_instr){

    FILE *f = fopen("saida.asm", "w");

    for(int i=0; i<qtd_instr; i++){
       
        char *instr = mem[i].bits;

        char opcode1[5];
        strncpy(opcode1, instr, 4);
        opcode1[4] = '\0';

        int opcode2 = binario_para_int(opcode1);

        // -------- TIPO R --------
        if(opcode2 == 0){

            char rs1[4], rt1[4], rd1[4], funct1[4];

            strncpy(rs1, instr+4, 3); rs1[3]='\0';
            strncpy(rt1, instr+7, 3); rt1[3]='\0';
            strncpy(rd1, instr+10, 3); rd1[3]='\0';
            strncpy(funct1, instr+13, 3); funct1[3]='\0';

            int rs2 = binario_para_int(rs1);
            int rt2 = binario_para_int(rt1);
            int rd2 = binario_para_int(rd1);
            int funct2 = binario_para_int(funct1);

            if(funct2 == 0)
                fprintf(f, "add $t%d, $t%d, $t%d\n", rd2, rs2, rt2);

            else if(funct2 == 1)
                  fprintf(f, "sub $t%d, $t%d, $t%d\n", rd2, rs2, rt2);

            else if(funct2 == 2)
                 fprintf(f, "and $t%d, $t%d, $t%d\n", rd2, rs2, rt2);

            else if(funct2 == 3)
                   fprintf(f, "or $t%d, $t%d, $t%d\n", rd2, rs2, rt2);
        }

        // -------- TIPO I --------
        else if(opcode2 >= 1 && opcode2 <= 5){

            char rs1[4], rt1[4], imediato1[7];

            strncpy(rs1, instr+4, 3); rs1[3]='\0';
            strncpy(rt1, instr+7, 3); rt1[3]='\0';
            strncpy(imediato1, instr+10, 6); imediato1[6]='\0';

            int rs2 = binario_para_int(rs1);
            int rt2 = binario_para_int(rt1);
            int imediato2 = binario_para_int(imediato1);

            if(opcode2 == 1)
               fprintf(f, "addi $t%d, $t%d, %d\n", rt2, rs2, imediato2);

            else if(opcode2 == 2)
              fprintf(f, "subi $t%d, $t%d, %d\n", rt2, rs2, imediato2);

			else if(opcode2 == 3)
			fprintf(f, "lw $t%d, %d($t%d)\n", rt2, imediato2, rs2);

			else if(opcode2 == 4)
			fprintf(f, "sw $t%d, %d($t%d)\n", rt2, imediato2, rs2);
           
            else if(opcode2 == 5)
            fprintf(f, "beq $t%d, $t%d, %d\n", rs2, rt2, imediato2);
        }

        // -------- TIPO J --------
        else if(opcode2 == 8){

            char addr_s[13];
            strncpy(addr_s, instr+4, 12);
            addr_s[12] = '\0';

            int addr = binario_para_int(addr_s);

            fprintf(f, "j %d\n", addr);
        }

        else{
            fprintf(f, "instrucao_invalida\n");
        }
    }

    fclose(f);
}




void salvar_dat(int memdados[], int tamanho){

    FILE *f = fopen("saida.dat", "w");

    for(int i = 0; i < tamanho; i++){
        fprintf(f, "%d\n", memdados[i]);
    }

    fclose(f);
}




void imprimir_tudo(struct memoria_instrucao mem[], int qtd_instr, int memdados[], int reg[], int pc){

    printf("\n.. ESTADO DO SIMULADOR ..n");

    printf("\nPC = %d\n", pc);

    printf("\n.. REGISTRADORES ..\n");
    for(int i=0; i<8; i++)
        printf("R%d = %d\n", i, reg[i]);

    printf("\n---- MEMORIA DE INSTRUCOES ----\n");
    for(int i=0; i<qtd_instr; i++)
        printf("%d: %s\n", i, mem[i].bits);

    printf("\n---- MEMORIA DE DADOS ----\n");
    for(int i=0; i<256; i++)
        printf("%d: %d\n", i, memdados[i]);
}
