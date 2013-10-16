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
#include <aio.h>
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
#define PORTA_PADRAO 2709
#define MAX_SALAS 10
#define MAX_NICK 15

struct sala {
    int id;
    int contadorLeituras; //antes de modificar o buffer de mensagem, verificar se todos os participantes leram.
    int nParticipantes;
    char  bufferMensagemAtual[MAX_MENSAGEM];
    int numeroSequenciaMensagemAtual;
};


//////Variaveis compartilhadas pelas threads
struct sala salas[MAX_SALAS];
sem_t semaforosSalas[MAX_SALAS]; //1 semaforo para cada sala
///////////////////////////////////////////////////////////////////////////////////


//strings de resposta aos comandos do cliente:
char erroSalaCreate[]="/Erro: numero de sala invalido";
char createSucesso[]="/create %d";


void limpaBufferMensagemSala(int id){
    memset(salas[id].bufferMensagemAtual,0,MAX_MENSAGEM);
}

void inicializaSalas(){
    bzero(salas,sizeof(salas));
}

int atualizaCliente(int socket,int idSala,int seqCliente,int seqSala){
    if(seqCliente<seqSala){
        write(socket,salas[idSala].bufferMensagemAtual,MAX_MENSAGEM);
        salas[idSala].contadorLeituras--;
        return seqCliente+1;
    }
    return seqCliente;
}

void atualizaContadorLeituras(int idSala){
    salas[idSala].contadorLeituras=salas[idSala].nParticipantes-1;
}

void incrementaNumeroSequenciaServidor(int idSala){
    salas[idSala].numeroSequenciaMensagemAtual++;

}

void enviaMensagemControle(int socket, char *mensagem,int controle){

    char mensagemEnviada[MAX_MENSAGEM];

    if(!mensagem) return;

    bzero(mensagemEnviada,MAX_MENSAGEM);
    sprintf(mensagemEnviada,mensagem,controle);


}

void executaComando(int socket,char * mensagem){

    char *savedptr=0;
    char mensagemOriginal[MAX_MENSAGEM];
    char *comando;
    if(socket<0) return;

    bzero(mensagemOriginal,MAX_MENSAGEM);
    strcpy(mensagemOriginal,mensagem);
    
    comando=strtok_r(mensagem," ",&savedptr);
    
    /* //Nao necessario, tratado locamente 
    if(strcmp(comando,"/nick")==0){
        //altera nick - realizado localmente
        char * nick = strtok(NULL," ");
        alteraNick(nick);
        return;
    }*/


    if(strcmp(comando,"/create") == 0){
        int sala;
        char *sala_s=strtok_r(NULL," ",&savedptr);
        if(!sala_s) sala=-1;
        else sala=atoi(sala_s);
        
        
        if ((sala<0) || (sala>=MAX_SALAS )) {
            //envia mensagem ao cliente.
            fprintf(stdout,"Erro ao criar sala %d\n",sala);
            //write(socket,erroSalaCreate,sizeof(erroSalaCreate));
            return;
        }
        
        
        sem_wait(&semaforosSalas[sala]);
        if(salas[sala].nParticipantes==0){ 
            //cria sala
            salas[sala].nParticipantes=1;            
            enviaMensagemControle(socket,createSucesso,sala);    
                fprintf(stdout,"Sala %d criada.\n",sala); 
        }
        sem_post(&semaforosSalas[sala]);
        return;
    }

    if(strcmp(comando,"/join") == 0){
        //entre em uma sala
        return;
    }

    if(strcmp(comando,"/leave") == 0){
        //sai de uma sala
        return;
    }

    if(strcmp(comando,"/close") == 0){
        //fecha conexoes e programa
        return;
    }
    
   
    //comando nao reconhecido
    //exibeMensagemErro("Comando nao reconhecido"); 

}




