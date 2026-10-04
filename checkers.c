#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

// Set up the initial board configuration
void setup_board(int board[8][8]) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if ((row + col) % 2 == 0) {
                if (row < 3) {
                    board[row][col] = 1; 
                } else if (row > 4) {
                    board[row][col] = 2;
                }
            }
        }
    }
}

// Print the current state of the board
void print_board(int board[8][8]) {
    for (int row = 0; row < 8; row++) {
        printf("%d ", abs(row-8));
        for (int col = 0; col < 8; col++) {
            if(board[row][col] == 0) {
                if((row + col) % 2 == 0) {
                    printf("\033[38;5;67m■ \033[0m");
                } else printf("\033[97m■ \033[0m");
            } else if (board[row][col] == 1) {
                printf("○ ");
            } else if (board[row][col] == 2) {
                printf("● ");
            } else if (board[row][col] == 3) {
                printf("\033[38;5;67m♚ \033[0m");
            } else if (board[row][col] == 4) {
                printf("\033[97m♚ \033[0m");
            }
        }
        printf("\n");
    }
    printf("  a b c d e f g h\n");
}

// Check if a piece has any available captures
bool check_available_captures(int board[8][8], int row, int col) {
    int piece = board[row][col];
    int directions[4][2];
    int count;

    if (piece == 1) {
        directions[0][0] = 2;
        directions[0][1] = -2;
        directions[1][0] = 2;
        directions[1][1] = 2;
        count = 2;
    }
    else if (piece == 2) {
        directions[0][0] = -2;
        directions[0][1] = -2;
        directions[1][0] = -2;
        directions[1][1] = 2;
        count = 2;
    }
    else if (piece == 3 || piece == 4) {
        directions[0][0] = 2;
        directions[0][1] = -2;
        directions[1][0] = 2;
        directions[1][1] = 2;
        directions[2][0] = -2;
        directions[2][1] = -2;
        directions[3][0] = -2;
        directions[3][1] = 2;
        count = 4;
    }
    else {
        return false;
    }

    for (int i = 0; i < count; i++) {
        int new_row = row + directions[i][0];
        int new_col = col + directions[i][1];

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8) {
            int mid_row = (row + new_row) / 2;
            int mid_col = (col + new_col) / 2;
            int opponent;

            if (piece == 1 || piece == 3)
                opponent = 2;
            else if (piece == 2 || piece == 4)
                opponent = 1;
            else
                return false;

            if (board[new_row][new_col] == 0 && (board[mid_row][mid_col] == opponent || board[mid_row][mid_col] == opponent + 2)) {
                return true;
            }
        }
    }

    return false;
}

// Check if a player has any available captures
bool check_capture_available(int board[8][8], int player) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == player || board[row][col] == player + 2) {
                if (check_available_captures(board, row, col)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Check if a piece has any available moves
bool check_available_moves(int board[8][8], int row, int col) {
    int piece = board[row][col];
    int directions[4][2];
    int count;

    if (piece == 1) {
        directions[0][0] = 1;
        directions[0][1] = -1;
        directions[1][0] = 1;
        directions[1][1] = 1;
        count = 2;
    }
    else if (piece == 2) {
        directions[0][0] = -1;
        directions[0][1] = -1;
        directions[1][0] = -1;
        directions[1][1] = 1;
        count = 2;
    }
    else if (piece == 3 || piece == 4) {
        directions[0][0] = 1;
        directions[0][1] = -1;
        directions[1][0] = 1;
        directions[1][1] = 1;
        directions[2][0] = -1;
        directions[2][1] = -1;
        directions[3][0] = -1;
        directions[3][1] = 1;
        count = 4;
    }
    else {
        return false;
    }

    for (int i = 0; i < count; i++) {
        int new_row = row + directions[i][0];
        int new_col = col + directions[i][1];

        if (new_row >= 0 && new_row < 8 && new_col >= 0 && new_col < 8) {
            if (board[new_row][new_col] == 0) {
                return true;
            }
        }
    }

    return false;
}

// Check if a player has any available moves
bool check_move_available(int board[8][8], int player) {
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == player || board[row][col] == player + 2) {
                if (check_available_moves(board, row, col)) {
                    return true;
                }
            }
        }
    }
    return false;
}

