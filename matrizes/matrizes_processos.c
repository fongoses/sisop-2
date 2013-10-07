#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h> /* for pid_t */
#include <sys/wait.h> /* for wait */
#define MAX_DIMENSION 200

int m1,n,m2;
int * M1,*M2;


void leMatrizesEntrada(){
    FILE * fp;
    char PATH[]="./in1.txt";
    fp = fopen("r+w",PATH);

    if (fp<0) {
        fprintf(stdout,"No Input Files\n");
        exit(0);
    }

    char * token;
    char linha[MAX_DIMENSION];
    //indices da matriz
    while(fgets(linha,MAX_DIMENSION,fp) != NULL){
        fprintf(stderr,"%s",linha);
        //token = strtok(linha," ");
        //fprintf(stderr,"Token: %s",token);

    }
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
