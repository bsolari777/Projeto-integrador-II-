#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct memoria_instrucao{

    char bits[17]; // aqui é carregado os bits que vem do .mem
    char opcode[5]; //depois colocado em seus respectivos lugares 
    char rs[5];
    char rd[5]; 
    char rt[5];
    char funct[5];
    char imediato[5];
    char endereco[13];
};

struct memoria_dados{
    
  char dados[17];
    
};

struct sinais_controle {
    int RegWrite;
    int MemRead;
    int MemWrite;
    int ALUSrc;
    int Jump;
    int ALUOp;
};

void carregar_mem_instr(struct memoria_instrucao mem[],int *qntd_instr);
void carregar_mem_dados(struct memoria_dados data[],int *qntd_dados);
void imprimir_memorias_instrdata(struct memoria_instrucao mem[],struct memoria_dados data[],int *qntd_instr,int *qntd_dados);
void salvardata(struct memoria_dados data[],int *qnt_dados);
void cicloinstrucao(int pc,struct memoria_instrucao mem[],int reg[],int *qntd_instr);
void verificarTipo(int pc,struct memoria_instrucao mem[],int reg[]);
void decodificacao_instrucao_I(int pc, struct memoria_instrucao mem[],int reg[]);
int ULA(int a, int b,int c);
int binario_para_int(char *bits);


int main() {

    struct memoria_instrucao mem[256];
    struct memoria_dados data[256];
    int opcao,a;
    int qntd_instr=0,qntd_dados=0;
    int reg[8] = {0}; //inicializando banco de registradores
    int pc = 0; //incializando o pc
    int memoriadados[256];
    do {

        printf("\n---- MENU ----\n");
        printf("1 - Carregar memoria de instrucoes (.mem)\n");
        printf("2 - Carregar memoria de dados (.dat)\n");
        printf("3 - Imprimir memorias (instrucoes e dados)\n");
        printf("4 - Imprimir banco de registradores\n");
        printf("5 - Imprimir todo o simulador\n");
        printf("6 - Salvar .asm\n");
        printf("7 - Salvar .dat\n");
        printf("8 - Executar progama (run)\n");
        printf("9 - Executar uma instrucao (step)\n");
        printf("10 - Voltar uma instrucao (back)\n");
        printf("0 - Sair\n");
        printf("Escolha uma opcao: ");
        scanf("%d", &opcao);

        switch (opcao) {
            case 1:
                printf("Carregando memoria de instrução...\n");
                carregar_mem_instr(mem,&qntd_instr);
                break;

            case 2:
                printf("Carregando memoria de dados...\n");
                carregar_mem_dados(data,&qntd_dados);
                break;

            case 3:
                printf("Imprimindo memorias......\n");
                imprimir_memorias_instrdata(mem,data,&qntd_instr,&qntd_dados);
                break;

            case 4:
                 printf("---- Banco de Registradores ----\n");
                 for(int i=0;i<8;i++){
                 printf("R%d = %d\n", i, reg[i]); 
                 }
                 break;

            case 5:
                printf("Opcao 5 selecionada\n");
                break;

            case 6:
                printf("Opcao 6 selecionada\n");
                break;

            case 7:
                printf("Salvando .dat.......\n");
                salvardata(data,&qntd_dados);
                break;

            case 8:
                printf("Executando programa (run)...\n");
                cicloinstrucao(pc,mem,reg,&qntd_instr);
                break;

            case 9:
                printf("Executando uma instrucao (step)...\n");
                break;

            case 10:
                printf("Voltando uma instrucao (back)...\n");
                break;

            case 0:
                printf("Encerrando programa......\n");
                break;

            default:
                printf("Opcao invalida, tente novamente.\n");
        }

    } while (opcao != 0);

    return 0;
}


//carregar a memoria de instruçoes pra struct de memoria de instrucoes

