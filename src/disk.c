#include <stdio.h>
#include <stdlib.h>
#include "../include/sysmonitor.h"

// Retrieves mounted filesystems (pseudo-fs excluded)
int retrieve_fs(){
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    int fs_qty = 0;
    char line[1024];
    while( fgets(line, sizeof(line), fp) != NULL ){
        printf("%s\n", line);
        fs_qty++;
    }

    if(ferror(fp)){
        perror("Error: failed to read /proc/mounts");
        fclose(fp);
        exit(EXIT_FAILURE);
    }
    if(fclose(fp) != 0){
        perror("Error: failed to close /proc/mounts");
        exit(EXIT_FAILURE);
    }
    printf("There are %d mounted filesystems\n", fs_qty);
    
    return EXIT_SUCCESS;
}