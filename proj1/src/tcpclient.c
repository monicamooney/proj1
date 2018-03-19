/* tcp_ client.c */
/* Programmed by Matt Andreas and Monica Mooney */
/* March 22, 2018 */

#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */
#include "datastruct.h"

#define STRING_SIZE 80
#define HOSTNAME "cisc450.cis.udel.edu" /* Server's hostname */
#define PORT_NUM 46464 /* Port number used by server (remote port) */

int main(void) {

	int sock_client;  /* Socket used by client */

	struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
	struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
	char filename[STRING_SIZE];  /* send message */
	char line_fromfile[STRING_SIZE]; /* receive message */
	unsigned int filename_len;  /* length of message */
	int bytes_sent, bytes_recd; /* number of bytes sent or received */

	/* OPEN SOCKET */

	if ((sock_client = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Client: can't open stream socket");
		exit(1);
	}

	/* Note: there is no need to initialize local client address information
            unless you want to specify a specific local port
            (in which case, do it the same way as in udpclient.c).
            The local address initialization and binding is done automatically
            when the connect function is called later, if the socket has not
            already been bound. */

	/* INITIALIZE SERVER:
	 * address information */

	printf("The server hostname is: %s \n", HOSTNAME);
	if ((server_hp = gethostbyname(HOSTNAME)) == NULL) {
		perror("Client: invalid server hostname");
		close(sock_client);
		exit(1);
	}

	printf("The server port number is: %d \n", PORT_NUM);


	/* USER INTERFACE */

	//Client prompts user for filename
	printf("Please input a filename:\n");
	scanf("%s", filename);

	/* INITIALIZE SERVER:
	 * clear server address structure and initialize with server address */

	memset(&server_addr, 0, sizeof(server_addr));
	server_addr.sin_family = AF_INET;
	memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
			server_hp->h_length);
	server_addr.sin_port = htons(PORT_NUM);

	/* CONNECT TO SERVER */

	//Client sends a connection request
	if (connect(sock_client, (struct sockaddr *) &server_addr,
			sizeof (server_addr)) < 0) {
		perror("Client: can't connect to server");
		close(sock_client);
		exit(1);
	}

	/* SEND MESSAGE (the filename) */

	//When connection is established, client sends filename to server
	bytes_sent = send(sock_client, filename, filename_len, 0);

	FILE *file;
	file = fopen("output.txt", "w");
	/* GET RESPONSE from server (the contents of the file) */
	if (file) {
		do {
			int * header;
			bytes_recd = recv(sock_client, header, 2, 0);
			bytes_recd = recv(sock_client, line_fromfile, header[1], 0);
		} while (fprintf(file, line_fromfile) > 0);
	}
	if (ferror(file)) {
		/* deal with error */
	}
	printf("\nServer responding...\n");

	/* close the socket */

	close (sock_client);
}

