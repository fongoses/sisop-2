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
#include <time.h>

#define PORTA_APLICACAO 2709
#define MAX_MENSAGEM 60
#define MAX_NICK 15
///////Secao Critica////////////////
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
WINDOW * mainWindow;
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

void exibeStringMensagemRodape(){

    mvprintw(linhaStringMensagem,0,"%s",stringMensagem);
    refresh();
}

void limpaAreaMensagem(){
    mvhline(linhaStringMensagem,strlen(stringMensagem),' ',colunaMax);
    refresh();
}

void reposicionaCursorMensagem(){
 
    limpaAreaMensagem();
    exibeStringMensagemRodape();
       
}

void inicializaRodape(){

    mvhline(linhaBarraHorizontal,0,'=',colunaMax);
    limpaAreaMensagem();    
    exibeStringMensagemRodape();
}

void limpaTelaPrincipal(){
    int i;
    for(i=0;i<linhaBarraHorizontal;i++){
        mvhline(i,0,' ',colunaMax);
    }
    linhaAtual=0;
    colunaAtual=0; 
    refresh();
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
    mvprintw(linhaAtual,1,"%s",mensagem);
    refresh();
}

void ecoaMensagemControleTela(char * mensagem){
    aumentaLinhaTelaPrincipal();
    mvprintw(linhaAtual,1,"++system: %s",mensagem);    
    reposicionaCursorMensagem();
}



void exibeMensagemErro(char * mensagem){
    if(!mensagem) return;
    limpaAreaMensagem();
    mvprintw(linhaStringMensagem,strlen(stringMensagem)+1,mensagem);
    refresh();    
    
}

//usado nos comandos create e join
void exibeTelaSala(){

    limpaTelaPrincipal();
    exibeMensagemSala(); 
    limpaAreaMensagem();
    exibeStringMensagemRodape();
    linhaAtual=1;
    colunaAtual=0;  
}    

//usado no comando /leave
void exibeTelaServidor(){
    limpaTelaPrincipal();    
    exibeMensagemServidor();
    inicializaRodape();


}
void alteraNick(char*novoNick){


    char mensagemErroNick[MAX_MENSAGEM];
    char mensagemAlteracaoNick[MAX_MENSAGEM];
    struct timespec time;
     
    if(!novoNick) {
        clock_gettime(CLOCK_REALTIME,&time);
        srand(time.tv_nsec);
        sprintf(nickAtual,"user%d",rand()%10000);
        sprintf(mensagemAlteracaoNick,"Nick alterado para: %s.",nickAtual);
        ecoaMensagemControleTela(mensagemAlteracaoNick); 

        return;
    }

    if (strlen(novoNick)>MAX_NICK){
        sprintf(mensagemErroNick,"Nick deve ter entre 0 e %d caracteres.",MAX_NICK);
        ecoaMensagemControleTela(mensagemErroNick);
        return;
    }else{
        strcpy(nickAtual,novoNick);
        sprintf(mensagemAlteracaoNick,"Nick alterado para: %s.",nickAtual);
        //sem_wait(&semaforoSC);
        ecoaMensagemControleTela(mensagemAlteracaoNick); 
        //sem_post(&semaforoSC
    }

}
/*
void leStringEntrada(char * buffer, int len){

    if((buffer=NULL)||(len < 1)) return;
    
    char c;
    int i=0;
    int x,y;
    i=0;
    char a;
    
    while(((c=getchar()) != '\n') && (i < len)){ 
        getsyx(y,x);
        *buffer=c;
        //a=c;
        if((x<strlen(stringMensagem)) || (y != linhaStringMensagem)) {
            setsyx(linhaStringMensagem,2);
            i=2;
        }else if((x>MAX_MENSAGEM+strlen(stringMensagem)) || (y != linhaStringMensagem)) {
                i=MAX_MENSAGEM+strlen(stringMensagem);
                setsyx(linhaStringMensagem,i);
             }else if ((c == 127) || (c == 8)){ 
                    buffer[i]=' ';
                    i--;
                }else {
                    i++;
                }
        
       //buffer[i+1]='\n';     
       //ecoaMensagemTela(buffer);     
//       refresh();
    }    
    setsyx(linhaStringMensagem,strlen(stringMensagem));
    ecoaMensagemTela(buffer); 
    refresh();

}
*/
void executaComando(int socket, char * mensagem){
  
    char *savedptr=0;
    char mensagemOriginal[MAX_MENSAGEM];
    char *comando; 

    if(socket<0) return;
     
    bzero(mensagemOriginal,MAX_MENSAGEM);
    strcpy(mensagemOriginal,mensagem);

    comando=strtok_r(mensagem," ",&savedptr);

    if(strcmp(comando,"/nick")==0){
        //altera nick - realizado localmente
        char * nick = strtok_r(NULL," ",&savedptr);
        
        sem_wait(&semaforoSC);
        alteraNick(nick);        
        sem_post(&semaforoSC);
        return;
    }

    if(strcmp(comando,"/create") == 0){
        //cria uma sala
        char * sala_s=strtok_r(NULL," ",&savedptr);
        if(!sala_s) {
            sem_wait(&semaforoSC);
            ecoaMensagemControleTela("sintaxe invalida.");
            sem_post(&semaforoSC);
            return;
        }
               
        //sem_wait(&semaforoSC);
        //salaAtual=atoi(sala_s); atualiza sala na resposta
        //sem_post(&semaforoSC);
        write(socket,mensagemOriginal,strlen(mensagemOriginal));
        //resposta do comando eh tratata em recebeDados        
        return;
    }

    if(strcmp(comando,"/join") == 0){
        //entra em uma sala
        char * sala_s=strtok_r(NULL," ",&savedptr);
        if(!sala_s) {
            sem_wait(&semaforoSC);
            ecoaMensagemControleTela("sintaxe invalida.");
            sem_post(&semaforoSC);
            return;
        }
        write(socket,mensagemOriginal,strlen(mensagemOriginal));
        //resposta do comando eh tratata em recebeDados        
        return;
    }

    if(strcmp(comando,"/leave") == 0){
        sem_wait(&semaforoSC);
        //sai de uma sala
        if(salaAtual < 0){
            ecoaMensagemControleTela("voce nao esta em nenhuma sala.");
            sem_post(&semaforoSC);
            return;
        }        
        sem_post(&semaforoSC);
        write(socket,mensagemOriginal,strlen(mensagemOriginal));
        return;
    }

    if(strcmp(comando,"/close") == 0){
        //fecha conexoes e programa
        write(socket,mensagemOriginal,strlen(mensagemOriginal));        
        return;
    }
 
    if(strcmp(comando,"/help") == 0){
        //fecha conexoes e programa
        
        sem_wait(&semaforoSC);
        ecoaMensagemControleTela(stringServidor_Help);
        sem_post(&semaforoSC);
        return;
    }
    
   
    //comando nao reconhecido

    sem_wait(&semaforoSC);
    ecoaMensagemControleTela("Comando nao reconhecido."); 
    sem_post(&semaforoSC);

}

