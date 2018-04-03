/* tcpclient.c */
/* Programmed by Matt Andreas and Monica Mooney */
/* March 22, 2018 */

#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <sys/types.h>
#include <sys/unistd.h>


#define STRING_SIZE 1024
#define HOSTNAME "cisc450.cis.udel.edu" /* Server's hostname */
#define PORT_NUM 46464 /* Port number used by server (remote port) */

int main(void) {

	int sock_client;  /* Socket used by client */

	struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
	struct hostent * server_hp;      /* Structure to store server's IP
                                        address */

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

	/* USER INTERFACE */

	//Client prompts user for filename
	char filename[STRING_SIZE];
	printf("Please input a filename:\n");
	scanf("%s", filename);

	/* SEND MESSAGE (the filename) */

	int bytes_sent, bytes_recd; /* number of bytes sent or received */
	int seqnum = 0, linelen = strlen(filename) + 1;
	//When connection is established, client sends filename to server
	unsigned short header[2] = {htons((unsigned short)seqnum), htons((unsigned short)linelen)};
	/* This header contains the sequence number in the [0] position and
	 * the line length in the [1] position. Both of these consist of unsigned short
	 * integers. The integers are converted to Network Order by the htons() function.
	 * The header is now ready to be sent. */
	if ((bytes_sent = send(sock_client, header, sizeof(header), 0)) != 4 || (bytes_sent = send(sock_client, filename, linelen, 0)) != linelen) {
		/* Send the header and the filename while simultaneously checking for errors. If the
		 * header is not 4 bytes or the bytes sent for the filename is not equal to the length
		 * of the filename, there is an error. */
		printf("Error sending filename, exiting\n");
		close (sock_client);
		exit(1);
	}
	printf("Packet %d with %d data bytes sent for the filename\n", seqnum, linelen);
	int totalbytes = 0;	// Keep track of total bytes received
	FILE * file;	// Pointer to the file to write to
	char line_fromfile[STRING_SIZE];	// A pointer to the line that will be received
	file = fopen("output.txt", "w");	// File to write to
	/* GET RESPONSE from server (the contents of the file) */
	if (file) {
		do {
			unsigned short header[2];	// The header that the line info will be held in
			int prevseq;	// Keep track of the previous seqnum for error checking
			if ((bytes_recd = recv(sock_client, header, 4, 0)) != 4 || (prevseq = seqnum) != ((seqnum = ntohs(header[0])) - 1) || (linelen = ntohs(header[1])) < 0) {
				/* Receive the header and convert the seqnum and linelen to host order
				 * while simultaneously checking for errors. If the header is not 4 bytes long,
				 * the seqnum is not one more than the previous, or the linelen is less than zero,
				 * then there is an error. */
				printf("Error on server side\n");
				break;
			}
			else if (linelen == 0) {
				/* If the linelen is equal to zero, then that signifies that all the 
				 * lines have been sent. There is no line to be received so the script
				 * should exit.
				 */
				printf("End of Transmission Packet with sequence number %d received with %d data bytes\n", seqnum, linelen);
				printf("Total number of packets received: %d\n", seqnum-1);
				printf("Total number of data bytes received: %d\n", totalbytes);
				break;
			}
			line_fromfile[linelen] = '\0';	// Declaring buffer size
			if ((bytes_recd = recv(sock_client, line_fromfile, linelen, 0)) != linelen) {
				/* Receive the line to write while simultaneously checking for
				 * errors. If the bytes received is not the same as the linelen previously set 
				 * in the header, then there is an error and the script should stop. */
				printf("bytes_recd is different from linelen, error\n");
				break;
			}
			printf("Packet %d with %d data bytes received\n", seqnum, linelen);
			totalbytes += linelen;	// Add bytes received to total
		} while (fputs(line_fromfile, file) > 0);	// Write line to file
		fclose(file);	// Close the file
	}
	else if (ferror(file)) 	// If there is a file error
		printf("Error with file\n");
	printf("\nExiting\n");

	/* close the socket */

	close (sock_client);
}

