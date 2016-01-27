/* tcpserver.c */
/* Programmed by Ye Wang */
/* October 11, 2015 */

#include <ctype.h>              /* for toupper */
#include <stdio.h>              /* for standard I/O functions */
#include <stdlib.h>             /* for exit */
#include <string.h>             /* for memset */
#include <sys/socket.h>         /* for socket, bind, listen, accept */
#include <netinet/in.h>         /* for sockaddr_in */
#include <unistd.h>             /* for close */

#define STRING_SIZE 1024
#define SERV_TCP_PORT 12345
//union:Money is used to store info about account and used to tranfer message between clinet and server
typedef union
{
    int	amount;
    char	amountStr[4];
}Money;
//Account structure is used to store account info
struct Account {
    Money	checking_account;
    Money	saving_account;
};
//define functions used to store account info into the response message
void StoreOriginalCheckingAccount(char * ResponsedMsg,struct Account account);
void StoreOriginalSavingAccount(char * ResponsedMsg,struct Account account);
void StoreNewCheckingAccount(char * ResponsedMsg,struct Account account);
void StoreNewSavingAccount(char * ResponsedMsg,struct Account account);

int main( void )
{
    int	sock_server;                    /* Socket on which server listens to clients */
    int	sock_connection;                /* Socket on which server exchanges data with client */
    
    struct sockaddr_in server_addr;         /* Internet address structure that
                                             *  stores server address */
    unsigned int	server_addr_len;        /* Length of server address structure */
    unsigned short	server_port;            /* Port number used by server (local port) */
    
    struct sockaddr_in client_addr;         /* Internet address structure that
                                             *  stores client address */
    unsigned int client_addr_len;           /* Length of client address structure */

    unsigned int	msg_len;                /* length of message */
    int		bytes_sent, bytes_recd; /* number of bytes sent or received */
    unsigned int	i;                      /* temporary loop variable */

    //initialize account
    struct Account account;
    account.checking_account.amount = 0;
    account.saving_account.amount	= 0;
    
    
    /* open a socket */
    
    if ( (sock_server = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    {
        perror( "Server: can't open stream socket" );
        exit( 1 );
    }
    
    /* initialize server address information */
    
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family		= AF_INET;
    server_addr.sin_addr.s_addr	= htonl( INADDR_ANY );  /* This allows choice of
                                                         * any host interface, if more than one
                                                         * are present */
    server_port			= SERV_TCP_PORT;        /* Server will listen on this port */
    server_addr.sin_port		= htons( server_port );
    
    /* bind the socket to the local server port */
    
    if ( bind( sock_server, (struct sockaddr *) &server_addr,
              sizeof(server_addr) ) < 0 )
    {
        perror( "Server: can't bind to local address" );
        close( sock_server );
        exit( 1 );
    }
    
    /* listen for incoming requests from clients */
    
    if ( listen( sock_server, 50 ) < 0 )            /* 50 is the max number of pending */
    {
        perror( "Server: error on listen" );    /* requests that will be queued */
        close( sock_server );
        exit( 1 );
    }
    printf( "Server of Wang's Bank : listen on port %hu\n\n", server_port );
    
    client_addr_len = sizeof(client_addr);          /* get the length of client adress structure  */
    
    /* wait for incoming connection requests in an indefinite loop */
    for (;; )
    {
        sock_connection = accept( sock_server, (struct sockaddr *) &client_addr,
                                 &client_addr_len );
        
        if ( sock_connection < 0 )
        {
            perror( "Server: accept() error\n" );
            close( sock_server );
            exit( 1 );
        }
        /* receive the message */
        while ( 1 )
        {
            //initialize request and response message
            char	* RequestMsg	= (char *) malloc( sizeof(char) * STRING_SIZE );
            char	* ResponsedMsg	= (char *) malloc( sizeof(char) * STRING_SIZE );
            bytes_recd = recv( sock_connection, RequestMsg, STRING_SIZE, 0 );
            /* print request and response message */
            if ( bytes_recd > 0 )
            {
                printf( "***\nRequest message is:" );
                for ( i = 0; i < bytes_recd; i++ )
                {
                    printf( "%c", RequestMsg[i] );
                }
                printf( "\n" );
                printf( "Length of request message: %d bytes\n", bytes_recd );
                if(bytes_recd>1){
                    printf("The transaction type code is %c\nThe account type code is: %c\n",RequestMsg[0],RequestMsg[1]);
                }

                if ( RequestMsg[0] == '5' )
                {
                    //print response message
                    printf("The operation type code is: %c\n\n",'5');
                    printf( "Response message is:" );
                    printf( "5\n" );
                    printf( "Length of response message: 1 bytes\n" );
                    printf("The operation type code is: %c\n***\n\n",'5');
                    bytes_sent = send( sock_connection, "5", 1, 0 );
                    //close connection socket
                    close( sock_connection );
                    free( RequestMsg );
                    free( ResponsedMsg );
                    break;
                }else if ( RequestMsg[0] == '1' && RequestMsg[1] == '1' )
                {
                    //print the amount stored in the response message
                    printf("\nThe integer stored in ResponsedMsg[2-5] is: %d\n",account.checking_account.amount);
                    //  check checking balance
                    ResponsedMsg[0] = '1';
                    ResponsedMsg[1] = '1';
                    StoreNewCheckingAccount(ResponsedMsg,account);
                    msg_len		= 6;
                }else if ( RequestMsg[0] == '1' && RequestMsg[1] == '2' )
                {
                    //print the amount stored in the response message
                    printf("\nThe integer stored in ResponsedMsg[2-5] is: %d\n",account.saving_account.amount);
                    /* check saving balance */
                    ResponsedMsg[0] = '1';
                    ResponsedMsg[1] = '2';
                    StoreNewSavingAccount(ResponsedMsg,account);
                    msg_len		= 6;
                }else  {
                    //get the money info from request message
                    Money ReceivedMoney;
                    ReceivedMoney.amountStr[0]	= RequestMsg[2];
                    ReceivedMoney.amountStr[1]	= RequestMsg[3];
                    ReceivedMoney.amountStr[2]	= RequestMsg[4];
                    ReceivedMoney.amountStr[3]	= RequestMsg[5];
                    printf("The integer stored in RequestMsg[2-5] is: %d\n",ReceivedMoney.amount);
                    //handle some error conditions
                    if ( ReceivedMoney.amount > 1000000 )
                    {
                        ResponsedMsg[0] = 'f';
                        ResponsedMsg[1] = '1';
                        msg_len		= 2;
                    }else if ( ReceivedMoney.amount < 0 )
                    {
                        ResponsedMsg[0] = 'f';
                        ResponsedMsg[1] = '7';
                        msg_len		= 2;
                    //organize response message
                    }else if ( RequestMsg[0] == '2' && RequestMsg[1] == '1' )
                    {
                        //print the amount stored in the response message
                        printf("\nThe original checking account is: %d, it is stored in ResponsedMsg[6-9] \n",account.checking_account.amount);
                        /* deposit to checking */
                        //store original account info
                        StoreOriginalCheckingAccount(ResponsedMsg,account);
                        //handle request
                        account.checking_account.amount += ReceivedMoney.amount;
                       //print the amount stored in the response message
                        printf("The new checking account is: %d, it is stored in ResponsedMsg[2-5] \n",account.checking_account.amount);
                        //store operation type
                        ResponsedMsg[0] = '2';
                        ResponsedMsg[1] = '1';
                        //store new account info
                        StoreNewCheckingAccount(ResponsedMsg,account);
                        msg_len		= 10;
                    }else if ( RequestMsg[0] == '2' && RequestMsg[1] == '2' )
                    {
                        //print the amount stored in the response message
                        printf("\nThe original saving account is: %d, it is stored in ResponsedMsg[6-9] \n",account.saving_account.amount);
                        /* deposit to saving */
                        StoreOriginalSavingAccount(ResponsedMsg,account);
                        account.saving_account.amount	+= ReceivedMoney.amount;
                       //print the amount stored in the response message
                        printf("The new saving account is: %d, it is stored in ResponsedMsg[2-5] \n",account.saving_account.amount);
                        ResponsedMsg[0] = '2';
                        ResponsedMsg[1] = '2';
                        StoreNewSavingAccount(ResponsedMsg,account);
                        msg_len		= 10;
                    }else if ( RequestMsg[0] == '3' && RequestMsg[1] == '1' )
                    {
                        /* withdraw from checking->check 20n,<1000000,no cents */
                        if ( ReceivedMoney.amount % 20 != 0 )
                        {
                            ResponsedMsg[0] = 'f';
                            ResponsedMsg[1] = '3';
                            msg_len		= 2;
                        }else if ( ReceivedMoney.amount > account.checking_account.amount )
                        {
                            ResponsedMsg[0] = 'f';
                            ResponsedMsg[1] = '4';
                            msg_len		= 2;
                        }else  {
                            //print the amount stored in the response message
                            printf("\nThe original checking account is: %d, it is stored in ResponsedMsg[6-9] \n",account.checking_account.amount);
                            StoreOriginalCheckingAccount(ResponsedMsg,account);
                            account.checking_account.amount -= ReceivedMoney.amount;
                            //print the amount stored in the response message
                            printf("The new checking account is: %d, it is stored in ResponsedMsg[2-5] \n",account.checking_account.amount);
                            ResponsedMsg[0] = '3';
                            ResponsedMsg[1] = '1';
                            StoreNewCheckingAccount(ResponsedMsg,account);
                            msg_len		= 10;
                        }
                    }else if ( RequestMsg[0] == '3' && RequestMsg[1] == '2' )
                    {
                        /* withdraw from saving: error */
                        ResponsedMsg[0] = 'f';
                        ResponsedMsg[1] = '2';
                        msg_len		= 2;
                    }else if ( RequestMsg[0] == '4' && RequestMsg[1] == '1' )
                    {
                        /* transfer from checking to saving */
                        if ( ReceivedMoney.amount > account.checking_account.amount )
                        {
                            ResponsedMsg[0] = 'f';
                            ResponsedMsg[1] = '5';
                            msg_len		= 2;
                        }else  {
                            //print the amount stored in the response message
                            printf("\nThe original saving account is: %d, it is stored in ResponsedMsg[6-9] \n",account.saving_account.amount);
                            //handle the request message and create response message
                            StoreOriginalSavingAccount(ResponsedMsg,account);
                            account.checking_account.amount -= ReceivedMoney.amount;
                            account.saving_account.amount	+= ReceivedMoney.amount;
                            //print the amount stored in the response message
                            printf("The new saving account is: %d, it is stored in ResponsedMsg[2-5] \n",account.saving_account.amount);
                            ResponsedMsg[0] = '4';
                            ResponsedMsg[1] = '2';
                            StoreNewSavingAccount(ResponsedMsg,account);
                            msg_len		= 10;
                        }
                    }else if ( RequestMsg[0] == '4' && RequestMsg[1] == '2' )
                    {
                        /* transfer saving to checking */
                        if ( ReceivedMoney.amount > account.saving_account.amount )
                        {
                            //handle the request message and create response message
                            ResponsedMsg[0] = 'f';
                            ResponsedMsg[1] = '6';
                            msg_len		= 2;
                        }else  {
                            //print the amount stored in the response message
                            printf("\nThe original checking account is: %d, it is stored in ResponsedMsg[6-9] \n",account.checking_account.amount);
                            //handle the request message and create response message
                            StoreOriginalCheckingAccount(ResponsedMsg,account);
                            account.saving_account.amount	-= ReceivedMoney.amount;
                            account.checking_account.amount += ReceivedMoney.amount;
                            //print the amount stored in the response message
                            printf("The new checking account is: %d, it is stored in ResponsedMsg[2-5] \n",account.checking_account.amount);
                            ResponsedMsg[0] = '4';
                            ResponsedMsg[1] = '1';
                            StoreNewCheckingAccount(ResponsedMsg,account);
                            msg_len		= 10;
                        }
                    }else  {
                        ResponsedMsg[0] = 'f';
                        ResponsedMsg[1] = '9';
                        msg_len		= 2;
                    }
                }
                //print Response message when error happened
                if(ResponsedMsg[0]=='f'){
                    printf("\nThe operation type code is: f\nThe error code is %c\n",ResponsedMsg[1]);
                }
                else{
                printf("The transaction type code is: %c\nThe account type code is %c\n",ResponsedMsg[0],ResponsedMsg[1]);
                }
                //print the information of the response message
                printf( "Response message is:" );
                for ( i = 0; i < msg_len; i++ )
                {
                    printf( "%c", ResponsedMsg[i] );
                }
                printf( "\n" );
                printf( "Length of response message: %d bytes\n***\n\n", msg_len );


                //send response message
                bytes_sent = send( sock_connection, ResponsedMsg, msg_len, 0 );
                free( RequestMsg );
                free( ResponsedMsg );
            }
        }
    }
}
//functions to store checking or saving account to the response message
void StoreOriginalCheckingAccount(char * ResponsedMsg,struct Account account){
    ResponsedMsg[6] = account.checking_account.amountStr[0];
    ResponsedMsg[7] = account.checking_account.amountStr[1];
    ResponsedMsg[8] = account.checking_account.amountStr[2];
    ResponsedMsg[9] = account.checking_account.amountStr[3];
}
void StoreOriginalSavingAccount(char * ResponsedMsg,struct Account account){
    ResponsedMsg[6]	= account.saving_account.amountStr[0];
    ResponsedMsg[7]	= account.saving_account.amountStr[1];
    ResponsedMsg[8]	= account.saving_account.amountStr[2];
    ResponsedMsg[9]	= account.saving_account.amountStr[3];
}
void StoreNewCheckingAccount(char * ResponsedMsg,struct Account account){
    ResponsedMsg[2] = account.checking_account.amountStr[0];
    ResponsedMsg[3] = account.checking_account.amountStr[1];
    ResponsedMsg[4] = account.checking_account.amountStr[2];
    ResponsedMsg[5] = account.checking_account.amountStr[3];
}
void StoreNewSavingAccount(char * ResponsedMsg,struct Account account){
    ResponsedMsg[2] = account.saving_account.amountStr[0];
    ResponsedMsg[3] = account.saving_account.amountStr[1];
    ResponsedMsg[4] = account.saving_account.amountStr[2];
    ResponsedMsg[5] = account.saving_account.amountStr[3];
}