void trataRespostaComando(char * mensagem){
    
    char *savedptr=0;
    char mensagemOriginal[MAX_MENSAGEM];
    char *comando;

    bzero(mensagemOriginal,MAX_MENSAGEM);
    strcpy(mensagemOriginal,mensagem);
    
    comando= strtok_r(mensagem," ",&savedptr);
   
    /* //Nao necessario, tratado locamente 
    if(strcmp(comando,"/nick")==0){
        //altera nick - realizado localmente
        char * nick = strtok(NULL," ");
        alteraNick(nick);
        return;
    }*/
    if(strcmp(comando,"/create") == 0){
        
        sem_wait(&semaforoSC);
        char *sala_s = strtok_r(NULL," ",&savedptr);
        if(!sala_s) return;
        
        int salatemp=atoi(sala_s);
        if (salatemp<0) return;
        salaAtual=salatemp;
        
        //Cria sala
        exibeTelaSala();   
            
        sem_post(&semaforoSC);
        return;
    }

    if(strcmp(comando,"/join") == 0){
        //entre em uma sala
        sem_wait(&semaforoSC);
        char *sala_s = strtok_r(NULL," ",&savedptr);
        if(!sala_s) return;
       
        int salatemp=atoi(sala_s);
        salaAtual=salatemp;
 
        
        //Entra na sala
        exibeTelaSala();
        
        sem_post(&semaforoSC);
     
        return;
    }

    if(strcmp(comando,"/leave") == 0){
        //sai de uma sala
         
        sem_wait(&semaforoSC);
        salaAtual=-1;
        exibeTelaServidor();
        sem_post(&semaforoSC);
        return;
    }

    if(strcmp(comando,"/close") == 0){
        //fecha conexoes e programa
        system("reset");
        exit(0);
        return;
    }
    
   
    //comando nao reconhecido
    sem_wait(&semaforoSC);
    ecoaMensagemControleTela(mensagemOriginal); 
    sem_post(&semaforoSC);


}

