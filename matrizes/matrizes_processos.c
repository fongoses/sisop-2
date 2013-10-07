#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#define MAX_DIMENSION 30

int m1,n1,m2,n2;
int *M1,*M2;

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
    fprintf(stdout,"m1: %d\n",m1);

    fgets(linha,MAX_DIMENSION,fp1); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n1 = atoi(strtok(NULL," "));
    fprintf(stdout,"n1: %d\n",n1);


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
    fprintf(stdout,"m2: %d\n",m2);

    fgets(linha,MAX_DIMENSION,fp2); //2linha
    strtok(linha," ");
    strtok(NULL," ");
    n2 = atoi(strtok(NULL," "));
    fprintf(stdout,"n2: %d\n",n2);
    
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

    printaMatriz(M1,m1,n1);
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
    fprintf(stdout,"\n");
    printaMatriz(M2,m2,n2);

}

void doWork(char *arg) {
    while (1) {
        printf("%s\n", arg);
    }
}


int main()
{

    /*
    //Spawn a new process to run alongside us.
    pid_t pid = fork();
    if (pid == 0) {     // child process 
        doWork("child");  
        exit(0);
    }
    else {          // pid!=0; parent process 
        //printf("sou o pai e vou acabar logo");
        doWork("parent");
        waitpid(pid,0,0);   // wait for child to exit 
    }

    */
    leMatrizesEntrada();
    return 0;
}
