/* 
Universidade Federal do Rio Grande do Sul - Instituto de Informatica
Departamento de Informatica Aplicada
Sistemas Operacionais IIN - 2013/2
Professor: Alberto Egon Schaeffer Filho
Alunos: Luiz Gustavo Frozi e  Mario Gasparoni Junior

Implementacao de um servidor/cliente de chat
utilizando threads e sockets.

Versao cliente. 
*/
#include <aio.h>
#include <pthread.h>
#include <semaphore.h>
#include <ncurses.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 

#define PORTA_APLICACAO 2709
#define MAX_MENSAGEM 60
#define MAX_NICK 15
///////Secao Critica////////////////
int socketServer;
//controle da tela
int linhaAtual,colunaAtual;
int salaAtual=-1;
char nickAtual[MAX_NICK];
/////FIM DA SECAO CRITICA//////////


sem_t semaforoSC; //semaforo para controle da sc

//Strings utilizadas no programa
char stringMensagem[] = "> ";
char stringSala[] = "Sala %d ";
char stringServidor_1[]= "Ola, bem vindo ao nosso servidor.";
char stringServidor_2[]= "Nesse momento voce nao esta em nenhuma sala.";
char stringServidor_Help[]= "Comandos Uteis:\n\t/nick - alterar seu nick.\n"
    "\t/create - criar uma sala.\n"
    "\t/join - entrar em uma sala.\n"
    "\t/leave - para sair da sala.\n"
    "\t/close - encerrar o programa.\n";



int linhaMax,colunaMax;
int linhaStringMensagem,colunaStringMensagem; //posicao da string 'Mensagem:'
int linhaBarraHorizontal;
//todas as funcoes abaixo que lidam com a tela, necessitam ser chamadas
//dentro de um semaforo, ja que as variaveis de controle da tela
//sao compartilhadas
void exibeMensagemSala(){
    
    int tamMaxStringSala = 8;

    //limpa String sala
    mvprintw(0,(colunaMax-tamMaxStringSala)/2,"        ");
    mvprintw(0,(colunaMax-strlen(stringSala))/2,stringSala,salaAtual);
    refresh();
}

void limpaTelaPrincipal(){
    int i;
    for(i=0;i<linhaBarraHorizontal;i++){
        mvhline(i,0,' ',colunaMax);
    }
    linhaAtual=0;
    colunaAtual=0; 
}

void aumentaLinhaTelaPrincipal(){
    linhaAtual++;
    if(linhaAtual==linhaBarraHorizontal) limpaTelaPrincipal(); 
}

void exibeMensagemServidor(){
    
    //limpa String sala   
    mvprintw(0,(colunaMax-strlen(stringServidor_1))/2,stringServidor_1);
    mvprintw(1,(colunaMax-strlen(stringServidor_2))/2,stringServidor_2);
    mvprintw(2,(colunaMax-strlen(stringServidor_Help))/2,stringServidor_Help);
    linhaAtual=8; 
    refresh();
}


void ecoaMensagemTela(char * mensagem){
    aumentaLinhaTelaPrincipal();
    mvprintw(linhaAtual,1,"eu: %s",mensagem);
    refresh();
}

void ecoaMensagemControleTela(char * mensagem){
    aumentaLinhaTelaPrincipal();
    mvprintw(linhaAtual,1,"++system: %s",mensagem);
    refresh();
}

void limpaAreaMensagem(){
    int i;
    for(i=strlen(stringMensagem);i<strlen(stringMensagem)+MAX_MENSAGEM;i++)
        mvprintw(linhaStringMensagem,i," ");
    refresh();
}

void exibeMensagemErro(char * mensagem){
    if(!mensagem) return;
    limpaAreaMensagem();
    mvprintw(linhaStringMensagem,strlen(stringMensagem)+1,mensagem);
    refresh();    
    getchar();
    limpaAreaMensagem();

}

