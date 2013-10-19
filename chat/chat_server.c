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

//arg para pthread_create
struct args{
    int socket;
    int id; //id do par de threads,
};

struct threadControl{
    int sala; 
    int nSeq;
};
//////Variaveis compartilhadas pelas threads
struct sala salas[MAX_SALAS];
sem_t semaforosSalas[MAX_SALAS]; //1 semaforo para cada sala
sem_t semaforosThreads[MAX_SALAS];//sem da var abaixo
struct threadControl threadSala[MAX_SALAS]; //armazena sala da thread e numero de sequencia
///////////////////////////////////////////////////////////////////////////////////


//strings de resposta aos comandos do cliente:
char erroSalaCreate[]="/Erro: numero de sala invalido";
char createSucesso[]="/create %d";
char erroSalaJoin[]="/Erro: nao foi possivel entra na sala";
char joinSucesso[]="/join %d";
char erroSalaLeave[]="/Erro: nao foi possivel processar o leave";
char leaveSucesso[]="/leave %d";


void limpaBufferMensagemSala(int id){
    memset(salas[id].bufferMensagemAtual,0,MAX_MENSAGEM);
}

void inicializaSalas(){
    bzero(salas,sizeof(salas));
}

int enviaMensagemCliente(int socket,int idSala,int seqCliente,int seqSala){
//nao garante exclusao mutua no semaforo de salas, necessario o chamador garantir o acesso
// a esse semaforo
    if(seqCliente<seqSala){
        fprintf(stdout,"Vou atualizar o cliente\n");
        write(socket,salas[idSala].bufferMensagemAtual,strlen(salas[idSala].bufferMensagemAtual));
        salas[idSala].contadorLeituras--; //atualiza servidor
        return seqSala;
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
    write(socket,mensagemEnviada,strlen(mensagemEnviada));


}

//garante exclusao mutua entre as salas, e threads
void executaComando(int socket,char * mensagem,int idCliente,int * salaNova){

    //salaNova armazena, para o chamador o novo valor da sala.
    //Essa funcao faz isso pq ela eh quem tem acesso ao semaforo diretamente, garantindo
    //a exclusao mutua

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
            write(socket,erroSalaCreate,strlen(erroSalaCreate));
            return;
        }
        
        
        sem_wait(&semaforosSalas[sala]);
        sem_wait(&semaforosThreads[idCliente]);
        
        if(salas[sala].nParticipantes==0){ 
            //cria sala
            salas[sala].nParticipantes=1;            
            enviaMensagemControle(socket,createSucesso,sala);    
            fprintf(stdout,"Sala %d criada.\n",sala);
            threadSala[idCliente].sala =sala;
            *salaNova=sala;
        }
        sem_post(&semaforosThreads[idCliente]);
        sem_post(&semaforosSalas[sala]);
        return;
    }

    if(strcmp(comando,"/join") == 0){
        int sala;
        char *sala_s=strtok_r(NULL," ",&savedptr);
        if(!sala_s) sala=-1;
        else sala=atoi(sala_s);
        
        
        if ((sala<0) || (sala>=MAX_SALAS )) {
            //envia mensagem ao cliente.
            fprintf(stdout,"Erro ao entrar na sala %d\n",sala);
            write(socket,erroSalaJoin,strlen(erroSalaJoin));
            return;
        }
        
        
        sem_wait(&semaforosSalas[sala]);
        sem_wait(&semaforosThreads[idCliente]);
        if(salas[sala].nParticipantes>0){ 
            //entra na sala
            salas[sala].nParticipantes++;            
            enviaMensagemControle(socket,joinSucesso,sala);    
            fprintf(stdout,"Sala %d recebeu um novo membro.\n",sala);
            threadSala[idCliente].sala=sala; 
            *salaNova=sala;
        }else{
            fprintf(stdout,"Sala %d nao existente\n",sala);
            write(socket,erroSalaJoin,strlen(erroSalaJoin));
        
        }
           
        sem_post(&semaforosThreads[idCliente]);
        sem_post(&semaforosSalas[sala]);
        return;

    }

    if(strcmp(comando,"/leave") == 0){
    
        //sai de uma sala
        if ((threadSala[idCliente].sala<0) || (threadSala[idCliente].sala>=MAX_SALAS )) {
            //envia mensagem ao cliente.
            fprintf(stdout,"Usuario nao esta na sala %d\n",threadSala[idCliente].sala);
            write(socket,erroSalaLeave,strlen(erroSalaLeave));
            return;
        }
        
        sem_wait(&semaforosSalas[threadSala[idCliente].sala]);
        sem_wait(&semaforosThreads[idCliente]);

        //sai da sala
        salas[threadSala[idCliente].sala].nParticipantes--; 
        threadSala[idCliente].sala=-1;
        *salaNova=threadSala[idCliente].sala;

        enviaMensagemControle(socket,leaveSucesso,threadSala[idCliente].sala);    
        fprintf(stdout,"Sala %d perde um participante.\n",threadSala[idCliente].sala);
          
        sem_post(&semaforosThreads[idCliente]);
        sem_post(&semaforosSalas[threadSala[idCliente].sala]);
        return;
    }

    if(strcmp(comando,"/close") == 0){
        //fecha conexoes e programa
        //sai de uma sala
        if ((threadSala[idCliente].sala<0) || (threadSala[idCliente].sala>=MAX_SALAS )) {
            //envia mensagem ao cliente.
            fprintf(stdout,"Erro: Usuario nao esta na sala %d\n",threadSala[idCliente].sala);
            exit(0);
            return;
        }
        
        sem_wait(&semaforosSalas[threadSala[idCliente].sala]);
        sem_wait(&semaforosThreads[idCliente]);
        
    //entra na sala
        salas[threadSala[idCliente].sala].nParticipantes--;         
        threadSala[idCliente].sala=-1;
        *salaNova=threadSala[idCliente].sala;

        enviaMensagemControle(socket,leaveSucesso,threadSala[idCliente].sala);    
        fprintf(stdout,"Sala %d perde um participante.\n",threadSala[idCliente].sala);
        
        sem_post(&semaforosThreads[idCliente]);
        sem_post(&semaforosSalas[threadSala[idCliente].sala]);
 
        return;
    }
    
   
    //comando nao reconhecido
    //exibeMensagemErro("Comando nao reconhecido"); 

}
void gravaMensagemSala(int sala, int id, char * bufferMensagemRecebida){
        
    while(1){ //grava mensagem na sala
        sem_wait(&semaforosSalas[sala]);
        sem_wait(&semaforosThreads[id]);
        fprintf(stdout,"recebe() entrou no semaforo\n");
       
        if(salas[sala].contadorLeituras <=0){  

            //fprintf(stdout,"Todos leram a ultima mensagem da sala %d.\n",sala);
            //atualiza mensagem da sala,e seq do cliente
            bzero(salas[sala].bufferMensagemAtual,MAX_MENSAGEM);
            strcpy(salas[sala].bufferMensagemAtual,bufferMensagemRecebida);
            atualizaContadorLeituras(sala);
            incrementaNumeroSequenciaServidor(sala); //atualiza numero sequencia servidor
            //threadSala[id].nSeq=salas[sala].numeroSequenciaMensagemAtual; //atualiza numero seq
           
            //fprintf(stdout,"numero seq servidor: %d, do cliente atual: %d\n",salas[sala].numeroSequenciaMensagemAtual, threadSala[id].nSeq);
            //bzero(bufferMensagemRecebida,MAX_MENSAGEM); 
            //fprintf(stdout,"Gravei uma mensagem no servidor\n");
           
            sem_post(&semaforosThreads[id]);
            sem_post(&semaforosSalas[sala]);
            //fprintf(stdout,"recebe() saiu do semaforo\n");
            break;
        }

        sem_post(&semaforosThreads[id]);
        sem_post(&semaforosSalas[sala]);
        fprintf(stdout,"recebe() saiu do semaforo\n");
        //fprintf(stdout,"esperando para gravar no servidor %d\n",salas[sala].contadorLeituras);
    }


}

