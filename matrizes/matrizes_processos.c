/* 
Universidade Federal do Rio Grande do Sul - Instituto de Informatica
Departamento de Informatica Aplicada
Sistemas Operacionais IIN - 2013/2
Professor: Alberto Egon Schaeffer Filho
Alunos: Luiz Gustavo Frozi e  Mario Gasparoni Junior

Calculo da multiplicacao de matrizes utilizando paralelismo
atraves das primitivas para gerencia de processos filho no Linux.
Tambem foi utilizada uma estrutura de memoria compartilhada.
 
*/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <sys/time.h>
#include <sys/resource.h>
#define MAX_DIMENSION 200
#define MAX_PROCESSOS 30
#define DEBUG 0
int m1,n1,m2,n2,m3,n3;
int *M1,*M2;
int N_THREADS=0;

 
//memoria compartilhada
int segmentSize=0;
int sharedSegmentId;
char * sharedMemory;

void printaMatriz(int *M,int m,int n){
    int i,j;
    
    if (!M) return;

    for(i=0;i<m;i++){
        for(j=0;j<n;j++)
            fprintf(stdout,"%d ",M[i*n+j]);
        fprintf(stdout,"\n");   
    }
}

void leMatrizesEntrada(char * PATH1,char*PATH2){
    FILE * fp1,*fp2;
    
    fp1 = fopen(PATH1,"r");
    
    if (!fp1) {
        fprintf(stdout,"No Input Files\n");
        exit(0);
    }

    char * token;
    int lineDimension = MAX_DIMENSION*3;
    char linha[lineDimension]; //considerando espacos entre as linhas, no maximo temos 3* a dimensao

    //indices da matriz 1
    fgets(linha,lineDimension,fp1); //1linha
    strtok(linha," ");
    strtok(NULL," ");
    m1 = atoi(strtok(NULL," "));
    if (DEBUG)fprintf(stdout,"m1: %d\n",m1);

    fgets(linha,lineDimension,fp1); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n1 = atoi(strtok(NULL," "));
    if (DEBUG) fprintf(stdout,"n1: %d\n",n1);

    if(n1>MAX_DIMENSION){
        fclose(fp1);
        fprintf(stdout,"Max dimesion excedeed\n");
        exit(3);
    }

    //matriz 2
    fp2 = fopen(PATH2,"r");
    if (!fp2) {
        fprintf(stdout,"No Input Files\n");
        exit(0);
    }
   
    //indices da matriz 2
    fgets(linha,lineDimension,fp2); //1linha
    strtok(linha," ");
    strtok(NULL," ");
    m2 = atoi(strtok(NULL," "));
    if(DEBUG)fprintf(stdout,"m2: %d\n",m2);

    fgets(linha,lineDimension,fp2); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n2 = atoi(strtok(NULL," "));
    if(DEBUG)fprintf(stdout,"n2: %d\n",n2);
    
    if(n1!=m2){
        fprintf(stdout,"Error: different dimensions\n");
        fclose(fp1);
        fclose(fp2);
        exit(3);
    }

    if(n2>MAX_DIMENSION){
        fclose(fp1);
        fclose(fp2);
        fprintf(stdout,"Max dimesion excedeed\n");
        exit(3);
    }
    //restante da matriz 1
    M1 = (int*)malloc(m1*n1*sizeof(int));
    int i1=0,j1=0;
    while(fgets(linha,lineDimension*3,fp1) != NULL){        
        token = strtok(linha," ");       
        while(token){ 
            //fprintf(stdout,"Token: M1[%d,%d]=%s\n",i1,j1,token);
            M1[i1*n1+ j1]=atoi(token);            
            token=strtok(NULL," ");
            j1++;
        }
        i1++;
        j1=0; 
    }
    fclose(fp1);

    memset(linha,0,lineDimension);

    //restante da matriz 2 
    M2 = (int*)malloc(m2*n2*sizeof(int));
    int i2=0,j2=0;
    while(fgets(linha,lineDimension,fp2) != NULL){        
        token = strtok(linha," "); 
        while(token){ 
            //fprintf(stdout,"Token: M2[%d,%d]=%s\n",i2,j2,token);
            M2[i2*n2+j2]=atoi(token);            
            token=strtok(NULL," ");
            j2++;
        }
        i2++;
        j2=0; 
    }
    fclose(fp2);

}

void multiplicaLinhaM1ColunaM2ArmazenandoEmM3(int linha,int coluna){
    int i;
    int j;
    int *M3 = (int*) sharedMemory;

    if (DEBUG) fprintf(stdout,"Soma: ");

    for(j=0;j<n1;j++){ //j eh coluna em M1
        M3[linha*n2 + coluna]+=M1[linha*n1+j] * M2[j*n2+coluna];
        if(DEBUG)fprintf(stdout,"M1[%d,%d]=%d * M2[%d,%d]=%d +",linha,j,M1[linha*n1+j],j,coluna,M2[j*n2 +coluna]);
    }
    
    if(DEBUG)fprintf(stdout," = M3[%d,%d] = %d\n",linha,coluna,M3[linha*n2+coluna]);
}


