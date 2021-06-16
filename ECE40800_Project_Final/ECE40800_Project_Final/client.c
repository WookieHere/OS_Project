/* A simple TCP client */
#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h> //Added string library
#include <stdlib.h> //Added standard library
#include <stdbool.h> //add booleans

#define SERVER_TCP_PORT    3000
#define BUFLEN 256    /* buffer length */

int connect_to_server_for_request(struct hostent * hp, struct sockaddr_in server, char * host, int port) {
    int sd;
        /* Create a stream socket */
    if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        fprintf(stderr, "Can't create a socket\n");
        exit(1);
    }

    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;
    server.sin_port = htons(port);
    if ((hp = gethostbyname(host)) == NULL) {
        fprintf(stderr, "Can't get server's address\n");
        exit(1);
    }

    printf("h_length = %d\n", hp->h_length);

    bcopy(hp->h_addr, (char *)&server.sin_addr, hp->h_length);

    /* Connecting to the server */
    if (connect(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't connect\n");
        exit(1);
    }
    printf("Connected: server's address is %s\n", hp->h_name);
    return sd;
}

void send_command(char * command, int sd) {
    unsigned char bp[BUFLEN];
    int n = 0;
    int bytes_to_read = sizeof(unsigned char) * BUFLEN;
    int cmpNum = strcmp(command, "exit");
    if (cmpNum != 0) {
        write(sd, command, BUFLEN);    /* send it out */
        if (command[0] == 'g' && command[1] == 'e' && command[2] == 't') { // get file
            char * fileName = (strtok(command, "."));
            char * fileExtension = (strtok(NULL, "."));
            printf("RX:\n");
            write_file(sd, &bp, fileExtension);
        } else { // list file
            printf("RX:\n");
            char sender;
            while ((n = read(sd, &bp, sizeof(bp))) > 0) {
                printf("%s\n", bp);
            }
        }
    }
}

void client_script(char * scriptName, int sd, struct hostent * hp, struct sockaddr_in server, char * host, int port) {
    int scriptIssuedExit = 0;
    char command[BUFLEN];
    FILE * fp = fopen(scriptName, "r");
    if (fp != NULL) { // file was opened successfully, attempt to parse commands line by line and send to server
        while ((fgets(&command, BUFLEN, fp) != NULL) && scriptIssuedExit == 0) {
            if (command[0] == '#' || strtok(command, "#") == NULL) { // line is either a comment or potentially empty (skip this line)
                //ain't care do nothing
            } else {
                char * currCommand1 = strtok(command, "\r\n");
                printf("%s",currCommand1);
                char * currCommand = strtok(currCommand1, "#");
                if (currCommand != NULL) {
                    if (currCommand[0] == 'e' && currCommand[1] == 'x' && currCommand[2] == 'i' && currCommand[3] == 't') {
                        scriptIssuedExit = 1;
                    } else { // potential command found, comments filtered out
                        //send to server
                        sd = connect_to_server_for_request(hp, server, host, port);
                        send_command(currCommand, sd);
                    }
                }
            }
        }
    } else { // file was not opened successfully
        printf("\nFile: %s not found or could not be opened", scriptName);
    }
}

void write_file(int sd, void * bp, char * ext) {
    FILE * fp;
    size_t n = 0;
    char  filename[BUFLEN] = "output.";
    //char buffer[BUFLEN] = {0};
    
    strcat(filename, ext);
    int counter;

    fp = fopen(filename, "ab");

    while ((n = read(sd, bp, (sizeof(unsigned char) * BUFLEN))) > 0) {
        fwrite(bp, sizeof(unsigned char), n, fp);
        bzero(bp, BUFLEN);
    }
    fclose(fp);
    return;
}

int main(int argc, char **argv) {
    int n;
    int bytes_to_read;
    int sd;
    int port;
    bool scriptArgv = false;
    struct hostent * hp;
    struct sockaddr_in server;

    char * host;
    unsigned char bp[BUFLEN];
    char rbuf[BUFLEN];
    char sbuf[BUFLEN];
    char * scriptName;


    switch(argc) {
        case 2:
            host = argv[1];
            port = SERVER_TCP_PORT;
            break;
        case 3:
            host = argv[1];
            port = atoi(argv[2]);
        break;
        case 4:
            host = argv[1];
            port = atoi(argv[2]);
            scriptName = argv[3];
            scriptArgv = true;
        break;
        default:
            fprintf(stderr, "Usage: %s host [port]\n", argv[0]);
            exit(1);
    }

    if(scriptArgv) { // run client as script
        client_script(scriptName, sd, hp, server, host, port);
    } else { // run in interactive mode
        sd = connect_to_server_for_request(hp, server, host, port);
        printf("TX:");
        gets(sbuf);        /* get user's text */
        send_command(sbuf, sd);
    }
    close(sd);
    return(0);
}
