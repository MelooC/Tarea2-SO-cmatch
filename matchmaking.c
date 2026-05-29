#include "tipos.h"

/* ===== estado global compartido ===== */
Jugador *jugadores = NULL;
Tablero *tableros  = NULL;

int *lobby_espera = NULL;
int  lobby_n = 0;
pthread_mutex_t lobby_mutex = PTHREAD_MUTEX_INITIALIZER;

void *jugador_thread(void *arg) {
    Jugador *yo = (Jugador *)arg;

    pthread_mutex_lock(&lobby_mutex);
    lobby_espera[lobby_n] = yo->id;
    lobby_n++;
    printf("Jugador %d entró al lobby (ELO %d). En espera: %d\n",
           yo->id, yo->elo, lobby_n);
    pthread_mutex_unlock(&lobby_mutex);

    return NULL;
}