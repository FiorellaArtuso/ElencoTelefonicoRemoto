/*

  TCPCLIENT.C
  ==========

*/


#include <sys/socket.h>       /*  socket definitions        */
#include <sys/types.h>        /*  socket types              */
#include <arpa/inet.h>        /*  inet (3) funtions         */
#include <unistd.h>           /*  misc. UNIX functions      */
#include <netinet/in.h>
#include <netdb.h>

#include "helper.h"           /*  Our own helper functions  */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <signal.h>
#include <errno.h>


/*  Global constants  */

#define MAX_LINE           (1000)

int       conn_s;                /*  connection socket         */

void validation(char* aux,char*buffer,int conn_s);
void gestione_segnali(int signal) ;
void chiusura();


/*  main()  */

int main(int argc, char *argv[])
{
    short int port;                  /*  port number               */
    struct    sockaddr_in servaddr;  /*  socket address structure  */
    char      buffer[MAX_LINE];      /*  character buffer          */
    char     *szAddress;             /*  Holds remote IP address   */
    char     *szPort;                /*  Holds remote port         */
    char     *endptr;                /*  for strtol()              */
	struct	  hostent *he;
    
    char      scelta[2];
    char      aux[MAX_LINE];


	he=NULL;

    signal(SIGHUP, gestione_segnali);
	signal(SIGINT, gestione_segnali);
	signal(SIGQUIT, gestione_segnali);
	signal(SIGILL, gestione_segnali);
	signal(SIGSEGV, gestione_segnali);
	signal(SIGTERM, gestione_segnali);
    signal(SIGPIPE, SIG_IGN);


    /*  Get command line arguments  */

    ParseCmdLine(argc, argv, &szAddress, &szPort);


    /*  Set the remote port  */

    port = strtol(szPort, &endptr, 0);
    if ( *endptr )
	{
		printf("client: porta non riconosciuta.\n");
		exit(EXIT_FAILURE);
    }
	

    /*  Create the listening socket  */

    if ( (conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 )
	{
		fprintf(stderr, "client: errore durante la creazione della socket.\n");
		exit(EXIT_FAILURE);
    }


    /*  Set all bytes in socket address structure to
        zero, and fill in the relevant data members   */
	memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family      = AF_INET;
    servaddr.sin_port        = htons(port);

    /*  Set the remote IP address  */

    if ( inet_aton(szAddress, &servaddr.sin_addr) <= 0 )
	{
		printf("client: indirizzo IP non valido.\nclient: risoluzione nome...");
		
		if ((he=gethostbyname(szAddress)) == NULL)
		{
			printf("fallita.\n");
  			exit(EXIT_FAILURE);
		}
		printf("riuscita.\n\n");
		servaddr.sin_addr = *((struct in_addr *)he->h_addr);
    }
    
    
/*---------------------------------------------------------------ACQUISIZIONE DATI LOGIN----------------------------------------------------------------*/

    //salvare operazione
    printf("Benvenuto al servizio Elenco Telefonico!\n Digita '1' per inserire un nuovo numero nell' elenco oppure '2' per cercare un numero.\n");
    fgets(aux, MAX_LINE, stdin);
    strcpy(scelta,aux);
    strcpy(buffer,"L ");
    strtok(aux, "\n");
    strcat(buffer,aux);
    strcat(buffer," ");
    
    //controllo che la scelta sia valida
    if((strcmp(aux,"1")!=0) && (strcmp(aux,"2")!=0)) {
        printf("la scelta effettuata non è valida!\n");
		exit(EXIT_FAILURE);
        }
    
    //inserimeto username
    printf("Digitare l'username per il login:\n");
	fgets(aux, MAX_LINE, stdin);
    
    validation(aux,buffer,conn_s);
    
    strtok(aux, "\n");
    strcat(aux," ");
    strcat(buffer,aux);
    
    //inserimento password
    printf("Digitare la password per il login:\n");
	fgets(aux, MAX_LINE, stdin);

	validation(aux,buffer,conn_s);
    
    strcat(aux," ");
    strcat(buffer,aux);
    
    
    /*  connect() to the server  */

    if ( connect(conn_s, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ){
		printf("client: errore durante la connect.\n");
		exit(EXIT_FAILURE);
    }
    
    /*printf("inizio sleep\n");
    sleep(5);
    printf("fine sleep\n");*/
    
    
    /*  Send string to server, and retrieve response  */

    Writeline(conn_s, buffer, strlen(buffer));
	memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
    
    
    Readline(conn_s, buffer, MAX_LINE-1);
    printf("%s\n", buffer);
    
    if(strcmp(buffer,"Impossibile accedere, username o password errati\n")==0) {
		exit(EXIT_FAILURE);
	}
    
    if(strcmp(buffer,"")==0){
        printf("Impossibile erogare il servizio, problemi con il server o tempo per la scelta scaduto. Provare a riconnettere \n");
        exit(EXIT_FAILURE);
        }
    
    if ( close(conn_s) < 0 ) {
        fprintf(stderr, "server: errore durante la close.\n");
		exit(EXIT_FAILURE);
	}
    
    buffer[0]='\0';
	aux[0]='\0';
    
/*---------------------------------------------------------------ACQUISIZIONE DATI OPERAZIONI----------------------------------------------------------------*/
    
    /*operazione "aggiungi record"*/
    
    if(strcmp(scelta,"1\n")==0){
        
        strcpy(buffer,"O 1");
        
        //inserimento cognome
        printf("Inserisci il cognome del contatto che si desidera aggiungere (20 caratteri consentiti):\n ");
		fgets(aux, MAX_LINE, stdin);

		validation(aux,buffer,conn_s);
    
        strtok(aux, "\n");
        strcat(buffer," ");
        strcat(buffer,aux);
        
        //inserimento nome
        printf("Inserisci il nome del contatto che si desidera aggiungere (20 caratteri consentiti):\n ");
		fgets(aux, MAX_LINE, stdin);
		
		validation(aux,buffer,conn_s);
        
        strtok(aux, "\n");
        strcat(buffer," ");
        strcat(buffer,aux);
        
        //inserimento numero
        printf("Inserisci il numero del contatto che si desidera aggiungere (20 caratteri consentiti):\n ");
		fgets(aux, MAX_LINE, stdin);
		
		validation(aux,buffer,conn_s);
        
        strcat(buffer," ");
        strcat(buffer,aux);
    }
    
    
    /*operazione "cerca record"*/
    
    else {
		
        strcpy(buffer,"O 2");
        
        //inserimento cognome
		printf("Inserisci il cognome del contatto che si desidera cercare:\n ");
		fgets(aux, MAX_LINE, stdin);	

		validation(aux,buffer,conn_s);

		strtok(aux, "\n");
        strcat(buffer," ");
        strcat(buffer,aux);
        
        //inserimento nome
		printf("Inserisci il nome del contatto che si desidera cercare:\n ");
		fgets(aux, MAX_LINE, stdin);
	
		validation(aux,buffer,conn_s);

        strcat(buffer," ");
        strcat(buffer,aux);
        }
    
    /*avvio seconda connessione per inviare i dati relativi all' operazione*/
    
    if ( (conn_s = socket(AF_INET, SOCK_STREAM, 0)) < 0 ) {
		fprintf(stderr, "client: errore durante la creazione della socket.\n");
		exit(EXIT_FAILURE);
    	}
    
    if ( connect(conn_s, (struct sockaddr *) &servaddr, sizeof(servaddr) ) < 0 ) {
		printf("client: errore durante la connect.\n");
		exit(EXIT_FAILURE); 
    	}
        
    /*printf("inizio sleep\n");
    sleep(5);
    printf("fine sleep\n");*/
        
    Writeline(conn_s, buffer, strlen(buffer));
    memset(buffer, 0, sizeof(char)*(strlen(buffer)+1));
    
    Readline(conn_s, buffer, MAX_LINE-1);
    printf("%s\n",buffer);
    
    if(strcmp(buffer,"Impossibile inserire il numero poichè è già presente nell'elenco\n")==0 || strcmp(buffer,"Numero non presente nell' elenco\n")==0){
        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "server: errore durante la close.\n");
			}
        exit(EXIT_FAILURE);
    }
    if(strcmp(buffer,"")==0){
        printf("Impossibile erogare il servizio, problemi con il server o tempo per la scelta scaduto. Provare a riconnettere \n");
        if ( close(conn_s) < 0 ) {
            fprintf(stderr, "server: errore durante la close.\n");
			}
        exit(EXIT_FAILURE);
        }
        
    
    if ( close(conn_s) < 0 ) {
        fprintf(stderr, "server: errore durante la close.\n");
			}
    
    return EXIT_SUCCESS;
}


