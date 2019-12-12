#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

// Max buffer size is 20
#define BUF_SIZE 20

// Function to interpret a user command
int interpretCommand(char* cmd, FILE* stm) {

	// Lists commands if user enters help
	if (strcmp(cmd, "help") == 0) {
		printf("Available Commands:\nquit\nCommands Not Implemented:\ncreate\ndelete\nopen\nclose\nnext\nput\n");
	}
	// If user enters quit, sends server GDBYE and quits on success
	else if (strcmp(cmd, "quit") == 0) {
		char buf[BUF_SIZE];
		strcpy(buf, "GDBYE");
		fwrite(buf, 1, 20, stm);
		fflush(stm);
		printf("Successful Quit.\n");
		return 1;
	}
	// Any other command is unsupported
	else {
		printf("Command not supported\n");
	}
	return 0;
}

int main(int argc, char* argv[]) {

	if (argc != 3) {
		printf("Need to specify ip and port number as arguments.\n");
		return 0;
	}
	int portNumber = atoi(argv[2]);
	if (portNumber < 4097) {
		printf("Port must be a number greater than 4096.\n");
		return 0;
	}

	// Necessary stuff to make connection
	struct addrinfo hints;
	struct addrinfo* result;
	struct addrinfo* resultPointer;
	int socketfd;
	int addrInfo;
	char buf[BUF_SIZE];

	/* Obtain address(es) matching host/port */

	memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;    /* Allow IPv4 or IPv6 */
	hints.ai_socktype = SOCK_STREAM; /* Stream socket */
	hints.ai_flags = 0;
	hints.ai_protocol = 0;          /* Any protocol */

	addrInfo = getaddrinfo(argv[1], argv[2], &hints, &result);
	if (addrInfo != 0) {
		printf("Could not connect with given ip and port.\n");
		return 0;
	}

	/* getaddrinfo() returns a list of address structures.
	   Try each address until we successfully connect(2).
	   If socket(2) (or connect(2)) fails, we (close the socket
	   and) try the next address. */

	for (resultPointer = result; resultPointer != NULL; resultPointer = resultPointer->ai_next) {
		socketfd = socket(resultPointer->ai_family, resultPointer->ai_socktype,
			resultPointer->ai_protocol);
		if (socketfd == -1)
			continue;

		if (connect(socketfd, resultPointer->ai_addr, resultPointer->ai_addrlen) != -1) {
			break;                  /* Success */
		}
		
	}

	if (resultPointer == NULL) {               /* No address succeeded */
		fprintf(stderr, "Could not connect\n");
		exit(EXIT_FAILURE);
	}
	
	// Send HELLO on successful connection
	FILE* stm = fdopen(socketfd, "r+");
	fflush(stm);
	strcpy(buf, "HELLO");
	fwrite(buf, 1, 20, stm);
	fflush(stm);
	fread(buf, 1, 20, stm);
	// If server responds with proper response, allow user to enter commands
	if (strcmp(buf, "HELLO DUMBv0 ready!") == 0) {
		printf("Connected to DUMB Server\n");
	}
	else {
		printf("Improper connection response from server. Closing.\n");
		return 0;
	}
	while (1) {
		printf("Enter command: ");
		scanf("%s", buf);
		if (interpretCommand(buf, stm)) break;
	}

	close(socketfd);
	freeaddrinfo(result);           /* No longer needed */
	
	return 0;
}