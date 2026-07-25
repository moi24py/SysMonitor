#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../include/sysmonitor.h"

int main(void){

    int rep = 3;
    while(rep--){
        get_proc();
        printf("\n");
    
        get_disk_stats(false);
        printf("\n");
    
        get_CPU_usage();
        printf("\n");

        get_memory_usage();
        printf("\n");
        
        get_used_bandwidth();
        printf("\n");

        if (rep > 1) sleep(3);
    }
    return EXIT_SUCCESS;
}