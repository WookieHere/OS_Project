Skip to content
 Enterprise
Search or jump to…

Pull requests
Issues
Explore
 
@lenasby
Github.iu.edu may be intermittently unavailable Sunday 5/2 beginning at 9AM till noon for upgrade to version 3.0.6.

Learn Git and GitHub without any code!
Using the Hello World guide, you’ll start a branch, write comments, and open a pull request.


lcginn
/
CSCI-403-project4
Private
1
00
Code
Issues
Pull requests
Projects
Wiki
Insights
CSCI-403-project4/server.c

Logan Ginn SJF comments
Latest commit 9c2a6dd 21 minutes ago
 History
 1 contributor
Executable File  640 lines (567 sloc)  22.1 KB
  
/* A simple echo server using TCP */
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>    //Added string library
#include <stdlib.h>    //Added standard library
#include <dirent.h> //Allows directory travel
#include <time.h>
#include <sys/time.h>
#include <pthread.h>
#include "clientInst.h"

#define SERVER_TCP_PORT    3000    /* well-known port */
#define BUFLEN 256    /* buffer length */
#define THREAD_MAX 1
#define MAX_CONNECTIONS 100


typedef struct queue_system{
    int * data;
    int * portNums;
    struct sockaddr_in * clientSockAddr;
    int arraySize;
    int currElements;
} Array;


void newQueue(Array *arr){
    arr->arraySize = 5;
    arr->currElements = 0;
    arr->data = malloc((sizeof(int) * arr->arraySize));
    arr->portNums = malloc((sizeof(int) * arr->arraySize));
    arr->clientSockAddr = malloc((sizeof(struct sockaddr_in) * arr->arraySize));
    if(!arr->data){
        printf("Could not Create Queue\n");
        exit(1);
    }else{
        printf("Queue Initialized Successfully\n");
    }
    for(int i = 0; i < arr->arraySize; i++)
    {
        arr->data[i] = 0;
        arr->portNums[i] = 0;
    }
}

void pop(int index, Array *arr){
    struct sockaddr_in client;
    struct sockaddr_in client2;
    arr->data[index] = 0;
    arr->portNums[index] = 0;
    arr->clientSockAddr[index] = client;
    for(int i = index; i < arr->arraySize - 1; i++)
    {
        arr->data[i] = arr->data[i+1];
        arr->portNums[i] = arr->portNums[i+1];
        arr->clientSockAddr[i] = arr->clientSockAddr[i+1];
    }
    arr->data[arr->arraySize - 1] = 0; //this sets last element to 0 in case it was full
    arr->portNums[arr->arraySize - 1] = 0;
    arr->clientSockAddr[arr->arraySize - 1] = client2;
    arr->currElements--;
}

void push(Array *arr, int clientNum, int portNum, struct sockaddr_in client){

    if(arr->currElements >= arr->arraySize){
        perror("Error Pushing onto Array");
        exit(1);
    } else {
        for(int i = 0; i < arr->arraySize; i++){
            if(arr->data[i] == 0)
            {
                arr->data[i] = clientNum;
                arr->portNums[i] = portNum;
                arr->clientSockAddr[i] = client;
                break;
            }
        }
        arr->currElements++;
    }
}

int randomGrabber(int currElements){
    int random = (rand() % (currElements));
    return random;
}

int getClientID(Array *arr, int index) {
    if(index >= arr->arraySize) {
        //Throw Exception
        return -1;
    } else {
        return arr->data[index];
    }
}

int getClientPortNum(Array *arr, int index) {
    if(index >= arr->arraySize) {
        //Throw Exception
        return -1;
    } else {
        return arr->portNums[index];
    }
}

struct sockaddr_in getClientSockAddr(Array *arr, int index) {
    if(index >= arr->arraySize) {
        //Throw Exception
    } else {
        return arr->clientSockAddr[index];
    }
}


//global variables for multithreaded stepdata
int mutex[THREAD_MAX];
int sem_flag = 0;
Array queue;



int initServer(int port) {
    struct sockaddr_in server;
    int sd = -1;
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
    return sd;
}

