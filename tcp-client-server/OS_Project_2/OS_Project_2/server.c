/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>	//Added string library
#include <stdlib.h>	//Added standard library

#define SERVER_TCP_PORT		3000	/* well-known port */
#define BUFLEN			256	/* buffer length */


int main(int argc, char **argv)
{
	int 	n, bytes_to_read;
	int 	sd, new_sd, client_len, port;
	struct 	sockaddr_in 	server, client;
	char 	*bp, buf[BUFLEN];

	switch(argc) {
	case 1:
		port = SERVER_TCP_PORT;
		break;
	case 2:
		port = atoi(argv[1]);
		break;
	default:
		fprintf(stderr, "Usage: %s [port]\n", argv[0]);
		exit(1);
	}
		
	/* Create a stream socket */
	if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		fprintf(stderr, "Can't create a socket\n");
		exit(1);
	}

	/* Bind an address to the socket */
	bzero((char *)&server, sizeof(struct sockaddr_in));
	server.sin_family = AF_INET;
	server.sin_port = htons(port);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if (bind(sd, (struct sockaddr *)&server, 
	sizeof(server)) == -1) {
		fprintf(stderr, "Can't bind name to socket\n");
		exit(1);
	}

	/* queue up to 5 connect requests */
	listen(sd, 5);

	while (1) {
		client_len = sizeof(client);
		if ((new_sd = accept(sd, (struct sockaddr *)&client,
		&client_len)) == -1) {
			fprintf(stderr, "Can't accept client\n");
			exit(1);
		}

		bp = buf;
		bytes_to_read = BUFLEN;
		while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
			bp += n;
			bytes_to_read -= n;
		}
		printf("RCVD: %s\n", buf);

		write(new_sd, buf, BUFLEN);
		printf("SEND: %s\n", buf);		
		close(new_sd);
	}
	close(sd);
	return(0);
}