/*-------------------------------------------------------------------FUNZIONI----------------------------------------------------------------------------------*/


/*---------------------------------------------------------------INPUT VALIDATION----------------------------------------------------------------*/

void validation(char* aux,char*buffer,int conn_s){
    if(strlen(aux)>(21)) { 
        printf("Numero di caratteri consentiti superato, impossibile completare l'operazione. \n");
        exit(EXIT_FAILURE);
		}

    if(strcmp(aux,"\n")==0) {
        printf("il campo non può essere vuoto\n");
        exit(EXIT_FAILURE);
		}
    }
    
    
void gestione_segnali(int signal) {

	switch (signal) {

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
    if ( close(conn_s) < 0 ) {
        fprintf(stderr, "server: errore durante la close.\n");
			}
    exit(EXIT_FAILURE);
    }

    

/*---------------------------------------------------------------PARSING---------------------------------------------------------------------------*/

int ParseCmdLine(int argc, char *argv[], char **szAddress, char **szPort)
{
    int n = 1;

    while ( n < argc )
	{
		if ( !strncmp(argv[n], "-a", 2) || !strncmp(argv[n], "-A", 2) )
		{
		    *szAddress = argv[++n];
		}
		else 
			if ( !strncmp(argv[n], "-p", 2) || !strncmp(argv[n], "-P", 2) )
			{
			    *szPort = argv[++n];
			}
			else
				if ( !strncmp(argv[n], "-h", 2) || !strncmp(argv[n], "-H", 2) )
				{
		    		printf("Sintassi:\n\n");
			    	printf("    client -a (indirizzo server) -p (porta del server) [-h].\n\n");
			    	exit(EXIT_SUCCESS);
				}
		++n;
    }
	if (argc==1)
	{
   		printf("Sintassi:\n\n");
    	printf("    client -a (indirizzo server) -p (porta del server) [-h].\n\n");
	    exit(EXIT_SUCCESS);
	}
    return 0;
}
