#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <sys/statvfs.h>
#include <sys/types.h>
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


// Clears STDIN for want_overlay()
void clear_stdin_line() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}
// Prompts the user to decide whether to include pseudo-filesystems
bool want_overlay() {
    int max_attempts = 4;
    while (max_attempts--) {
        printf("Include pseudo-filesystems as well? (y/n): ");
        char reply[4];
        if (!fgets(reply, sizeof reply, stdin)) return false;
        if (strchr(reply, '\n') == NULL) clear_stdin_line(); // input greater than 4 chars
        if (reply[0] == 'y' || reply[0] == 'Y') return true;
        if (reply[0] == 'n' || reply[0] == 'N') return false;
    }
    return false;
}

// Retrieves mounted filesystems (pseudo-fs excluded)
disk_t* retrieve_fs(){
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    // Struct that stores filesystems name and mount point 
    disk_t* fsys = (disk_t*) malloc(sizeof(disk_t)*MAX_FS);
    char line[1024];
    int fsys_qty = 0;
    // Retrieve filesystems type and mount point
    while( fgets(line, sizeof(line), fp) != NULL ){
        // Filters out mounts with virtual filesystem fstype (e.g., procfs/sysfs/tmpfs)
        // so the disk report focuses on real storage. Optionally includes virtual filesystems
        sscanf(line, "%*s %s %s", (fsys+fsys_qty)->mount, (fsys+fsys_qty)->fstype);
        //printf("%2d ] fstype: %s\n", num++, fsys[fsys_qty].fstype);
        //printf("     mount point:%s\n", fsys[fsys_qty].mount);
        if (is_pseudo_fstype((fsys+fsys_qty)->fstype)) (fsys+fsys_qty)->pseudo = true;
        else (fsys+fsys_qty)->pseudo = false;
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

    return fsys;
}

// Checks if the filesystem type is virtual
bool is_pseudo_fstype(char *fsname){
    size_t i;
    for(i=0; i<size_ignored_fstype; i++){
        if ( strncmp(fsname, pseudo_fstype[i], strlen(fsname)) == 0 ){
            return true;
        }
    }
    return false;
}


// Computes a filesystem total, free and used space
void compute_fs_space(disk_t *fs, struct statvfs *stats){
    fs->total_space = stats->f_blocks * stats->f_frsize;
    fs->free_space = stats->f_bavail * stats->f_frsize;
    fs->used_space = fs->total_space - fs->free_space;
}

// Converts bytes to human readable bytes and prints the result
char* human_readable(const unsigned long long bytes){
    const char *units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    double human_read = (double)bytes;
    int i=0;
    int max_i = (int)(sizeof(units)/sizeof(units[0]))-1;

    while(human_read >= 1024.0 && i < max_i){
        human_read /= 1024.0;
        i++;
    }
    size_t human_bytes_s = 50;
    char *human_bytes = (char*) malloc(sizeof(char)*human_bytes_s);

    if (i==0) snprintf(human_bytes, human_bytes_s, "%llu %s", bytes, units[i]);
    else snprintf(human_bytes, human_bytes_s, "%.1f %s", human_read, units[i]);
    
    return human_bytes;
}

// Prints an array of disk_t struct that stores filesystem data
void print_fs_stats(disk_t *fs, int overlay){
    struct statvfs stats;
    printf("%3s | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n", "#", "FSTYPE", "PSEUDO", "MOUNT POINT", "TOTAL", "FREE", "USED");
    printf("%3s---%30s---%5s--%40s---%10s---%10s---%10s\n",
        "---", "------------------------------", "----------", "------------------------------------------------------------", "----------", "----------", "----------");
    for(size_t i=0; i<MAX_FS; i++){
        char s = (fs+i)->fstype[0];
        if (s != '\0' &&  isalpha(s)){
            // Shows non-pseudo filesystems only
            if (overlay == 0 && (fs+i)->pseudo == false){
                if (statvfs((fs+i)->mount, &stats) != 0) printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n", i+1, "N/A", "N/A", (fs+i)->mount, "N/A", "N/A", "N/A");
                else{
                    compute_fs_space(fs+i, &stats);
                    printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n",
                        i+1,
                        (fs+i)->fstype,
                        (fs+i)->pseudo ? "yes" : "no",
                        (fs+i)->mount,
                        human_readable((fs+i)->total_space),
                        human_readable((fs+i)->free_space),
                        human_readable((fs+i)->used_space)
                    );
                }
            } // Shows all filesystems
            else if (overlay == 1){
                if (statvfs((fs+i)->mount, &stats) != 0) printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n", i+1, "N/A", "N/A", (fs+i)->mount, "N/A", "N/A", "N/A");
                else{
                    compute_fs_space(fs+i, &stats);
                    printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n",
                        i+1,
                        (fs+i)->fstype,
                        (fs+i)->pseudo ? "yes" : "no",
                        (fs+i)->mount,
                        human_readable((fs+i)->total_space),
                        human_readable((fs+i)->free_space),
                        human_readable((fs+i)->used_space)
                    );
                }
            }
        }
    }
}


// Retrieves and prints disk stats
void get_disk_stats(){
    disk_t *fsys = retrieve_fs();
    bool overlay = want_overlay();
    print_fs_stats(fsys, overlay);
}