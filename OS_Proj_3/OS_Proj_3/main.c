/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>    //Added string library
#include <stdlib.h>    //Added standard library
#include <dirent.h> //Allows directory travel

#define SERVER_TCP_PORT    3000    /* well-known port */
#define BUFLEN 256    /* buffer length */

typedef struct queue_system{
    struct node* first_node;
    int numElements;
    int maxSize;
} Array;

typedef struct node
{
    struct data* param;
    size_t packetSize;
    struct node* next;
}node;

typedef struct data
{
    int ID;
    //whatever else you want here
}data;

struct node* new_node(int data, size_t packetSize)
{
    node* new = (struct node*)malloc(sizeof(struct node));
    new->packetSize = packetSize;
    new->param->ID = data;
    //initialize whatever you like here
    return new;
}

void newQueue(Array *arr){
    arr->maxSize = 5;
    arr->numElements = 0;
    arr->first_node = NULL;
    /*if(!arr->data){
        printf("Could not Create Queue\n");
        exit(1);
    }else{
        printf("Queue Initialized Successfully\n");
    }*/
    //use a try/catch
}

void pop(size_t index, Array *arr){

    node* temp = arr->first_node;
    arr->first_node = temp->next;
    //now process temp

    /*
    for(int i = 0; i < arr->arraySize; i++){
        if(arr->data[i] == 0){
            for(int j = i; j < arr->arraySize; j++){
                arr->data[j] = arr->data[j+1];
            }
            arr->currElements -= 1;
            arr->data[4] = 0;
            break;
        }
    }
     */
    free(temp);
    arr->numElements--;
}

void push(Array *arr, int clientNum){
    
    if(arr->numElements >= arr->maxSize)
    {
        perror("Error Pushing onto Array");
        exit(1);
    }
    node* temp = arr->first_node;
    for(int i = 0; i < arr->numElements; i++)
    {
        temp = temp->next;
    }
    temp->next = new_node(clientNum, sizeof(int));
    arr->numElements++;
    
}

int getClientID(Array *arr, int index){
    if(index >= arr->numElements){
        //Throw Exception
        perror("Index out of range exception\n");
        exit(1);
    }else
    {
        node* temp = arr->first_node;
        for(int i = 0; i < index; i++)
        {
            temp = temp->next;
        }
        return temp->param->ID;
    }
}


void send_file(FILE * fp, int sd){
    int n;
    unsigned char data[BUFLEN];


    while((n = fread(&data, sizeof(unsigned char), BUFLEN, fp)) > 0) {
        if (send(sd, &data, (sizeof(unsigned char) * n),  0) == -1) {
            perror("Error in sending the file to the client.");
            exit(1);
        }
        
        bzero(&data, sizeof(data));
    }
    fclose(fp);
    close(sd);
}

