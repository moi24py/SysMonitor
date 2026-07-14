#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../include/sysmonitor.h"

int parse_mem(mem_stat_t *mem){
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/meminfo [mem.c parse_mem()]");
        exit(EXIT_FAILURE);
    }

    char line[512];
    int read = 0;
    while( fgets(line, sizeof(line), fp) != NULL) {
        if (strncmp(line, "MemTotal", 8) == 0)
            read = sscanf(line, "%*s %llu %*s", &(mem->mem_total));
        else if(strncmp(line, "MemFree", 7) == 0)
            read = sscanf(line, "%*s %llu %*s", &(mem->mem_free));
        else if (strncmp(line, "MemAvailable", 12) == 0)
            read = sscanf(line, "%*s %llu %*s", &(mem->mem_available));
        
        if (read < 1){
            perror("Error: parsing failed [mem.c parse_mem()]");
            exit(EXIT_FAILURE);
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

    return EXIT_SUCCESS;
}