#include "../include/hw4.h"

int main() {
    int listenfd, connfd;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Set options to reuse the IP address and IP port if either is already in use
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))");
        return -1;
    }
    if (setsockopt(listenfd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &opt, sizeof(opt))");
        return -1;
    }

    // Bind socket to port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);
    if (bind(listenfd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(listenfd, 1) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    INFO("Server listening on port %d", PORT);
    // Accept incoming connection
    if ((connfd = accept(listenfd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
        perror("accept");
        exit(EXIT_FAILURE);
    }

    INFO("Server accepted connection");

		char buffer[1024] = {0};
		ChessGame game;
		initialize_game(&game);
    while (1) {
			int result_code;
			// Receiving
			do {
				printf("[Server] Waiting for client command...\n");
				memset(buffer, 0, sizeof(buffer));
				int nbytes = read(connfd, buffer, sizeof(buffer));
				if (nbytes <= 0) {
					perror("[Server] Read failed");
					exit(EXIT_FAILURE);
				}
				printf("[Server] Received from client: %s\n", buffer);
				result_code = receive_command(&game, buffer, connfd, false);
				if (result_code == COMMAND_FORFEIT) {close(connfd); break;}
			} while (game.currentPlayer == WHITE_PLAYER);

			if (result_code == COMMAND_FORFEIT) break;

			// Sending
			do {
				printf("[Server] Please enter a command: ");
				memset(buffer, 0, sizeof(buffer));
				fgets(buffer, sizeof(buffer), stdin);
				buffer[strlen(buffer)-1] = '\0';
				printf("[Server] Command Entered: %s\n", buffer);
				result_code = send_command(&game, buffer, connfd, false);
				printf("Result code: %d\n", result_code);
				if (result_code == COMMAND_DISPLAY) continue;
				if (result_code == COMMAND_FORFEIT) {close(connfd); break;}
				if (result_code == COMMAND_IMPORT) {
					if (game.currentPlayer == WHITE_PLAYER) break;
				}
				if (result_code == COMMAND_ERROR) {
					printf("[Server] COMMAND_ERROR\n");
					continue;
				} 
				if (result_code == COMMAND_UNKNOWN) {
					printf("[Server] COMMAND_UNKNOWN\n");
					continue;
				}
			} while (result_code != COMMAND_MOVE);

			if (result_code == COMMAND_FORFEIT) break;
    }

    close(listenfd);
    return 0;
}