void * recebe(void * sock){
    int n;
    int socket = *(int*)sock;
    char resposta[MAX_MENSAGEM]; //mensagem enviada do servidor para o cliente
    //struct aiocb acb; //struct para leitura assincrona
    
    if (socket < 0)  exit(3);

    while(1){
        //sem_wait(&semaforoSC); 

        bzero(resposta,MAX_MENSAGEM);
       // ecoaMensagemTela("Recebendo resposta...");

        n=read(socket,resposta,MAX_MENSAGEM);
        //aio_read(&acb);
        //n = acb.aio_offset;
        if(n>0){
            //ecoaMensagemTela("Mensagem recebida...");
            if(resposta[0]=='/') trataRespostaComando(resposta);
            else {
            
                sem_wait(&semaforoSC);
                ecoaMensagemTela(resposta);
                limpaAreaMensagem();
                exibeStringMensagemRodape();
                sem_post(&semaforoSC);
            }
        }

        //sem_post(&semaforoSC);
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
        bzero(mensagem, MAX_MENSAGEM);
        
        sem_wait(&semaforoSC);
        reposicionaCursorMensagem();
        sem_post(&semaforoSC);

        //leStringEntrada(mensagem,MAX_MENSAGEM);
        mvgetnstr(linhaMax-2,strlen(stringMensagem),mensagem,MAX_MENSAGEM);
    
        if ( mensagem[0] == '/') executaComando(socket,mensagem);
        else{      
            if(salaAtual < 0){
                sem_wait(&semaforoSC);
                ecoaMensagemControleTela("Voce nao esta em nenhuma sala.");
                sem_post(&semaforoSC);
            }else{       
                //envia
                bzero(mensagemEnviada,MAX_MENSAGEM+MAX_NICK+2);
                strcpy(mensagemEnviada,nickAtual);
                strcat(mensagemEnviada,": ");
                strcat(mensagemEnviada,mensagem);
                n=write(socket, mensagemEnviada, strlen(mensagemEnviada));
                if (n>0){
                    
                    //sem_wait(&semaforoSC);
                    //ecoaMensagemTela(nickAtual);
                    //sem_post(&semaforoSC);
                }
            }
        }

        sem_wait(&semaforoSC);
        limpaAreaMensagem();
        exibeStringMensagemRodape();
        
        sem_post(&semaforoSC);
    }
    exit(0);
}


int main(int argc, char *argv[])
{
    struct sockaddr_in serv_addr;
    struct hostent *server;
    pthread_t threadEnvia,threadRecebe;
    char * hostname;
    int porta;
    int socketWrite,socketRead;
    
    
    if (argc < 2) {
        fprintf(stderr,"uso %s host\n", argv[0]);
        exit(0);
    }
 
    if(argc>=3){
        hostname=argv[1];
        porta=atoi(argv[2]);
        if(porta<0)porta=PORTA_APLICACAO;
    }else {
        porta=PORTA_APLICACAO;
        hostname=argv[1];
    }
   
    server = gethostbyname(hostname);
    if (server == NULL) {
        fprintf(stderr,"Erro, host nao encontrado\n");
        exit(0);
    }
    
    if ((socketWrite = socket(AF_INET, SOCK_STREAM, 0)) == -1) 
        printf("Erro na criacao do socket\n");
    
    serv_addr.sin_family = AF_INET;    
    serv_addr.sin_port = htons(porta);    
    serv_addr.sin_addr = *((struct in_addr *)server->h_addr);
    bzero(&(serv_addr.sin_zero), 8);     
    
    if (connect(socketWrite,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
        printf("Erro ao conectar\n");
        exit(3);
    }

    socketRead = dup(socketWrite); //duplica o socket, para leitura  
 
    //inicializa variaveis da tela
    mainWindow = initscr();
    cbreak();
    nonl();
    intrflush(stdscr, FALSE);
    keypad(stdscr, TRUE);

   //noecho();
    getmaxyx(stdscr,linhaMax,colunaMax);    
    linhaAtual=0;
    colunaAtual=0;
    linhaStringMensagem = linhaMax-2; //posicao da string 'Mensagem: '
    colunaStringMensagem = 0;
    //exibeMensagemSala();
    exibeMensagemServidor();
    linhaBarraHorizontal=linhaStringMensagem-1;
    mvhline(linhaBarraHorizontal,0,'=',colunaMax);
    alteraNick(NULL); 
    //inicializa configs de sala
    salaAtual=-1;
    

    //cria threads de envio e recebimentio    
    sem_init(&semaforoSC,0,1);
    pthread_create(&threadEnvia,NULL,envia,(void*)&socketWrite);
    pthread_create(&threadRecebe,NULL,recebe,(void*)&socketRead);
    pthread_join(threadEnvia,NULL);
    pthread_join(threadRecebe,NULL);
    
    endwin();
    close(socketWrite);
    close(socketRead);
    return 0;
}