void carregar_mem_instr(struct memoria_instrucao mem[], int *qtd_instr){

    char linha[17];
    FILE *f;

    f = fopen("instrucoes.mem", "r");

    if (f == NULL) {
        printf("Erro ao abrir o arquivo...\n");
        return;
    }

    int i = 0;

    while (fgets(linha, sizeof(linha), f) != NULL && i < 256) {

        strcpy(mem[i].bits, linha);

        linha[strcspn(linha, "\n")] = '\0';


        i++;
    }

    *qtd_instr = i;

    fclose(f);
}



//carregar a memoria de dados para a struct da memoria de dados

void carregar_mem_dados(struct memoria_dados data[], int *qtd_dados){

    char linha[256];
    FILE *f;

    f = fopen("dados.dat", "r");

    if (f == NULL) {
        printf("Erro ao abrir o arquivo...\n");
        return;
    }

    int i = 0;

    while (fgets(linha, sizeof(linha), f) != NULL && i < 256) {

        linha[strcspn(linha, "\n")] = '\0';

        strcpy(data[i].dados, linha);

        i++;
    }

    *qtd_dados = i;

    fclose(f);
}


// imprimir memorias , instrucao e dados

void imprimir_memorias_instrdata(struct memoria_instrucao mem[],struct memoria_dados data[],int *qntd_instr,int *qntd_dados){
    

    printf("----Memoria de instrução----\n");
    
    for(int i = 0 ; i<*qntd_instr;i++){
        
        printf(" %dª instrução : %s\n",i+1,mem[i].bits);
        
    }
    
    
}


void salvarmemoriadedados(int memoria_dados[],int *qnt_dados){
    
    for(int i=0;i<256;i++){
       memoria_dados[i] = 0;
    }
   
}


void pc(struct memoria_instrucao mem[],int reg[],int *qntd_instr){
    
    int pc = 0;
    
    while(pc < *qntd_instr){
        pc++;
      
        imprimir_memorias_instrdata(mem, NULL, qntd_instr, NULL);
    }
    
}



//controle 
void controle(int pc, struct memoria_instrucao mem[], int reg[], int *qntd_instr){
    
    char temp[5];

    for(int i=0;i<4;i++){
        temp[i] = mem[pc].bits[i];
    }
    temp[4] = '\0';

    if(strcmp(temp,"0000")==0){
        printf("Tipo R\n");

        decodificacao_instrucao_R(mem, pc);

        int rs = binario_para_int(mem[pc].rs);
        int rt = binario_para_int(mem[pc].rt);
        int rd = binario_para_int(mem[pc].rd);
        int funct = binario_para_int(mem[pc].funct);

        reg[rd] = ULA(funct, reg[rs], reg[rt]);
    }
    else if(strcmp(temp,"1000")==0){
        printf("Tipo J\n");

        decodificacao_instrucao_J(mem, pc, qntd_instr);
        pc=binario_para_int(mem[pc].endereco);
    }
    else{
        printf("Tipo I\n");

        decodificacao_instrucao_I(pc, mem, reg);
    }
}



int ULA(int a, int b, int c){

    int resultado = 0;

    switch(a){
        case 0: resultado = b + c;
        break;
        
        case 2: resultado = b - c;
        break;
        
        case 4: resultado = b & c;
        break;
        
        case 5: resultado = b | c;
        break;


        default:
            printf("Operacao invalida\n");
            break;
    }

    if (resultado > 127 || resultado < -128){
        printf("OVERFLOW\n");
    }

    return resultado;
}

 

int binario_para_int(char *bits) {
    
    int valor = 0;
    int tamanho = strlen(bits);

    // verifica se é negativo (bit mais significativo)
    int negativo = (bits[0] == '1');

    for(int i = 0; i < tamanho; i++){
        valor = valor * 2 + (bits[i] - '0');
    }

    // se for negativo, aplica complemento de dois
    if(negativo){
        valor -= (1 << tamanho); 
    }

    return valor;
}


