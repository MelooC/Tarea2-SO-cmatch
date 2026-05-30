#include "tipos.h"
#include <signal.h>

Config config;  // definición real (en tipos.h va como extern)

// variable global
volatile bool keep_running = true;

// se ejecuta cuando se presiona Ctrl+C
void handle_sigint(int sig) {
    (void)sig; // para evitar warning de compilación
    keep_running = false; 
    printf("\n[Sistema] Terminando...\n");
}

/* ---- lectura del archivo de configuración (.env) ---- */
void load_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("error al abrir archivo de configuración");
        exit(EXIT_FAILURE);
    }

    char line[256];

    while (fgets(line, sizeof(line), file)) {

        // saltar comentarios (#) o líneas vacías
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        char key[64];
        char value[128];

        if (sscanf(line, "%63[^=]=%127s", key, value) == 2) {
            if (strcmp(key, "N_PLAYERS") == 0) config.N_PLAYERS = atoi(value);
            else if (strcmp(key, "K_BOARDS") == 0) config.K_BOARDS = atoi(value);
            else if (strcmp(key, "K_ELO") == 0) config.K_ELO = atoi(value);
            else if (strcmp(key, "MAX_ELO_DIFF") == 0) config.MAX_ELO_DIFF = atoi(value);
            else if (strcmp(key, "TURN_DELAY_MS") == 0) config.TURN_DELAY_MS = atoi(value);
            else if (strcmp(key, "REENTER_PROBABILITY") == 0) config.REENTER_PROBABILITY = (float)atof(value);
            else if (strcmp(key, "SNAPSHOT_PATH") == 0) strcpy(config.SNAPSHOT_PATH, value);
        }
    }

    fclose(file);
}

int main(void) {

    //registrar la señal
    signal(SIGINT, handle_sigint);

    srand(time(NULL));            // semilla para aleatoriedad (ELO inicial, jugadas, etc.)
    load_config("config.env");

    // reservar memoria para jugadores, tableros y la cola del lobby
    jugadores    = malloc(sizeof(Jugador) * config.N_PLAYERS);
    tableros     = malloc(sizeof(Tablero) * config.K_BOARDS);
    lobby_espera = malloc(sizeof(int)     * config.N_PLAYERS);
    if (!jugadores || !tableros || !lobby_espera) {
        perror("malloc");
        exit(EXIT_FAILURE);
    }

    // inicializar cada jugador
    for (int i = 0; i < config.N_PLAYERS; i++) {
        jugadores[i].id        = i;
        jugadores[i].elo       = 400 + rand() % 401;  // ELO inicial 400-800 (decisión de diseño)
        jugadores[i].ganadas   = 0;
        jugadores[i].perdidas  = 0;
        jugadores[i].empatadas = 0;
        pthread_mutex_init(&jugadores[i].lock, NULL);
        pthread_cond_init(&jugadores[i].cond_ready, NULL);
    }

    // inicializar cada tablero
    for (int i = 0; i < config.K_BOARDS; i++) {
        tableros[i].id           = i;
        tableros[i].player1_id   = -1;
        tableros[i].player2_id   = -1;
        tableros[i].current_turn = -1;
        tableros[i].active       = false;
        pthread_mutex_init(&tableros[i].lock, NULL);
        pthread_cond_init(&tableros[i].cond_turn, NULL);
    }

    // crear los N threads de jugador
    for (int i = 0; i < config.N_PLAYERS; i++) {
        pthread_create(&jugadores[i].thread, NULL, jugador_thread, &jugadores[i]);
    }

    // esperar a que todos terminen
    for (int i = 0; i < config.N_PLAYERS; i++) {
        pthread_join(jugadores[i].thread, NULL);
    }

    // limpiar recursos
    for (int i = 0; i < config.N_PLAYERS; i++) {
        pthread_mutex_destroy(&jugadores[i].lock);
        pthread_cond_destroy(&jugadores[i].cond_ready);
    }
    for (int i = 0; i < config.K_BOARDS; i++) {
        pthread_mutex_destroy(&tableros[i].lock);
        pthread_cond_destroy(&tableros[i].cond_turn);
    }
    free(jugadores);
    free(tableros);
    free(lobby_espera);

    return 0;
}