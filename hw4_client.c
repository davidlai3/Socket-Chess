#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include "../include/hw4.h"

#define PORT 8080
#define BUFFER_SIZE 1024

int main() {
    ChessGame game;
    int connfd = 0;
    struct sockaddr_in serv_addr;

    // Connect to the server
    if ((connfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    if (connect(connfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("connect failed");
        exit(EXIT_FAILURE);
    }

    initialize_game(&game);
    display_chessboard(&game);

		char buffer[1024];
    while (1) {
			// Sending
			int result_code;
			do {
				printf("[Client] Please enter a command: ");
				memset(buffer, 0, sizeof(buffer));
				fgets(buffer, sizeof(buffer), stdin);
				buffer[strlen(buffer)-1] = '\0';
				printf("[Client] Command Entered: %s\n", buffer);
				result_code = send_command(&game, buffer, connfd, true);
				if (result_code == COMMAND_DISPLAY) continue;
				if (result_code == COMMAND_FORFEIT) {close(connfd); break;}
				if (result_code == COMMAND_IMPORT) {
					if (game.currentPlayer == BLACK_PLAYER) break;
				}
				if (result_code == COMMAND_ERROR) {
					printf("[Client] COMMAND_ERROR\n");
					continue;
				} 
				if (result_code == COMMAND_UNKNOWN) {
					printf("[Client] COMMAND_UNKNOWN\n");
					continue;
				}
				printf("Current player: %d\n", game.currentPlayer);
			} while (result_code != COMMAND_MOVE);

			if (result_code == COMMAND_FORFEIT) break;

			// Receiving
			do {
				printf("[Client] Waiting for server command...\n");
				memset(buffer, 0, sizeof(buffer));
				int nbytes = read(connfd, buffer, sizeof(buffer));
				if (nbytes <= 0) {
					perror("[Client] Read failed");
					exit(EXIT_FAILURE);
				}
				printf("[Client] Received from server: %s\n", buffer);
				result_code = receive_command(&game, buffer, connfd, true);
				if (result_code == COMMAND_FORFEIT) {close(connfd); break;}
			} while (game.currentPlayer == BLACK_PLAYER);

			if (result_code == COMMAND_FORFEIT) break;
    }

    // Please ensure that the following lines of code execute just before your program terminates.
    // If necessary, copy and paste it to other parts of your code where you terminate your program.
    FILE *temp = fopen("./fen.txt", "w");
    char fen[200];
    chessboard_to_fen(fen, &game);
    fprintf(temp, "%s", fen);
    fclose(temp);
    close(connfd);
    return 0;
}
