#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
// Struct para armazenar instruções em binário (16 bits + '\0')
struct memoria_instrucao{
    char bits[18];
};
 
// Struct para armazenar o estado da CPU/memória em cada passo
struct estado{
    int reg[8];        
    int memdados[256]; 
    int pc;            
};

struct flags {
    int zero;
    int negativo;
    int overflow;
}; 
 
// assinatura das funções a serem utilizadas no código
void executarinstrucao(struct memoria_instrucao mem[],int reg[], int memdados[], int *pc, struct estado **historico, int *topo, int *capacidade,int step);
void executarstep(struct memoria_instrucao mem[], int qtd_instr, int reg[], int memdados[], int *pc, struct estado **historico, int *topo,int *capacidade);
void executarback(int reg[], int memdados[], int *pc, struct estado historico[], int *topo);
int binario_para_int(char *bits, int modo);
int ULA(int op, int a, int b, struct flags *f);
void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr);
void carregar_mem_dados(int memdados[], int *qtd_dados);
void imprimir_memorias(struct memoria_instrucao mem[], int memdados[]);
void imprimir_tudo(struct memoria_instrucao mem[], int qtd_instr, int memdados[], int reg[], int pc);
void salvar_dat(int memdados[], int tamanho);
void salvar_asm(struct memoria_instrucao mem[], int qtd_instr);
void inserir_memoria(int memdados[]);
void inserir_registrador(int reg[]);
void inicializar(struct memoria_instrucao mem[]);
void aumentar_historico(struct estado **historico, int *capacidade);
void limitar_pc(int *pc);
void menu_clear(struct memoria_instrucao mem[],int reg[],int memdados[],int *pc,struct estado **historico,int *topo,int *capacidade);

