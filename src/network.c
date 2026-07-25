#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
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

// Generates a net container and populates its vector with network statistics
net_v_t* net_sample(void){
    net_t* array = (net_t*) malloc(sizeof(net_t)*MAX_IFACES);
    if (array == NULL){
        perror("Error: failed to allocate memory");
        exit(EXIT_FAILURE);
    }

    net_v_t* net_container = (net_v_t*) malloc(sizeof(net_v_t));
    if (net_container == NULL){
        perror("Error: failed to allocate memory");
        exit(EXIT_FAILURE);
    }
    net_container->count = 0; // Inizializes the count of stored interface information
    net_container->vector = array;
    
    // Populates the array with retrieved interfaces
    retrieve_netstat(net_container);
    
    // Reallocates the array to the exact size; if realloc fails, keeps the original pointer
    net_t *v = realloc(array, net_container->count * sizeof(net_t));
    if (v != NULL) net_container->vector = v;
    else net_container->vector = array;

    return net_container;
}

// Computes received and transimtted bytes
computed_net_bytes_t* compute_used_bandwidth(net_v_t* net_sample1, net_v_t* net_sample2){
    if (net_sample1 == NULL || net_sample2 == NULL){
        perror("Error: invalid memory");
        exit(EXIT_FAILURE);
    }

    computed_net_bytes_t* computed_bytes = (computed_net_bytes_t*) malloc(sizeof(computed_net_bytes_t));
    if (computed_bytes == NULL){
        perror("Error: failed to allocate dinamic memory");
        exit(EXIT_FAILURE);
    }
    computed_bytes->tot_received = 0;
    computed_bytes->tot_sent = 0;
    computed_bytes->received_per_sec = 0.0;
    computed_bytes->sent_per_sec = 0.0;

    size_t max = 0;
    if (net_sample1->count <= net_sample2->count) max = net_sample1->count;
    else max = net_sample2->count;

    unsigned long long tot_received_sample1 = 0, tot_received_sample2 = 0;
    unsigned long long tot_sent_sample1 = 0, tot_sent_sample2 = 0;

    for(size_t i = 0; i < max; i++){
        tot_received_sample1 += net_sample1->vector[i].rx_bytes;
        tot_received_sample2 += net_sample2->vector[i].rx_bytes;
        tot_sent_sample1 += net_sample1->vector[i].tx_bytes;
        tot_sent_sample2 += net_sample2->vector[i].tx_bytes; 
    }
    computed_bytes->tot_received = tot_received_sample1 + tot_received_sample2;
    computed_bytes->tot_sent = tot_sent_sample1 + tot_sent_sample2;
    computed_bytes->received_per_sec = (tot_received_sample2 - tot_received_sample1) / 3.0;
    computed_bytes->sent_per_sec = (tot_sent_sample2 - tot_sent_sample1) / 3.0;

    return computed_bytes;
}

void print_net_bytes(computed_net_bytes_t* bytes){
    if (bytes == NULL){
        perror("Error: invalid memory");
        exit(EXIT_FAILURE);
    }
    printf("Bytes received: %llu bytes/s: %.1f\nBytes sent: %llu bytes/s: %.1f\n",
        bytes->tot_received,
        bytes->received_per_sec,
        bytes->tot_sent,
        bytes->sent_per_sec
    );
}

void get_used_bandwidth(void){

    net_v_t* net_sample1 = net_sample();
    print_net_vector(net_sample1);

    sleep(3);

    net_v_t* net_sample2 = net_sample();
    print_net_vector(net_sample2);

    computed_net_bytes_t *bytes = compute_used_bandwidth(net_sample1, net_sample2);
    print_net_bytes(bytes);
    
    free(net_sample1->vector);
    free(net_sample2->vector);
    free(net_sample1);
    free(net_sample2);
}