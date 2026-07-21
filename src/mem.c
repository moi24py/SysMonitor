#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include "../include/sysmonitor.h"

// Retrieves memory statistics
int parse_mem(mem_stat_t *mem){
    FILE *fp = fopen("/proc/meminfo", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/meminfo [mem.c parse_mem()]");
        exit(EXIT_FAILURE);
    }
    // Reads and retrieves memory statistics
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

    // Checks potential causes of read failure
    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return EXIT_FAILURE;
    }
	// Closes the file and handles errors, if they occur
    if (fclose(fp) != 0) {
        perror("fclose");
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}

// Prints memory statistics
void print_mem(mem_stat_t *mem){
    printf("Total memory: %12llu KB\n", mem->mem_total);
    printf("Free memory: %13llu KB\n", mem->mem_free);
    printf("Available memory: %8llu KB\n", mem->mem_available);
}

// Retrieves memory statistics and prints them
void get_memory_usage(void){
    mem_stat_t mem;
    parse_mem(&mem);
    print_mem(&mem);
}