int main(){
 
    struct memoria_instrucao mem[256]; // Memória de instruções

    int reg[8] = {0}; 
    int memdados[256] = {0};                
 
    struct estado *historico = NULL;
    int capacidade = 0;
    int topo = -1;                  
 
    int pc = 0;          // Program counter inicial
    int qtd_instr = 0;   // Quantidade de instruções carregadas
    int qtd_dados = 0;   // Quantidade de dados carregados
    int opcao;           // Opção do menu
 
    inicializar(mem);
    historico = malloc(10 * sizeof(struct estado));
    capacidade = 10;
 
 
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
        printf("13 - Clear/Reset\n");
        printf("0 - Sair\n");
        printf("Opcao: ");
        scanf("%d", &opcao);
 
        switch(opcao){
 
            case 1: 
                carregar_mem_instr(mem, &qtd_instr); // Carrega instruções
                break;
            
            case 2: 
                carregar_mem_dados(memdados, &qtd_dados); // Carrega dados
                break;
            
            case 3:
                imprimir_memorias(mem, memdados); // Mostra memórias
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
                executarinstrucao(mem, reg, memdados, &pc,  &historico, &topo, &capacidade,0); // Executa programa todo
                break;
 
            case 9:
                executarstep(mem, qtd_instr, reg, memdados, &pc, &historico, &topo, &capacidade); // Executa uma instrução (step)
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
            
            case 13:
                 menu_clear(mem, reg, memdados, &pc, &historico, &topo, &capacidade);            
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
 
void executarinstrucao(struct memoria_instrucao mem[], 
                        int reg[], int memdados[], int *pc,
                        struct estado **historico, int *topo, int *capacidade, int step){
 
    struct flags f;
    int proximo_pc = 0;
 
    while(*pc >= 0 && *pc <= 255 ){ 
        
         if(*topo >= *capacidade - 1){
            aumentar_historico(historico, capacidade);
           }
 
        (*topo)++;

        (*historico)[*topo].pc = *pc;

        for(int i = 0; i < 8; i++)
            (*historico)[*topo].reg[i] = reg[i];

        for(int i = 0; i < 256; i++)
            (*historico)[*topo].memdados[i] = memdados[i];
 
        proximo_pc = *pc + 1;
        
        char *instr = mem[*pc].bits;  
        char opcode1[5];
        
        strncpy(opcode1, instr, 4); 
        opcode1[4] = '\0';
        
        int opcode2 = binario_para_int(opcode1,0); 
 
        printf("\ninstrucao a ser executada = PC : %d | INSTR : %s\n", *pc, instr);
 
        // -------- TIPO R --------
        // opcode 0000 = 0
        if(opcode2 == 0){
            char rs1[4], rt1[4], rd1[4], funct1[4];
 
            strncpy(rs1,   instr+4, 3); rs1[3]   = '\0';
            strncpy(rt1,   instr+7, 3); rt1[3]   = '\0';
            strncpy(rd1,   instr+10, 3); rd1[3]  = '\0';
            strncpy(funct1, instr+13, 3); funct1[3] = '\0';
 
            int rs2    = binario_para_int(rs1,0);
            int rt2    = binario_para_int(rt1,0);
            int rd2    = binario_para_int(rd1,0);
            int funct2 = binario_para_int(funct1,0);
 
            // funct: 000=ADD, 010=SUB, 100=AND, 101=OR
            // mapeamento para ULA: 0=add, 1=sub, 2=and, 3=or
            int ula_op;
            if(funct2 == 0)      ula_op = 0; // ADD
            else if(funct2 == 2) ula_op = 1; // SUB
            else if(funct2 == 4) ula_op = 2; // AND
            else if(funct2 == 5) ula_op = 3; // OR
            else                 ula_op = 0; // default
 
            reg[rd2] = ULA(ula_op, reg[rs2], reg[rt2],&f); 
            
            rs2 = 0;
            rt2 = 0;
            rd2 = 0;
            funct2 = 0;
        }
 
        // -------- TIPO I --------
        // opcodes conforme tabela do PDF:
        //   0100 (4)  = ADDI
        //   1011 (11) = LW
        //   1111 (15) = SW
        //   1000 (8)  = BEQ
        else if(opcode2 == 4 || opcode2 == 11 || opcode2 == 15 || opcode2 == 8){
            
            char rs1[4], rt1[4], imediato1[7];
 
            strncpy(rs1,      instr+4,  3); rs1[3]      = '\0';
            strncpy(rt1,      instr+7,  3); rt1[3]      = '\0';
            strncpy(imediato1, instr+10, 6); imediato1[6] = '\0';
 
            int rs2      = binario_para_int(rs1,0);
            int rt2      = binario_para_int(rt1,0);
            int imediato2 = binario_para_int(imediato1,1); // já trata complemento de 2
 
            if(opcode2 == 4){
                // ADDI: rt = rs + imm  — passa pela ULA (operação de soma)
                reg[rt2] = ULA(0, reg[rs2], imediato2,&f);
            }
            else if(opcode2 == 11){
                // LW: rt = Mem[rs + imm]
                reg[rt2] = memdados[ULA(0, reg[rs2], imediato2,&f)];
            }
            else if(opcode2 == 15){
                // SW: Mem[rs + imm] = rt
                memdados[ULA(0, reg[rs2], imediato2,&f)] = reg[rt2];
            }
            else if(opcode2 == 8){
                // BEQ: if (rs == rt) PC = PC + imm + 1
                // A ULA faz a subtração para verificar se são iguais (resultado == 0)
                int ula_result = ULA(1, reg[rs2], reg[rt2],&f); // subtração
                if(f.zero == 1){ // se a flag de zero for == 1 ( verdadeira ) realiza o branch
                    proximo_pc = *pc + imediato2; // +1 será feito pelo (*pc)++ no final
                }
            }
            
            rs2 = 0;
            rt2 = 0;
            imediato2 = 0;            
            
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

             addr = addr * 2;

          if(addr_s[i] == '1'){
                 addr = addr + 1;
                     }
                    // se for '0', não soma nada
                 }
            proximo_pc = addr - 1; // -1 porque o (*pc)++ vai incrementar depois
        
        }
 
        else {
            printf("instrucao invalida! opcode = %d\n", opcode2); 
        }
 
 
        *pc = proximo_pc;
        
        if(*pc>255){
        
        limitar_pc(pc);
        printf("PC: %d\n",*pc);
        break;
        
            
        }
        
        limitar_pc(pc);
        printf("PC: %d\n",*pc);
        
        if(step==1){
            step = 0;
            break;
        }
    }
}
 
// ----------- executar instrucao uma de cada vez -----------
void executarstep(struct memoria_instrucao mem[], int qtd_instr, 
                int reg[], int memdados[],
                int *pc, struct estado **historico, int *topo, int *capacidade){
 
 
    if(*topo >= *capacidade - 1){
             aumentar_historico(historico, capacidade);
    }
 
         (*topo)++;

         (*historico)[*topo].pc = *pc;

    for(int i = 0; i < 8; i++)
         (*historico)[*topo].reg[i] = reg[i];

    for(int i = 0; i < 256; i++)
         (*historico)[*topo].memdados[i] = memdados[i];
 
    executarinstrucao(mem, reg, memdados, pc, historico, topo, capacidade,1); // Executa próxima instrução
}
 
// 			voltar instrucao executada 
void executarback(int reg[], int memdados[], int *pc, struct estado historico[], int *topo){
 
    if(*topo < 0){ 
        printf("Não tem nenhuhma instrução para voltar.\n");
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
int ULA(int op, int a, int b, struct flags *f){

    int resultado;
    int overflow = 0;

    if(op == 0){ // ADD
        resultado = a + b;
        if(resultado < -128 || resultado > 127)
            overflow = 1;
    }
    else if(op == 1){ // SUB
        resultado = a - b;
        if(resultado < -128 || resultado > 127)
            overflow = 1;
    }
    else if(op == 2){ // AND
        resultado = a & b;
        overflow = 0;
    }
    else if(op == 3){ // OR
        resultado = a | b;
        overflow = 0;
    }
    else {
        resultado = 0;
        overflow = 0;
    }

    if(resultado == 0){
        f->zero = 1;
    }else{
        f->zero = 0;
    }
    
    if(resultado<0){
        f->negativo = 1;
    }else{
        f->negativo = 0;
    }

    f->overflow = overflow;

    return resultado;
}
 
int binario_para_int(char *bits, int modo){
    int tamanho = strlen(bits);
    int valor = 0;

    // -------- SEM SINAL --------
    if(modo == 0){

        for(int i = 0; i < tamanho; i++){
            if(bits[i] == '1'){
                int expoente = tamanho - 1 - i;

                // calcula 2^expoente
                int potencia = 1;
                for(int j = 0; j < expoente; j++){
                    potencia = potencia * 2;
                }

                valor = valor + potencia;
            }
        }

        return valor;
    }

    // -------- COM SINAL --------
    if(modo == 1){

        // -------- POSITIVO --------
        if(bits[0] == '0'){

            for(int i = 0; i < tamanho; i++){
                if(bits[i] == '1'){
                    int expoente = tamanho - 1 - i;

                    int potencia = 1;
                    for(int j = 0; j < expoente; j++){
                        potencia = potencia * 2;
                    }

                    valor = valor + potencia;
                }
            }

            return valor;
        }

        // -------- NEGATIVO --------

        // 1. inverter bits
        char invertido[33];

        for(int i = 0; i < tamanho; i++){
            if(bits[i] == '1'){
                invertido[i] = '0';
            } else {
                invertido[i] = '1';
            }
        }
        invertido[tamanho] = '\0';

        // 2. converter o invertido usando expoente
        for(int i = 0; i < tamanho; i++){
            if(invertido[i] == '1'){
                int expoente = tamanho - 1 - i;

                int potencia = 1;
                for(int j = 0; j < expoente; j++){
                    potencia = potencia * 2;
                }

                valor = valor + potencia;
            }
        }

        // 3. somar 1
        valor = valor + 1;

        // 4. aplicar sinal negativo
        return -valor;
    }

    return 0;
}
 
 
void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr){
 
    char nome_arquivo[100];
 
    printf("digite o nome do arquivo de instrucoes (ex: instrucoes.mem): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';
 
    FILE *f = fopen(nome_arquivo, "r"); 
    char linha[21];
 
    if(f==NULL){ 
        printf("Erro ao abrir arquivo\n");
        return;
    }
    
    int i=0;
    while(fgets(linha, sizeof(linha), f) && i<256){ // Lê linha por linha
        linha[strcspn(linha,"\r\n")] = '\0'; // Remove '\n'
        strcpy(mem[i].bits, linha);        // Salva na memória
        i++;
    }
 
    *qtd_instr = i; // Atualiza quantidade de instruções
    fclose(f);      // Fecha arquivo
}
 
// ----------- CARREGAR DADOS -----------
void carregar_mem_dados(int memdados[], int *qtd_dados){
    
    char nome_arquivo[100];

    printf("Digite o nome do arquivo de dados (ex: dados.dat): \n");
    getchar();
    fgets(nome_arquivo, sizeof(nome_arquivo), stdin);
    nome_arquivo[strcspn(nome_arquivo, "\n")] = '\0';

    FILE *f = fopen(nome_arquivo, "r");

    if(f == NULL){
        printf("Erro ao abrir arquivo\n");
        return;
    }

    // zera memória
    for(int i = 0; i < 256; i++){
        memdados[i] = 0;
    }

    int i = 0;

    // lê números diretamente
    while(fscanf(f, "%d", &memdados[i]) == 1 && i < 256){
        i++;
    }

    *qtd_dados = i;

    fclose(f);
}
 
// ----------- PRINT MEMORIAS -----------
void imprimir_memorias(struct memoria_instrucao mem[], int memdados[]){
    
    printf("\n---- INSTRUCOES ----\n");
    for(int i = 0; i < 256; i++){
        printf("%d: %s\n", i, mem[i].bits);
    }

    printf("\n---- MEMORIA DE DADOS ----\n");
    for(int i = 0; i < 256; i++){
        printf("%d: %d\n", i, memdados[i]);
    }
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
 
        int opcode2 = binario_para_int(opcode1,0);
 
        // -------- TIPO R --------
        // opcode 0000 (0): ADD, SUB, AND, OR — diferenciados pelo funct
        if(opcode2 == 0){
 
            char rs1[4], rt1[4], rd1[4], funct1[4];
 
            strncpy(rs1,    instr+4,  3); rs1[3]    = '\0';
            strncpy(rt1,    instr+7,  3); rt1[3]    = '\0';
            strncpy(rd1,    instr+10, 3); rd1[3]    = '\0';
            strncpy(funct1, instr+13, 3); funct1[3] = '\0';
 
            int rs2    = binario_para_int(rs1,0);
            int rt2    = binario_para_int(rt1,0);
            int rd2    = binario_para_int(rd1,0);
            int funct2 = binario_para_int(funct1,0);
 
            // funct: 000=ADD, 010=SUB, 100=AND, 101=OR
            if(funct2 == 0)
                fprintf(f, "add $r%d, $r%d, $r%d\n", rd2, rs2, rt2);
            else if(funct2 == 2)
                fprintf(f, "sub $r%d, $r%d, $r%d\n", rd2, rs2, rt2);
            else if(funct2 == 4)
                fprintf(f, "and $r%d, $r%d, $r%d\n", rd2, rs2, rt2);
            else if(funct2 == 5)
                fprintf(f, "or $r%d, $r%d, $r%d\n",  rd2, rs2, rt2);
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
 
            int rs2       = binario_para_int(rs1,0);
            int rt2       = binario_para_int(rt1,0);
            int imediato2 = binario_para_int(imediato1,1);
 
            if(opcode2 == 4)
                fprintf(f, "addi $r%d, $r%d, %d\n", rt2, rs2, imediato2);
            else if(opcode2 == 11)
                fprintf(f, "lw $r%d, %d($r%d)\n",   rt2, imediato2, rs2);
            else if(opcode2 == 15)
                fprintf(f, "sw $r%d, %d($r%d)\n",   rt2, imediato2, rs2);
            else if(opcode2 == 8)
                fprintf(f, "beq $r%d, $r%d, %d\n",  rs2, rt2, imediato2);
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
    for(int i=0; i<256; i++)
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


void inicializar(struct memoria_instrucao mem[]){
    
        for(int i = 0; i < 256; i++){
        strcpy(mem[i].bits, "0000000000000000\0");
    }
     
};

void aumentar_historico(struct estado **historico, int *capacidade){

    int nova_capacidade;

    if(*capacidade == 0)
        nova_capacidade = 10;   // começo pequeno
    else
        nova_capacidade = (*capacidade) * 2; // dobra tamanho

    *historico = realloc(*historico, nova_capacidade * sizeof(struct estado));

    if(*historico == NULL){
        printf("Erro ao alocar memoria do historico!\n");
        exit(1);
    }

    *capacidade = nova_capacidade;
}

void limitar_pc(int *pc){
    if(*pc < 0) *pc = 0;
    if(*pc > 255) *pc = 255;
}

void menu_clear(struct memoria_instrucao mem[],
                int reg[],
                int memdados[],
                int *pc,
                struct estado **historico,
                int *topo,
                int *capacidade)
{
    int opcao;

    printf("\n---- MENU CLEAR ----\n");
    printf("1 - Limpar registradores\n");
    printf("2 - Limpar memoria de dados\n");
    printf("3 - Limpar memoria de instrucoes\n");
    printf("4 - Resetar PC\n");
    printf("5 - Limpar historico\n");
    printf("6 - CLEAR GERAL (tudo)\n");
    printf("0 - Cancelar\n");

    printf("Opcao: ");
    scanf("%d", &opcao);

    switch(opcao)
    {
        case 1:
            for(int i = 0; i < 8; i++)
                reg[i] = 0;
            printf("Registradores limpos.\n");
            break;

        case 2:
            for(int i = 0; i < 256; i++)
                memdados[i] = 0;
            printf("Memoria de dados limpa.\n");
            break;

        case 3:
            for(int i = 0; i < 256; i++)
                strcpy(mem[i].bits, "0000000000000000");
            printf("Memoria de instrucoes limpa.\n");
            break;

        case 4:
            *pc = 0;
            printf("PC resetado.\n");
            break;

        case 5:
            *topo = -1;
            free(*historico);
            *historico = malloc(10 * sizeof(struct estado));
            *capacidade = 10;
            printf("Historico limpo.\n");
            break;

        case 6:
            for(int i = 0; i < 8; i++)
                reg[i] = 0;

            for(int i = 0; i < 256; i++)
            {
                memdados[i] = 0;
                strcpy(mem[i].bits, "0000000000000000");
            }

            *pc = 0;

            *topo = -1;
            free(*historico);
            *historico = malloc(10 * sizeof(struct estado));
            *capacidade = 10;

            printf("CLEAR GERAL EXECUTADO.\n");
            break;

        case 0:
            printf("Cancelado.\n");
            break;

        default:
            printf("Opcao invalida.\n");
    }
}