void atualizaNumeroSeqCliente(int id,int  nseq){
    sem_wait(&semaforosThreads[id]);
    threadSala[id].nSeq=nseq;
    sem_post(&semaforosThreads[id]);
}

void obtemSalaENumeroSeqAtuaisCliente(int id,int * sala, int * nseq){
//obtem sala e numero de sequenciaatual do cliente.
//ja garante exclusao mutua sobre a variavel de salas
    sem_wait(&semaforosThreads[id]);
    *sala=threadSala[id].sala;
    *nseq=threadSala[id].nSeq;
    sem_post(&semaforosThreads[id]);
}

void * atualizaClientes(void * args){

    int id=(*(struct args*)args).id;
    int socket = (*(struct args*)args).socket;

    char nick[MAX_NICK];
    int sala=-1;
    char bufferMensagemRecebida[MAX_MENSAGEM];    
    char bufferMensagemEnviada[MAX_MENSAGEM];
    int numeroSequenciaCliente=0; 
    int numeroSequenciaServidor=0; 
    int nSeqAntigo;
    if (socket < 0)  exit(3);
    
    bzero(bufferMensagemRecebida,MAX_MENSAGEM);

    while(1){
        obtemSalaENumeroSeqAtuaisCliente(id,&sala,&numeroSequenciaCliente);
        

        if(sala>=0){
            //tenta atualizar cliente  
            sem_wait(&semaforosSalas[sala]);
            numeroSequenciaServidor = salas[sala].numeroSequenciaMensagemAtual;
            nSeqAntigo=numeroSequenciaCliente; 
            
            numeroSequenciaCliente=enviaMensagemCliente(socket,sala,numeroSequenciaCliente,numeroSequenciaServidor);
            if(nSeqAntigo < numeroSequenciaCliente){
                fprintf(stdout,"Cliente da sala %d foi atualizado\n",sala);
                atualizaNumeroSeqCliente(id,numeroSequenciaCliente);
            }
            sem_post(&semaforosSalas[sala]);                
         }
    }
}



