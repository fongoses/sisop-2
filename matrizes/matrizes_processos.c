#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <errno.h>
#define MAX_DIMENSION 30
#define MAX_PROCESSOS 10
int m1,n1,m2,n2;
int *M1,*M2;
int N_THREADS=0;

 
//memoria compartilhada
int segmentSize=0;
int sharedSegmentId;
char * sharedMemory;

void printaMatriz(int *M,int m,int n){
    int i,j;
    
    if (!M) return;

    for(i=0;i<n;i++){
        for(j=0;j<m;j++)
            fprintf(stdout,"%d ",M[i*n+j]);
        fprintf(stdout,"\n");   
    }
}

void leMatrizesEntrada(){
    FILE * fp1,*fp2;
    char PATH1[]="teste1/in1.txt";
    char PATH2[]="teste1/in2.txt";
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
    //fprintf(stdout,"m1: %d\n",m1);

    fgets(linha,MAX_DIMENSION,fp1); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n1 = atoi(strtok(NULL," "));
    //fprintf(stdout,"n1: %d\n",n1);


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
    //fprintf(stdout,"m2: %d\n",m2);

    fgets(linha,MAX_DIMENSION,fp2); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n2 = atoi(strtok(NULL," "));
    //fprintf(stdout,"n2: %d\n",n2);
    
    if(n1!=n2){
        fprintf(stdout,"Error: different dimensions\n");
        fclose(fp1);
        fclose(fp2);
        exit(0);
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
            j1++;
        }
        i1++;
        j1=0; 
    }
    fclose(fp1);

    //printaMatriz(M1,m1,n1);
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
    
    //fprintf(stdout,"\n");
    //printaMatriz(M2,m2,n2);

}

void multiplicaLinhaM1ColunaM2ArmazenandoEmM3(int linha,int coluna){
    int i;
    int j;
    int *M3 = (int*) sharedMemory;

    //fprintf(stdout,"Soma: ");

    for(j=0;j<n1;j++){ //j eh coluna em M1
        //fprintf(stdout,"M1[%d,%d]=%d *M2[%d,%d]=%d +",linha,j,j,coluna,M1[linha*n1+j],M2[j*n2 +coluna]);
        M3[linha*n1 + coluna]+=M1[linha*n1+j] * M2[j*n2+coluna];
    }

    //fprintf(stdout,"\n");
}


//realiza a multiplicacao dos valores das matrizes
//void multiplica(int m1,int m2) {
void multiplica(int start,int end){
    int i,j;
    
    //fprintf(stdout,"Vou operar nas linhas em [%d,%d)\n",start,end);
    for(i=start;i<end;i++)
        for(j=start;j<end;j++)
            multiplicaLinhaM1ColunaM2ArmazenandoEmM3(i,j);
}


int main(int argc, char **argv){

    int nProcessos;
    int nLinhasPorProcesso;
    pid_t pid_filhos[MAX_PROCESSOS];
    pid_t pid;
    int i;
    char filho[10];
   
    if(argc < 2){
        fprintf(stdout,"uso: matrizes_processos [NUMERO_PROCESSOS]\n");
        exit(0);
    } 
    
    nProcessos = atoi(argv[1]);
    if(nProcessos>MAX_PROCESSOS){
        fprintf(stdout,"Numero maximo de Processos permitidos: %d",MAX_PROCESSOS);
        exit(0);
    
    }
 
    leMatrizesEntrada();
    if (nProcessos > m1){
        fprintf(stdout,"Numero de Processos deve ser menor ou igual ao numero de linhas\n");
        exit(0); 
    
    }

    nLinhasPorProcesso=nProcessos/m1;
    memset(pid_filhos,0,sizeof(pid));
    
    segmentSize=sizeof(int)*m1*n2; //memoria compartilhada armazena o resultado final da multiplicacao
    sharedSegmentId  = shmget(IPC_PRIVATE , segmentSize, S_IRUSR | S_IWUSR); //cria segmento compartilhado
    
    if ((sharedMemory = (char*)shmat(sharedSegmentId,NULL,0)) == (char*)-1){
        fprintf(stdout,"Error in shmat: %s\n",strerror(errno));
        exit(0);
    }
    memset(sharedMemory,0,segmentSize);
    
    for(i=0;i<nProcessos;i++){
        pid = fork();
        if (pid <0) exit(0); //fork falhou
        if (pid == 0) {
            //Codigo do filho
    
            sharedMemory = shmat(sharedSegmentId,NULL,0);
            multiplica(i*nLinhasPorProcesso,i*nLinhasPorProcesso + nLinhasPorProcesso);
           
            //Fim do codigo do filho
            return 1;
        }
        else {          // pid!=0; parent process 
            //printf("sou o pai e vou acabar logo");
            pid_filhos[i]=pid;
            sprintf(filho,"Pai ve o pid do filho: %d",pid_filhos[i]); 
            //multiplica(filho);
            //getchar();
            waitpid(pid,0,0);//pai espera por todos os filhos 
        }
    }

    fprintf(stdout,"M1\n");
    printaMatriz(M1,m1,n1);
    fprintf(stdout,"\n X \n\nM2\n"); 
    printaMatriz(M2,m2,n2);
    fprintf(stdout,"\n = \n\nM3\n"); 
    printaMatriz((int*)sharedMemory,n1,m2);

    return 0;
}
