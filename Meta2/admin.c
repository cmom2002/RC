#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <math.h>
#include <netdb.h>

#define BUFLEN 1024

struct sockaddr_in si_outra;
int recv_len;
socklen_t slen = sizeof(si_outra);
int sock;

void erro(char *s) {
	perror(s);
	exit(1);
}

void send_to_server(char *buf){
    if ((sendto(sock, buf, strlen(buf), 0, (struct sockaddr *) &si_outra, slen) == -1))
        erro("Failed to send message to client.");
}

int main(int argc, char *argv[]) {
    if (argc != 3)
        erro("new_admin {endereço do servidor} {PORTO_CONFIG}");
  	
	char buffer[BUFLEN];
	char *hostname = *(argv + 1);
    int port = atoi(*(argv + 2));

	if (gethostbyname(argv[1]) == 0) {
        fprintf(stderr, "Host not found!\n");
        exit(-1);
    }
	memset((void *) &si_outra, 0, sizeof(si_outra));

	si_outra.sin_family = AF_INET;
	si_outra.sin_port = htons(port);
	si_outra.sin_addr.s_addr = htonl(INADDR_ANY);
	
	if((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1) {
		erro("Erro na criação do socket");
	}
	
	if (connect(sock, (struct sockaddr *) &si_outra, slen) < 0)
        erro("Erro na conexão");
	
	send_to_server("asdfgf");
    while(1){
		
        if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
            erro("Erro no recvfrom");
        }
		buffer[recv_len]='\0';

		while(1){
			printf("%s", buffer);
			memset(buffer, 0, BUFLEN);
			scanf("%s", buffer);
			send_to_server(buffer);
			memset(buffer, 0, BUFLEN);
			if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
				erro("Erro no recvfrom");
			}
			buffer[recv_len]='\0';
			if(strcmp(buffer, "Password: ") == 0){
				break;
			}
		}

		while(1){
			printf("%s", buffer);
			memset(buffer, 0, BUFLEN);
			scanf("%s", buffer);
			send_to_server(buffer);
			memset(buffer, 0, BUFLEN);
			if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
				erro("Erro no recvfrom");
			}
			buffer[recv_len]='\0';
			if(strcmp(buffer, "Welcome!\n") == 0){
				break;
			}
		}
		printf("%s", buffer);
		memset(buffer, 0, BUFLEN);
		while(1){
			scanf("%s", buffer);
			send_to_server(buffer);
			char *token = strtok(buffer, " \n");
			char aux[4][BUFLEN];
			int pos = 0;
			while (token != NULL) {
				strcpy(aux[pos++], token);
				token = strtok(NULL, " \n");
			}
			if(strcmp(aux[0], "QUIT\n") == 0){
				close(sock);
				break;
			}
			else if(strcmp(aux[0], "ADD_USER") == 0){
				if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
					erro("Erro no recvfrom");
				}
				buffer[recv_len]='\0';
				printf("%s", buffer);
				memset(buffer, 0, BUFLEN);
			}
			else if(strcmp(aux[0], "DEL") == 0){
				if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
					erro("Erro no recvfrom");
				}
				buffer[recv_len]='\0';
				printf("%s", buffer);
				memset(buffer, 0, BUFLEN);
			}
			else if(strcmp(aux[0], "LIST") == 0){
				if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
					erro("Erro no recvfrom");
				}
				buffer[recv_len]='\0';
				printf("%s", buffer);
				memset(buffer, 0, BUFLEN);
			}
			else if(strcmp(aux[0], "QUIT_SERVER") != 0){
				if((recv_len = recvfrom(sock, buffer, BUFLEN, 0, (struct sockaddr *) &si_outra, (socklen_t *)&slen)) == -1) {
					erro("Erro no recvfrom");
				}
				buffer[recv_len]='\0';
				printf("%s", buffer);
				memset(buffer, 0, BUFLEN);
			}
			memset(buffer, 0, BUFLEN);
		}
		




	close(sock);
	return 0;
	}
}