void open_fileForSend(int new_sd, char buf[BUFLEN]) {
    //attempt to open file, send file to client in chunks of bytes
    //tokenize the client input twice by " " to try to obtain the file name requested by the client
    char * filename = (strtok(buf," "));
    filename = (strtok(NULL," "));
    printf("%s\n", filename);
    char fileGrabber[BUFLEN] = "media/";
    strcat(fileGrabber, filename);
    printf("%s", fileGrabber);
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

void send_fileList(int new_sd) {
    DIR * directPointer;
    struct dirent *direct; //Pointer for the actual Directory
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
}

int temp_array[MAX_CONNECTIONS]; //100 is the max amount of request the server can have

void * serve_request(void * clientPtr) {
    //for thread coordination;
    int num = sem_flag;
    mutex[num] = 1;
    struct clientStruct * clientInstPtr = (struct clientStruct *)clientPtr;
    struct sockaddr_in client = (*clientInstPtr).client;
    int new_sd = (*clientInstPtr).clientId;
    int portNum = (*clientInstPtr).portNum;
    printf("\n client id: %d, portNum: %d, client port: %d\n\n", new_sd, portNum, client.sin_port);

    /*
    char buf[BUFLEN];
    unsigned char * bp = buf;
    int bytes_to_read = BUFLEN;
    int n = 0;
     */
    struct timeval tv;
    struct timezone tz;
    struct timespec ts;
    struct tm * today;
    /*
     * Response Start Time, Client ID, Port Num, and IP Printing to Server
     */
    timespec_get(&ts, TIME_UTC);
    gettimeofday(&tv, &tz);
    char str[25] = "";
    inet_ntop(AF_INET, &(client.sin_addr), str, sizeof(str));
    today = localtime(&tv.tv_sec);
    printf("\nResponse Start Time: %d:%0d:%0d.%ld\nClient ID: %d\nPort Number: %d\nIP Address: %s\n" , today->tm_hour, today->tm_min, today->tm_sec, ts.tv_nsec, new_sd, client.sin_port, str);
    /*
     * end Response Start Time, Client ID, Port Num, and IP Printing to Server
     */
    
    while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
        bp += n;
        bytes_to_read -= n;
    }//read a request
    
    
    
    printf("RCVD: %s\n", buf);

    if (strcmp(buf, "list") == 0) { // client wants a list of file names
        send_fileList(new_sd);
    }else if (buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') { // client wants to download a file
        open_fileForSend(new_sd, buf);
    } else if (strcmp(buf, "exit") == 0) {
        //client wants to exit
        
    } else {
        //invalid request
        
    }
    //write(new_sd, *buf, BUFLEN);
    printf("\nSEND: %s\n", buf);
    close(new_sd);
    /*
     * Response Completed Time, Client ID, Port Num, and IP Printing to Server
     */
    timespec_get(&ts, TIME_UTC);
    gettimeofday(&tv, &tz);
    str[25] = "";
    inet_ntop(AF_INET, &(client.sin_addr), str, sizeof(str));
    today = localtime(&tv.tv_sec);
    printf("\nResponse Completed Time: %d:%0d:%0d.%ld\nClient ID: %d\nPort Number: %d\nIP Address: %s\n" , today->tm_hour, today->tm_min, today->tm_sec, ts.tv_nsec, new_sd, portNum, str);
    /*
     * end Response Completed Time, Client ID, Port Num, and IP Printing to Server
     */
    mutex[num] = 0;
}

