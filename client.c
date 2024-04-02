#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER 1024
#define MAX_USERNAME_LEN 32

int main(int argc, char *argv[]){
	
	//handle arguments
	
	if(argc != 2){
		printf("./client [username]\n");
		return(0);
	}else if(strlen(argv[1]) > MAX_USERNAME_LEN){
		printf("username too long.\nMax length: %d \n",MAX_USERNAME_LEN);
		return(0);
	}
	
	char *username = calloc(strlen(argv[1]) + 3, sizeof(char));
	username[0] = 0;
	memcpy(username, argv[1], strlen(argv[1]) * sizeof(char));
	strcat(username, ": ");
	
	//innitialize client
	
    int socketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(socketFD < 0){
        printf("An error occured when trying to prepare a socket\n");
        exit(1);
    }    

    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = PORT;
    inet_pton(AF_INET, IP, &address.sin_addr.s_addr);

    if (-1 == connect(socketFD, (struct sockaddr*)&address, sizeof address)){
        printf("An error occured when trying to connect\n");
        exit(1);
    }
	
	//handle communication with server
    
	int pid = fork();
	char* message = calloc (BUFFER, sizeof(char));
	int messageLength = BUFFER - strlen(username);
	strcat(message, username);

	if(pid == 0){
		
		//handle sending messages (get input, format input, send message)
		
		while(1){
			
			//get input
			memset(&message[strlen(username)], 0, messageLength * sizeof(char));
			fgets(&message[strlen(username)], messageLength, stdin);
			
			int c;
			if(message[BUFFER-2] != 0 && message[BUFFER-2] != '\n'){
				while ((c = getchar()) != '\n' && c != EOF);
				printf("message too long to be sent\n");
				continue;
			}

			if(message[strlen(username)] == '\n'){
				continue;
			}
			
			//send message
			if(send(socketFD, message, BUFFER * sizeof(char), 0) == -1){
				printf("Something went wrong when tryining to send a message\n");
				close(socketFD);
				exit(0);
			}
		}
	}else{
		
		//handle receiving messages
		
		while(1){
			memset(message, 0, BUFFER * sizeof(char));
			if(recv(socketFD, message, BUFFER, 0) <= 0){
				printf("Something went wrong when tryining to receive a message or the connection was closed\n");
				close(socketFD);
				exit(0);
			}

			printf("%s", message);
		}
	}
	
    return 0;
}
