#include <stdio.h>
#include <stdlib.h>
#include "../include/sysmonitor.h"

// Retrives received and transimetted bytes
void retrieve_netstat(net_v_t *retrieved_v){
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    char line[512];
    // Skips header and first line
    for (size_t skip = 1; skip <= 2; skip++){
        if (!fgets(line, sizeof(line), fp)) {
            perror("fgets failed");
            fclose(fp);
            return;
        }
    }

    // Parses the file and populates the array with each interface name, received bytes, and transmitted bytes
    int parsed = 0;
    while ( fgets(line, sizeof(line), fp) != NULL ) {
        if (retrieved_v->count >= MAX_IFACES) break; // Handle interface overflow beyond MAX_IFACES
        parsed = sscanf(line, "%63[^:]: %llu %*s %*s %*s %*s %*s %*s %*s %llu %*s %*s %*s %*s %*s %*s %*s",
            retrieved_v->vector[retrieved_v->count].iface,
            &retrieved_v->vector[retrieved_v->count].rx_bytes,
            &retrieved_v->vector[retrieved_v->count].tx_bytes
        );
        if ( parsed == 3 ) retrieved_v->count++;
    }
    
    // Checks potential causes of read failure
    if (ferror(fp)) {
        perror("fgets");
        fclose(fp);
        return;
    }

	// Closes the file and handles errors, if they occur
    if (fclose(fp) != 0) {
        perror("fclose");
        return;
    }
}

void print_net_vector(net_v_t *n){
    for(int i = 0; i < n->count ; i++)
        printf("%s:     rx_bytes: %llu       tx_bytes: %llu\n",
            n->vector[i].iface,
            n->vector[i].rx_bytes,
            n->vector[i].tx_bytes
        );
}

// TODO: COMPUTE RECEVIED AND TRANSMITTED BYTES

void get_used_bandwidth(void){
    net_t* array = (net_t*) malloc(sizeof(net_t)*MAX_IFACES);
    if (array == NULL){
        perror("Error: failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    net_v_t net_container;
    net_container.count = 0; // Inizializes the count of stored interface information
    net_container.vector = array;
    
    // Populates the array with retrieved interfaces
    retrieve_netstat(&net_container);
    
    // Reallocates the array to the exact size; if realloc fails, keeps the original pointer
    net_t *v = realloc(array, net_container.count * sizeof(net_t));
    if (v != NULL) net_container.vector = v;
    else net_container.vector = array;

    print_net_vector(&net_container);
    
    free(net_container.vector);
}