void carregarbancoreg(int reg[]){
    int quantos, valor, indice;

    printf("quantos registradores deseja carregar?\n");
    scanf("%d", &quantos);

    for(int i = 0; i < quantos; i++){

        printf("qual registrador voce deseja carregar?\n");
        scanf("%d", &indice);

        printf("qual valor?\n");
        scanf("%d", &valor);

        reg[indice] = valor;
    }
}




        // decodificação 

void decodificao_instrucao_I(int pc, struct memoria_instrucao mem[], int reg[], int memoriadados[]){

    // opcode
    for(int i=0;i<4;i++){
        mem[pc].opcode[i] = mem[pc].bits[i];
    }
    mem[pc].opcode[4] = '\0';
    
    // rs
    for(int i=0;i<4;i++){ 
        mem[pc].rs[i] = mem[pc].bits[4+i];
    }
    mem[pc].rs[4] = '\0';
    
    // rt
    for(int i=0;i<4;i++){ 
        mem[pc].rt[i] = mem[pc].bits[8+i];
    }
    mem[pc].rt[4] = '\0';
    
    // imediato
    for(int i=0;i<4;i++){ 
        mem[pc].imediato[i] = mem[pc].bits[12+i];
    }
    mem[pc].imediato[4] = '\0';
    
    // conversão
    int opcode = binario_para_int(mem[pc].opcode);
    int rs = binario_para_int(mem[pc].rs);
    int rt = binario_para_int(mem[pc].rt);
    int imediato = binario_para_int(mem[pc].imediato);

    printf("opcode=%d rs=%d rt=%d im=%d\n", opcode, rs, rt, imediato);

    // ADDI (ou operações normais)
    if(opcode != 11 && opcode != 15){
        reg[rt] = ULA(opcode, reg[rs], imediato);
    }

    // lw
    else if(opcode == 11){

        int endereco = reg[rs] + imediato;

        if(endereco >= 0 && endereco < 256){
            reg[rt] = memoriadados[endereco];
            printf("LW: R%d = memoria[%d] (%d)\n", rt, endereco, reg[rt]);
        } else {
            printf("Erro de memoria (lw)\n");
        }
    }

    // sw
    else if(opcode == 15){

        int endereco = reg[rs] + imediato;

        if(endereco >= 0 && endereco < 256){
            memoriadados[endereco] = reg[rt];
            printf("SW: memoria[%d] = %d\n", endereco, reg[rt]);
        } else {
            printf("Erro de memoria (sw)\n");
        }
    }
}


void decodificacao_instrucao_R(struct memoria_instrucao mem[],int pc){
  // opcode
for(int i=0;i<4;i++){
mem[pc].opcode[i] = mem[pc].bits[i];
}
mem[pc].opcode[4] = '\0';

// rs
for(int i=0;i<3;i++){ 
mem[pc].rs[i] = mem[pc].bits[4+i];
}
mem[pc].rs[3] = '\0';

// rt
for(int i=0;i<3;i++){ 
mem[pc].rt[i] = mem[pc].bits[7+i];
}
mem[pc].rt[3] = '\0';

// rd 
for(int i=0;i<3;i++){ 
    mem[pc].rd[i] = mem[pc].bits[10+i];
}
mem[pc].rd[3] = '\0';

// funct 
for(int i=0;i<3;i++){
    mem[pc].funct[i] = mem[pc].bits[13+i];
}
mem[pc].funct[3] = '\0';
}



void decodificacao_instrucao_J(struct memoria_instrucao mem[],int pc, int *qtd_instr){
  // opcode
  for(int i=0;i<4;i++){
    mem[pc].opcode[i] = mem[pc].bits[i];
  }
  mem[pc].opcode[4] = '\0';

  // endereco
  for(int i=0;i<12;i++){
    mem[pc].endereco[i] = mem[pc].bits[4+i];
  
if(mem[pc].endereco >= *qtd_instr){
    printf("Endereco de salto invalido\n");
    return;
  }

}
  mem[pc].endereco[12] = '\0';


}
