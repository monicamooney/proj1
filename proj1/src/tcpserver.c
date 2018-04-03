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
		char sentence[STRING_SIZE];  /* receive message */
		int bytes_sent, bytes_recd; /* number of bytes sent or received */
		unsigned short header[2];	// The header that the filename info will be held in
		int seqnum ,linelen;	// The length of the line to be read in
		if ((bytes_recd = recv(sock_connection, header, 4, 0)) != 4  || (seqnum = ntohs(header[0])) != 0 || (linelen = ntohs(header[1])) < 0) {
			/* The recv function will place the header into the header array. The seqnum
			 * and linelen are converted from network byte order to host byte order. These
			 * happen while simultaneously checking for errors, such as the header not
			 * being of length 4, the seqnum not being the first in the sequence, and the
			 * linelen being less that zero. If any of these errors occur, exit the script. */
			printf("Error on client side\n");
			break;
		}
		sentence[linelen] = '\0';	// Declaring buffer size
		if ((bytes_recd = recv(sock_connection, sentence, linelen, 0)) != linelen) {
			/* Receive the filename with the length preset in linelen. If
			 * the amount of bytes received is different than the linelen
			 * sent through the header, than there is an error and the script
			 * should stop. */
			printf("bytes_recd is different from linelen, error\n");
			break;
		}
		printf("Packet %d with %d data bytes received for the filename\n", seqnum, linelen);
		/* send message */
		int totalbytes = 0;	// Total bytes of data sent
		FILE *file;	// Pointer to the file to read in
		char *line = sentence;	// A pointer to the line that will be read in
		size_t buff = STRING_SIZE;	// Buffer size
		file = fopen(sentence, "r");	// The file to read in
		if (file) {
			for (seqnum++; (linelen = getline(&line, &buff, file)) > 0; seqnum++) {
				/* Getting the next line, continuously going until no more lines left, increasing the seqnum each iteration */
				unsigned short header[2] = {htons((unsigned short)seqnum), htons((unsigned short)linelen)};
				/* This header contains the sequence number in the [0] position and
				 * the line length in the [1] position. Both of these consist of unsigned short
				 * integers. The integers are converted to Network Order by the htons() function.
				 * The header is now ready to be sent. */
				if ((bytes_sent = send(sock_connection, header, sizeof(header), 0)) != 4 || (bytes_sent = send(sock_connection, line, linelen, 0)) != linelen) {
					/* Send the header and the line with the number of bytes to send equal to the
					 * linelen in the header. If either function send less than 1 byte, there is an error
					 * and the script should stop. */
					printf("Error sending line, exiting\n");
					close (sock_connection);
					exit(1);
				}
				printf("Packet %d with %d data bytes sent\n", seqnum, linelen);
				totalbytes += linelen;	// Add the number of bytes just sent to the total bytes
			} // No more lines
			unsigned short header[2] = {htons(seqnum), htons(0)};
			/* After all of the lines are read through, a header is sent
			 * with a data-size of the next line as 0 indicating no more lines
			 * left, and the sequence value is incremented */
			if ((bytes_sent = send(sock_connection, header, sizeof(header), 0)) != 4) {
				/* Send the final header, making sure only 4 bytes are sent. */
				printf("Error sending final message. Exiting");
				close (sock_connection);
				exit(1);
			}
			printf("End of Transmission Packet with sequence number %d transmitted with 0 data bytes\n", seqnum);
			printf("Total number of packets sent: %d\n", seqnum-1);
			printf("Total number of data bytes sent: %d\n", totalbytes);
		}
		if (ferror(file)) {	// If there is a file error
			close(sock_connection);
			exit(1);
			/* Close the connection and exit */
		}
		fclose(file);	// Close the file
		break;	// Break out of the loop
	}
	/* close the socket */
	close(sock_connection);
	printf("Socket closed.\n");
}
