#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "../include/sysmonitor.h"

int main(){

    // Apertura file statistiche di sistema /proc/stat
    FILE *fp = fopen("/proc/stat", "r");
    if (fp == NULL){
        perror("File cannot be open\n");
        exit(EXIT_FAILURE);
    }
    printf("File /proc/stat aperto.\n");

    // Prima rilevazione statistiche
    printf("Lettura statistiche iniziali...\n");
    stat_t* stat1 = get_stats(fp);
    print_stats(stat1);

    // Seconda rilevazione statistiche
    printf("Lettura statistiche finali...\n");
    sleep(10);
    stat_t* stat2 = get_stats(fp);
    print_stats(stat2);

    // TODO: calcolo tempo

    // Deallocazione memoria dinamica
    free(stat1);
    free(stat2);

    // Chiusura file /proc/stat
    if (fclose(fp) != 0){
        perror("Errore nella chiusura del file\n");
        return 1;
    }

    printf("File /proc/stat chiuso.\n");

    return 0;
}