//threa para gerencia de cada um dos clientes
void * recebe(void * args){

    
    int id=(*(struct args*)args).id;
    int socket = (*(struct args*)args).socket;
    char nick[MAX_NICK];
    int sala=-1;
    char bufferMensagemRecebida[MAX_MENSAGEM];    
    int numeroSequenciaCliente=0; 
    int numeroSequenciaServidor=0; 
    int n;
    
    if (socket < 0)  exit(3);

    bzero(bufferMensagemRecebida,MAX_MENSAGEM);
     
    while(1){

        bzero(bufferMensagemRecebida,MAX_MENSAGEM);
        n=read(socket,bufferMensagemRecebida,MAX_MENSAGEM);
        bufferMensagemRecebida[MAX_MENSAGEM-1]='\0';
        if(n>0){
            //fprintf(stdout,"Oi sou uma thread cliente:\n%s\n",bufferMensagemRecebida);
            fprintf(stdout,"mensagem recebida\n");
             
            if(sala>=0){
                if(bufferMensagemRecebida[0]=='/'){
                    executaComando(socket,bufferMensagemRecebida,id,&sala); //ja garante exclusao mutua             
                } else gravaMensagemSala(sala,id,bufferMensagemRecebida); //bloqueante , e ja garante exclusao mutua
            }else {//sala nao conhecida ainda
                executaComando(socket,bufferMensagemRecebida,id,&sala);
                fprintf(stdout,"Comando fora da sala\n");
                
            }
        }
    }//fim loop principal



}


int main(int argc, char *argv[ ]) {
   
    int i;
    pthread_t threadsClientesEnvio[MAX_CLIENTES];
    pthread_t threadsClientesRecebimento[MAX_CLIENTES];
    int socketAplicacao, socketClienteEnvio,socketClienteRecebimento;
    socklen_t clilen;
    struct sockaddr_in serv_addr, cli_addr;
    int porta=0; 

    //inicializacao
    inicializaSalas();
    memset(threadsClientesEnvio,0,sizeof(pthread_t)*MAX_CLIENTES);
    memset(threadsClientesRecebimento,0,sizeof(pthread_t)*MAX_CLIENTES);
    for(i=0;i<MAX_SALAS;i++){ 
        sem_init(&semaforosSalas[i],0,1); //exclusao mutua (S=1) na variavel compartilhada 
        sem_init(&semaforosThreads[i],0,1); //exclusao mutua (S=1) na variavel compartilhada 
    }
    memset(threadSala,0,sizeof(struct threadControl)*MAX_SALAS); 
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
        if ((socketClienteEnvio = accept(socketAplicacao, (struct sockaddr *) &cli_addr, &clilen)) == -1) 
            printf("Erro em aceitar uma nova conexao\n");
        else {
            if(i>=MAX_CLIENTES) {
                fprintf(stdout,"Numero maximo de clientes atingido. Ignorando novas conexoes");
                break;
            }else{
                struct args argEnvio;
                struct args argRecebe;
                
                argEnvio.socket = socketClienteEnvio;
                argEnvio.id=i;
                argRecebe.socket=dup(socketClienteEnvio);
                argRecebe.id=i;
                
                pthread_create(&threadsClientesEnvio[i],NULL,atualizaClientes,(void*)&argEnvio);
                
                pthread_create(&threadsClientesRecebimento[i],NULL,recebe,(void*)&argRecebe);
                i++;
            }
        }
    }

    int j=0;
    for(j=0;j<i;j++){
        pthread_join(threadsClientesEnvio[j],NULL);
        pthread_join(threadsClientesRecebimento[j],NULL);
    }
    pthread_exit(0);
    return 0;
}
