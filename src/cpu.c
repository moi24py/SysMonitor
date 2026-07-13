#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../include/sysmonitor.h"

// Estrapolazione dati rilevazioni CPU
int parse_stats(cpu_stat_t *cpu_stat){
    // Apertura file e gestione eventuale fallimento
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL){
        perror("Error: Failed to open file /proc/stat [ cpu.c parse_stats() ]");
        exit(EXIT_FAILURE);
    }

    // Lettura dal file ed estrapolazione rilevazioni CPU
    char line[512];
    int read = 0;
    size_t cpus = 0;
    while (fgets(line, sizeof(line), fp) != NULL) {
        if(strncmp(line, "cpu", 3) == 0){
            read = sscanf( line, "%*s %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu",
                    &(cpu_stat[cpus].user),
                    &(cpu_stat[cpus].nice),
                    &(cpu_stat[cpus].system),
                    &(cpu_stat[cpus].idle),
                    &(cpu_stat[cpus].iowait),
                    &(cpu_stat[cpus].irq),
                    &(cpu_stat[cpus].softirq),
                    &(cpu_stat[cpus].steal),
                    &(cpu_stat[cpus].guest),
                    &(cpu_stat[cpus].guest_nice) );
                    //print_stats(&cpu_stat[cpus]);
            if (read < 10){
                fprintf(stderr, "Error: Failed to parse file /proc/stat [/cpu.c parse_stats()]\n");
                exit(EXIT_FAILURE);
            }
            cpus++;
        }
    }

    // Testa la causa del fallimento della lettura
    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return EXIT_FAILURE;
    }

	// Chiusura del file e gestione dell'eventuale fallimento
    if (fclose(fp) != 0) {
        perror("fclose");
        return EXIT_FAILURE;
    }
    return cpus;
}

// Stampa statistiche
void print_stats(cpu_stat_t* stats){
    printf("cpu %llu %llu %llu %llu %llu %llu %llu %llu %llu %llu\n",
        stats->user, stats->nice, stats->system, stats->idle, stats->iowait,
        stats->irq, stats->softirq, stats->steal, stats->guest, stats->guest_nice);
}

// Calcolo tempi: totale e idle
cpu_stat_tot_idle_t* sum_total(cpu_stat_t *cpu_stats){
    cpu_stat_tot_idle_t *sums = (cpu_stat_tot_idle_t*) malloc(sizeof(cpu_stat_tot_idle_t));
    if (sums==NULL){
         perror("Error: not enough memory to allocate stat_tot_idle [cpu.c sum_total()]");
         exit(EXIT_FAILURE);
    }

    sums->total = 
            cpu_stats[0].user + cpu_stats[0].nice + cpu_stats[0].system + cpu_stats[0].idle +
            cpu_stats[0].iowait + cpu_stats[0].irq + cpu_stats[0].softirq + cpu_stats[0].steal +
            cpu_stats[0].guest + cpu_stats[0].guest_nice;

    sums->idle = cpu_stats[0].idle + cpu_stats[0].iowait;
    return sums;
}

// Calcolo utilizzo totale CPU (tutti i core)
// Valore di ritorno: tempo non idle tra due rilevazioni, espresso in percentuale
double total_CPU_time(cpu_stat_tot_idle_t* sum1, cpu_stat_tot_idle_t* sum2){
    double idles = (double)sum1->idle - (double)sum2->idle;
    double totals = (double)sum1->total - (double)sum2->total;
    double usage = 100.0 * (1.0 - idles / totals);
    return usage;
}

// Funzione che estrapola i dati di rilevazione dell'utilizzo della CPU,
// calcola il tempo non idle tra due rilevazioni, e lo stampa espresso in percentuale 
void get_CPU_usage(){
    // Array per le rilevazioni iniziali: CPU totale e ciacun core
    cpu_stat_t stats1[MAX_CPUS];
    // Lettura dati rilevazioni iniziali
    int cpus_qty = parse_stats(stats1);

    sleep(1);

    // Array per le rilevazioni finali: CPU totale e ciacun core
    cpu_stat_t stats2[MAX_CPUS];
    // Lettura dati rilevazioni finali
    parse_stats(stats2);

    // Calcolo tempo di utilizzo CPU totale
    int cpu_n = 0;
    double usage = 0;
    while(cpu_n < cpus_qty){
       usage = total_CPU_time(sum_total(stats1+cpu_n), sum_total(stats2+cpu_n));
       printf("L'utilizzo CPU%d è: %.1f%%\n", cpu_n, usage);
       cpu_n++;
    }
}