// Check if a move is legal
bool check_legal(int board[8][8], int start_row, int start_col, int end_row, int end_col, int player) {
    if (start_row < 0 || start_row >= 8 || start_col < 0 || start_col >= 8 || end_row < 0 || end_row >= 8 || end_col < 0 || end_col >= 8){
        printf("Invalid move: Out of bounds\n");
        return false;
    }

    int piece = board[start_row][start_col];

    if (piece == 0 || board[end_row][end_col] != 0){
        printf("Invalid move: Destination is not empty\n");
        return false;
    }

    if (((piece == 1 || piece == 3) && player != 1)|| ((piece == 2 || piece == 4) && player != 2)){
        printf("Invalid move: Not your piece\n");
        return false;
    }

    int dr = end_row - start_row;
    int dc = end_col - start_col;
    bool king = piece > 2;
    bool capture = abs(dr) == 2 && abs(dc) == 2;
    bool move = abs(dr) == 1 && abs(dc) == 1;

    if (!king && ((piece == 1 && dr != 1) || (piece == 2 && dr != -1)))
        move = false;

    if (!move && !capture)
        return false;

    if (capture) {
        int mid_row = (start_row + end_row) / 2;
        int mid_col = (start_col + end_col) / 2;
        int middle = board[mid_row][mid_col];

        if (middle == 0 || (player == 1 && middle != 2 && middle != 4) || (player == 2 && middle != 1 && middle != 3)){
            printf("Invalid move: No opponent piece to capture\n");
            return false;
        }
    }

    if (!capture && check_capture_available(board, player)){
        printf("You have a capture available. You must capture.\n");
        return false;
    }

    return true;
}

// Change pieces to kings if they reach the opposite end of the board, return true if any pieces were kinged, false otherwise
bool make_kings(int board[8][8]) {
    int kinged = 0;
    for (int col = 0; col < 8; col++) {
        if (board[0][col] == 2) {
            board[0][col] = 4;
            kinged++;
        }
        if (board[7][col] == 1) {
            board[7][col] = 3;
            kinged++;
        }
    }
    if(kinged > 0) return true;
    return false;
}

// Count the number of pieces for a given player
int count_pieces(int board[8][8], int player) {
    int count = 0;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board[row][col] == player || board[row][col] == player + 2) {
                count++;
            }
        }
    }
    if(count == 0 && player == 1) {
        printf("Black wins!\n");
    } else if (count == 0 && player == 2) {
        printf("White wins!\n");
    }
    return count;
}

int main() {
    int arr[8][8] = {0};
    setup_board(arr);
    print_board(arr);
    char player[6] = "Black";
    int player_num = 1;
    bool switch_player = true;
    bool game_over = false;
    while(1){
        if(switch_player == true) {
            if(player_num == 2){
                strcpy(player, "Black");
                player_num = 1;
            }
            else{
                strcpy(player, "White");
                player_num = 2;
            }
        }
        printf("%s>", player);
        if(count_pieces(arr, player_num) == 0) {
            game_over = true;
            printf("Game over! %s wins!\n", player);
        } 
        if(!check_capture_available(arr, player_num) && !check_move_available(arr, player_num)) {
            if(player_num == 2) printf("Game over! Black wins!\n");
            else printf("Game over! White wins!\n");
            game_over = true;
        }
        if(!game_over){
            char input[6];
            if (scanf(" %5[^\n]", input) == 1) {
                if(strcmp(player, "White") == 0) player_num = 2;
                else player_num = 1;
                if(input[0] < 'a' || input[0] > 'h' || input[1] < '1' || input[1] > '8' ||
                input[2] != ' ' || input[3] < 'a' || input[3] > 'h' || input[4] < '1' || input[4] > '8') {
                    printf("Invalid input format. Please use the format: a1 b2\n");
                    switch_player = false;
                } else{
                    bool captured = check_capture_available(arr, player_num);
                    if(check_legal(arr, 8 - (input[1] - '0'), input[0] - 'a', 8 - (input[4] - '0'), input[3] - 'a', player_num)) {
                        int start_row = 8 - (input[1] - '0');
                        int start_col = input[0] - 'a';
                        int end_row = 8 - (input[4] - '0');
                        int end_col = input[3] - 'a';
                        if(abs(start_row - end_row) == 2 && abs(start_col - end_col) == 2) {
                            int mid_row = (start_row + end_row) / 2;
                            int mid_col = (start_col + end_col) / 2;
                            arr[mid_row][mid_col] = 0;
                        }
                        arr[end_row][end_col] = arr[start_row][start_col];
                        arr[start_row][start_col] = 0;
                        if(check_capture_available(arr, player_num) && captured) {
                            switch_player = false;
                        } else {
                            switch_player = true;
                        }
                        if(make_kings(arr)) switch_player = true;
                        print_board(arr);
                    } else switch_player = false;
                }    
            } else return 0;
        } else return 0;
    }
    return 0;
}