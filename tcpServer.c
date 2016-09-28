/*

  TCPSERVER.C
  ==========

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <errno.h>

#include "helper.h"           /*  our own helper functions  */

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <signal.h>

typedef enum { false=0, true=1 } bool;


/*  Global constants  */

#define MAX_LINE           (1000)

int ParseCmdLine(int argc, char *argv[], char **szPort);
void service(char *buffer, int conn_s);
void login(char*file ,char *user, char *password, int conn_s);
void aggiungi(char *buffer, int conn_s, char *cognome, char *nome, char *numero);
void cerca(char *buffer, int conn_s, char *cognome, char *nome);
void gestione_segnali(int signal) ;
void chiusura();

bool var;
int       conn_s;                       /*  connection socket         */
int       list_s;                       /*  listening socket          */


int main(int argc, char *argv[]){
    
    short int port;                         /*  port number               */
    struct    sockaddr_in servaddr;         /*  socket address structure  */
    struct	  sockaddr_in their_addr;
    char      buffer[MAX_LINE]={ '\0' };    /*  character buffer          */
    char     *endptr;                       /*  for strtol()              */
	int 	  sin_size;
    
    
    signal(SIGALRM, gestione_segnali);
	signal(SIGHUP, gestione_segnali);
	signal(SIGINT, gestione_segnali);
	signal(SIGQUIT, gestione_segnali);
	signal(SIGILL, gestione_segnali);
	signal(SIGSEGV, gestione_segnali);
	signal(SIGTERM, gestione_segnali);
    signal(SIGPIPE,SIG_IGN);

	/*  Get command line arguments  */

    ParseCmdLine(argc, argv, &endptr);
	
	port = strtol(endptr, &endptr, 0);
	if ( *endptr )
	{
	    fprintf(stderr, "server: porta non riconosciuta.\n");
	    exit(EXIT_FAILURE);
	}
    
	printf("Server in ascolto sulla porta %d\n",port);
	
	/*  Create the listening socket  */

    if ( (list_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		fprintf(stderr, "server: errore nella creazione della socket.\n");
		exit(EXIT_FAILURE);
    }


    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */

    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port        = htons(port);


    /*  Bind our socket addresss to the 
	listening socket, and call listen()  */

    if ( bind(list_s, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0 )
	{
		fprintf(stderr, "server: errore durante la bind.\n");
		exit(EXIT_FAILURE);
    }

    if ( listen(list_s, LISTENQ) < 0 )
	{
		fprintf(stderr, "server: errore durante la listen.\n");
		exit(EXIT_FAILURE);
    }

    
    /*  Enter an infinite loop to respond
        to client requests and echo input  */

    while ( 1 ){

		/*  Wait for a connection, then accept() it  */
		sin_size = sizeof(struct sockaddr_in);
		if ( (conn_s = accept(list_s, (struct sockaddr *)&their_addr, &sin_size) ) < 0 )
		{
		    fprintf(stderr, "server: errore nella accept.\n");
	    	exit(EXIT_FAILURE);
		}

		printf("server: connessione da %s\n", inet_ntoa(their_addr.sin_addr));
        
        alarm(300);
		
        // Retrieve an input line from the connected socket

		Readline(conn_s, buffer, MAX_LINE-1);
        
        service(buffer,conn_s);
        
        /*  Close the connected socket  */
        
        
		if ( close(conn_s) < 0 && errno!=EBADF){
	    	fprintf(stderr, "server: errore durante la close.\n");
            printf("an error: %s\n", strerror(errno));
		    exit(EXIT_FAILURE);
		}
    }
        
        if ( close(list_s) < 0 ){
            fprintf(stderr, "server: errore durante la close.\n");
            exit(EXIT_FAILURE);
        }
}

/*-------------------------------------------------------------------FUNZIONI----------------------------------------------------------------------------------*/



/*-------------------------------------------------------------------SERVICE---------------------------------------------------------------------------------*/

void service(char *buffer, int conn_s) {
    char service[2];
    char op[2];
	char user[25];
	char password[25];
    char nome[20];
    char cognome[20];
    char numero[20];
    
    sscanf(buffer,"%s %s",service,op);

    if(strcmp(service,"L")==0){

        printf("----LOGIN-----\n");
        
        sscanf(buffer,"%s %s %s %s",service,op,user,password);
        printf("operazione : %s\n",op);
        printf("user : %s\n",user);
        printf("password : %s\n",password);
    
        if(strcmp(op,"1")==0)       login("loginAdd.txt",user,password,conn_s);
        else if(strcmp(op,"2")==0)  login("loginFind.txt",user,password,conn_s);
        }
        
    else{
        
        if(strcmp(op,"1")==0){
            
            printf("----AGGIUNGI RECORD-----\n");
            
            sscanf(buffer,"%s %s %s %s %s",service,op,nome,cognome,numero);
            printf("operazione : %s\n",op);
            printf("nome : %s\n",nome);
            printf("cognome : %s\n",cognome);
            printf("numero : %s\n",numero);
            aggiungi(buffer,conn_s,cognome,nome,numero);
            }
            
        if(strcmp(op,"2")==0){
            
            printf("----CERCA RECORD-----\n");
            
            sscanf(buffer,"%s %s %s %s",service,op,nome,cognome);
            printf("operazione : %s\n",op);
            printf("nome : %s\n",nome);
            printf("cognome : %s\n",cognome);
            cerca(buffer,conn_s,cognome,nome);
        }
    }
    }
    
/*-------------------------------------------------------------------LOGIN---------------------------------------------------------------------------------*/
    
void login(char*file,char *user, char *password, int conn_s){
    
    FILE *fd;
	char user1[20];
	char password1[20];
	char buf[200];
  	char *res;
    char buffer[MAX_LINE]={ '\0' };
	bool trovato=false;

	fd=fopen(file,"r");

	if( fd==NULL ) {
    		perror("Errore in apertura del file");
    		exit(1);
  	}
	
	while(1) {
        res=fgets(buf,70, fd);
        if( res==NULL ) {
            break;
		}
    
    sscanf(buf,"%s %s", user1, password1);
		
    if((strcmp(user1,user)==0) && (strcmp(password1,password)==0)) {
        trovato=true;
        break;
		}
    }
    
	
	if(trovato) {
		strcpy(buffer,"Login effettuato con successo\n");
        printf("%s",buffer);
        
        //DECOMMENTA PER PROVE SEGNALI
        /*printf("inizio sleep\n");
        sleep(5);
        printf("fine sleep\n");
        */
        
		Writeline(conn_s, buffer, strlen(buffer));
		memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
	} 

	else {
		strcpy(buffer,"Impossibile accedere, username o password errati\n");
		Writeline(conn_s, buffer, strlen(buffer));
		memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
	}
	fclose(fd); 
}


/*-------------------------------------------------------------------AGGIUNGI---------------------------------------------------------------------------------*/

void aggiungi(char *buffer, int conn_s, char *cognome, char *nome, char *numero) {

	char buf[200];
	char nome1[25];
	char cognome1[25];
	char numero1[25];
	char *res;
	bool trovato=false;

	FILE *pf;
	pf=fopen("Elenco.txt","r");
	if( pf==NULL ) {
    		perror("Errore in apertura del file");
    		exit(1);
  	}

	while(1) {

		res=fgets(buf,70, pf);
    		
		if( res==NULL ) {
			break;
		}


		sscanf(buf,"%s %s %s",cognome1,nome1,numero1);

		if(strcmp(numero,numero1)==0) {
			trovato=true;
			break;
		}
	}

	if(trovato) {
		strcpy(buffer,"Impossibile inserire il numero poichè è già presente nell'elenco\n");
		Writeline(conn_s, buffer, strlen(buffer));
		memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
		trovato=false;
	}

	else {
		fclose(pf);
		pf=fopen("Elenco.txt","a");

		if( pf==NULL ) {
    			perror("Errore in apertura del file");
    			exit(1);
  		}
		fprintf(pf, "%s %s %s\n",cognome,nome, numero);
		strcpy(buffer,"Contatto aggiunto correttamente!\n");
		Writeline(conn_s, buffer, strlen(buffer));
		memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
	}
		
	fclose(pf);
}


/*-------------------------------------------------------------------CERCA---------------------------------------------------------------------------------*/


void cerca(char *buffer, int conn_s, char *cognome1, char *nome1) {

	FILE *fd;
  	char buf[200];
  	char *res;
	char cognome[25];
	char nome[25];
	char numero[25];
	bool trovato=false;

	buffer[0]='\0';
		
  	fd=fopen("Elenco.txt", "r");

  	if( fd==NULL ) {
    		perror("Errore in apertura del file");
    		exit(1);
  	}

	while(1) {

    		res=fgets(buf,70, fd);

    		if( res==NULL ) {
			break;
		}

		sscanf(buf,"%s %s %s",cognome,nome,numero);
		
		if((strcmp(cognome1,cognome)==0) && (strcmp(nome1,nome)==0)) {
			if(!trovato) {
				trovato=true;
				strcpy(buffer,"I numeri associati al contatto sono: ");
			}
			
			strcat(numero," ");
			strcat(buffer,numero);
			
		}
	
	}

	if(strlen(buffer)!=0) {
		strcat(buffer,"\n");
		printf("%s\n",buffer);
	}

	else {
		strcpy(buffer,"Numero non presente nell' elenco\n");
	}

	Writeline(conn_s, buffer, strlen(buffer));
	memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
	fclose(fd);
		

}

/*--------------------------------------------------------------GESTIONE SEGNALI------------------------------------------------------------------------------*/


void gestione_segnali(int signal) {

	switch (signal) {

		case SIGALRM:
            if ( close(conn_s) < 0) {
                if(errno!=EBADF){
                    fprintf(stderr, "server: errore durante la close.\n");
                    printf("an error: %s\n", strerror(errno));
                    exit(EXIT_FAILURE);
                    }
               //else printf("l'ho ignorato\n");
            }
            else printf("Timeout scaduto\n");
            break;

		case SIGHUP:
            printf("Catturato il segnale SIGHUP\n");
            chiusura();
            break;

		case SIGINT:
            printf("Catturato il segnale SIGINT\n");
            chiusura();
            break;

		case SIGQUIT:
            printf("Catturato il segnale SIQUIT\n");
            chiusura();
            break;

		case SIGILL:
            printf("Catturato il segnale SIGILL\n");
            chiusura();
            break;

		case SIGSEGV:
            printf("Catturato il segnale SIGSEGV\n");
            chiusura();
            break;

		case SIGTERM:
            printf("Catturato il segnale SIGTERM\n");
            chiusura();
            break;
	}
}

void chiusura(){
    if ( close(conn_s) < 0 && errno!=EBADF) {
        fprintf(stderr, "server: errore durante la close.\n");
			}
    exit(EXIT_FAILURE);
    }

/*-------------------------------------------------------------------PARSE---------------------------------------------------------------------------------*/

int ParseCmdLine(int argc, char *argv[], char **szPort)
{
    int n = 1;

    while ( n < argc )
	{
		if ( !strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2) )
			*szPort = argv[++n];
		else 
			if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) )
			{
			    printf("Sintassi:\n\n");
	    		printf("    server -p (porta) [-h]\n\n");
			    exit(EXIT_SUCCESS);
			}
		++n;
    }
	if (argc==1)
	{
	    printf("Sintassi:\n\n");
		printf("    server -p (porta) [-h]\n\n");
	    exit(EXIT_SUCCESS);
	}
    return 0;
}


