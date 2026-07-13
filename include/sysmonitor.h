#include <stdio.h>
#ifndef _SYSMONITOR_H
#define _SYSMONITOR_H
#define MAX_CPUS 256
// Struttura che memorizza i valori dei rilevamenti della CPU
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
} cpu_stat_t;

// Struttura che memorizza 
typedef struct {
    unsigned long long total;
    unsigned long long idle;
} cpu_stat_tot_idle_t;


/* Funzione che estrapola dati rilevazioni CPU */
int parse_stats(cpu_stat_t *cpu_stat);

// Funzione che stampa l'oggetto statistiche
void print_stats(cpu_stat_t* stats);

// Calcolo tempi: totale e idle
cpu_stat_tot_idle_t* sum_total(cpu_stat_t *cpu_stats);

// Calcolo utilizzo totale CPU (tutti i core)
double total_CPU_time(cpu_stat_tot_idle_t* sum1, cpu_stat_tot_idle_t* sum2);

// Funzione che estrapola i dati delle rilevazioni CPU e calcola l'utilizzo in percentuale
void get_CPU_usage();

#endif