void randomSchedule(int argc, char **argv){
    int n;
    int bytes_to_read;
    int sd;
    int new_sd;
    int client_len;
    int port = SERVER_TCP_PORT;
    int threads = 1;
    int buffers = 1;

    struct sockaddr_in server;
    struct sockaddr_in client;
    struct dirent *direct; //Pointer for the actual Directory

    unsigned char * bp;
    char buf[BUFLEN];
    char * directory = "media";
    char * sched = "LOTTERY";
    

/*
<!--Beginning Test of Queueing System--!>
*/
    /*Array arr ;
    newQueue(&arr);
    printf("Passed Array init\n");

    
    printf("Queue Size is %d\n", arr.arraySize);
    printf("\n");
    push(&arr, 1);
    printf("Getting Data in Index 0: %d\n", getClientID(&arr, 0));
    push(&arr, 2);
    printf("Getting Data in Index 1: %d\n", getClientID(&arr, 1));
    push(&arr, 3);
    printf("Getting Data in Index 2: %d\n", getClientID(&arr, 2));
    push(&arr, 4);
    printf("Getting Data in Index 3: %d\n", getClientID(&arr, 3));
    push(&arr, 5);
    printf("Getting Data in Index 4: %d\n", getClientID(&arr, 4));

    pop(2, &arr);
    for(int i = 0; i < arr.arraySize; i++){
        printf("Array Data: %d\n", getClientID(&arr, i));
    }
    printf("\n");
    pop(0, &arr);

    for(int j = 0; j < arr.currElements; j++){
        printf("Array Data: %d\n", getClientID(&arr, j));
    }

    printf("\n");
    push(&arr, 6);
    for(int k = 0; k < arr.currElements; k++){
        printf("Array Data: %d\n", getClientID(&arr, k));
    }
    
    printf("\n");
    push(&arr, 7);
    for(int t = 0; t < arr.currElements; t++){
        printf("Array Data: %d\n", getClientID(&arr, t));
    }*/
    
    
/*
<!--Ending Test of Queueing System--!>
*/

    switch(argc) {
        case 1:
            break;
        case 2: //cmd line args port num specified
            port = atoi(argv[2]);
            break;
        case 3: //cmd line args # threads specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            break;
        case 4: //cmd line args # buffers specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            break;
        case 5: //cmd line args sched specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            sched = argv[5];
            break;
        case 6: //cmd line args directory specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            sched = argv[5];
            directory = argv[6];
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

    /* Bind an address to the socket. Typically Sockets have no names, but binding like this gives it a "name" to connect to. */
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;             //Sets the Family/Type of Address to work with
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    /* queue up to 5 connect requests */
    listen(sd, 5);

    printf("[+] Listening...\n");
    while (1) {
        client_len = sizeof(client);
        if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) { //Accepts new Clients.
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        printf("[+] Connection Established\n");
        printf("[+] Connection on Port: %d\n", port);

        bp = buf;
        bytes_to_read = BUFLEN;
        while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
            bp += n;
            bytes_to_read -= n;
        }
        printf("RCVD: %s\n", buf);

        if (strcmp(buf, "list") == 0) { // client wants a list of file names
            DIR * directPointer;
            char * dirName = "media";
            char fileList[BUFLEN] = "";
            directPointer = opendir(dirName); //Returns a Pointer of DIR type
            if(directPointer == NULL) {  //If the Directory couldn't be opened, Error Msg will be Printed
                printf("Could not Open Directory");
            } else {
                direct = readdir(directPointer);
                while(direct != NULL){//If Directory can be opened, list files in directory
                    if (direct != NULL) {
                        char * currFileName = direct->d_name;
                        if (strcmp(currFileName, ".") != 0 && strcmp(currFileName, "..") != 0) {
                            strcat(fileList, currFileName);
                            strcat(fileList, "\n");
                        }
                        direct = readdir(directPointer);
                    }
                }
                closedir(directPointer); //Close DIR Pointer
            }
            write(new_sd, fileList, BUFLEN);
        }else if (buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') { // client wants to download a file
            //attempt to open file, send file to client in chunks of bytes
            
            //tokenize the client input twice by " " to try to obtain the file name requested by the client
            char * filename = (strtok(buf," "));
            filename = (strtok(NULL," "));
            
            printf("%s\n", filename);

            char fileGrabber[BUFLEN] = "media/";
            strcat(fileGrabber, filename);

            printf(fileGrabber);
            

            if (filename != NULL) {

                FILE * fp = fopen(fileGrabber, "rb"); // file pointer to attempt to open the file
                printf("\n<-File Found & Opened->\n");

                if (fp != NULL) {
                    // file was opened successfully, attempt to send to the client
                    send_file(fp, new_sd);
                    
                } //else {
                    // file was not opened successfully, send file not found (404) header to client

                //}
            

            } else {
                //filename not found in client's input

            }
            
            
            


        } else if (strcmp(buf, "exit") == 0) {
            //client wants to exit
            
        } else {
            //invalid request
            
        }
        //write(new_sd, *buf, BUFLEN);
        printf("\nSEND: %s\n", buf);
        close(new_sd);
        
    }
    close(sd);
}

