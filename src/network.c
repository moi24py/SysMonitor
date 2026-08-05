#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "sysmonitor.h"

// Retrives received and transimetted bytes
void retrieve_netstat(net_v_t *retrieved){
    FILE *fp = fopen("/proc/net/dev", "r");
    if (fp == NULL){
        perror("retrieve_netstat: failed to open /proc/net/dev");
        exit(EXIT_FAILURE);
    }

    char line[512];
    // Skip header lines (first line + the second header line)
    for (size_t skip = 1; skip <= 2; skip++){
        if (!fgets(line, sizeof(line), fp)) {
            perror("retrieve_netstat: fgets failed");
            fclose(fp);
            exit(EXIT_FAILURE);
        }
    }

    // Parses the file and populates the array with each interface name, received bytes, and transmitted bytes
    int parsed = 0;
    while ( fgets(line, sizeof(line), fp) != NULL ) {
        if (retrieved->count >= MAX_IFACES) break; // Handle interface overflow beyond MAX_IFACES
        parsed = sscanf(line, "%63[^:]: %llu %*s %*s %*s %*s %*s %*s %*s %llu %*s %*s %*s %*s %*s %*s %*s",
            retrieved->vector[retrieved->count].iface,
            &retrieved->vector[retrieved->count].rx_bytes,
            &retrieved->vector[retrieved->count].tx_bytes
        );
        if ( parsed == 3 ) retrieved->count++;
    }
    
    // Checks potential causes of read failure
    if (ferror(fp)) {
        perror("retrieve_netstat: fgets/read error");
        fclose(fp);
        return;
    }

	// Closes the file and handles errors, if they occur
    if (fclose(fp) != 0) {
        perror("retrieve_netstat: fclose failed");
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
        perror("net_sample: malloc failed");
        exit(EXIT_FAILURE);
    }

    net_v_t *net_container = malloc(sizeof(net_v_t));
    if (net_container == NULL){
        perror("net_sample: malloc net_v_t failed");
        free(array);
        exit(EXIT_FAILURE);
    }
    net_container->count = 0; // Inizializes the count of stored interface information
    net_container->vector = array;
    
    // Populates the array with retrieved interfaces
    retrieve_netstat(net_container);
    
    // Reallocates the array to the exact size; if realloc fails, keeps the original pointer
    // If count is 0, just keep the original array
    if (net_container->count > 0) {
        net_t *v = realloc(array, net_container->count * sizeof(net_t));
        if (v != NULL) net_container->vector = v;
        else net_container->vector = array;
    }

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

// Prints out the computed received and transmitted bytes
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

// Deallocates the dynamically allocated memory
void free_net(net_v_t* s1){
    if (!s1) return;
    free(s1->vector);
    free(s1);
}

// Retrieves and prints network used bandwidth
void get_used_bandwidth(void){

    printf("NETWORK: USED BANDWIDTH\n");
    net_v_t* net_sample1 = net_sample();
    print_net_vector(net_sample1);

    sleep(3);

    net_v_t* net_sample2 = net_sample();
    print_net_vector(net_sample2);

    computed_net_bytes_t *bytes = compute_used_bandwidth(net_sample1, net_sample2);
    print_net_bytes(bytes);

    free_net(net_sample1);
}