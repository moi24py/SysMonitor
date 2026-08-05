#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <ctype.h>
#include <stdint.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <limits.h>

#include "sysmonitor.h"


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
void clear_stdin_line(void) {
    int c;
    while ((c = getchar()) != '\n' && c != EOF) { }
}
// Prompts the user to decide whether to include pseudo-filesystems
bool want_overlay(void) {
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
disks_container* retrieve_fs(void){
    FILE *fp = fopen("/proc/mounts", "r");
    if (fp == NULL){
        perror("Error: failed to open /proc/mounts");
        exit(EXIT_FAILURE);
    }

    // Struct that stores filesystems name and mount point 
    disk_t* fsys = calloc(MAX_FS, sizeof(disk_t));

    if (fsys == NULL){
        perror("Error: failed to allocate dinamic memory");
        exit(EXIT_FAILURE);
    }
    char line[1024];
    int fsys_qty = 0;
    disk_t* base = fsys;
    // Retrieve filesystems type and mount point
    while( fgets(line, sizeof(line), fp) != NULL && fsys_qty < MAX_FS){
        // Filters out mounts with virtual filesystem fstype (e.g., procfs/sysfs/tmpfs)
        // so the disk report focuses on real storage. Optionally includes virtual filesystems
        int n = sscanf(line, "%*s %s %s",
            (base+fsys_qty)->mount,
            (base+fsys_qty)->fstype);
        if (n != 2) continue;
        (base+fsys_qty)->pseudo = is_pseudo_fstype((base+fsys_qty)->fstype);
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
    
    disks_container *disks = (disks_container*) malloc(sizeof(disks_container));
    if (disks == NULL){
        perror("Error: failed to allocate dinamic memory");
        exit(EXIT_FAILURE);
    }
    disks->v = fsys;
    disks->size = fsys_qty;

    return disks;
}


// Checks if the filesystem type is virtual
bool is_pseudo_fstype(const char *fsname){
    for(size_t i = 0; i < size_ignored_fstype; i++){
        if (strcmp(fsname, pseudo_fstype[i]) == 0) return true;
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
void human_readable(unsigned long long bytes, char* out, size_t out_sz) {
    const char* units[] = {"B", "KiB", "MiB", "GiB", "TiB", "PiB"};
    double human = (double)bytes;

    int i = 0;
    int max_i = (int)(sizeof(units)/sizeof(units[0])) - 1;

    while (human >= 1024.0 && i < max_i) {
        human /= 1024.0;
        i++;
    }

    if (i == 0) snprintf(out, out_sz, "%llu %s", bytes, units[i]);
    else        snprintf(out, out_sz, "%.1f %s", human, units[i]);
}

// Prints a filesystem total space, free space, used space
void print_fs_space(disk_t *fs, struct statvfs *stats, size_t i){
    if (statvfs((fs)->mount, stats) != 0)
        printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n", i+1, "N/A", "N/A", (fs)->mount, "N/A", "N/A", "N/A");
    else{
        compute_fs_space(fs+i, stats);
        char buf1[32];
        char buf2[32];
        char buf3[32];
        human_readable((fs)->total_space, buf1, sizeof(buf1));
        human_readable((fs)->free_space, buf2, sizeof(buf2));
        human_readable((fs)->used_space, buf3, sizeof(buf3));
        printf("%3ld | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n",
            i+1,
            (fs)->fstype,
            (fs)->pseudo ? "yes" : "no",
            (fs)->mount,
            buf1, buf2, buf3);
    }              
}


// Prints an array of disk_t struct that stores filesystem data
void print_fs_stats(disks_container *fs, int overlay){
    struct statvfs stats;
    printf("\n%3s | %-30s | %-5s | %-60s | %-10s | %-10s | %-10s\n", "#", "FSTYPE", "PSEUDO", "MOUNT POINT", "TOTAL", "FREE", "USED");
    printf("%3s---%30s---%5s--%40s---%10s---%10s---%10s\n",
        "---", "------------------------------", "----------",
        "------------------------------------------------------------", "----------", "----------", "----------");
    for(size_t i=0; i < fs->size; i++){
        char s = ((fs->v)+i)->fstype[0];
        if (s != '\0' &&  isalpha(s)){
            // Shows non-pseudo filesystems only
            if (overlay == 0 && ((fs->v)+i)->pseudo == false){
                print_fs_space((fs->v)+i, &stats, i);
            } // Shows all filesystems
            else if (overlay == 1){
                print_fs_space((fs->v)+i, &stats, i);
            }
        }
    }
}

// Avoids overflow in 64 bits expressions
static inline unsigned long long u64(unsigned long n) {
    return (unsigned long long)n;
}

// Iterates every mountpoint in fsv and filters out pseudo-filesystems, fstype that not corresponds to target_fstype, fs failed by statvfs
// Picks the filesystem with the greatest capacity (total)
// Returns: index of the "best" mount, or -1 if not found
int pick_best_mount_by_fstype(const disks_container *fsv, const char *target_fstype) {
    int best = -1;
    unsigned long long best_total = 0; // current best filesystem with max capacity

    for (size_t i = 0; i < fsv->size; i++) {
        if (fsv->v[i].pseudo) continue; // skips pseudo filesystems

        const char *fsname = fsv->v[i].fstype;

        // matching filesystem type
        bool match = false;
        if (strcmp(target_fstype, "fuse.vr") == 0) { // if target is "fuse.vr" every fsname that starts with "fuse." is valid
            match = (strncmp(fsname, "fuse.", 5) == 0); // fuse.* => fuse.vr
        } else {
            match = (strcmp(fsname, target_fstype) == 0); // otherwise exact match
        }

        if (!match) continue;

        // Reads capacity
        struct statvfs st;
        if (statvfs(fsv->v[i].mount, &st) != 0) continue;

        // Chooses block size
        unsigned long long frsize = (unsigned long long)st.f_frsize; // fragment size
        if (frsize == 0) frsize = (unsigned long long)st.f_bsize;

        // Computes total: blocks quanity * fragment size
        unsigned long long total = (unsigned long long)st.f_blocks * frsize;

        // Picks best mount
        if (best == -1 || total > best_total) {
            best = (int)i;
            best_total = total;
        }
    }
    return best;
}

// Chooses the best mounts thanks to pick_best_mount_by_fstype() and computes total space, free space and used space
disk_choice compute_choice_for_fstype(const disks_container *fsv, const char *target_fstype) {
    disk_choice out;
    out.space = (disk_space){0,0,0};
    out.mount_point = NULL;

    int idx = pick_best_mount_by_fstype(fsv, target_fstype);
    if (idx < 0) return out;

    out.mount_point = fsv->v[idx].mount;

    struct statvfs st;
    if (statvfs(fsv->v[idx].mount, &st) != 0) return out;

    unsigned long long frsize = (unsigned long long)st.f_frsize;
    if (frsize == 0) frsize = (unsigned long long)st.f_bsize;

    unsigned long long total = (unsigned long long)st.f_blocks * frsize;
    unsigned long long free  = (unsigned long long)st.f_bavail * frsize;
    unsigned long long used  = (total >= free) ? (total - free) : 0;

    out.space.total = total;
    out.space.free  = free;
    out.space.used  = used;
    return out;
}
