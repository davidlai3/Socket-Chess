#include "../include/hw4.h"

void initialize_game(ChessGame *game) {
	char (*board)[8] = game->chessboard;
	char row1[8] = {'r','n','b','q','k','b','n','r'};
	char row2[8] = {'p','p','p','p','p','p','p','p'};
	char row3[8] = {'.','.','.','.','.','.','.','.'};
	char row4[8] = {'P','P','P','P','P','P','P','P'};
	char row5[8] = {'R','N','B','Q','K','B','N','R'};

	memcpy(board[0], row1, sizeof(row1));
	memcpy(board[1], row2, sizeof(row2));
	memcpy(board[2], row3, sizeof(row3));
	memcpy(board[3], row3, sizeof(row3));
	memcpy(board[4], row3, sizeof(row3));
	memcpy(board[5], row3, sizeof(row3));
	memcpy(board[6], row4, sizeof(row4));
	memcpy(board[7], row5, sizeof(row5));

	game->moveCount = 0;
	game->capturedCount = 0;
	game->currentPlayer = 0;

}

void chessboard_to_fen(char fen[], ChessGame *game) {
	char (*board)[8] = game->chessboard;
	char next = game->currentPlayer == 0 ? 'w' : 'b';
	memset(fen, 0, 72); // Set fen to '0'
	int fen_length = 0;
	for (int i = 0; i < 8; i++) {
		for (int j = 0; j < 8; j++) {
			if (board[i][j] != '.') { // Piece encountered
				if (fen[fen_length] != '\0') {
					++fen_length; // Check if prev was empty
				} 
				fen[fen_length++] = board[i][j];; // Update
			}
			else if (fen[fen_length] != '\0') (fen[fen_length])++; // Increment space
			else fen[fen_length] = '1'; // Update
		}
		if (fen[fen_length] != '\0') {
			fen_length++; // Check if prev was empty
		}
		fen[fen_length++] = '/';
	}
	fen[fen_length-1] = ' ';
	fen[fen_length] = next;
	fen[++fen_length] = '\0';
}

bool is_valid_pawn_move(char piece, int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) {
	char (*board)[8] = game->chessboard;
	int row_dif = dest_row - src_row;
	int col_dif = abs(dest_col - src_col);
	char dest_cell = board[dest_row][dest_col];

	// Magnitude of move error
	if (row_dif == 0 || abs(row_dif) > 2) return false;
	if (col_dif > 1) return false;
	if (abs(row_dif) == 2 && col_dif == 1) return false;

	// Move 2 only on first move
	if (abs(row_dif) == 2) {
		if (src_row != 1 && src_row != 6) return false;
		if (board[dest_row - (row_dif/2)][dest_col] != '.') return false;
	}

	// Direction of move error
	if (row_dif < 0 && piece == 'p') return false;
	if (row_dif > 0 && piece == 'P') return false;

	// Occupied square
	if (col_dif == 0 && dest_cell != '.') return false;
	if (col_dif == 1 && dest_cell == '.') return false;

	return true;
}

bool is_valid_rook_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) {
	char (*board)[8] = game->chessboard;
	int row_dif = dest_row - src_row;
	int col_dif = dest_col - src_col;
	
	// Horizontal or vertical movement
	if (!((row_dif == 0) ^ (col_dif == 0))) return false;

	// Going over pieces
	// Decrement 1 in for loop for capture case
	int dir = row_dif <= 0 ? -1 : 1;
	for (int i = 1; i < abs(row_dif); i++) {
		if (board[src_row + (i * dir)][src_col] != '.') return false;
	}
	dir = col_dif <= 0 ? -1 : 1;
	for (int i = 1; i < abs(col_dif); i++) {
		if (board[src_row][src_col + (i * dir)] != '.') return false;
	}

	return true;
}

bool is_valid_knight_move(int src_row, int src_col, int dest_row, int dest_col) {
	int row_dif = abs(dest_row - src_row);
	int col_dif = abs(dest_col - src_col);

	return ((row_dif == 2 && col_dif == 1) || (row_dif == 1 && col_dif == 2));
}

