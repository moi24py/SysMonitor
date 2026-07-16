#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include "../include/sysmonitor.h"

// Array that lists pseudo-filesystem types
const char *pseudo_fstype[] = {
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
bool is_pseudo_fstype(char *fsname){
    size_t i;
    bool is_ign = false;
    for(i=0; i<size_ignored_fstype; i++){
        if ( strncmp(fsname, pseudo_fstype[i], strlen(fsname)) == 0 ){
            //printf("UGUALI: [fsname: %s] [ignored_fstype: %s]\n\n", fsname, ignored_fstype[i]);
            return true;
        }
    }
    return false;
}

void print_fs_array(disk_t *fs){
    printf("        FSTYPE                   PSEUDO      MOUNT POINT\n");
    for(size_t i=0; i<MAX_FS; i++){
        char s = (fs+i)->fstype[0];
        if (s != '\0' &&  isalpha(s))
            printf("#%-3ld    %-20s     %d           %s\n", i+1, (fs+i)->fstype, (fs+i)->pseudo, (fs+i)->mount);
    }
}

// Retrieves mounted filesystems (pseudo-fs excluded)
int retrieve_fs(){
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    // Struct that stores filesystems name and mount point 
    disk_t fsys[MAX_FS];

    char line[1024];
    int fsys_qty = 0;
    int num = 1;
    // Retrieve filesystems type and mount point
    while( fgets(line, sizeof(line), fp) != NULL ){
        // Filters out mounts with virtual filesystem fstype (e.g., procfs/sysfs/tmpfs)
        // so the disk report focuses on real storage. Optionally includes virtual filesystems
        sscanf(line, "%*s %s %s", fsys[fsys_qty].mount, fsys[fsys_qty].fstype);
        //printf("%2d ] fstype: %s\n", num++, fsys[fsys_qty].fstype);
        //printf("     mount point:%s\n", fsys[fsys_qty].mount);
        if (is_pseudo_fstype(fsys[fsys_qty].fstype)) fsys[fsys_qty].pseudo = true;
        else fsys[fsys_qty].pseudo = false;
        fsys_qty++;
    }

    print_fs_array(fsys);

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