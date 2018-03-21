/* tcpserver.c */
/* Programmed by Adarsh Sethi */
/* February 21, 2018 */

#include <ctype.h>          /* for toupper */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset */
#include <sys/socket.h>     /* for socket, bind, listen, accept */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include "datastruct.h"

#define STRING_SIZE 1024

/* SERV_TCP_PORT is the port number on which the server listens for
   incoming requests from clients. You should change this to a different
   number to prevent conflicts with others in the class. */

#define SERV_TCP_PORT 46464

int main(void) {

	int sock_server;  /* Socket on which server listens to clients */
	int sock_connection;  /* Socket on which server exchanges data with client */

	struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
	unsigned int server_addr_len;  /* Length of server address structure */
	unsigned short server_port;  /* Port number used by server (local port) */

	struct sockaddr_in client_addr;  /* Internet address structure that
                                        stores client address */
	unsigned int client_addr_len;  /* Length of client address structure */

	char sentence[STRING_SIZE];  /* receive message */
	char modifiedSentence[STRING_SIZE]; /* send message */
	int bytes_sent, bytes_recd; /* number of bytes sent or received */
	unsigned int i;  /* temporary loop variable */

	/* open a socket */

	if ((sock_server = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Server: can't open stream socket");
		exit(1);
	}

	/* initialize server address information */

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = htonl (INADDR_ANY);  /* This allows choice of
                                        any host interface, if more than one
                                        are present */
	server_port = SERV_TCP_PORT; /* Server will listen on this port */
	server_addr.sin_port = htons(server_port);

	/* bind the socket to the local server port */

	if (bind(sock_server, (struct sockaddr *) &server_addr,
			sizeof (server_addr)) < 0) {
		perror("Server: can't bind to local address");
		close(sock_server);
		exit(1);
	}

	/* listen for incoming requests from clients */

	if (listen(sock_server, 50) < 0) {    /* 50 is the max number of pending */
		perror("Server: error on listen"); /* requests that will be queued */
		close(sock_server);
		exit(1);
	}
	printf("I am here to listen ... on port %hu\n\n", server_port);

	client_addr_len = sizeof (client_addr);

	/* wait for incoming connection requests in an indefinite loop */

	for (;;) {

		sock_connection = accept(sock_server, (struct sockaddr *) &client_addr,
				&client_addr_len);
		/* The accept function blocks the server until a
                        connection request comes from a client */
		if (sock_connection < 0) {
			perror("Server: accept() error\n");
			close(sock_server);
			exit(1);
		}

		/* receive the message */
		bytes_recd = recv(sock_connection, sentence, STRING_SIZE, 0);
		if (bytes_recd > 0){
			printf("Received file is:\n");
			printf("%s", sentence);
			printf("\nwith length %d\n\n", bytes_recd);
			/* send message */
			FILE *file;
			file = fopen(sentence, "r");	// The file to read in
			if (file) {
				char * line;	// A pointer to the line that will be read in
				unsigned short seqnum = 0;	// Sequence number of the packet to be sent
				size_t linelen = 0;	// The length of the line to be read in
				while (getline(&line, &linelen, file) > 0) {	
					/* Getting the next line, continuously going until no more lines left */
					printf("Read line is:\n");
					printf("%s", line);
					unsigned short header[2] = {htons((seqnum++)-1), htons((unsigned short) linelen)};
					/* This header contains the sequence number in the [0] position and
					 * the line length in the [1] position. Both of these consist of unsigned short
					 * integers. The integers are converted to Network Order by the htons() function.
					 * The header is now ready to be sent. */
					bytes_sent = send(sock_connection, header, sizeof(header), 0);
					/* The header is sent using the send() function to the client */
					bytes_sent = send(sock_connection, line, linelen, 0);
					/* Send the line read in */
					printf("Sent line is:\n");
					printf("%s", line);
				} // No more lines
				unsigned short header[2] = {htons((seqnum++)-1), htons(0)};
				/* After all of the lines are read through, a header is sent
				 * with a data-size of the next line as 0 indicating no more lines
				 * left, and the sequence value is incremented */
				bytes_sent = send(sock_connection, header, sizeof(header), 0);
				/* Send the header, indicating communication is done */
			}
			printf("All lines sent\n");
			if (ferror(file)) {	// If there is a file error
				close(sock_connection); 
				exit(1);
				/* Close the connection and exit */
			}
			fclose(file);	// Close the file
		}
		break;	// Break out of the loop
	}
	/* close the socket */
	close(sock_connection);
	printf("Socket closed.\n");
}
