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

// Calcolo tempi: totale e idle
stat_tot_idle_t* sum_total(stat_t *stats){
    stat_tot_idle_t *sums = (stat_tot_idle_t*) malloc(sizeof(stat_tot_idle_t));
    if (sums==NULL){
         perror("Impossibile allocare in memoria per stat_tot_idle");
         exit(EXIT_FAILURE);
    }

    sums->total = 
            stats->user + stats->nice + stats->system + stats->idle +
            stats->iowait + stats->irq + stats->softirq + stats->steal +
            stats->guest + stats->guest_nice;

    sums->idle = stats->idle + stats->iowait;
    return sums;
}

// Calcolo utilizzo totale CPU 
// tempo non idle tra due rilevazioni, espresso in percentuale
double total_CPU_time(stat_tot_idle_t* sum1, stat_tot_idle_t* sum2){
    double idles = (double)sum1->idle - (double)sum2->idle;
    double totals = (double)sum1->total - (double)sum2->total;
    double usage = 100.0 * (1.0 - idles / totals);
    return usage;
}