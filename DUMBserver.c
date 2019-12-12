#include<sys/types.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/socket.h>
#include<netdb.h>
#include<pthread.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<time.h>

// Max buffer size is 20
#define BUF_SIZE 20

int* numberOfConnections;

typedef struct _ThreadArgs {
	int clientfd;
	struct sockaddr_in clientAddress;
} ThreadArgs;

// Returns a timestap string
char* getTime() {
	time_t rawtime;
	struct tm* timeinfo;

	time(&rawtime);
	timeinfo = localtime(&rawtime);
	return asctime(timeinfo);
}

// Deals with client connections and communications
void* establishConnection(void* _args) {
	ThreadArgs* args = _args;
	int clientfd = args->clientfd;
	struct sockaddr_in clientAddress = args->clientAddress;

	printf("%s\t%s\tconnected\n", getTime(), inet_ntoa(clientAddress.sin_addr));

	FILE* stm = fdopen(clientfd, "r+");
	char buf[BUF_SIZE];
	while (1) {
		// Only commands supported are HELLO and GDBYE
		fread(buf, 1, 20, stm);
		fflush(stm);
		if (strcmp(buf, "HELLO") == 0) {
			printf("%s\t%s\tHELLO\n", getTime(), inet_ntoa(clientAddress.sin_addr));
			strcpy(buf, "HELLO DUMBv0 ready!");
			fwrite(buf, 1, 20, stm);
			fflush(stm);
		}
		else if (strcmp(buf, "GDBYE") == 0) {
			printf("%s\t%s\tGDBYE\n", getTime(), inet_ntoa(clientAddress.sin_addr));
			printf("%s\t%s\tdisconnected\n", getTime(), inet_ntoa(clientAddress.sin_addr));
			break;
		}
	}
	// Closes file descriptor when the client disconnects
	if (close(clientfd)) {
		printf("Error closing client fd\n");
	}
	*numberOfConnections -= 1;
}

int main(int argc, char* argv[]) {

	if (argc != 2) {
		printf("Need to specify port number as argument.\n");
		return 0;
	}
	int portNumber = atoi(argv[1]);
	if (portNumber < 4097) {
		printf("Port must be a number greater than 4096.\n");
		return 0;
	}

	numberOfConnections = malloc(sizeof(int));
	*numberOfConnections = 0;

	// Necessary stuff to make connection
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* resultPointer;
	int socketfd;
	int addrInfo;
	char buf[BUF_SIZE];

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = AI_PASSIVE;    /* For wildcard IP address */
	hints.ai_protocol = 0;          /* Any protocol */
	hints.ai_canonname = NULL;
	hints.ai_addr = NULL;
	hints.ai_next = NULL;

	addrInfo = getaddrinfo("localhost", argv[1], &hints, &result);
	if (addrInfo != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(addrInfo));
		exit(EXIT_FAILURE);
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully bind(2).
	   If socket(2) (or bind(2)) fails, we (close the socket
	   and) try the next address. */

	for (resultPointer = result; resultPointer != NULL; resultPointer = resultPointer->ai_next) {
		socketfd = socket(resultPointer->ai_family, resultPointer->ai_socktype, resultPointer->ai_protocol);
		if (socketfd == -1)
			continue;

		if (bind(socketfd, resultPointer->ai_addr, resultPointer->ai_addrlen) == 0) {
			printf("Socket Bound Successfully\n");
			break;                  /* Success */
		}
	}

	if (resultPointer == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not bind\n");
		exit(EXIT_FAILURE);
	}

	// Max of 10 connections allowed
	if ((listen(socketfd, 10)) < 0) {
		printf("Listen failed...\n");
		exit(0);
	}
	else
		printf("Server listening..\n");

	// Preparing threads for clients
	socklen_t connection0Len;
	struct sockaddr_in connection0Address;

	pthread_t threadArray[10];
	ThreadArgs** args = malloc(sizeof(ThreadArgs*) * 10);
	for (int i = 0; i < 10; i++) {
		args[i] = malloc(sizeof(ThreadArgs));
	}
	while (1) {
		if (*numberOfConnections < 9) {
			connection0Len = sizeof(struct sockaddr_in);
			args[*numberOfConnections]->clientfd = accept(socketfd,
				(struct sockaddr*) &(args[*numberOfConnections]->clientAddress),
				&connection0Len);

			if (args[*numberOfConnections]->clientfd < 0) {
				printf("server acccept failed...\n");
				exit(0);
			}
			if (pthread_create(threadArray + *numberOfConnections, NULL, establishConnection, (void*)(args[*numberOfConnections]))) {
				printf("Thread creation failed.\n");
				exit(0);
			}
			else {
				*numberOfConnections += 1;
			}
		}
		else {
			pthread_join(threadArray[*numberOfConnections], NULL);
		}
	}

	return 0;
}