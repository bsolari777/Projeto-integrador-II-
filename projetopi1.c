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

// Struct para armazenar o estado da ULA

struct resultado_ula {
    int resultado;
    int zero;
    int overflow;
    int negativo;
};
 
// assinatura das funções a serem utilizadas no código
void executarinstrucao(struct memoria_instrucao mem[], int contador, int reg[], int memdados[], int *pc);
void executarstep(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc, struct estado historico[], int *topo);
void executarback(int reg[], int memdados[], int *pc, struct estado historico[], int *topo);
int binario_para_int(char *bits);
void ULA(int funct, int a, int b, struct resultado_ula *r);
void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr);
void carregar_mem_dados(struct memoria_dados data[], int *qtd_dados);
void imprimir_memorias(struct memoria_instrucao mem[], struct memoria_dados data[], int qtd_instr, int qtd_dados);
void imprimir_tudo(struct memoria_instrucao mem[], int qtd_instr, int memdados[], int reg[], int pc);
void salvar_dat(int memdados[], int tamanho);
void salvar_asm(struct memoria_instrucao mem[], int qtd_instr);
void inserir_memoria(int memdados[]);
void inserir_registrador(int reg[]);


int main(){
 
    struct memoria_instrucao mem[256]; // Memória de instruções
    struct memoria_dados data[256];    // Memória de dados
 
    int reg[8] = {0}; 
    int memdados[256] = {0};                
 
    struct estado historico[256]; 
    int topo = -1;                  
 
    int pc = 0,pc_executar;          // Program counter inicial
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
        printf("11 - Inserir valor em registrador\n");
        printf("12 - Inserir valor na memoria de dados\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
 
        switch(opcao){
 
            case 1: 
                carregar_mem_instr(mem, &qtd_instr); // Carrega instruções
                break;
            
            case 2: 
                carregar_mem_dados(data, &qtd_dados); // Carrega dados
                for(int i = 0; i < qtd_dados; i++){
                memdados[i] = binario_para_int(data[i].dados);
                }
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
                pc_executar = pc - pc;
                executarinstrucao(mem, 256, reg, memdados, &pc_executar); // Executa programa todo
                break;
 
            case 9:
                executarstep(mem, qtd_instr, reg, memdados, &pc, historico, &topo); // Executa uma instrução (step)
                break;
 
            case 10:
                executarback(reg, memdados, &pc, historico, &topo); // Volta instrução (back)
                break;
            case 11:
                inserir_registrador(reg);
            break;

            case 12:
                inserir_memoria(memdados);
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
 
void executarinstrucao(struct memoria_instrucao mem[], int contador, int reg[], int memdados[], int *pc){
 
    struct resultado_ula r;
 
    while(*pc < contador){ 
 
        char *instr = mem[*pc].bits;  
        
        if(strcmp(instr, "0") == 0){
            (*pc)++;
            continue;
        }        
        
        char opcode1[5];
        strncpy(opcode1, instr, 4); 
        opcode1[4] = '\0';
        
        int opcode2 = binario_para_int(opcode1); 
 
        printf("\ninstrucao do pc = %d | %s\n", *pc, instr);
 
        // -------- TIPO R --------
        // opcode 0000 = 0
        if(opcode2 == 0){
            char rs1[4], rt1[4], rd1[4], funct1[4];
 
            strncpy(rs1,   instr+4, 3); rs1[3]   = '\0';
            strncpy(rt1,   instr+7, 3); rt1[3]   = '\0';
            strncpy(rd1,   instr+10, 3); rd1[3]  = '\0';
            strncpy(funct1, instr+13, 3); funct1[3] = '\0';
 
            int rs2    = binario_para_int(rs1);
            int rt2    = binario_para_int(rt1);
            int rd2    = binario_para_int(rd1);
            int funct2 = binario_para_int(funct1);
 
            // funct: 000=ADD, 010=SUB, 100=AND, 101=OR
            // mapeamento para ULA: 0=add, 1=sub, 2=and, 3=or
            int ula_op;
            if(funct2 == 0)      ula_op = 0; // ADD
            else if(funct2 == 2) ula_op = 1; // SUB
            else if(funct2 == 4) ula_op = 2; // AND
            else if(funct2 == 5) ula_op = 3; // OR
            else                 ula_op = 0; // default
 

            ULA(ula_op, reg[rs2], reg[rt2], &r);
            reg[rd2] = r.resultado;
        
            
        }
 
        // -------- TIPO I --------
        // opcodes conforme tabela do PDF:
        //   0100 (4)  = ADDI
        //   1011 (11) = LW
        //   1111 (15) = SW
        //   1000 (8)  = BEQ
      else if(opcode2 == 4 || opcode2 == 11 || opcode2 == 15 || opcode2 == 8){

    char rs1[4], rt1[4], imediato1[7];

    strncpy(rs1, instr+4, 3); rs1[3] = '\0';
    strncpy(rt1, instr+7, 3); rt1[3] = '\0';
    strncpy(imediato1, instr+10, 6); imediato1[6] = '\0';

    int rs2 = binario_para_int(rs1);
    int rt2 = binario_para_int(rt1);
    int imediato2 = binario_para_int(imediato1);

    int addr;

    if(opcode2 == 4){
        ULA(0, reg[rs2], imediato2, &r);
        reg[rt2] = r.resultado;
    }

    else if(opcode2 == 11){
        ULA(0, reg[rs2], imediato2, &r);
        addr = r.resultado;

        if(addr >= 0 && addr < 256)
            reg[rt2] = memdados[addr];
    }

    else if(opcode2 == 15){
        ULA(0, reg[rs2], imediato2, &r);
        addr = r.resultado;

        if(addr >= 0 && addr < 256)
            memdados[addr] = reg[rt2];
    }

    else if(opcode2 == 8){
        ULA(1, reg[rs2], reg[rt2], &r);

        if(r.zero == 1){
            *pc = *pc + imediato2 - 1;
        }
    }
}
 
        // -------- TIPO J --------
        // opcode 0010 (2) = JUMP
        else if(opcode2 == 2){
            char addr_s[13];
            strncpy(addr_s, instr+4, 12); // 12 bits de endereço
            addr_s[12] = '\0';
 
            // Para Jump, o endereço é sempre positivo (0-255)
            // usamos conversão sem sinal (os 8 bits menos significativos)
            int addr = 0;
            int tamanho = strlen(addr_s);
            for(int i = 0; i < tamanho; i++){
                addr = addr * 2 + (addr_s[i] == '1' ? 1 : 0);
            }
 
            *pc = addr - 1; // -1 porque o (*pc)++ vai incrementar depois
        }
 
        else {
            printf("instrucao invalida! opcode = %d\n", opcode2); 
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
void ULA(int funct, int a, int b, struct resultado_ula *r){

    int resultado;

    switch(funct){
        case 0:
            resultado = a + b;
            r->overflow = ((a > 0 && b > 0 && resultado < 0) ||
                           (a < 0 && b < 0 && resultado > 0));
            break;

        case 1:
            resultado = a - b;
            r->overflow = ((a > 0 && b < 0 && resultado < 0) ||
                           (a < 0 && b > 0 && resultado > 0));
            break;

        case 2:
            resultado = a & b;
            r->overflow = 0;
            break;

        case 3:
            resultado = a | b;
            r->overflow = 0;
            break;

        default:
            resultado = 0;
            r->overflow = 0;
    }

    r->resultado = resultado;
    r->zero = (resultado == 0);
    r->negativo = (resultado < 0);
}
 
int binario_para_int(char *bits){
 
    int valor = 0;
    int tamanho = strlen(bits);
    int negativo = 0;
    
    // Verifica se é negativo (bit mais significativo = 1)
    if(bits[0] == '1'){
        negativo = 1;
    }
 
    for(int i = 0; i < tamanho; i++){
        
        if(bits[i] == '1'){
            valor = valor * 2 + 1;  // adiciona 1
        }else{
            valor = valor * 2;      // adiciona 0
        }
    }
    
    // Ajuste para número negativo (complemento de 2)
    if(negativo == 1){
        valor = valor - (1 << tamanho);
    }
    
    return valor;
}
 
void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr){
 
    char nome_arquivo[100];
 
    printf("digite o nome do arquivo de instrucoes (ex: instrucoes.mem): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
 
    FILE *f = fopen(nome_arquivo, "r"); 
    char linha[20];
 
    if(f==NULL){ 
        printf("Erro ao abrir arquivo\n");
        return;
    }
 
 
   for(int i = 0; i < 256; i++){
        strcpy(mem[i].bits, "0\0");
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
 
    char nome_arquivo[100];
 
    printf("digite o nome do arquivo de dados (ex: dados.dat): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';  
  
  
  
    FILE *f = fopen(nome_arquivo, "r"); // Abre arquivo de dados
    char linha[20];
 
    if(f==NULL){ // Verifica erro
        printf("Erro ao abrir arquivo\n");
        return;
    }
 
 
    for(int i = 0; i < 256; i++){
        strcpy(data[i].dados, "0\0");
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
    for(int i=0;i<256;i++)
        printf("%d: %s\n", i, mem[i].bits); // Mostra instruções
 
    printf("\n---- DADOS ----\n");
    for(int i=0;i<256;i++)
        printf("%d: %s\n", i, data[i].dados); // Mostra dados
}
 
 
 
void salvar_asm(struct memoria_instrucao mem[], int qtd_instr){
 
    char nome_arquivo[100];
 
    printf("digite o nome do arquivo de saida de .asm (ex: instrucoes.asm): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
 
    FILE *f = fopen(nome_arquivo, "w");
 
    for(int i=0; i<qtd_instr; i++){
       
        char *instr = mem[i].bits;
 
        char opcode1[5];
        strncpy(opcode1, instr, 4);
        opcode1[4] = '\0';
 
        int opcode2 = binario_para_int(opcode1);
 
        // -------- TIPO R --------
        // opcode 0000 (0): ADD, SUB, AND, OR — diferenciados pelo funct
        if(opcode2 == 0){
 
            char rs1[4], rt1[4], rd1[4], funct1[4];
 
            strncpy(rs1,    instr+4,  3); rs1[3]    = '\0';
            strncpy(rt1,    instr+7,  3); rt1[3]    = '\0';
            strncpy(rd1,    instr+10, 3); rd1[3]    = '\0';
            strncpy(funct1, instr+13, 3); funct1[3] = '\0';
 
            int rs2    = binario_para_int(rs1);
            int rt2    = binario_para_int(rt1);
            int rd2    = binario_para_int(rd1);
            int funct2 = binario_para_int(funct1);
 
            // funct: 000=ADD, 010=SUB, 100=AND, 101=OR
            if(funct2 == 0)
                fprintf(f, "add $t%d, $t%d, $t%d\n", rd2, rs2, rt2);
            else if(funct2 == 2)
                fprintf(f, "sub $t%d, $t%d, $t%d\n", rd2, rs2, rt2);
            else if(funct2 == 4)
                fprintf(f, "and $t%d, $t%d, $t%d\n", rd2, rs2, rt2);
            else if(funct2 == 5)
                fprintf(f, "or $t%d, $t%d, $t%d\n",  rd2, rs2, rt2);
            else
                fprintf(f, "instrucao_invalida\n");
        }
 
        // -------- TIPO I --------
        // opcodes: 4=ADDI, 11=LW, 15=SW, 8=BEQ
        else if(opcode2 == 4 || opcode2 == 11 || opcode2 == 15 || opcode2 == 8){
 
            char rs1[4], rt1[4], imediato1[7];
 
            strncpy(rs1,       instr+4,  3); rs1[3]       = '\0';
            strncpy(rt1,       instr+7,  3); rt1[3]       = '\0';
            strncpy(imediato1, instr+10, 6); imediato1[6] = '\0';
 
            int rs2       = binario_para_int(rs1);
            int rt2       = binario_para_int(rt1);
            int imediato2 = binario_para_int(imediato1);
 
            if(opcode2 == 4)
                fprintf(f, "addi $t%d, $t%d, %d\n", rt2, rs2, imediato2);
            else if(opcode2 == 11)
                fprintf(f, "lw $t%d, %d($t%d)\n",   rt2, imediato2, rs2);
            else if(opcode2 == 15)
                fprintf(f, "sw $t%d, %d($t%d)\n",   rt2, imediato2, rs2);
            else if(opcode2 == 8)
                fprintf(f, "beq $t%d, $t%d, %d\n",  rs2, rt2, imediato2);
        }
 
        // -------- TIPO J --------
        // opcode 0010 (2): JUMP
        else if(opcode2 == 2){
 
            char addr_s[13];
            strncpy(addr_s, instr+4, 12);
            addr_s[12] = '\0';
 
            // endereço positivo (sem complemento de 2)
            int addr = 0;
            for(int j = 0; j < 12; j++){
                addr = addr * 2 + (addr_s[j] == '1' ? 1 : 0);
            }
 
            fprintf(f, "j %d\n", addr);
        }
 
        else{
            fprintf(f, "instrucao_invalida\n");
        }
    }
 
    fclose(f);
}
 
 
 
 
void salvar_dat(int memdados[], int tamanho){
 
    char nome_arquivo[100];
 
    printf("digite o nome do arquivo de saida de dados (ex: dados.dat): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
 
 
    FILE *f = fopen(nome_arquivo, "w");
 
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



void inserir_registrador(int reg[]){
    int r, valor;

    printf("\nDigite o registrador (0 a 7): ");
    scanf("%d", &r);

    if(r < 0 || r > 7){
        printf("Registrador invalido!\n");
        return;
    }

    printf("Digite o valor para R%d: ", r);
    scanf("%d", &valor);

    reg[r] = valor;

    printf("R%d agora = %d\n", r, reg[r]);
}



void inserir_memoria(int memdados[]){
    int endereco, valor;

    printf("\nDigite o endereco da memoria (0 a 255): ");
    scanf("%d", &endereco);

    if(endereco < 0 || endereco > 255){
        printf("Endereco invalido!\n");
        return;
    }

    printf("Digite o valor para MEM[%d]: ", endereco);
    scanf("%d", &valor);

    memdados[endereco] = valor;

    printf("MEM[%d] agora = %d\n", endereco, memdados[endereco]);
}
