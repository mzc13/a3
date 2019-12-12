all:
	gcc DUMBserver.c -o DUMBserve -pthread -std=gnu99
	gcc DUMBclient.c -o DUMBclient -std=gnu99
client:
	gcc DUMBclient.c -o DUMBclient -std=gnu99
serve:
	gcc DUMBserver.c -o DUMBserve -pthread -std=gnu99
clean:
	rm -f DUMBclient DUMBserve
