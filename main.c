#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include "tipos.h"
#include <pthread.h>
#include <stdbool.h>

Config config;

//para abrir el archivo de configuración y leerlo, asignando las variables.

//para abrirlo
void load_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (!file) {
        perror("error al abrir archivo de configuración");
        exit(EXIT_FAILURE);
    }

    char line[256];

    // para leerlo y asignar. 
    // fgets lee hasta encontrar un salto de línea o el fin del archivo

    while (fgets(line, sizeof(line), file)) {
        
        // 1. saltar comentarios (#) o líneas vacías
        if (line[0] == '#' || line[0] == '\n' || line[0] == '\r') {
            continue;
        }

        char key[64];
        char value[128];

        // 2. sscanf extrae los datos del string que se leyeron con fgets
        //  "%63[^=]=%127s" busca la parte antes del '=' y la parte después
        if (sscanf(line, "%63[^=]=%127s", key, value) == 2) {
            
            // 3. asignación de valores
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

int main() {

    load_config("config.env");

    printf("%d\n", config.N_PLAYERS);
    printf("%d\n", config.K_BOARDS);
    printf("%d\n", config.K_ELO);
    printf("%d\n", config.MAX_ELO_DIFF);
    printf("%d\n", config.TURN_DELAY_MS);
    printf("%f\n", config.REENTER_PROBABILITY);
    printf("%s\n", config.SNAPSHOT_PATH);

    return 0;
}

/*fgets y sscanf en vez de uno solo para complementar. 
fgets sirve para ignorar comentarios y líneas vacías, y 
sscanf mira esa línea específica ya limpia y extrae la información (clave y valor).
*/

/*Jugador *jugadores = malloc(sizeof(Jugador) * config.N_PLAYERS);
Tablero *tableros = malloc(sizeof(Tablero) * config.K_BOARDS);*/