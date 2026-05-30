#ifndef TIPOS_H
#define TIPOS_H

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <pthread.h>
#include <stdbool.h>

//para el .env
typedef struct {

    int N_PLAYERS;
    int K_BOARDS;
    int K_ELO;
    int MAX_ELO_DIFF;
    int TURN_DELAY_MS;

    float REENTER_PROBABILITY;

    char SNAPSHOT_PATH[256];

} Config;


//tienen mutex para proteger a cada jugador y a cada tablero.

typedef struct {
    int id;

    int elo;

    int ganadas;
    int perdidas;
    int empatadas;

    pthread_t thread;

    pthread_mutex_t lock;
    pthread_cond_t cond_ready; // para esperar oponente o turno (??)

} Jugador;


typedef struct {
    int id; //id del tablero

    //ids de los jugadores que están jugando en el tablero. Si no hay jugadores, ambos son -1.
    int player1_id;
    int player2_id;

    char grid[3][3];    // estado del tablero: 'X', 'O', ' '
    int current_turn;   // ID del jugador que le toca
    bool active;        // 0 si el tablero no está en uso, 1 si está en uso

    pthread_t thread;

    pthread_mutex_t lock;
    pthread_cond_t cond_turn; // pra coordinar los turnos de los jugadores

} Tablero;


typedef struct {
    int id;

    Jugador *p1;
    Jugador *p2;

    Tablero *board;

    int finished;

} Partida;


/* ===== estado global compartido (definido en main.c / matchmaking.c) ===== */
extern Config config;

extern Jugador *jugadores;   // arreglo de N jugadores
extern Tablero *tableros;    // arreglo de K tableros

extern int *lobby_espera;          // ids de jugadores esperando, en orden de llegada
extern int  lobby_n;               // cuántos están esperando ahora
extern pthread_mutex_t lobby_mutex; // protege lobby_espera y lobby_n
extern volatile bool keep_running;

/* ===== prototipos ===== */
// matchmaking.c
void *jugador_thread(void *arg);
void *tablero_thread(void *arg); 
int buscar_rival(Jugador *yo);

// gato.c
int  check_winner(char board[3][3]);
void init_grid(char board[3][3]);
void print_grid(char board[3][3]);

// tablero.c
void actualizar_elo(Jugador *p1, Jugador *p2, int resultado_gato);
#endif // TIPOS_H