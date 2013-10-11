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
#define MAX_DIMENSION 30
#define MAX_THREADS 15
#define DEBUG 0
int m1,n1,m2,n2,m3,n3;
int *M1,*M2,*M3;
int N_THREADS=0; //recebe o numero de threads a serem criadas

 

//global  ja que as threads utilizam essas variaveis
int nThreads;    
int nLinhasPorThread;
int linhasRestantes;

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
    char linha[MAX_DIMENSION];

    //indices da matriz 1
    fgets(linha,MAX_DIMENSION,fp1); //1linha
    strtok(linha," ");
    strtok(NULL," ");
    m1 = atoi(strtok(NULL," "));
    if(DEBUG) fprintf(stdout,"m1: %d\n",m1);

    fgets(linha,MAX_DIMENSION,fp1); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n1 = atoi(strtok(NULL," "));
    if(DEBUG)fprintf(stdout,"n1: %d\n",n1);

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
    fgets(linha,MAX_DIMENSION,fp2); //1linha
    strtok(linha," ");
    strtok(NULL," ");
    m2 = atoi(strtok(NULL," "));
    if (DEBUG) fprintf(stdout,"m2: %d\n",m2);

    fgets(linha,MAX_DIMENSION,fp2); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n2 = atoi(strtok(NULL," "));
    if (DEBUG) fprintf(stdout,"n2: %d\n",n2);
    
    if(n1!=m2){
        fprintf(stdout,"Error: different dimensions\n");
        fclose(fp1);
        fclose(fp2);
        exit(0);
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
    while(fgets(linha,MAX_DIMENSION,fp1) != NULL){        
        token = strtok(linha," ");       
        while(token){ 
            //fprintf(stdout,"Token: %s\n",token);
            M1[i1*n1+ j1]=atoi(token);            
            token=strtok(NULL," ");
            //fprintf(stdout,"Lendo [%d,%d] = %d\n",i1,j1, M1[i1*n1+j1]);
            j1++;
        }
        i1++;
        j1=0; 
    }
    fclose(fp1);

    memset(linha,0,MAX_DIMENSION);

    //restante da matriz 2 
    M2 = (int*)malloc(m2*n2*sizeof(int));
    int i2=0,j2=0;
    while(fgets(linha,MAX_DIMENSION,fp2) != NULL){        
        token = strtok(linha," "); 
        while(token){ 
            //fprintf(stdout,"Token: %s\n",token);
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

    if (DEBUG) fprintf(stdout,"Soma: ");

    for(j=0;j<n1;j++){ //j eh coluna em M1
        M3[linha*n1 + coluna]+=M1[linha*n1+j] * M2[j*n2+coluna];
        if(DEBUG)fprintf(stdout,"M1[%d,%d]=%d * M2[%d,%d]=%d +",linha,j,M1[linha*n1+j],j,coluna,M2[j*n2 +coluna]);
    }
    
    if(DEBUG)fprintf(stdout," = %d\n",M3[linha*n1+coluna]);
}


//realiza a multiplicacao dos valores das matrizes
//void multiplica(int m1,int m2) {
void multiplica(int start,int end){
    int i,j;
    
    if(DEBUG) fprintf(stdout,"Sou uma thread e vou operar nas linhas em M1[%d,%d)\n",start,end);
    for(i=start;i<end;i++)
        for(j=0;j<n2;j++) 
            multiplicaLinhaM1ColunaM2ArmazenandoEmM3(i,j);
        
}


void threadMain(void *logical_id){
    
    int id = *(int*)logical_id; //id logico da thread, no contexto dessa aplicacao.
                        //Numerado de 0 a nThreads.
    
    int i=*(int*)logical_id; //a partir do id da thread, determinamos qual seu subconjunto de linhas
    //fprintf(stdout,"i: %d, j: %d, nLinhasPorThread: %d, linhasRestantes: %d",i,i,nLinhasPorThread,linhasRestantes); 
    if(i<nThreads-1) multiplica(i*nLinhasPorThread,i*nLinhasPorThread + nLinhasPorThread);
    else multiplica(i*nLinhasPorThread,i*nLinhasPorThread + nLinhasPorThread+linhasRestantes);   
    pthread_exit(0); 

}

int main(int argc, char **argv){

    pthread_t threads[MAX_THREADS];    
    int i;
    struct timeval tempoInicioExecucao,tempoFimExecucao;
    useconds_t delay=100*1000; //100ms
    char * PATH_1,*PATH_2;
   
    if(argc < 4){
        fprintf(stdout,"uso: matrizes_threads <NUMERO_THREADS> <arquivo_matriz_1> <arquivo_matriz_2>\n");
        exit(0);
    } 

    PATH_1 = argv[2];
    PATH_2 = argv[3];
   
    if((!PATH_1) || (!PATH_2)){
        fprintf(stdout,"Informe os arquivos que descrevem a matriz\n");
        exit(3);
    }
     
    nThreads = atoi(argv[1]);
    if(nThreads>MAX_THREADS){
        fprintf(stdout,"Numero maximo de Threads permitidas: %d\n",MAX_THREADS);
        exit(3);
    
    }
    
    if(nThreads == 0){
        fprintf(stdout,"NUMERO_THREADS > 0\n");
        exit(3);
    }
 
    memset(threads,0,sizeof(pthread_t)*MAX_THREADS);
    leMatrizesEntrada(PATH_1,PATH_2);
    if (nThreads > m1){
        nThreads=m1;
        fprintf(stdout,"Numero de Threads reduzido a 1 para cada linha da matriz.\n\n");
    }        

    nLinhasPorThread=m1/nThreads;
    linhasRestantes = m1 % nThreads;

    m3=m1;
    n3=n2;
    M3 = (int*)malloc(sizeof(int)*m3*n3);
    memset(M3,0,sizeof(int)*m3*n3);
    //executa os nThreads filhos, tomando nota do tempo
    gettimeofday(&tempoInicioExecucao);

    for(i=0;i<nThreads;i++){
        pthread_create(&threads[i],NULL,threadMain,(void*)&i);
        usleep(delay); //dorme um pouco, para n incrementar o i antes da thread inicializar
    }
    
    for(i=0;i<nThreads;i++)
        pthread_join(threads[i],NULL); //aguarda termino da thread recem criada

    //Termino da execucao dos filhos
    gettimeofday(&tempoFimExecucao);
    
    //imprime estatistica
    fprintf(stdout,"M1\n");
    printaMatriz(M1,m1,n1);
    fprintf(stdout,"\n X \n\nM2\n"); 
    printaMatriz(M2,m2,n2);
    fprintf(stdout,"\n = \n\nM3\n"); 
    printaMatriz(M3,n1,m2);
    fprintf(stdout,"\nTempo total da execucao(s:us): %d:%d\n\n",(tempoFimExecucao.tv_sec-tempoInicioExecucao.tv_sec),abs(tempoFimExecucao.tv_usec - tempoInicioExecucao.tv_usec));
    return 0;
}