bool is_valid_bishop_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) {
	char (*board)[8] = game->chessboard;
	int row_dif = dest_row - src_row;
	int col_dif = dest_col - src_col;

	// Non-diagonal movement
	if (abs(row_dif) != abs(col_dif)) {
		return false;
	}

	// Going over pieces
	// Decrement 1 for capture case
	int hdir = col_dif <= 0 ? -1 : 1; 
	int vdir = row_dif <= 0 ? -1 : 1;
	for (int i = 1; i < abs(row_dif); i++) {
		if (board[src_row + (vdir * i)][src_col + (hdir * i)] != '.') return false;
	}

	return true;
}

bool is_valid_queen_move(int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) {
	bool valid_rook = is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
	bool valid_bish = is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);

	return (valid_rook || valid_bish);
}

bool is_valid_king_move(int src_row, int src_col, int dest_row, int dest_col) {
	int row_dif = abs(dest_row - src_row);
	int col_dif = abs(dest_col - src_col);
	if (row_dif > 1 || col_dif > 1) return false;
	return ((row_dif == 1) || (col_dif == 1));
}

bool is_valid_move(char piece, int src_row, int src_col, int dest_row, int dest_col, ChessGame *game) {
	char (*board)[8] = game->chessboard;
	char src_cell = board[src_row][src_col];
	switch(src_cell) {
		case 'p':
		case 'P':
			return is_valid_pawn_move(piece, src_row, src_col, dest_row, dest_col, game);
		case 'r':
		case 'R':
			return is_valid_rook_move(src_row, src_col, dest_row, dest_col, game);
		case 'n':
		case 'N':
			return is_valid_knight_move(src_row, src_col, dest_row, dest_col);
		case 'b':
		case 'B':
			return is_valid_bishop_move(src_row, src_col, dest_row, dest_col, game);
		case 'q':
		case 'Q':
			return is_valid_queen_move(src_row, src_col, dest_row, dest_col, game);
		case 'k':
		case 'K':
			return is_valid_king_move(src_row, src_col, dest_row, dest_col);
		default:
			return false;
	}
}

void fen_to_chessboard(const char *fen, ChessGame *game) {
	int i = 0; int row = 0; int col = 0;
	char (*board)[8] = game->chessboard;
	while (fen[i] != ' ') {
		if (fen[i] == '/') {
			row++; col = 0;
			i++;
			continue;
		}
		else if (fen[i] < 57) { // Check for numbers
			for (int j = 0; j < fen[i] - 48; j++) {
				board[row][col++] = '.';
			}
		}
		else if (fen[i] > 64) board[row][col++] = fen[i]; // Check for pieces
		i++;
	}
	game->currentPlayer = fen[++i] == 'w' ? 0 : 1;
}
void update_chessmove(const char *move, ChessMove *parsed_move) {
	char src[3] = {move[0], move[1], '\0'};
	char dest[4] = {move[2], move[3], '\0'};
	if (strlen(move) == 5) {
		dest[2] = move[4];
		dest[3] = '\0';
	}
	memcpy(parsed_move->startSquare, src, sizeof(src));
	memcpy(parsed_move->endSquare, dest, sizeof(dest));
}

int parse_move(const char *move, ChessMove *parsed_move) {
	// Invalid move length or column letter
	if (strlen(move) != 4 && strlen(move) != 5) return PARSE_MOVE_INVALID_FORMAT;
	if (move[0] < 97 || move[0] > 104) return PARSE_MOVE_INVALID_FORMAT;
	if (move[2] < 97 || move[2] > 104) return PARSE_MOVE_INVALID_FORMAT;

	// Invalid row number
	if (move[1] < 49 || move[1] > 56) return PARSE_MOVE_OUT_OF_BOUNDS;
	if (move[3] < 49 || move[3] > 56) return PARSE_MOVE_OUT_OF_BOUNDS;

	// If we are not promoting then success
	if (strlen(move) == 4) {
		update_chessmove(move, parsed_move);
		return 0;
	} 

	// Invalid promotion destination
	if (!(move[3] == '8' || move[3] == '1')) return PARSE_MOVE_INVALID_DESTINATION;

	// Invalid promotion
	char pieces[] = {'q', 'r', 'b', 'n'};
	bool found = false;
	for (int i = 0; i < 4; i++) {
		if (move[4] == pieces[i]) found = true;
	}
	if (!found) return PARSE_MOVE_INVALID_PROMOTION;

	update_chessmove(move, parsed_move);
	return 0;
}

