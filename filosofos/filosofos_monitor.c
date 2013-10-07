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
#define DELAY_MAX 10
#define THINKING 'T'
#define EATING 'E'
#define HUNGRY 'H'
#define ms 1000

///////////////////////////////////////////////////////////////////////////////////
//////Variaveis compartilhadas do monitor
int FILOSOFOS[N_FILOSOFOS];
pthread_cond_t varCondicao[N_FILOSOFOS];
int AGE[N_FILOSOFOS]; //'idade' associada ao tempo de espera no estado 'hungry'

pthread_mutex_t mutexVarCondicao[N_FILOSOFOS]; //mutex para cada var de condicao
pthread_mutex_t mutexObjeto; //mutex para acesso dos metodos do monitor
///////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////
//Metodos auxiliares do monitor.
//Obs.: Os metodos auxiliares nao garantem exclusao mutua. Sendo assim,
//o metodo chamador eh quem garante sua exclusao mutua.
///////////////////////////////////////////////////////////////////////////////////
//lista todos os estados 
void printaEstado(){
    int i;
    for(i=0;i<N_FILOSOFOS;i++){
        fprintf(stderr,"%c ",FILOSOFOS[i]);
    }    
    fprintf(stderr,"\n");    
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

//selecao de hungry's realizada por sorteio.
int sorteia3(int id1, int id2, int id3){

    int n1,n2,n3;
    n1=rand()%10;
    n2=rand()%10;
    n3=rand()%10;
    
    if(n1>=n2)
        if(n1>=n3) return id1;
        else return id3;
    else 
        if(n2>=n3) return id2;
        else return id3;
}

int sorteia2(int id1, int id2){
    int n1,n2;
    n1=rand()%10;
    n2=rand()%10;

    if(n1>=n2) return n1;
    else return n2;
 

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
        envelhece(); //os hungry nao estao comendo, envelhecem
        printaEstado();
        //printaAge();
    }
}




//verifica se ambos garfos da esquerda e direita estao disponiveis, 
//sinalizando que a thread pode comer, quando ha disponibilidade.
int garfosDisponiveis(id){
    int felizardo;
 
    if ( ((FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] == HUNGRY) || (FILOSOFOS[(id+1)%N_FILOSOFOS] == THINKING)) && (FILOSOFOS[id] == HUNGRY)){ 
        //felizardo=sorteia3((id+N_FILOSOFOS-1)%N_FILOSOFOS,id,(id+1)%N_FILOSOFOS);
        felizardo=selecionaMaisVelho(id,(id+N_FILOSOFOS-1)%N_FILOSOFOS,(id+1)%N_FILOSOFOS);
        if(felizardo == id) { //se o felizardo eh o processo atual, come, senao espera os mais famintos/velhos comerem primeiro
            alteraEstadoFilosofo(felizardo,EATING);             
            pthread_cond_signal(&varCondicao[id]);
        }
        return 1;
    } 


    if ( ((FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] == THINKING) || (FILOSOFOS[(id+1)%N_FILOSOFOS] == HUNGRY)) && (FILOSOFOS[id] == HUNGRY)){ 
        //felizardo=sorteia3((id+N_FILOSOFOS-1)%N_FILOSOFOS,id,(id+1)%N_FILOSOFOS);
        felizardo=selecionaMaisVelho(id,(id+N_FILOSOFOS-1)%N_FILOSOFOS,(id+1)%N_FILOSOFOS);
        if(felizardo == id) { //se o felizardo eh o processo atual, come, senao espera os mais famintos/velhos comerem primeiro
            alteraEstadoFilosofo(felizardo,EATING);             
            pthread_cond_signal(&varCondicao[id]);
        }
        return 1;
    } 
 
    
    if ((FILOSOFOS[(id+N_FILOSOFOS-1)%N_FILOSOFOS] != EATING) && (FILOSOFOS[(id+1)%N_FILOSOFOS] != EATING) && (FILOSOFOS[id] == HUNGRY)){
        alteraEstadoFilosofo(id,EATING);
        pthread_cond_signal(&varCondicao[id]);
        return 1;
    }
    else return -1;
    
}
///////////////////////////////////////////////////////////////////////////////////
//Metodos principais do monitor.
//Esses metodos sao os acessiveis pelo usuario do monitor.
//Ambos garantem exclusao mutua atraves da variavel MUTEX 'mutexObjeto'
void pegaGarfo(int id){
    pthread_mutex_lock(&mutexObjeto);

    alteraEstadoFilosofo(id,HUNGRY);
    garfosDisponiveis(id);
    while(FILOSOFOS[id] != EATING){       
        pthread_mutex_unlock(&mutexObjeto);
        pthread_cond_wait(&varCondicao[id],&mutexVarCondicao[id]);
        pthread_mutex_lock(&mutexObjeto); 
    }
     
    alteraEstadoFilosofo(id,EATING);
    pthread_mutex_unlock(&mutexObjeto);
}

//politica: signal and continue
void largaGarfo(int id){
    pthread_mutex_lock(&mutexObjeto);

    AGE[id] = 0;
    alteraEstadoFilosofo(id,'T');
    garfosDisponiveis((id+1)%N_FILOSOFOS);
    garfosDisponiveis((id+N_FILOSOFOS-1)%N_FILOSOFOS);

    pthread_mutex_unlock(&mutexObjeto); 
}

void *filosofando(void *arg){
  
    int id = *(int*)arg;

    while(1){
        pegaGarfo(id);
        sleep(rand()%DELAY_MAX); //pensa/come/hungry por um tempo
        largaGarfo(id);
        sleep(rand()%DELAY_MAX); 
    }
}

//programa principal
int main(int argc, char ** argv){

    int i;
    
    useconds_t delay=100*1000; //100ms
   
    pthread_t filosofosThread[N_FILOSOFOS];

    //
    memset(AGE,0,sizeof(AGE));
    
    //filosofos iniciam pensando 
    memset(FILOSOFOS,THINKING,sizeof(FILOSOFOS)); 
    
    //1 mutex para controle do lock do objeto (ou seja,
    //de todas suas variaveis)
    pthread_mutex_init(&mutexObjeto,NULL);


    //obs.: o mutex da variavel de condicao, um para cada var de condicao.
    for(i=0;i<N_FILOSOFOS;i++){        
        pthread_cond_init(&varCondicao[i],NULL);
        pthread_mutex_init(&mutexVarCondicao[i],NULL);
        pthread_create(&filosofosThread[i],NULL,filosofando,(void*)&i);
        usleep(delay); //dorme um pouco, para n incrementar o i antes da thread inicializar
    }

    getchar();
    pthread_exit(0);
    return 0; 
}
