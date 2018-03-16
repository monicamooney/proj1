/* tcp_ client.c */ 
/* Programmed by Adarsh Sethi */
/* February 21, 2018 */     

#include "main.h"
#include <stdio.h>          /* for standard I/O functions */
#include <stdlib.h>         /* for exit */
#include <string.h>         /* for memset, memcpy, and strlen */
#include <netdb.h>          /* for struct hostent and gethostbyname */
#include <sys/socket.h>     /* for socket, connect, send, and recv */
#include <netinet/in.h>     /* for sockaddr_in */
#include <unistd.h>         /* for close */

#define STRING_SIZE 80
#define HOSTNAME "cisc450.cis.udel.edu"
#define PORT_NUM 46464

int main(void) {

   int sock_client;  /* Socket used by client */

   struct sockaddr_in server_addr;  /* Internet address structure that
                                        stores server address */
   struct hostent * server_hp;      /* Structure to store server's IP
                                        address */
   //char HOSTNAME[STRING_SIZE]; /* Server's hostname */
   //unsigned short PORT_NUM;  /* Port number used by server (remote port) */

   char filename[STRING_SIZE];  /* send message */
   char line_fromfile[STRING_SIZE]; /* receive message */
   unsigned int msg_len;  /* length of message */                      
   int bytes_sent, bytes_recd; /* number of bytes sent or received */
  
   /* open a socket */

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

   /* initialize server address information */

   printf("Enter hostname of server: ");
   scanf("%s", HOSTNAME);
   if ((server_hp = gethostbyname(HOSTNAME)) == NULL) {
      perror("Client: invalid server hostname");
      close(sock_client);
      exit(1);
   }

   printf("Enter port number for server: ");
   //scanf("%hu", &PORT_NUM);

   /* Clear server address structure and initialize with server address */
   memset(&server_addr, 0, sizeof(server_addr));
   server_addr.sin_family = AF_INET;
   memcpy((char *)&server_addr.sin_addr, server_hp->h_addr,
                                    server_hp->h_length);
   server_addr.sin_port = htons(PORT_NUM);

    /* connect to the server */
 		
   if (connect(sock_client, (struct sockaddr *) &server_addr, 
                                    sizeof (server_addr)) < 0) {
      perror("Client: can't connect to server");
      close(sock_client);
      exit(1);
   }
  
   /* user interface */

   printf("Please input a filename:\n");
   scanf("%s", filename);
   
   
   
   msg_len = strlen(filename) + 1;

   /* send message (the filename) */
   
   //bytes_sent = send(sock_client, structAddr, sizeof(p_packet), 0);

   /* get response from server (the contents of the file) */
  
   bytes_recd = recv(sock_client, line_fromfile, STRING_SIZE, 0);

   printf("\nServer responding...\n");
   printf("%s\n\n", line_fromfile);

   /* close the socket */

   close (sock_client);
}