void update_board(ChessGame *game, ChessMove *move) {
	char (*board)[8] = game->chessboard;
	int src_row = abs(move -> startSquare[1] - 56); int src_col = move->startSquare[0] - 97;
	int dest_row = abs(move -> endSquare[1] - 56); int dest_col = move->endSquare[0] - 97;
	char src_cell = board[src_row][src_col];
	char dest_cell = board[dest_row][dest_col];

	if (dest_cell != '.') {
		game->capturedPieces[game->capturedCount] = dest_cell;
		game->capturedCount++;
	}
	if (strlen(move->endSquare) == 3) {
		if (islower(src_cell)) board[dest_row][dest_col] = move->endSquare[2];
		else board[dest_row][dest_col] = toupper(move->endSquare[2]);
	}
	else {
		board[dest_row][dest_col] = src_cell;
	}

	board[src_row][src_col] = '.';
	game->moves[game->moveCount] = *move;
	game->moveCount++;
}

// white is client black is server
// 0 is white 1 is black
int make_move(ChessGame *game, ChessMove *move, bool is_client, bool validate_move) {
	char (*board)[8] = game->chessboard;
	int current_player = game->currentPlayer;
	int src_row = abs(move -> startSquare[1] - 56); int src_col = move->startSquare[0] - 97;
	int dest_row = abs(move -> endSquare[1] - 56); int dest_col = move->endSquare[0] - 97;
	char src_cell = board[src_row][src_col];
	char dest_cell = board[dest_row][dest_col];

	if (validate_move) {
		if (current_player == is_client) return MOVE_OUT_OF_TURN;
		if (src_cell == '.') return MOVE_NOTHING;
		if (islower(src_cell) && is_client) return MOVE_WRONG_COLOR;
		if (isupper(src_cell) && !is_client) return MOVE_WRONG_COLOR;
		if (dest_cell != '.') {
			if (isupper(src_cell) == isupper(dest_cell)) return MOVE_SUS;
		}
		if (strlen(move->endSquare) == 3) {
			if ((src_cell != 'p') && (src_cell != 'P')) return MOVE_NOT_A_PAWN;
		}
		if (strlen(move->endSquare) == 2) {
			if (src_cell == 'p' && move->endSquare[1] == 1) return MOVE_MISSING_PROMOTION;
			if (src_cell == 'P' && move->endSquare[1] == 7) return MOVE_MISSING_PROMOTION;
		}
		if (!is_valid_move(src_cell, src_row, src_col, dest_row, dest_col, game)) return MOVE_WRONG;

	}
	game->currentPlayer ^= 1;
	update_board(game, move);
	return 0;
}

int move_command(ChessGame *game, const char *message, char *move, int socketfd, bool is_client, bool validate_move) {
	printf("Parsing move\n");
	ChessMove *parsed_move = malloc(sizeof(ChessMove));
	int result;
	if ((result = parse_move(move, parsed_move)) != 0) {
		printf("Parsing Error: Code %d\n", result);
		return COMMAND_ERROR;
	} 
	printf("Making move\n");
	if ((result = make_move(game, parsed_move, is_client, validate_move)) != 0) {
		printf("Move Error: Code %d\n", result);
		return COMMAND_ERROR;
	} 
	printf("Move successful\n");
	free(parsed_move);
	if (validate_move) send(socketfd, message, strlen(message), 0);
	return COMMAND_MOVE;
}

int import_command(ChessGame *game, const char *message, char *fen, int socketfd, bool is_client) {
	char to_play = fen[strlen(fen)-1];
	game->currentPlayer = to_play == 'w' ? WHITE_PLAYER : BLACK_PLAYER;
	if (!is_client) {
		fen_to_chessboard(fen, game);
		send(socketfd, message, strlen(message), 0);
	}
	return COMMAND_IMPORT;
}

bool is_valid_username(const char *username) {
	if (username == NULL) return false;
	int length = (int) strlen(username);
	if (length == 0) return false;
	for (int i = 0; i < length; i++) {
		if (username[i] == ' ') return false;
	}
	return true;
}


int load_command(ChessGame *game, const char *message, char *username, char *game_num, int socketfd, bool is_send) {
	printf("Loading game\n");
	if (game_num == NULL) return COMMAND_ERROR;
	int game_number = (int) strtol(game_num, NULL, 10);
	if (load_game(game, username, "game_database.txt", game_number) == -1) return COMMAND_ERROR;
	if (is_send) send(socketfd, message, strlen(message), 0);
	return COMMAND_LOAD;
}

