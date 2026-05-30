#define _DEFAULT_SOURCE 
#include <unistd.h>

#include "tipos.h"
#include <unistd.h>

/* ===== estado global compartido ===== */
Jugador *jugadores = NULL;
Tablero *tableros  = NULL;

int *lobby_espera = NULL;
int  lobby_n = 0;
pthread_mutex_t lobby_mutex = PTHREAD_MUTEX_INITIALIZER;

extern volatile bool keep_running;

// función para eliminar a alguien del lobby por su posición 'idx'
void remover_del_lobby(int idx) {
    // mover los datos a la izquierda para llenar el hueco
    for (int i = idx; i < lobby_n - 1; i++) {
        lobby_espera[i] = lobby_espera[i + 1];
    }
    lobby_n--;
}

//funcion para buscar rival en el lobby, por el ELO y por el tiempo de espera.
int buscar_rival(Jugador *yo) {
    pthread_mutex_lock(&lobby_mutex);
    
    int rival_id = -1;
    
    // 1. buscar en el lobby
    for (int i = 0; i < lobby_n; i++) { //el jugador con más tiempo esperando siempre será el del índice 0
        int id_candidato = lobby_espera[i];
        if (id_candidato == yo->id) continue; // para que no se enpareje consigo mismo jasj

        Jugador *rival = &jugadores[id_candidato];
        
        // 2. condición del ELO.
        if (abs(yo->elo - rival->elo) <= config.MAX_ELO_DIFF) {
            rival_id = id_candidato;
            
            // 3. cuando se encuentra rival que cumple con ambas condiciones. se remueve del lobby.
            // el rival es el que está en i
            remover_del_lobby(i);
            
            // 4. luego remover al jugador actual yo. 
            // como ya no está el rival, hay que buscar dónde quedó 'yo'
            for (int j = 0; j < lobby_n; j++) {
                if (lobby_espera[j] == yo->id) {
                    remover_del_lobby(j);
                    break;
                }
            }
            break; 
        }
    }
    
    pthread_mutex_unlock(&lobby_mutex);
    return rival_id; // retorna el ID del rival o -1 si no encontró a nadie
}




void *jugador_thread(void *arg) {
    Jugador *yo = (Jugador *)arg;

    while (keep_running) {
        // entrar al lobby
        pthread_mutex_lock(&lobby_mutex);
        lobby_espera[lobby_n++] = yo->id;
        pthread_mutex_unlock(&lobby_mutex);

        // Intentar buscar pareja
        int rival_id = -1;
        while (rival_id == -1) {
            rival_id = buscar_rival(yo);
            if (rival_id == -1) usleep(100000); // espera 0.1s para no saturar la CPU
        }
        
        // PARTIDA
        
        // 1. avisar al rival que fue elegido
        Jugador *rival = &jugadores[rival_id];
        pthread_mutex_lock(&rival->lock);
        pthread_cond_signal(&rival->cond_ready); 
        pthread_mutex_unlock(&rival->lock);

        // 2. el jugador 1 (por id menor) creará la mesa
        if (yo->id < rival_id) {

        // ya hay rival, hay que buscar un tablero libre
        int tablero_idx = -1;
        for(int i = 0; i < config.K_BOARDS; i++) {
            pthread_mutex_lock(&tableros[i].lock);
            if(tableros[i].active == false) {
                tableros[i].active = true;
                tableros[i].player1_id = yo->id;
                tableros[i].player2_id = rival_id;
                tablero_idx = i;
                pthread_mutex_unlock(&tableros[i].lock);
                break; 
            }
            pthread_mutex_unlock(&tableros[i].lock);
        }

        // si encontró tablero, iniciar partidaa
        if(tablero_idx != -1) {
            Partida *p = malloc(sizeof(Partida));
            p->p1 = yo;
            p->p2 = &jugadores[rival_id];
            p->board = &tableros[tablero_idx];
            
            //se crea el hilo del tablero.
            pthread_create(&tableros[tablero_idx].thread, NULL, tablero_thread, (void*)p);
            pthread_join(tableros[tablero_idx].thread, NULL);
            free(p); 
        }

        } else {
            // el jugador 2 se queda esperando a que el jugador 1 le avise que el tablero está listo.
            pthread_mutex_lock(&yo->lock);
            pthread_cond_wait(&yo->cond_ready, &yo->lock);
            pthread_mutex_unlock(&yo->lock);
        }

        // desición autónoma de retirarse después de cada partida.
        float probabilidad = (float)rand() / (float)RAND_MAX;
        if (probabilidad > config.REENTER_PROBABILITY) {
            printf("Jugador %d decidió retirarse.\n", yo->id);
            break; // Sale del while y el hilo termina
        }
        
    }
    return NULL;
}