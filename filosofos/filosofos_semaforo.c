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
#define THINKING 'T'
#define EATING 'E'
#define HUNGRY 'H'
#define ms 1000

//////Variaveis compartilhadas do monitor
int FILOSOFOS[N_FILOSOFOS];
sem_t semaforoObjeto; //Semaforo para sc
sem_t semaforoGarfos[N_FILOSOFOS]; //Semaforo para sc
int AGE[N_FILOSOFOS]; //'idade' associada ao tempo de espera no estado 'hungry'
///////////////////////////////////////////////////////////////////////////////////


useconds_t delay=100*ms; //100ms

//lista todos os estados 
void printaEstado(){
    int i;
    for(i=0;i<N_FILOSOFOS;i++){
        fprintf(stdout,"%c ",FILOSOFOS[i]);
    }    
    fprintf(stdout,"\n");    
}

//lista todos as idades 
void printaAge(){
    int i;
    for(i=0;i<N_FILOSOFOS;i++){
        fprintf(stderr,"%d ",AGE[i]);
    }    
    fprintf(stderr,"\n");    
}

//selecao de hungry's realizada pela idade/age
int selecionaMaisVelho(int id1,int id2,int id3){

    int a1,a2,a3;
    
    a1=AGE[id1];
    a2=AGE[id2];
    a3=AGE[id3];
    
    if(a1>=a2)
        if(a1>=a3) return id1;
        else return id3;
    else 
        if(a2>=a3) return id2;
        else return id3;
}


void envelhece(){
    int i;
    //aumenta idade dos processos Hungry.
    for(i=0;i<N_FILOSOFOS;i++)
        if (FILOSOFOS[i] == HUNGRY) AGE[i]++;
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
        envelhece();
        printaEstado();
        printaAge();
    }
}
//verifica se ambos garfos da esquerda e direita estao disponiveis
int garfosDisponiveis(id){

    int felizardo;

    if ( (FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] == HUNGRY) && (FILOSOFOS[(id+1)%N_FILOSOFOS] == THINKING) && (FILOSOFOS[id] == HUNGRY)){ 
        //felizardo=sorteia3((id+N_FILOSOFOS-1)%N_FILOSOFOS,id,(id+1)%N_FILOSOFOS);
        felizardo=selecionaMaisVelho(id,(id+N_FILOSOFOS-1)%N_FILOSOFOS,(id+1)%N_FILOSOFOS);
        if(felizardo == id) { //se o felizardo eh o processo atual, come, senao espera os mais famintos/velhos comerem primeiro
            alteraEstadoFilosofo(felizardo,EATING);
            sem_post(&semaforoGarfos[id]);
        }
        return 1;
    } 

   if ( (FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] == THINKING) && (FILOSOFOS[(id+1)%N_FILOSOFOS] == HUNGRY) && (FILOSOFOS[id] == HUNGRY)){ 
        //felizardo=sorteia3((id+N_FILOSOFOS-1)%N_FILOSOFOS,id,(id+1)%N_FILOSOFOS);
        felizardo=selecionaMaisVelho(id,(id+N_FILOSOFOS-1)%N_FILOSOFOS,(id+1)%N_FILOSOFOS);
        if(felizardo == id) { //se o felizardo eh o processo atual, come, senao espera os mais famintos/velhos comerem primeiro
            alteraEstadoFilosofo(felizardo,EATING);
            sem_post(&semaforoGarfos[id]);
        }
        return 1;
    } 
        
    if ((FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] != EATING) && (FILOSOFOS[(id+1)%N_FILOSOFOS] != EATING) && (FILOSOFOS[id] == HUNGRY)){
        alteraEstadoFilosofo(id,EATING);
        sem_post(&semaforoGarfos[id]);
        return 1;
    }
    else return -1;

    
}


void pegaGarfo(int id){

    sem_wait(&semaforoObjeto); //adquire semaforo    

    alteraEstadoFilosofo(id,HUNGRY);    
    garfosDisponiveis(id);

    while(FILOSOFOS[id]!=EATING){
        sem_post(&semaforoObjeto);
        sem_wait(&semaforoGarfos[id]);
        sem_post(&semaforoObjeto);
    }

    sem_post(&semaforoObjeto); //libera semaforo
}     

void largaGarfo(int id){
    sem_wait(&semaforoObjeto);    
    AGE[id]=0;
    alteraEstadoFilosofo(id,THINKING);
    garfosDisponiveis((id+1)%N_FILOSOFOS);
    garfosDisponiveis((id+N_FILOSOFOS-1)%N_FILOSOFOS);

    sem_post(&semaforoObjeto);
}

void *filosofando(void *arg){
  
    int id = *(int*)arg;

    while(1){
        pegaGarfo(id);          
        sleep(rand()%10); //come por um tempo
        largaGarfo(id);
    }
}

int main(int argc, char *argv[ ]) {
   
    int i;

    
    memset(AGE,0,sizeof(AGE));
    //filosofos iniciam pensando 
    memset(FILOSOFOS,THINKING,sizeof(FILOSOFOS)); 
    pthread_t filosofosThread[N_FILOSOFOS];
    
    sem_init(&semaforoObjeto,0,1); //exclusao mutua (S=1) na variavel compartilhada 
    
    for(i=0;i<N_FILOSOFOS;i++){
        sem_init(&semaforoGarfos[i],0,1);
        pthread_create(&filosofosThread[i],NULL,filosofando,(void*)&i);
        usleep(delay); //dorme um pouco, para n incrementar o i antes da thread inicializar
    }
    
    getchar();//segura execucao do programa
    pthread_exit(0);
    return 0;

}
