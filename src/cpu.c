#include <stdlib.h>
#include <stdio.h>
#include "../include/sysmonitor.h"

/* Funzione che legge la prima riga del file proc/stat,
 e ritorna un oggetto di tipo stat_t con i valori letti */
stat_t* get_stats(FILE* file){

    // Legge la prima riga del file
    char line[256];
    if (!fgets(line, sizeof(line), file)) return NULL;

    // Struttura dinamica per le statistiche del sistema
    stat_t* stats = (stat_t*) malloc(sizeof(stat_t));
    if (stats == NULL){
        perror("Impossibile allocare in memoria stats");
        exit(EXIT_FAILURE);
    }

    // Lettura dei 10 parametri di sistema
    int read = sscanf(line, "cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
        &stats->user, &stats->nice, &stats->system, &stats->idle, &stats->iowait,
        &stats->irq, &stats->softirq, &stats->steal, &stats->guest, &stats->guest_nice);
    if (read != 10){
        perror("Errore lettura dal file proc/stat");
        exit(EXIT_FAILURE);
    }

    // Riporta all'inizio il cusore del file
    if (fseek(file, 0, SEEK_SET) != 0) {
        perror("Errore fseek().");
        exit(EXIT_FAILURE);
    }

    return stats;
}

// Stampa statistiche
void print_stats(stat_t* stats){
    printf("cpu  %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
        stats->user, stats->nice, stats->system, stats->idle, stats->iowait,
        stats->irq, stats->softirq, stats->steal, stats->guest, stats->guest_nice);
}
