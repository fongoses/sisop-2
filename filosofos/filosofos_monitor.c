/* 
Universidade Federal do Rio Grande do Sul - Instituto de Informatica
Departamento de Informatica Aplicada
Sistemas Operacionais IIN - 2013/2
Professor: Alberto Egon Schaeffer Filho
Alunos: Luiz Gustavo Frozi e  Mario Gasparoni Junior

Ilustracao da solucao do problema leitores-descritores
utilizando monitor implementado com um mutex

o monitor eh composto pelas funcoes come() e libera()
 
*/

#include    <pthread.h>
#include    <semaphore.h>
#include    <stdlib.h>
#include    <stdio.h>
#include    <unistd.h>
#include    <errno.h> //EAGAIN
#include    <string.h>

#define N_FILOSOFOS 5
#define PENSANDO 'T'
#define COMENDO 'E'
#define HUNGRY 'H'
#define ms 1000

int FILOSOFOS[N_FILOSOFOS];
useconds_t delay=100*ms; //100ms

//vars mutex
pthread_cond_t cond;
pthread_mutex_t mutex;

//verifica se ambos garfos da esquerda e direita estao disponiveis
int garfosDisponiveis(id){
    if ((FILOSOFOS[(id-1)%N_FILOSOFOS] != COMENDO) && (FILOSOFOS[(id+1)%N_FILOSOFOS] != COMENDO) && (FILOSOFOS[id] == HUNGRY))
        return 1;
    else return -1;
    
}

//lista todos os estados 
void printaEstado(int id){
    int i;
    for(i=0;i<N_FILOSOFOS;i++){
        fprintf(stdout,"%c ",FILOSOFOS[i]);
    }    
    fprintf(stdout,"\n");    
}

//informa se ha alteracao de estado, comparando o atual com o novo
int haAlteracaoEstado(int id,char novoEstado){
    if (FILOSOFOS[id] == novoEstado)  return -1;
    else return 1;
}
//...
void alteraEstadoFilosofo(int id,char estado){
    if (haAlteracaoEstado(id,estado)==1){
        FILOSOFOS[id]=estado;
        printaEstado(id);
    }
}


//funcoes associadas a monitores 
//id eh a id associada a thread em execucao
void pegaGarfo(int id){
    pthread_mutex_lock(&mutex);
    alteraEstadoFilosofo(id,'H');
    while(garfosDisponiveis(id) <0){
        pthread_cond_wait(&cond,&mutex);    
    }   
    alteraEstadoFilosofo(id,'E');
          
}

//politica: signal and continue
void largaGarfo(int id){
    alteraEstadoFilosofo(id,'T');
    pthread_cond_signal(&cond);
    pthread_mutex_unlock(&mutex);
}

void *filosofando(void *arg){
  
    int id = *(int*)arg;

    while(1){
        pegaGarfo(id);
        sleep(rand()%10); //pensa/come/hungry por um tempo
        largaGarfo(id);
        sleep(rand()%10);                  
    }
}

//programa principal
int main(int *argc, char ** argv){

    pthread_cond_init(&cond,NULL);
    pthread_mutex_init(&mutex,NULL);

    int i;
   
    //filosofos iniciam pensando 
    memset(FILOSOFOS,PENSANDO,sizeof(FILOSOFOS)); 
    pthread_t filosofosThread[N_FILOSOFOS];
    
    
    for(i=0;i<N_FILOSOFOS;i++){
        pthread_create(&filosofosThread[i],NULL,filosofando,(void*)&i);
        usleep(delay); //dorme um pouco, para n incrementar o i antes da thread inicializar
    }

    getchar();
    pthread_exit(0);
    return 0; 
}
