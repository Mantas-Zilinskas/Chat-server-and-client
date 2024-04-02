#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <sys/select.h>
#include <errno.h>

#define PORT 8080
#define IP "127.0.0.1"
#define BUFFER 1024
#define BACKLOG 10

int addFD(int fd ,int *arr, int *arrLen, fd_set *fdSet);
int removeFD(int fd ,int *arr, int *arrLen, fd_set *fdSet);
int broadcast(int fd, int serverFD, int *fdArr, int arrLen, char *message, int messageLen);
void removeElement(int *arr, int arrLen, int elem);

int main(){

    //Innitialize server
	
    int serverSocketFD = socket(AF_INET, SOCK_STREAM, 0);
    if(serverSocketFD < 0){
        printf("An error occured when trying to prepare a socket\n");
        exit(1);
    }    

    
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_port = PORT;
    inet_pton(AF_INET, IP, &address.sin_addr.s_addr);

    if(bind(serverSocketFD, (struct sockaddr*)&address, sizeof(address)) == -1){
        printf("Something went wrong while trying to bind a socket\n");
        close(serverSocketFD);
        exit(1);
    }
    
    if(-1 == listen(serverSocketFD, BACKLOG)){
            printf("Something went wrong while listening on socket\n");
            close(serverSocketFD);
            exit(1);
    }
	
	// handle client connections
	
	char *buffer = calloc(BUFFER, sizeof(char));
	fd_set readyFD, storeFD;
	
	FD_ZERO(&readyFD);
	FD_ZERO(&storeFD);
	FD_SET(serverSocketFD, &storeFD);
	
	int *listFD = calloc(1, sizeof(int));
	listFD[0] = serverSocketFD;
	int fds = 1;
	
    while(1){
		
		memcpy(&readyFD, &storeFD, sizeof(fd_set));
		
		if(select(FD_SETSIZE, &readyFD, NULL, NULL, NULL) < 0){
			printf("Something went wrong in select()");
            exit(1);
		
		}
		for(int i = 0, j = fds; i < j; ++i){
			if(FD_ISSET(listFD[i], &readyFD)){
				if(listFD[i] == serverSocketFD){
					
					//handle new client
					
					struct sockaddr theirAddress;
					socklen_t addrSize = sizeof (theirAddress);
					int newFD;
					if((newFD = accept(serverSocketFD, &theirAddress, &addrSize)) == -1){
						printf("Something went wrong when trying to accept\n");
       				}else{
						printf("Connection accepted \n");
						addFD(newFD, listFD, &fds, &storeFD);
					}
					
					//handle server at max capacity
					
					if(fds == FD_SETSIZE){
						send(newFD, "Servers is at max capacity\n", 27 * sizeof(char), 0);
						printf("Connection terminated\n");
						close(listFD[i]);
						removeFD(listFD[i], listFD, &fds, &storeFD);
					}
				} else{
					
					//recieve message from a client
					
					int result;
					memset(buffer, 0, sizeof(char)*BUFFER);
					if((result = recv(listFD[i], buffer, BUFFER, 0)) == -1){
						printf("Something went wrong when tryining to receive a message");
					}else if(result == 0){
						
						//handle client disconnecting
						
						printf("Connection terminated\n");
						close(listFD[i]);
						removeFD(listFD[i], listFD, &fds, &storeFD);
					}else{
						
						//broadcast message to all other clients
						
						broadcast(listFD[i], serverSocketFD, listFD, fds, buffer, BUFFER);
					}
				}
			}
		}
    }
	
    return 0;
}

int addFD(int fd ,int *fdArr, int *arrLen, fd_set *fdSet){
	FD_SET(fd, fdSet);
	if((fdArr = realloc(fdArr ,(*arrLen + 1) * sizeof(int))) == NULL){
		return -1;
	}
	fdArr[*arrLen] = fd;
	++(*arrLen);
	
	return 1;
}

int broadcast(int fd, int serverFD, int *fdArr, int arrLen, char *message, int messageLen){
	for(int i = 0; i < arrLen; ++i){
		if(fdArr[i] != fd && fdArr[i] != serverFD){
			send(fdArr[i], message, messageLen, 0);
		}
	}
}

int removeFD(int fd ,int *fdArr, int *arrLen, fd_set *fdSet){
	FD_CLR(fd, fdSet);
	removeElement(fdArr, *arrLen, fd);
	--(*arrLen);
	
	if((fdArr = realloc(fdArr ,(*arrLen) * sizeof(int))) == NULL){
		return -1;
	}
	
	return 0;
}

void removeElement(int *arr, int arrLen, int elem){
	for(int i = 0; i < arrLen - 1; ++i){
		if(arr[i] == elem){
			arr[i] = arr[i+1];
			arr[i+1] = elem;
		}
	}
	arr[arrLen-1] = 0;
}