//realiza a multiplicacao dos valores das matrizes
//void multiplica(int m1,int m2) {
void multiplica(int start,int end){
    int i,j;
    
    if(DEBUG) fprintf(stdout,"Sou um processo filho e vou operar nas linhas em M1[%d,%d)\n",start,end);
    for(i=start;i<end;i++)
        for(j=0;j<n2;j++)
            multiplicaLinhaM1ColunaM2ArmazenandoEmM3(i,j);
}



int main(int argc, char **argv){

    int nProcessos;    
    int nLinhasPorProcesso;
    int linhasRestantes;
    pid_t pid_filhos[MAX_PROCESSOS];
    pid_t pid;
    int i;
    struct timeval tempoInicioExecucao,tempoFimExecucao;
    char * PATH_1,*PATH_2;
    clock_t t1,t2;
   
    if(argc < 4){
        fprintf(stdout,"uso: matrizes_processos [NUMERO_PROCESSOS] <arquivo_matriz_1> <arquivo_matriz_2>\n");
        exit(0);
    } 
 
    PATH_1 = argv[2];
    PATH_2 = argv[3];
   
    if((!PATH_1) || (!PATH_2)){
        fprintf(stdout,"Informe os arquivos que descrevem a matriz\n");
        exit(3);
    }
     
 
    nProcessos = atoi(argv[1]);
    if(nProcessos>MAX_PROCESSOS){
        fprintf(stdout,"Numero maximo de Processos permitidos: %d\n",MAX_PROCESSOS);
        exit(3);
    
    }
    
    if(nProcessos == 0){
        fprintf(stdout,"NUMERO_PROCESSOS > 0\n");
        exit(3);
    }
 
    memset(pid_filhos,0,sizeof(pid));
    leMatrizesEntrada(PATH_1,PATH_2);
    if (nProcessos >= m1){
        nProcessos=m1;
        fprintf(stdout,"Numero de Processos reduzido a 1 para cada linha da matriz.\n\n");
    }        

    nLinhasPorProcesso=m1/nProcessos;
    linhasRestantes = m1 % nProcessos;

    m3=m1;
    n3=n2;

    //cria e inicializa memoria compartilhada
    segmentSize=sizeof(int)*m1*n2; //memoria compartilhada armazena o resultado final da multiplicacao
    sharedSegmentId  = shmget(IPC_PRIVATE , segmentSize, S_IRUSR | S_IWUSR); //cria segmento compartilhado    
    if ((sharedMemory = (char*)shmat(sharedSegmentId,NULL,0)) == (char*)-1){
        fprintf(stdout,"Error in shmat: %s\n",strerror(errno));
        exit(0);
    }
    memset(sharedMemory,0,segmentSize);
    //t1 = clock();
    gettimeofday(&tempoInicioExecucao,NULL);
    //executa os nProcessos filhos, tomando nota do tempo
    for(i=0;i<nProcessos;i++){
        pid = fork();
        if (pid <0) exit(0); //fork falhou
        if (pid == 0) {
            //Codigo do filho
    
            if(i<nProcessos-1) multiplica(i*nLinhasPorProcesso,i*nLinhasPorProcesso + nLinhasPorProcesso);
            else multiplica(i*nLinhasPorProcesso,i*nLinhasPorProcesso + nLinhasPorProcesso+linhasRestantes);

           
            //Fim do codigo do filho
            return 1;
        }
        else {
        //codigo do processo pai, daqui em diante 
            pid_filhos[i]=pid;
            //getchar();
        }
    }
    //apos criar os filhos, pai aguarda por cada um  deles    
    for(i=0;i<nProcessos;i++) 
        waitpid(pid_filhos[i],0,0);
  
    //Termino da execucao dos filhos
    gettimeofday(&tempoFimExecucao,NULL);
    //t2=clock();
    //imprime estatistica
    if(DEBUG){
        fprintf(stdout,"M1\n");
        printaMatriz(M1,m1,n1);
        fprintf(stdout,"\n X \n\nM2\n"); 
        printaMatriz(M2,m2,n2);
        fprintf(stdout,"\n = \n\nM3\n"); 
        printaMatriz((int*)sharedMemory,m3,n3);
    }
    fprintf(stdout,"\nTempo total da execucao(s:us): %ld:%ld\n\n",tempoFimExecucao.tv_sec-tempoInicioExecucao.tv_sec,tempoFimExecucao.tv_usec-tempoInicioExecucao.tv_usec);
    //fprintf(stdout,"\nTempo total da execucao(us): %ld\n\n",t2-t1);
    return 0;
}
