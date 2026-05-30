#include <stdio.h>
#include <stdbool.h>


// 1. verificar ganador
// retorna: 1 si ganó 'X' (jugador 1), 2 si ganó 'O' (jugador 2), 0 si empate, -1 si sigue el juego
int check_winner(char board[3][3]) {
    char winner_symbol = ' ';

    // verificar filas y columnas
    for (int i = 0; i < 3; i++) {
        if (board[i][0] != ' ' && board[i][0] == board[i][1] && board[i][1] == board[i][2]) winner_symbol = board[i][0];
        if (board[0][i] != ' ' && board[0][i] == board[1][i] && board[1][i] == board[2][i]) winner_symbol = board[0][i];
    }
    // diagonales
    if (board[0][0] != ' ' && board[0][0] == board[1][1] && board[1][1] == board[2][2]) winner_symbol = board[0][0];
    if (board[0][2] != ' ' && board[0][2] == board[1][1] && board[1][1] == board[2][0]) winner_symbol = board[0][2];

    // para saber si hubo ganador.
    if (winner_symbol == 'X') return 1; // jugador 1 ganó
    if (winner_symbol == 'O') return 2; // jugador 2 ganó

    // verificar si sigue el juego o es empate
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            if (board[i][j] == ' ') return -1; // sigue el juego

    return 0; // empate
}

// 2. inicializar (para limpiar la matriz)
void init_grid(char board[3][3]) {
    for (int i = 0; i < 3; i++)
        for (int j = 0; j < 3; j++)
            board[i][j] = ' ';
}

// 3. imprimir tablero
void print_grid(char board[3][3]) {
    printf("\n");
    for (int i = 0; i < 3; i++) {
        printf(" %c | %c | %c \n", board[i][0], board[i][1], board[i][2]);
        if (i < 2) printf("-----------\n");
    }
    printf("\n");
}