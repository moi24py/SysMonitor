#include <stdio.h>
#include <stdlib.h>
#include "../include/sysmonitor.h"

int retrieve_netstat(net_t *retrieved){
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    char line[512];
    fgets(line, sizeof(line), fp); // skips header line
    fgets(line, sizeof(line), fp); // skips first line

    // Allocates memory for the interface name
    retrieved->iface = (char*) malloc(sizeof(char)*50);
    if (retrieved->iface == NULL){
        perror("Error: failed to allocate memory");
        exit(EXIT_FAILURE);
    }  
    int read = 0;
    while ( fgets(line, sizeof(line), fp) ) {
        read = sscanf(line, "%s %llu %*s %*s %*s %*s %*s %*s %*s %llu %*s %*s %*s %*s %*s %*s %*s", retrieved->iface, &retrieved->rx_bytes, &retrieved->tx_bytes);
        //printf("%s ]     rx_bytes: %llu       tx_bytes: %llu\n", retrieved->iface, retrieved->rx_bytes, retrieved->tx_bytes);
        }
    fclose(fp);
    return read;
}