int save_command(ChessGame *game, const char *username) {
	if (save_game(game, username, "game_database.txt") == -1) return COMMAND_ERROR;
	return COMMAND_SAVE;
}

// TODO: strtok only takes first username word
int send_command(ChessGame *game, const char *message, int socketfd, bool is_client) {
	char *token; const char delim[2] = " ";
	char *str = strdup(message);
	token = strtok(str, delim);
	if (strcmp(token, "/move") == 0) {
		printf("Sending move\n");
		char *move = strtok(NULL, delim);
		return move_command(game, message, move, socketfd, is_client, true);
	}
	if (strcmp(token, "/forfeit") == 0) {
		send(socketfd, message, strlen(message), 0);
		return COMMAND_FORFEIT;
	}
	if (strcmp(token, "/chessboard") == 0) {
		display_chessboard(game);
		return COMMAND_DISPLAY;
	}
	if (strcmp(token, "/import") == 0) {
		if (is_client) return COMMAND_ERROR; // Only server can import
		char *fen = strtok(NULL, "");
		return import_command(game, message, fen, socketfd, is_client);
	}
	if (strcmp(token, "/load") == 0) {
		char *username = strtok(NULL, delim); 
		char *game_num = strtok(NULL, delim);
		return load_command(game, message, username, game_num, socketfd, true);
	}
	if (strcmp(token, "/save") == 0) {
		char *username = strtok(NULL, delim);
		return save_command(game, username);
	}
	free(str);
	return COMMAND_UNKNOWN;
}

// TODO: There's a space in the fen string not caught
int receive_command(ChessGame *game, const char *message, int socketfd, bool is_client) {
	char *token; const char delim[2] = " ";
	char *str = strdup(message);
	token = strtok(str, delim);
	if (strcmp(token, "/move") == 0) {
		char *move = strtok(NULL, delim);
		return move_command(game, message, move, socketfd, is_client, false);
	}
	if (strcmp(token, "/forfeit") == 0) {
		return COMMAND_FORFEIT;
	}
	if (strcmp(token, "/chessboard") == 0) {
		display_chessboard(game);
		return COMMAND_DISPLAY;
	}
	if (strcmp(token, "/import") == 0) {
		char *fen = strtok(NULL, "");
		if (is_client) fen_to_chessboard(fen, game);
		else return COMMAND_ERROR; // Only client can receive
		return COMMAND_IMPORT;
	}
	if (strcmp(token, "/load") == 0) {
		char *username = strtok(NULL, delim);
		char *game_num = strtok(NULL, delim);
		return load_command(game, message, username, game_num, socketfd, false);
	}
	free(str);
	return -1;
}

int save_game(ChessGame *game, const char *username, const char *db_filename) {
	if (!is_valid_username(username)) return COMMAND_ERROR;
	FILE *fp = fopen(db_filename, "a");
	if (fp == NULL) return COMMAND_ERROR;
	// Generate fen string
	char fen[64]; memset(fen, 0, 64);
	chessboard_to_fen(fen, game);
	printf("FEN: %s\n", fen);
	fprintf(fp, "%s:%s\n", username, fen);
	fclose(fp);
	return 0;
}

// Checking for null save number outside of this function
int load_game(ChessGame *game, const char *username, const char *db_filename, int save_number) {
	if (!is_valid_username(username)) return -1; // If invalid username break
	FILE *fp = fopen(db_filename, "r");
	if (fp == NULL) return -1;
	int counter = 0; // Counter for username
	int user_length = (int) strlen(username); // Length of username
	char buffer[user_length + 1];
	memset(buffer, 0, user_length + 1); // Clear buffer
	char fen[64];
	while (fgets(buffer, user_length + 1, fp) != NULL) { // Read username for each line
		buffer[user_length] = '\0';
		if (strcmp(buffer, username) == 0) counter++;
		if (counter == save_number) break;
		while (fgetc(fp) != '\n') continue; // Go to next line
	}
	if (fgets(fen, 64, fp) == NULL) return -1;
	fen_to_chessboard(fen, game);
	return 0;
}

void display_chessboard(ChessGame *game) {
    printf("\nChessboard:\n");
    printf("  0 1 2 3 4 5 6 7\n");
    for (int i = 0; i < 8; i++) {
        printf("%d ", i);
        for (int j = 0; j < 8; j++) {
            printf("%c ", game->chessboard[i][j]);
        }
        printf("%d\n", 8 - i);
    }
    printf("  a b c d e f g h\n");
}