void SJFSchedule(int sd, int port, int threads, int buffers, char * sched, char * directory) {
    int n;
    int bytes_to_read;
    int new_sd;
    int client_len;
    struct sockaddr_in client;
    unsigned char * bp;
    char buf[BUFLEN];
    
    newQueue(&queue);
    
    
    //initialize mutex
    for(int i = 0; i < THREAD_MAX; i++)
    {
        mutex[i] = 0;
    }
    //mutex starts at all 0's

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
        /*
        Beginning of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */
        struct timeval tv;
        struct timezone tz;
        struct timespec ts;
        struct tm * today;
        timespec_get(&ts, TIME_UTC);
        gettimeofday(&tv, &tz);
        char str[25];
        inet_ntop(AF_INET, &(client.sin_addr), str, sizeof(str));
        today = localtime(&tv.tv_sec);
        printf("\nArrival Time: %d:%0d:%0d.%ld\nClient ID: %d\nPort Number: %d\nIP Address: %s\n" , today->tm_hour, today->tm_min, today->tm_sec, ts.tv_nsec, new_sd, client.sin_port, str);
        /*
        End of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */

        /*
        * Start of queueing new client
        */
        push(&queue, new_sd, client.sin_port, client);
        /*
        * end of queueing new client
        */

        pthread_t tid[THREAD_MAX];
        char buf[BUFLEN];
        unsigned char * bp = buf;
        int bytes_to_read = BUFLEN;
        int n = 0;
        
        for(int j = 0; j < MAX_CONNECTIONS; j++)
        {
            temp_array[j] = -1;
        }//initialze all no. of bytes to -1
        
        int connections = 0;
        
        
        while ((n = read(new_sd, bp, bytes_to_read)) > 0) {
            temp_array[i] = n;
           i++;
        }//read a request
        
        if (queue.currElements > 0) { //requests are waiting in the queue, serve the first one
            int availableThreadFound = 0;
            //pthread goes here
            for(int i = 0; i < THREAD_MAX; i++) {
                if(mutex[i] == 0 && availableThreadFound == 0) {
                    // find shortest job in queue
                    



                    // -=-=-=-=-=-=-=-=-=-=-=-=- Start of SJF Algorithm -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
                    //
                    /*
                     * TODO: Determine shortest job in the queue and set the index to the shortest job's location in the array
                     *
                     */
                    // -=-=-=-=-=-=-=-=-=-=-=-=- End of SJF Algorithm -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
                    int index_of_min = 0;
                    int running_min = 1000;
                    for(int i = 0; i < queue.currElements; i++)
                    {
                        if (strcmp(buf, "list") == 0) { // client wants a list of file names
                            //send_fileList(new_sd);
                            //serve_request(queue.data[i]);
                            if(running_min > 2)
                            {
                                running_min = 2;
                                index_of_min = i;
                            }
                        }else if (buf[0] == 'g' && buf[1] == 'e' && buf[2] == 't') { // client wants to download a file
                            //open_fileForSend(new_sd, buf);
                            //serve_request(queue.data[i]);
                            if(running_min > 1)
                            {
                                running_min = 1;
                                index_of_min = i;
                            }
                        } else if (strcmp(buf, "exit") == 0) {
                            //client wants to exit
                            //serve_request(queue.data[i]);
                            if(running_min > 0)
                            {
                                running_min = 0;
                                index_of_min = i;
                            }
                        } else {
                            //invalid request
                            
                        }
                    }
                    int shortestJobInd = index_of_min;
                    
                    for(int i = 0; i < THREAD_MAX; i++) {
                        if(mutex[i] == 0 && availableThreadFound == 0) {
                            // Pop first client request
                            int clientId = getClientID(&queue, index_of_min);
                            int portNum = getClientPortNum(&queue, index_of_min);
                            struct sockaddr_in clientSockAddr = getClientSockAddr(&queue, index_of_min);
                            pop(0,&queue);
                            // build clientStruct here
                            struct clientStruct clientInst;
                            clientInst.clientId = clientId;
                            clientInst.portNum = portNum;
                            clientInst.client = clientSockAddr;
                            sem_flag = i;
                            int exitcode = pthread_create(&(tid[i]), NULL, serve_request, (void*)(&clientInst));
                            availableThreadFound = 1;
                        }
                    }



                    // build clientStruct here
                    int clientId = getClientID(&queue, shortestJobInd);
                    int portNum = getClientPortNum(&queue, shortestJobInd);
                    struct sockaddr_in clientSockAddr = getClientSockAddr(&queue, shortestJobInd);
                    pop(shortestJobInd,&queue);
                    struct clientStruct clientInst;
                    clientInst.clientId = clientId;
                    clientInst.portNum = portNum;
                    clientInst.client = clientSockAddr;
                    sem_flag = i;
                    int exitcode = pthread_create(&(tid[i]), NULL, serve_request, (void*)(&clientInst));
                    availableThreadFound = 1;
                    
                    
                }
            }
        }
        
    }
    close(sd);
}

