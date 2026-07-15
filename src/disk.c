#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/sysmonitor.h"

// Array that lists pseudo-filesystem types
const char *ignored_fstype[] = {
    "proc",
    "sysfs",
    "devpts",
    "devtmpfs",
    "tmpfs",
    "cgroup",
    "cgroup2",
    "pstore",
    "securityfs",
    "debugfs",
    "tracefs",
    "configfs",
    "rpc_pipefs",
    "fusectl",
    "mqueue"
};
size_t const size_ignored_fstype = 15;

// Checks if the filesystem type is virtual
bool is_ignored_fstype(char *fsname){
    size_t i;
    bool is_ign = false;
    for(i=0; i<size_ignored_fstype; i++){
        if ( strncmp(fsname, ignored_fstype[i], strlen(fsname)) == 0 ){
            printf("UGUALI: [fsname: %s] [ignored_fstype: %s]\n", fsname, ignored_fstype[i]);
            is_ign = true;
        }
        else {
            printf("diversi: [fsname: %s] [ignored_fstype: %s]\n", fsname, ignored_fstype[i]);
            is_ign = false;
        }
    }
    return is_ign;
}

// Retrieves mounted filesystems (pseudo-fs excluded)
int retrieve_fs(){
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    // Struct that stores filesystems name and mount point 
    disk_t fsys[100];
    char line[1024];
    int fsys_qty = 0;
    int num = 1;
    // Retrieve filesystems type and mount point
    while( fgets(line, sizeof(line), fp) != NULL ){
        // Filters out mounts with virtual filesystem fstype (e.g., procfs/sysfs/tmpfs)
        // so the disk report focuses on real storage. Optionally includes virtual filesystems
        sscanf(line, "%*s %s %s", fsys[fsys_qty].mount, fsys[fsys_qty].fstype);
        printf("%2d ] fstype: %s\n", num++, fsys[fsys_qty].fstype);
        printf("     mount point:%s\n", fsys[fsys_qty].mount);
        fsys[fsys_qty].fstype[strlen(fsys[fsys_qty].fstype)+1] = '\0';
        bool ign = is_ignored_fstype(fsys[fsys_qty].fstype);
        printf("%d\n", ign);
        fsys_qty++;
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
    printf("There are %d mounted filesystems\n", fsys_qty);

    return EXIT_SUCCESS;
}