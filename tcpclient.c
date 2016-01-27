/* tcp_ client.c */
/* Programmed by Ye Wang */
/* October 11, 2015 */

#include <stdio.h>                                      /* for standard I/O functions */
#include <stdlib.h>                                     /* for exit */
#include <string.h>                                     /* for memset, memcpy, and strlen */
#include <netdb.h>                                      /* for struct hostent and gethostbyname */
#include <sys/socket.h>                                 /* for socket, connect, send, and recv */
#include <netinet/in.h>                                 /* for sockaddr_in */
#include <unistd.h>                                     /* for close */

#define STRING_SIZE 1024

int main( void )
{
    int sock_client;                                /* Socket used by client */
    
    struct sockaddr_in server_addr;                 /* Internet address structure that
                                                     *  stores server address */
    struct hostent * server_hp;                     /* Structure to store server's IP
                                                     *  address */
    char		server_hostname[STRING_SIZE];   /* Server's hostname */
    unsigned short	server_port;                    /* Port number used by server (remote port) */
    
    char	operationString[STRING_SIZE];       /* send message */
    char	ResultString[STRING_SIZE];  /* receive message */
    unsigned int	msg_len;                        /* length of message */
    int		bytes_sent, bytes_recd;         /* number of bytes sent or received */
    
    char	operationNo;    //get operation input by user
    char	c;  //to extract user's input like '\n'
    int	i	= 0;    //used in loop
//used to store the amount of money user input
    typedef union
    {
        int	amount;
        char	amountStr[4];
    }Money;
    
    Money money, UpdatedAccount, OriginalAccount;
    
    /* open a socket */
    if ( (sock_client = socket( PF_INET, SOCK_STREAM, IPPROTO_TCP ) ) < 0 )
    {
        perror( "Client: can't open stream socket" );
        exit( 1 );      }
    
//    char	hostname[9]	= "localhost";//set hostname
//    int	j	= 0;
//    for (; j < 9; j++ )
//    {
//        server_hostname[j] = hostname[j];
//    }
    printf("Enter hostname of server: ");
    scanf("%s", server_hostname);
    if ( (server_hp = gethostbyname( server_hostname ) ) == NULL )
    {
        perror( "Client: invalid server hostname" );
        close( sock_client );
        exit( 1 );
    }
    /*   port number for server */
    server_port = 12345;
    /* Clear server address structure and initialize with server address */
    memset( &server_addr, 0, sizeof(server_addr) );
    server_addr.sin_family = AF_INET;
    memcpy( (char *) &server_addr.sin_addr, server_hp->h_addr,
           server_hp->h_length );
    server_addr.sin_port = htons( server_port );
    
    /* connect to the server */
    if ( connect( sock_client, (struct sockaddr *) &server_addr,
                 sizeof(server_addr) ) < 0 )
    {
        perror( "Client: can't connect to server" );
        close( sock_client );
        exit( 1 );
    }

    /* user interface */
    while ( 1 )
    {
        printf( "\033c" ); /* clear output */
        //initial strings
        strcpy( operationString, "" );
        strcpy( ResultString, "" );
        
        printf( "*************************Welcome to WANG'S bank*****************************\n" );
        printf( "*   What service do you want?                                              *\n" );
        printf( "*                    1. Check balance                                      *\n" );
        printf( "*                        2. Deposit                                        *\n" );
        printf( "*                            3. Withdraw                                   *\n" );
        printf( "*                                4. Transfer                               *\n" );
        printf( "*                                    5. Exit                               *\n" );
        printf( "******************Please input the operation number:************************\n" );
        
        //filter user's useless input
        while ( (c = getchar() ) == '\n' )
            ;
        operationNo = c;
        while ( (c = getchar() ) != '\n' && c != EOF )
            ;
        //1. Check balance
        if ( operationNo == '1' )
        {
            operationString[0] = operationNo;
            printf( "Please choose the account you want to operate.\n    1.Checking  2.Saving    3.Back to the main page\n" );
            printf( "Please input the operation number:\n" );
            while ( 1 )
            {
                //filter user's useless input
                while ( (c = getchar() ) == '\n' )
                    ;
                operationNo = c;
                while ( (c = getchar() ) != '\n' && c != EOF )
                    ;
                // choose the account 1.Checking  2.Saving
                if ( operationNo == '1' || operationNo == '2' )
                {
                    operationString[1] = operationNo;
                    bytes_sent	= send( sock_client, operationString, 2, 0 );
                    bytes_recd	= recv( sock_client, ResultString, STRING_SIZE, 0 );
                    //print request and response message
                    printf( "\n***\nRequest message is:" );
                    for ( i = 0; i < bytes_sent; i++ )
                    {
                        printf( "%c", operationString[i] );
                    }
                    printf( "\n" );
                    printf( "Length of request message: %d bytes\n", bytes_sent );
                    
                    printf( "\nResponse message is:" );
                    for ( i = 0; i < bytes_recd; i++ )
                    {
                        printf( "%c", ResultString[i] );
                    }
                    printf( "\n" );
                    printf( "Length of response message: %d bytes\n", bytes_recd );
                    // handle response message
                    if ( ResultString[0] == '1' )
                    {
                        int i;
                        for ( i = 2; i < 6; i++ )
                        {
                            UpdatedAccount.amountStr[i - 2] = ResultString[i];
                        }
                        printf("\nThe amount you get from ResponseMsg[2-5] is: %d\n***\n\n",UpdatedAccount.amount);
                        if ( ResultString[1] == '1' )
                        {
                            printf( "Your checking account: %d dollars\n\n", UpdatedAccount.amount );
                        }else if ( ResultString[1] == '2' )
                        {
                            printf( "Your saving account: %d dollars\n\n", UpdatedAccount.amount );
                        }
                        break;
                    }
                    break;
                //3.Back to the main page
                }else if ( operationNo == '3' )
                {
                    int i;
                    printf( "Back to the main page......\n" );
                    for ( i = 0; i < 1000000; i++ )
                    {
                        printf( "" );
                    }
                    break;
                }else  {
                    printf( "Wrong input, please input a right operation code:\n" );
                }
            }
        //handle transaction   2. Deposit  3. Withdraw  4. Transfer
        }else if ( operationNo == '2' || operationNo == '3' || operationNo == '4' )
        {
            operationString[0] = operationNo;
            printf( "Please choose the account you want to operate.\n    1.Checking  2.Saving    3.Back to the main page\n" );
            printf( "Please input the operation number:\n" );
            
            while ( 1 )
            {
                while ( (c = getchar() ) == '\n' )
                    ;
                operationNo = c;
                while ( (c = getchar() ) != '\n' && c != EOF )
                    ;
                if ( operationNo == '1' || operationNo == '2' )
                {
                    //input the amount of money
                    operationString[1] = operationNo;
                    printf( "Please input the amount of money you want to operate:\n" );
                    int t = scanf( "%d", &money.amount );
                    
                    c = getchar();
                    //check the input
                    while ( c == '.' || t < 1 )
                    {
                        printf( "Error: The amount of money should be integer!\n\n" );
                        printf( "Please input the amount of money you want to operate:\n" );
                        while ( (c = getchar() ) != '\n' && c != EOF )
                            ;
                        t	= scanf( "%d", &money.amount );
                        c	= getchar();
                    }
                    //store the amount into request message
                    operationString[2]	= money.amountStr[0];
                    operationString[3]	= money.amountStr[1];
                    operationString[4]	= money.amountStr[2];
                    operationString[5]	= money.amountStr[3];

                    bytes_sent	= send( sock_client, operationString, 6, 0 );
                    bytes_recd	= recv( sock_client, ResultString, STRING_SIZE, 0 );
                    //print request and response message
                    printf( "\n***\nRequest message is:" );
                    for ( i = 0; i < bytes_sent; i++ )
                    {
                        printf( "%c", operationString[i] );
                    }
                    printf( "\n" );
                    printf( "Length of request message: %d bytes\n", bytes_sent );
                    
                    printf( "\nResponse message is:" );
                    for ( i = 0; i < bytes_recd; i++ )
                    {
                        printf( "%c", ResultString[i] );
                    }
                    printf( "\n" );
                    printf( "Length of response message: %d bytes\n", bytes_recd );
                    //handle response message
                    if ( ResultString[0] != 'f' )
                    {
                        //get the amount from response message
                        int i;
                        for ( i = 2; i < 6; i++ )
                        {
                            UpdatedAccount.amountStr[i - 2] = ResultString[i];
                        }
                        for ( i = 6; i < bytes_recd; i++ )
                        {
                            OriginalAccount.amountStr[i - 6] = ResultString[i];
                        }
                        //print the infomation contained in the response message
                        printf("\nThe amount you get from ResponseMsg[2-5] is: %d\n",UpdatedAccount.amount);
                        printf("The amount you get from ResponseMsg[6-9] is: %d\n***\n\n",OriginalAccount.amount);
                        if ( ResultString[1] == '1' )
                        {
                            printf( "Your Original checking account: %d dollars\n", OriginalAccount.amount );
                            printf( "Your new checking account: %d dollars\n\n", UpdatedAccount.amount );
                        }else if ( ResultString[1] == '2' )
                        {
                            printf( "Your Original saving account: %d dollars\n", OriginalAccount.amount );
                            printf( "Your new saving account: %d dollars\n\n", UpdatedAccount.amount );
                        }
                        break;
                    }else      {
                        //handle error condition
                        printf( "\nFail!\n" );
                        /* Error code */
                        printf( "Error code %c:\n", ResultString[1] );
                        switch ( ResultString[1] )
                        {
                            //print different error condition according to the error code
                            case '1':
                                printf( "    The money operated should be less than 1,000,000 dollars!\n\n" );
                                break;
                            case '2':
                                printf( "    Withdrawing from saving account is not permitted!\n\n" );
                                break;
                            case '3':
                                printf( "    The amount to be withdrawn should only be in multiples of 20 dollars!\n\n" );
                                break;
                            case '4':
                                printf( "    The amount to be withdrawn should not be greater than the current balance in the checking account!\n\n" );
                                break;
                            case '5':
                                printf( "    The amount to be transferred should not be greater than the current balance in the checking account!\n\n" );
                                break;
                            case '6':
                                printf( "    The amount to be transferred should not be greater than the current balance in the saving account!\n\n" );
                                break;
                            case '7':
                                printf( "    The amount to be operated should be greater than zero!\n\n" );
                                break;
                            default:
                                printf( "    Unknown error!\n\n" );
                                break;
                        }
                        break;
                    }
                }else if ( operationNo == '3' )
                {
                    //back to the main page
                    printf( "Back to the main page......\n" );
                    break;
                }else  {
                    //unexpected input
                    printf( "Wrong input, please input a right operation code:\n" );
                }
            }
        }else if ( operationNo == '5' )
        {
            //eixt
            operationString[0]	= operationNo;
            bytes_sent		= send( sock_client, operationString, 1, 0 );
            bytes_recd		= recv( sock_client, ResultString, STRING_SIZE, 0 );
            //print request and response message
            printf( "\n***\nRequest message is:" );
            for ( i = 0; i < bytes_sent; i++ )
            {
                printf( "%c", operationString[i] );
            }
            printf( "\n" );
            printf( "Length of request message: %d bytes\n", bytes_sent );
            
            printf( "\nResponse message is:" );
            for ( i = 0; i < bytes_recd; i++ )
            {
                printf( "%c", ResultString[i] );
            }
            printf( "\n" );
            printf( "Length of response message: %d bytes\n***\n", bytes_recd );
           //handle response
            if ( ResultString[0] == '5' )
            {
                printf( "\n\nSuccess!\nBye!\n" );
                break;
            }
        }else  {
            //unexpected input
            printf( "Wrong input, please input a right operation code!\n\n" );
        }
        
        printf( "Input 'Enter' to turn back to the main page...\n" );
        while ( (c = getchar() ) != '\n' && c != EOF )
            ;
    }
    /* close the socket */
    close( sock_client );
}