void randomSchedule(int sd, int port, int threads, int buffers, char * sched, char * directory) {
    int n;
    int bytes_to_read;
    int new_sd;
    int client_len;
    struct sockaddr_in client;
    unsigned char * bp;
    char buf[BUFLEN];
    newQueue(&queue);

    //initialize mutex
    for(int i = 0; i < THREAD_MAX; i++)
    {
        mutex[i] = 0;
    }
    //mutex starts at all 0's

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


        /*
        Beginning of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */
        struct timeval tv;
        struct timezone tz;
        struct timespec ts;
        struct tm * today;
        timespec_get(&ts, TIME_UTC);
        gettimeofday(&tv, &tz);
        char str[25];
        inet_ntop(AF_INET, &(client.sin_addr), str, sizeof(str));
        today = localtime(&tv.tv_sec);
        printf("\nArrival Time: %d:%0d:%0d.%ld\nClient ID: %d\nPort Number: %d\nIP Address: %s\n" , today->tm_hour, today->tm_min, today->tm_sec, ts.tv_nsec, new_sd, client.sin_port, str);
        /*
        End of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */
        
        /*
         * Start of queueing new client
         */
        push(&queue, new_sd, client.sin_port, client);

        /*
         * end of queueing new client
         */

        pthread_t tid[THREAD_MAX];
        if (queue.currElements > 0) { //requests are waiting in the queue, serve the first one
            int availableThreadFound = 0;
            //pthread goes here
            for(int i = 0; i < THREAD_MAX; i++) {
                if(mutex[i] == 0 && availableThreadFound == 0) {
                    // Pick random client request to process here
                    int randInd = randomGrabber((queue.currElements));
                    // generate random index
                    int clientId = getClientID(&queue, randInd);
                    int portNum = getClientPortNum(&queue, randInd);
                    struct sockaddr_in clientSockAddr = getClientSockAddr(&queue, randInd);
                    pop(randInd, &queue);
                    // build clientStruct here
                    struct clientStruct clientInst;
                    clientInst.clientId = clientId;
                    clientInst.portNum = portNum;
                    clientInst.client = clientSockAddr;
                    sem_flag = i;
                    int exitcode = pthread_create(&(tid[i]), NULL, serve_request, (void*)(&clientInst));
                    availableThreadFound = 1;
                }
            }
            
        }
    }
    close(sd);
    free(&queue);
}

void fifoSchedule(int sd, int port, int threads, int buffers, char * sched, char * directory) {
    int n;
    int bytes_to_read;
    int new_sd;
    int client_len;
    struct sockaddr_in client;
    unsigned char * bp;
    char buf[BUFLEN];
    newQueue(&queue);
    
    
    //initialize mutex
    for(int i = 0; i < THREAD_MAX; i++)
    {
        mutex[i] = 0;
    }
    //mutex starts at all 0's

    /* queue up to 5 connect requests */
    listen(sd, 5);

    printf("[+] Listening...\n");
    while (1) {
        client_len = sizeof(client);   //added the /sizeof(sockaddr_in)
        
        if ((new_sd = accept(sd, (struct sockaddr *)&client, &client_len)) == -1) { //Accepts new Clients.
            fprintf(stderr, "Can't accept client\n");
            exit(1);
        }

        printf("[+] Connection Established\n");
        /*
        Beginning of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */
        struct timeval tv;
        struct timezone tz;
        struct timespec ts;
        struct tm * today;
        timespec_get(&ts, TIME_UTC);
        gettimeofday(&tv, &tz);
        char str[25];
        inet_ntop(AF_INET, &(client.sin_addr), str, sizeof(str));
        today = localtime(&tv.tv_sec);
        printf("\nArrival Time: %d:%0d:%0d.%ld\nClient ID: %d\nPort Number: %d\nIP Address: %s\n" , today->tm_hour, today->tm_min, today->tm_sec, ts.tv_nsec, new_sd, client.sin_port, str);
        /*
        End of Arrival Time, Client ID, Port Num, and IP Printing to Server
        */

        /*
        * Start of queueing new client
        */
        push(&queue, new_sd, client.sin_port, client);
        /*
        * end of queueing new client
        */

        pthread_t tid[THREAD_MAX];
        //pthread_create(tid[num], NULL, <whatever func>
        
        if (queue.currElements > 0) { //requests are waiting in the queue, serve the first one
            int availableThreadFound = 0;
            //pthread goes here
            for(int i = 0; i < THREAD_MAX; i++) {
                if(mutex[i] == 0 && availableThreadFound == 0) {
                    // Pop first client request
                    int clientId = getClientID(&queue, 0);
                    int portNum = getClientPortNum(&queue, 0);
                    struct sockaddr_in clientSockAddr = getClientSockAddr(&queue, 0);
                    pop(0,&queue);
                    // build clientStruct here
                    struct clientStruct clientInst;
                    clientInst.clientId = clientId;
                    clientInst.portNum = portNum;
                    clientInst.client = clientSockAddr;
                    sem_flag = i;
                    int exitcode = pthread_create(&(tid[i]), NULL, serve_request, (void*)(&clientInst));
                    availableThreadFound = 1;
                }
            }
        }
        
    }
    close(sd);
}

