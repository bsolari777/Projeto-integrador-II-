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
};

struct memoria_dados{
    
  char dados[17];
    
};



void carregar_mem_instr(struct memoria_instrucao mem[],int *qntd_instr);
void carregar_mem_dados(struct memoria_dados data[],int *qntd_dados);
void imprimir_memorias_instrdata(struct memoria_instrucao mem[],struct memoria_dados data[],int *qntd_instr,int *qntd_dados);
void imprimir_bancoreg();
void salvardata(struct memoria_dados data[],int *qnt_dados);
void pc(struct memoria_instrucao mem[],int *qntd_instr,int reg[]);
void decodificacao(int pc,struct memoria_instrucao mem[],int reg[]);
void tipoI(int pc, struct memoria_instrucao mem[],int reg[]);
void bancoregistradores(int a, int b, int c, int d,int reg[]);
int ULA(int a, int b,int c);
int binario_para_int(char *bits);


int main() {

    struct memoria_instrucao mem[256];
    struct memoria_dados data[256];
    int opcao,a;
    int qntd_instr=0,qntd_dados=0;
    int reg[8] = {0}; //inicializando banco de registradores

    do {

        printf("\n---- MENU ----\n");
        printf("1 - Carregar memoria de instrucoes (.mem)\n");
        printf("2 - Carregar memoria de dados (.dat)\n");
        printf("3 - Imprimir memorias (instrucoes e dados)\n");
        printf("4 - Imprimir banco de registradores\n");
        printf("5 - Imprimir todo o simulador\n");
        printf("6 - Salvar .asm\n");
        printf("7 - Salvar .dat\n");
        printf("8 - Executar programa (run)\n");
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
                pc(mem,&qntd_instr,reg);
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

    char linha[256];
    FILE *f;

    f = fopen("instrucoes.mem", "r");

    if (f == NULL) {
        printf("Erro ao abrir o arquivo...\n");
        return;
    }

    int i = 0;

    while (fgets(linha, sizeof(linha), f) != NULL && i < 256) {

        linha[strcspn(linha, "\n")] = '\0';

        strcpy(mem[i].bits, linha);

        i++;
    }

    *qtd_instr = i;

    fclose(f);
}



//carregar a memoria de dados para a struct da memoria de dados
void carregar_mem_dados(struct memoria_dados data[], int *qtd_dados){

    char linha[256];
    int i=0;
    FILE *f;

    if((f=fopen("dados.dat", "r"))==NULL){
        printf("Erro ao abrir o arquivo...\n");
        return;
    }

    while(fgets(linha, sizeof(linha), f)!=NULL && i<256){

        linha[strcspn(linha, "\n")]='\0';
        strcpy(data[i].dados, linha);

        i++;
    }

    *qtd_dados=i;

    fclose(f);
}

// imprimir memorias , instrucao e dados
void imprimir_memorias_instrdata(struct memoria_instrucao mem[],struct memoria_dados data[],int *qntd_instr,int *qntd_dados){
    
    int i;

    printf("\n    Memória de instrução    \n");
    
    for(i=0; i<*qntd_instr; i++){
        printf(" %dª instrução: %s\n", i+1, mem[i].bits);
    }
    
    printf("    Memória de dados    \n");
    
    for(i=0; i<*qntd_dados; i++){
        printf(" %dº dado : %s\n",i+1,data[i].dados);
    }    
}

// salvar o .dat
void salvardata(struct memoria_dados data[],int *qntd_dados){
    
    int i;
    FILE *f;
    
    if((f=fopen("dados.dat", "r"))==NULL){
        printf("Erro ao abrir arquivo para escrita.\n");
        return;
    }
    
    for(i=0; i<*qntd_dados; i++){    
        fprintf(f," %s",data[i].dados);
    }
    
    fclose(f);  
}


//aqui vai ser o pc


void pc(struct memoria_instrucao mem[],int *qntd_instr,int reg[]){
    
    int pc = 0;
    
    while(pc < *qntd_instr){
    
    decodificacao(pc, mem, reg);
    
    pc++;    
    
        
    }
    

}


// primeiro passo de decodificacao dos bits


void decodificacao(int pc,struct memoria_instrucao mem[],int reg[]){
    
    
    char temp[4];
    
    for(int i=0;i<4;i++){
        
        temp[i] = mem[pc].bits[i];
        
    }
    
    temp[4] = '\0';
    
    if(strcmp(temp,"0000")==0){
        printf("Instrucao do tipo R\n");
    }
    else if(strcmp(temp,"0100")==0){
        printf("Instrucao do tipo J\n");
    }
    else{
        printf("Instrucao do tipo I\n");
        tipoI(pc,mem,reg);
    }


}

//divide os bits para a configuração do tipo I

void tipoI(int pc, struct memoria_instrucao mem[],int reg[]){
    
    // opcode
    for(int i=0;i<4;i++){
    mem[pc].opcode[i] = mem[pc].bits[i];
    mem[pc].opcode[4] = '\0'; // importante!
    }
    
    // rs
    for(int i=0;i<4;i++){ 
    mem[pc].rs[i] = mem[pc].bits[4+i];
    mem[pc].rs[4] = '\0';
    }
    
    // rt
    for(int i=0;i<4;i++){ 
    mem[pc].rt[i] = mem[pc].bits[8+i];
    mem[pc].rt[4] = '\0';
    }
    
    // imediato
    for(int i=0;i<4;i++){ 
    mem[pc].imediato[i] = mem[pc].bits[12+i];
    mem[pc].imediato[4] = '\0';
    }
    
    // agora converte para decimal
    int a = binario_para_int(mem[pc].opcode);//opcode
    int b = binario_para_int(mem[pc].rs);//rs
    int c = binario_para_int(mem[pc].rt);//rt
    int d = binario_para_int(mem[pc].imediato);//imediato

    printf("opcode=%d rs=%d rt=%d rd=%d\n", a, b, c, d);

    bancoregistradores(a,b,c,d,reg);
    
}



void bancoregistradores(int a, int b, int c, int d,int reg[]){
    
    reg[c] = ULA(a, b, d);
    printf("R%d = %d\n", c, reg[c]); // opcional, debug
        
    
}


int ULA(int a, int b,int c){
    
    int resultado;
    
    switch(a){
        
    case 1:
    
    resultado = b + c;
    
    break;
    
    case 2:
    
    resultado = b - c;
    
    break;
    
    default:
    
    printf("operação invalida!!!!");
    
    break;
    
    }
    
    return resultado;
    
}

//transforma binario para inteiro 

int binario_para_int(char *bits) {
    
    int valor = 0;
    for(int i=0; bits[i] != '\0'; i++) {
        if(bits[i] == '1') valor = valor*2 + 1;
        else if(bits[i] == '0') valor = valor*2;
        else {
            printf("Erro: caractere invalido\n");
            return -1;
        }
    }
    return valor;
}



//Organizar todos os arquivos de código-fonte (.c, .h, etc.) de forma clara e documentada. Garantir que o repositório Git esteja atualizado e que todos os arquivos necessários para compilação e execução estejam presentes. Preparar o pacote final para entrega.

//OBS: Pode usar Enum para classificar as classes das instruções(R, I, Jump), para a partir da classe especifica demonstrar as instruções.
//OBS: setbuf antes de um scanf 