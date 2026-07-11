#include <stdio.h>
#ifndef _SYSMONITOR_H
#define _SYSMONITOR_H

// Struttura che memorizza i valori delle statistiche di sistema
typedef struct stat{
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
} stat_t;

typedef struct {
    unsigned long long total;
    unsigned long long idle;
} stat_tot_idle_t;


/* Funzione che legge la prima riga del file proc/stat,
 e ritorna un oggetto di tipo stat_t con i valori letti */
stat_t* get_stats(FILE *file);

// Funzione che stampa l'oggetto statistiche
void print_stats(stat_t* stats);

// Calcolo tempi: totale e idle
stat_tot_idle_t* sum_total(stat_t *stats);

// // Calcolo utilizzo totale CPU
double total_CPU_time(stat_tot_idle_t* sum1, stat_tot_idle_t* sum2);

#endif