int main(int argc, char **argv) {
    int port = SERVER_TCP_PORT;
    int buflen = 256;
    int threads = 1; //should be PORT_MAX
    int buffers = 1;
    char * sched = "FIFO";
    char * directory = "media";
    // -=-=-=-=-=-=-=-=-=-=-=-=- Start of reading server config -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
    // read mserver.config file or other config file if specified in cmd line args
    char * scriptName = "mserver.config";
    switch(argc) {
        case 1:
            break;
        case 2:
            printf("invalid number of command line arguments, running default configuration");
            break;
        case 3: //cmd line args config script specified
            scriptName = argv[2];
            break;
        default:
            break;
    }
    char command[BUFLEN];
    FILE * fp = fopen(scriptName, "r");
    if (fp != NULL) { // file was opened successfully, attempt to parse commands line by line and send to server
        while ((fgets(&command, BUFLEN, fp) != NULL)) {
            if (command[0] == '#' || strtok(command, "#") == NULL) { // line is either a comment or potentially empty (skip this line)
                //ain't care do nothing
            } else {
                char * currCommand1 = strtok(command, "\r\n");
                char * currCommand = strtok(currCommand1, "#");
                if (currCommand != NULL) {
                    char * property = strtok(currCommand,":");
                    char * value = strtok(NULL,": ");
                    if(strcmp(property,"PortNum") == 0) {
                        if(atoi(value) > 0 && atoi(value) < 65536) {
                            port = atoi(value);
                        }
                    } else if (strcmp(property,"Block") == 0) {
                        if(atoi(value) > 0 && atoi(value) < 65536) {
                            buflen = atoi(value);
                        }
                    } else if (strcmp(property,"Threads") == 0) {
                        if(atoi(value) > 0 && atoi(value) < 65536) {
                            threads = atoi(value);
                        }
                    } else if (strcmp(property,"Buffers") == 0) {
                        if(atoi(value) > 0 && atoi(value) < 65536) {
                            buffers = atoi(value);
                        }
                    } else if (strcmp(property,"Sched") == 0) {
                        if(value != NULL) {
                            if(strcmp(value, "FIFO") == 0) {
                                sched = "FIFO";
                            } else if(strcmp(value,"RANDOM") == 0) {
                                sched = "RANDOM";
                            } else if(strcmp(value, "SJF") == 0) {
                                sched = "SJF";
                            }
                        }
                    } else if (strcmp(property,"Directory") == 0) {
                        if (value != NULL) {
                            //check if directory exists before using
                            directory = value;
                        }
                    }
                }
            }
        }
    } else { // file was not opened successfully
        printf("\nFile: %s not found or could not be opened. Exiting . . .", scriptName);
    }
    printf("Server Config\n\nPort: %d\nBlock Size: %i\nThreads: %d\nBuffers: %d\nScheduling algorithm: %s\nMedia directory: %s\n\n", port, buflen, threads, buffers, sched, directory);
    // -=-=-=-=-=-=-=-=-=-=-=-=- end of reading server config -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
    // -=-=-=-=-=-=-=-=-=-=-=-=- start of server init -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
    // call server init function here
    //pthread_create(... initServer)
    int sd = initServer(port);

    // -=-=-=-=-=-=-=-=-=-=-=-=- start of server init -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //
    if (sd != -1) {
        if (strcmp(sched,"FIFO") == 0) {
            fifoSchedule(sd, port, threads, buffers, sched, directory); //we should not be passing argc and argv to these, init the socket in a central location and pass in reference to the socket id
        } else if (strcmp(sched, "RANDOM") == 0) {
            randomSchedule(sd, port, threads, buffers, sched, directory); //we should not be passing argc and argv to these, init the socket in a central location and pass in reference to the socket id
        } else if (strcmp(sched, "SJF") == 0) {
            SJFSchedule(sd, port, threads, buffers, sched, directory); //we should not be passing argc and argv to these, init the socket in a central location and pass in reference to the socket id
        }
    } else {
        printf("\n\nSocket init unsuccessful. Exiting . . .\n\n");
    }
    
    return 0;
}
© 2021 GitHub, Inc.
Help
Support
API
Training
Blog
About
GitHub Enterprise Server 2.22.5