void fifoSchedule(int argc, char **argv){
    int n;
    int bytes_to_read;
    int sd;
    int new_sd;
    int client_len;
    int port = SERVER_TCP_PORT;
    int threads = 1;
    int buffers = 1;

    struct sockaddr_in server;
    struct sockaddr_in client;
    struct dirent *direct; //Pointer for the actual Directory

    unsigned char * bp;
    char buf[BUFLEN];
    char * directory = "media";
    char * sched = "FIFO";

    switch(argc) {
        case 1:
            break;
        case 2: //cmd line args port num specified
            port = atoi(argv[2]);
            break;
        case 3: //cmd line args # threads specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            break;
        case 4: //cmd line args # buffers specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            break;
        case 5: //cmd line args sched specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            sched = argv[5];
            break;
        case 6: //cmd line args directory specified
            port = atoi(argv[2]);
            threads = atoi(argv[3]);
            buffers = atoi(argv[4]);
            sched = argv[5];
            directory = argv[6];
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

    /* Bind an address to the socket. Typically Sockets have no names, but binding like this gives it a "name" to connect to. */
    bzero((char *)&server, sizeof(struct sockaddr_in));
    server.sin_family = AF_INET;             //Sets the Family/Type of Address to work with
    server.sin_port = htons(port);
    server.sin_addr.s_addr = htonl(INADDR_ANY);
    if (bind(sd, (struct sockaddr *)&server, sizeof(server)) == -1) {
        fprintf(stderr, "Can't bind name to socket\n");
        exit(1);
    }

    /* queue up to 5 connect requests */
    listen(sd, 5);

    printf("[+] Listening...\n");
    while (1) {
        client_len = sizeof(client);
        if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) { //Accepts new Clients.
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        printf("[+] Connection Established\n");

        bp = buf;
        bytes_to_read = BUFLEN;
        while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
            bp += n;
            bytes_to_read -= n;
        }
        printf("RCVD: %s\n", buf);

        if (strcmp(buf, "list") == 0) { // client wants a list of file names
            DIR * directPointer;
            char * dirName = "media";
            char fileList[BUFLEN] = "";
            directPointer = opendir(dirName); //Returns a Pointer of DIR type
            if(directPointer == NULL) {  //If the Directory couldn't be opened, Error Msg will be Printed
                printf("Could not Open Directory");
            } else {
                direct = readdir(directPointer);
                while(direct != NULL){//If Directory can be opened, list files in directory
                    if (direct != NULL) {
                        char * currFileName = direct->d_name;
                        if (strcmp(currFileName, ".") != 0 && strcmp(currFileName, "..") != 0) {
                            strcat(fileList, currFileName);
                            strcat(fileList, "\n");
                        }
                        direct = readdir(directPointer);
                    }
                }
                closedir(directPointer); //Close DIR Pointer
            }
            write(new_sd, fileList, BUFLEN);
        }else if (buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') { // client wants to download a file
            //attempt to open file, send file to client in chunks of bytes
            
            //tokenize the client input twice by " " to try to obtain the file name requested by the client
            char * filename = (strtok(buf," "));
            filename = (strtok(NULL," "));
            
            printf("%s\n", filename);

            char fileGrabber[BUFLEN] = "media/";
            strcat(fileGrabber, filename);

            printf(fileGrabber);
            

            if (filename != NULL) {

                FILE * fp = fopen(fileGrabber, "rb"); // file pointer to attempt to open the file
                printf("\n<-File Found & Opened->\n");

                if (fp != NULL) {
                    // file was opened successfully, attempt to send to the client
                    send_file(fp, new_sd);
                    
                } //else {
                    // file was not opened successfully, send file not found (404) header to client

                //}
            

            } else {
                //filename not found in client's input

            }
            
            
            


        } else if (strcmp(buf, "exit") == 0) {
            //client wants to exit
            
        } else {
            //invalid request
            
        }
        //write(new_sd, *buf, BUFLEN);
        printf("\nSEND: %s\n", buf);
        close(new_sd);
        
    }
    close(sd);
    return(0);
}

int main(int argc, char **argv) {
    //fifoSchedule(argc, argv);
    randomSchedule(argc, argv);
    return 0;
}