void alteraNick(char*novoNick){


    char mensagemErroNick[MAX_MENSAGEM];
    char mensagemAlteracaoNick[MAX_MENSAGEM];
    
    if(!novoNick) {
        sprintf(nickAtual,"user%d",rand()%10000);
        sprintf(mensagemAlteracaoNick,"Nick alterado para: %s.",nickAtual);
        ecoaMensagemControleTela(mensagemAlteracaoNick); 

        return;
    }

    if (strlen(novoNick)>MAX_NICK){
        sprintf(mensagemErroNick,"Nick deve ter entre 0 e %d caracteres.",MAX_NICK);
        exibeMensagemErro(mensagemErroNick);
        return;
    }else{
        strcpy(nickAtual,novoNick);
        sprintf(mensagemAlteracaoNick,"Nick alterado para: %s.",nickAtual);
        ecoaMensagemControleTela(mensagemAlteracaoNick); 
    }

}
void executaComando(int socket, char * mensagem){
    char *comando =strtok(mensagem," ");
    
    if(strcmp(comando,"/nick")==0){
        //altera nick - realizado localmente
        char * nick = strtok(NULL," ");
        alteraNick(nick);
        return;
    }

    if(strcmp(comando,"/create") == 0){
        //cria uma sala
        write(socket,mensagem,sizeof(mensagem));
        //resposta do comando eh tratata em recebeDados        
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
    ecoaMensagemControleTela("Comando nao reconhecido."); 

}

void trataRespostaComando(char * mensagem){
    char *comando =strtok(mensagem," ");
   
    /* //Nao necessario, tratado locamente 
    if(strcmp(comando,"/nick")==0){
        //altera nick - realizado localmente
        char * nick = strtok(NULL," ");
        alteraNick(nick);
        return;
    }*/

    if(strcmp(comando,"/create") == 0){
        salaAtual=atoi(strtok(NULL," "));
        
        //Cria sala
        limpaTelaPrincipal();
        exibeMensagemSala(); 
        linhaAtual=1;
        colunaAtual=0;  
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

void * recebe(void * sock){
    int n;
    int socket = *(int*)sock;
    char resposta[MAX_MENSAGEM]; //mensagem enviada do servidor para o cliente
    struct aiocb acb; //struct para leitura assincrona
    
    if (socket < 0)  exit(3);

    bzero((char*)&acb,sizeof(struct aiocb));
    acb.aio_fildes=socket;
    acb.aio_buf=resposta;
    acb.aio_nbytes=MAX_MENSAGEM;

    while(1){
        sem_wait(&semaforoSC); 

        bzero(resposta,MAX_MENSAGEM);
        aio_read(&acb);
        n = acb.aio_offset;
        if(n>=0){
            if(resposta[0]=='/') trataRespostaComando(resposta);
            else ecoaMensagemTela(resposta);
        }

        sem_post(&semaforoSC);
    }
    exit(0);
}

void * envia(void * sock){
    
    int n;
    int socket = *(int*)sock;
    char mensagem[MAX_MENSAGEM]; //mensagem, propriamente dita
    char mensagemEnviada[MAX_MENSAGEM+MAX_NICK+2]; //cada mensagem enviada leva o nick da pessoa (+2 para o ': ')
    if(socket<0) exit(3);

    while(1){    
        sem_wait(&semaforoSC);
        mvprintw(linhaMax-2,0,"%s",stringMensagem);
        bzero(mensagem, MAX_MENSAGEM);
        mvgetnstr(linhaMax-2,sizeof(stringMensagem),mensagem,MAX_MENSAGEM);
    
        if ( mensagem[0] == '/') executaComando(socket,mensagem);
        else{      
            if(salaAtual < 0){
                ecoaMensagemControleTela("Voce nao esta em nenhum canal.");
            }else{       
                //envia
                bzero(mensagemEnviada,sizeof(mensagemEnviada));
                strcpy(mensagemEnviada,nickAtual);
                strcat(mensagemEnviada,": ");
                strcat(mensagemEnviada,mensagem);
                n=write(socketServer, mensagem, strlen(mensagemEnviada));
               
                if (n>0){
                    ecoaMensagemTela(mensagem);
                }
            }
        }

        limpaAreaMensagem();
        sem_post(&semaforoSC);
    }
    exit(0);
}


int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    pthread_t threadEnvia,threadRecebe;

    sem_init(&semaforoSC,0,1);
    
        if (argc < 2) {
        fprintf(stderr,"uso %s host\n", argv[0]);
        exit(0);
    }
    
    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr,"Erro, host nao encontrado\n");
        exit(0);
    }
    
    if ((socketServer = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("Erro na criacao do socket\n");
    
    serv_addr.sin_family = AF_INET;     
    serv_addr.sin_port = htons(PORTA_APLICACAO);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);     
    
    
    if (connect(socketServer,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro ao conectar\n");
        exit(3);
    }

   
    //inicializa variaveis da tela
    initscr();
    getmaxyx(stdscr,linhaMax,colunaMax);    
    linhaAtual=0;
    colunaAtual=0;
    linhaStringMensagem = linhaMax-2; //posicao da string 'Mensagem: '
    colunaStringMensagem = 0;
    //exibeMensagemSala();
    exibeMensagemServidor();
    linhaBarraHorizontal=linhaStringMensagem-1;
    mvhline(linhaBarraHorizontal,0,'=',colunaMax);
    
    //inicializa configs de sala
    salaAtual=-1;
    

    //cria threads de envio e recebimento
    pthread_create(&threadEnvia,NULL,envia,(void*)&socketServer);
    pthread_create(&threadRecebe,NULL,recebe,(void*)&socketServer);
    pthread_join(threadEnvia,NULL);
    pthread_join(threadRecebe,NULL);
    
    endwin();
    close(socketServer);
    return 0;
}
