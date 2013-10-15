/* 
Universidade Federal do Rio Grande do Sul - Instituto de Informatica
Departamento de Informatica Aplicada
Sistemas Operacionais IIN - 2013/2
Professor: Alberto Egon Schaeffer Filho
Alunos: Luiz Gustavo Frozi e  Mario Gasparoni Junior

Implementacao de um servidor/cliente de chat
utilizando threads e sockets.

Versao servidor
*/
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h> //EAGAIN
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define MAX_MENSAGEM 60
#define MAX_CLIENTES 10
#define PORTA_APLICACAO 2709
#define MAX_SALAS 10
#define MAX_NICK 15

struct sala {
    int id;
    int contadorLeituras; //antes de modificar o buffer de mensagem, verificar se todos os participantes leram.
    int nParticipantes;
    char  *bufferMensagemAtual;
};


//////Variaveis compartilhadas pelas threads
struct sala salas[MAX_SALAS];
sem_t semaforoObjeto; //Semaforo para sc Todo: criar um para cada sala
///////////////////////////////////////////////////////////////////////////////////

void limpaBufferMensagemSala(int id){
    memset(salas[id].bufferMensagemAtual,0,MAX_MENSAGEM);
}

void inicializaSalas(){
    bzero(salas,sizeof(salas));
}

//threa para gerencia de cada um dos clientes
void * gerenteCliente(void * sock){

////    int socket = *(int*) sock;
     
    while(1){

        sleep(1);




    }



}


int main(int argc, char *argv[ ]) {
   
    int i;
    pthread_t threadsClientes[MAX_CLIENTES];
    int socketAplicacao, socketCliente;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    

    //inicializacao
    fprintf(stdout,"teste");
    inicializaSalas();
    memset(threadsClientes,0,sizeof(pthread_t)*MAX_CLIENTES); 
    sem_init(&semaforoObjeto,0,1); //exclusao mutua (S=1) na variavel compartilhada 
    
    if ((socketAplicacao = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("Erro na criacao do socket");
    
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORTA_APLICACAO);
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8); 
    clilen = sizeof(struct sockaddr_in);
    
    if (bind(socketAplicacao, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) 
        printf("Erro de binding");
    
    listen(socketAplicacao, 5);
    
    i=0;

    while(1){
        
        //aguarda conexao
        if ((socketCliente = accept(socketAplicacao, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
            printf("ERROR on accept");
        else {
            if(i>=MAX_CLIENTES) {
                fprintf(stdout,"Numero maximo de clientes atingido. Ignorando novas conexoes");
                break;
            }else{
                //inicia thread para tratar nova conexao
                pthread_create(&threadsClientes[i],NULL,gerenteCliente,(void*)&socketCliente);
                i++;
            }
        }
    }

    getchar();
    pthread_exit(0);
    return 0;
}