//threa para gerencia de cada um dos clientes
void * gerenteCliente(void * sock){

    int socket = *(int*) sock;
    char nick[MAX_NICK];
    int sala=-1;
    char bufferMensagemRecebida[MAX_MENSAGEM];    
    char bufferMensagemEnviada[MAX_MENSAGEM];
    int numeroSequenciaCliente=0; 
    int numeroSequenciaServidor=0; 
    struct aiocb acb; //struct para leitura assincrona
    int n;
    if (socket < 0)  exit(3);

    bzero((char*)&acb,sizeof(struct aiocb));
    acb.aio_fildes=socket;
    acb.aio_buf=bufferMensagemRecebida;
    acb.aio_nbytes=MAX_MENSAGEM;

    bzero(bufferMensagemRecebida,MAX_MENSAGEM);
     
    while(1){

        //le mensagem recebida
        //aio_read(&acb);
        //n=acb.aio_offset;
        n=read(socket,bufferMensagemRecebida,MAX_MENSAGEM);
        bufferMensagemRecebida[MAX_MENSAGEM-1]='\0';
        if(n>0){
            //fprintf(stdout,"Oi sou uma thread cliente:\n%s\n",bufferMensagemRecebida);
            
            if(bufferMensagemRecebida[0]=='/') 
                executaComando(socket,bufferMensagemRecebida);
             else{/*
                if(sala>=0){
                    //atualiza cliente 
                    sem_wait(&semaforosSalas[sala]);
                    numeroSequenciaServidor = salas[sala].numeroSequenciaMensagemAtual;
                    numeroSequenciaCliente=atualizaCliente(socket,sala,numeroSequenciaCliente,numeroSequenciaServidor);
                    sem_post(&semaforosSalas[sala]);
                    
                    //aguarda todos receberem a mensagem atual
                    while(1){
                        sem_wait(&semaforosSalas[sala]);
                        if(salas[sala].contadorLeituras ==0){
                            //atualiza mensagem da sala,e seq do cliente
                            bzero(salas[sala].bufferMensagemAtual,MAX_MENSAGEM);
                            strcpy(salas[sala].bufferMensagemAtual,bufferMensagemRecebida);
                            atualizaContadorLeituras(sala);
                            numeroSequenciaCliente++;
                            incrementaNumeroSequenciaServidor(sala);
                            break;
                        }
                        sem_post(&semaforosSalas[sala]);
                        bzero(bufferMensagemRecebida,MAX_MENSAGEM);
                    
                    }

                } */
             }  
        
        }
    }//fim loop principal



}


int main(int argc, char *argv[ ]) {
   
    int i;
    pthread_t threadsClientes[MAX_CLIENTES];
    int socketAplicacao, socketCliente;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int porta=0; 

    //inicializacao
    inicializaSalas();
    memset(threadsClientes,0,sizeof(pthread_t)*MAX_CLIENTES);
    for(i=0;i<MAX_SALAS;i++) 
        sem_init(&semaforosSalas[i],0,1); //exclusao mutua (S=1) na variavel compartilhada 
    
    if ((socketAplicacao = socket(AF_INET, SOCK_STREAM, 0)) == -1){
        printf("Erro na criacao do socket");
        exit(3);

    }
    serv_addr.sin_family = AF_INET;
    if(argc >= 2){
        porta=atoi(argv[1]);
        if(porta<0){           
            porta=PORTA_PADRAO;
        }
    }else porta=PORTA_PADRAO;

    serv_addr.sin_port = htons(porta); 
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    bzero(&(serv_addr.sin_zero), 8); 
    clilen = sizeof(struct sockaddr_in);
    
    if (bind(socketAplicacao, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        printf("Erro de binding\n");
        close(socketAplicacao);
        exit(3);
    }
    
    listen(socketAplicacao, 5);
    fprintf(stdout,"Ouvindo na porta: %d\n",porta); 
    i=0;

    while(1){
        
        //aguarda conexao
        if ((socketCliente = accept(socketAplicacao, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
            printf("Erro em aceitar uma nova conexao\n");
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
