
/*

Nicholas LaRosa
CSE 30264, Program 1

usage: udpclient <IP_Address> OR <Host_Name> <Port_Number> <File_to_Send>

*/

#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#define BUFFER 1400

int main( int argc, char** argv )
{
	int sockfd, inputBytes, outputBytes, isHost, i;
	struct sockaddr_in serverAddress;
	struct addrinfo *hostInfo, *p;
	char sendLine[ BUFFER + 1 ];
	char recvLine[ BUFFER + 1 ];
	char ipstr[INET6_ADDRSTRLEN];
	int err;

	if( argc != 4 )
	{
		printf("\nusage: udpclient <IP_Address> OR <Host_Name> <Port_Number> <File_to_Send>\n\n");
		exit( 1 );
	}
	
	if( ( sockfd = socket( AF_INET, SOCK_DGRAM, 0 ) ) < 0 )		// open datagram socket, address family internet
	{
		perror( "Client - socket() error" );
		exit( 1 );
	}

	FILE *filePtr = fopen( argv[3], "r" );				// prepare storing of file to buffer
	if( filePtr != NULL )						// credit to Michael, StackOverflow
	{
		unsigned int length = fread( sendLine, sizeof( char ), BUFFER, filePtr );
		if( length == 0 )
		{
			fputs( "Error reading file.", stderr );
		}
		else
		{
			sendLine[ ++length ] = '\0';
		}

		fclose( filePtr );
	}
	else
	{
		fputs( "No such file exists.", stderr );
	}

	//bzero( &serverAddress, sizeof( serverAddress ) );
	memset( ( char * )&serverAddress, 0, sizeof( struct sockaddr_in ) );	// secure enough memory for the server socket
	serverAddress.sin_family = AF_INET;
	serverAddress.sin_port = htons( atoi( argv[2] ) );			// and receive server port

	isHost = 0;								// check for host name or IP address

	for( i = 0; i < strlen( argv[1] ); i++ )
	{
		if( argv[1][i] > 64 )
		{
			isHost = 1;
			break;
		}
	}
	
	if( isHost )
	{
   		if( ( err = getaddrinfo( argv[1], NULL, NULL, &hostInfo ) ) < 0 ) {
			printf("getaddrinfo() error %d\n", err);
			return 1;
		}
	
		// Source: Beej's Guide to Network Programming
		for( p = hostInfo; p != NULL; p = p->ai_next ) 
		{
			void *addr;

			if( p->ai_family == AF_INET )		// IPv4 
			{
				struct sockaddr_in *ipv4 = (struct sockaddr_in *)p->ai_addr;
				addr = &(ipv4->sin_addr);
			}
			else 					// IPv6
			{
				struct sockaddr_in6 *ipv6 = (struct sockaddr_in6 *)p->ai_addr;
				addr = &(ipv6->sin6_addr);
			}
			
			// convert the IP to a string and set it as our server
			inet_ntop(p->ai_family, addr, ipstr, sizeof ipstr);		
			serverAddress.sin_addr.s_addr = inet_addr( ipstr );
			printf( "%s\n", ipstr );
		}
		
		freeaddrinfo( hostInfo );		// free the linked list
	}
	else
	{
		serverAddress.sin_addr.s_addr = inet_addr( argv[1] );			// receive server address
	}

	if( strlen( sendLine ) > BUFFER )
	{
		printf( "File sent cutoff at 1400 bytes.\n" );
		sendLine[ BUFFER - 1 ] = '\0';
	}

	if( ( outputBytes = sendto( sockfd, sendLine, strlen( sendLine ), 0, ( struct sockaddr * )&serverAddress, sizeof( struct sockaddr_in ) ) ) < 0 )
	{
		perror( "Client - sendto() error" );
		exit( 1 );
	}
	
	if( ( inputBytes = recvfrom( sockfd, recvLine, BUFFER, 0, NULL, NULL ) ) < 0 )
	{
		perror( "Client - recvfrom() error" );
		exit( 1 );
	}

	recvLine[ inputBytes ] = '\0';	
	
	fputs( recvLine, stdout);	

	if( close( sockfd ) != 0 )					// close the socket
	{
		printf( "Client - sockfd closing failed!\n" );
	}		

	return 0;
}

