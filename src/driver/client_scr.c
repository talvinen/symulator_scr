#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <math.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <tpl.h>

#define NUMBER_N 500
#define THREADS_N 5

/*Implementacja zastosowania techniki RPC
 * architektura klient server
 * wysylanie serializowanych danych za pomoca biblioteki tpl ==> http://troydhanson.github.io/tpl/
 * wysylana jest paczka: string(nazwa funkcji jaka wywolac), int(rozmiar tablicy), double(tablica)
 * 
 * nalezy wczesniej uruchomic 5 serwerow w tle nasluchujacych na portach z tablicy ports
 * 
 * Paweł Łobacz gr. MZ02IP1
 * zad dom wersja RPC2
 */
 

int connectToServ(const char *host, const int port);

static const char *servAdrr = "localhost";

int main(int argc, char *argv[]) {	
		int sockfd = 0;
		sockfd = connectToServ(servAdrr, 8787);
		if (sockfd < 0) {
			fprintf(stderr, "ERROR unable to connect to server %s on port %d\n", servAdrr, 8787);
			exit(1);
		}
	while(1)
		sleep(1);
	return 0;
}

int connectToServ(const char *host, const int port) {
	int sockfd/*, n*/;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	
	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	
	if (sockfd < 0) {
		return -1;
	}
	
	server = gethostbyname(host);
	
	if (server == NULL) {
		return -1;
	}
	
	bzero((char *) &serv_addr, sizeof(serv_addr));
	
	serv_addr.sin_family = AF_INET;
	
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	
	serv_addr.sin_port = htons(port);
	
	if (connect(sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
		return -1;
	}
	
	return sockfd;
}
