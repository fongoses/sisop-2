/* 
Universidade Federal do Rio Grande do Sul - Instituto de Informatica
Departamento de Informatica Aplicada
Sistemas Operacionais IIN - 2013/2
Professor: Alberto Egon Schaeffer Filho
Alunos: Luiz Gustavo Frozi e  Mario Gasparoni Junior

Ilustracao da solucao do problema leitores-descritores
utilizando semaforos unnamed
 
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
#define ms 1000
char FILOSOFOS[N_FILOSOFOS]; //sc
sem_t semaforoVarFilosofos; //Semaforo para sc
useconds_t delay=100*ms; //100ms

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

//...
void come(id){
    alteraEstadoFilosofo(id,'E');
}

//...
void pensa(id){
    alteraEstadoFilosofo(id,'T');
}

//...
void hungry(id){
    alteraEstadoFilosofo(id,'H');
}

//verifica se ambos garfos da esquerda e direita estao disponiveis
int garfosDisponiveis(id){
    if ((FILOSOFOS[(id-1)%N_FILOSOFOS] != COMENDO) && (FILOSOFOS[(id+1)%N_FILOSOFOS] != COMENDO))
        return 1;
    else return -1;
    
}


void tentaPegarGarfo(int id){

    sem_wait(&semaforoVarFilosofos);

    if (garfosDisponiveis(id)==1){
        come(id);      
        sem_post(&semaforoVarFilosofos); //libera semaforo
        sleep(rand()%10); //come por um tempo
        sem_wait(&semaforoVarFilosofos);
        pensa(id);
        
    } else hungry(id);    
    
    sem_post(&semaforoVarFilosofos);
}

void *filosofando(void *arg){
  
    int id = *(int*)arg;

    while(1){
        sleep(rand()%10); //pensa/come/hungry por um tempo
        tentaPegarGarfo(id);          
    }
}

int main(int argc, char *argv[ ]) {
   
    int i;
    //filosofos iniciam pensando 
    memset(FILOSOFOS,PENSANDO,sizeof(FILOSOFOS)); 
    pthread_t filosofosThread[N_FILOSOFOS];
    
    sem_init(&semaforoVarFilosofos,0,1); //exclusao mutua (S=1) na variavel compartilhada 
    
    for(i=0;i<N_FILOSOFOS;i++){
        pthread_create(&filosofosThread[i],NULL,filosofando,(void*)&i);
        usleep(delay); //dorme um pouco, para n incrementar o i antes da thread inicializar
    }
    
    getchar();//segura execucao do programa
    pthread_exit(0);
    return 0;

}
