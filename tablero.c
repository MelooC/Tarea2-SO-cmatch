#define _DEFAULT_SOURCE
#include "tipos.h"
#include <unistd.h>
#include <math.h>


void *tablero_thread(void *arg) {
    Partida *p = (Partida *)arg;
    
    init_grid(p->board->grid);
    p->board->current_turn = p->p1->id; // empieza el jugador 1

    char simbolo = 'X'; // X para jugador 1, O para jugador 2

    while (check_winner(p->board->grid) == -1) { // mientras no haya ganador ni empate
        pthread_mutex_lock(&p->board->lock);
        
        // 1. para el movimiento aleatorio, coordenadas al azar.
        int r, c;
        do {
            r = rand() % 3;
            c = rand() % 3;
        } while (p->board->grid[r][c] != ' ');

        // 2. hacer el movimeinto en el tablero
        p->board->grid[r][c] = simbolo;
        printf("Tablero %d: Jugador %d marcó (%d,%d) con '%c'\n", p->board->id, p->board->current_turn, r, c, simbolo);
        
        // 3. imprimir tablero
        print_grid(p->board->grid);
        
        // 4. cambiar turno y símbolo
        if (p->board->current_turn == p->p1->id) {
            p->board->current_turn = p->p2->id;
            simbolo = 'O';
        } else {
            p->board->current_turn = p->p1->id;
            simbolo = 'X';
        }

        pthread_mutex_unlock(&p->board->lock);
        
        // 5. retardo pedido en la configuración
        usleep(config.TURN_DELAY_MS * 1000); 
    }

    // luego cuando termina. actualizar estadísticas y liberar tablero
    pthread_mutex_lock(&p->board->lock);
    
    //actualizar el ELO de p1 y p2
    actualizar_elo(p->p1, p->p2, check_winner(p->board->grid)); // resultado: 1 si gana p1, 0 si gana p2, 0.5 empate
    
    p->board->active = false; // liberar tablero para otros
    pthread_mutex_unlock(&p->board->lock);

    return NULL;
}


void actualizar_elo(Jugador *p1, Jugador *p2, int resultado_gato) {
    /*2.1. Cálculo y Actualización de ELO
    Al finalizar cada match, el ELO de los participantes debe actualizarse inmediatamente de forma
    consistente y segura. Se utilizará el sistema de puntuación Elo estándar:

    ELOnuevo = ELOanterior + KElo · (S − E)

    Donde:

    • KElo es el factor de ajuste del torneo, fijado en KElo = 32.
    • S es el resultado real del juego (1 para el ganador, 0 para el perdedor, 0.5 en caso de
    empate).
    • E es la expectativa de victoria del jugador, calculada mediante la función logística:
    EA = 1 / (1 + 10^( (1/400)·(ELOB − ELOA) ))
   */

   // 1. sacar S (puntuación real)
    double S1 = 0.5, S2 = 0.5;
    if (resultado_gato == 1) { // ganó P1
        S1 = 1.0; S2 = 0.0;
    } else if (resultado_gato == 2) { // ganó P2
        S1 = 0.0; S2 = 1.0;
    }
    // si es 0, se quedan en 0.5 (empate)

    // 2. expectativas E
    double E1 = 1.0 / (1.0 + pow(10.0, (double)(p2->elo - p1->elo) / 400.0));
    double E2 = 1.0 / (1.0 + pow(10.0, (double)(p1->elo - p2->elo) / 400.0));

    // 3. aplicar la formula y proteger con los mutex
    pthread_mutex_lock(&p1->lock);
    p1->elo = (int)(p1->elo + config.K_ELO * (S1 - E1));
    pthread_mutex_unlock(&p1->lock);

    pthread_mutex_lock(&p2->lock);
    p2->elo = (int)(p2->elo + config.K_ELO * (S2 - E2));
    pthread_mutex_unlock(&p2->lock);
}