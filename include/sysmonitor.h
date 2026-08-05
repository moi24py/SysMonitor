#include <stdio.h>
#include <stdbool.h>
#include <sys/statvfs.h>
#include <sys/types.h>
#include <stddef.h>
#include <stdlib.h>

#ifndef _SYSMONITOR_H
#define _SYSMONITOR_H

// ############################## CPU ################################ 

#define MAX_CPUS 256

/**
 * @brief CPU counters parsed from /proc/stat.
 *
 * Values correspond to fields in /proc/stat (user, nice, system, idle, ...).
 */
typedef struct {
    unsigned long long user;
    unsigned long long nice;
    unsigned long long system;
    unsigned long long idle;
    unsigned long long iowait;
    unsigned long long irq;
    unsigned long long softirq;
    unsigned long long steal;
    unsigned long long guest;
    unsigned long long guest_nice;
} cpu_stat_t;

/**
 * @brief Aggregated CPU times for utilization computation.
 * 
 */
typedef struct {
    unsigned long long total; /**< Total time across all CPU states */
    unsigned long long idle;  /**< Idle time across CPU states */
} cpu_stat_tot_idle_t;

/**
 * @brief Parse CPU counters from /proc/stat.
 * @param cpu_stat Output structure to fill with counters.
 * @return Number of CPUs parsed, or 0 on failure.
 * @note unit: jiffies
 */
int parse_stats(cpu_stat_t *cpu_stat);

/**
 * @brief Compute aggregated total and idle times from one sample.
 * @param cpu_stats Sample counters (e.g., "current").
 * @return Allocated pointer containing total/idle values; NULL on failure.
 * @warning Caller owns returned memory and must free it.
 */
cpu_stat_tot_idle_t* sum_total(cpu_stat_t *cpu_stats);

/**
 * @brief Compute total CPU utilization between two aggregated samples.
 * @param sum1 First aggregated sample (baseline).
 * @param sum2 Second aggregated sample (later).
 * @return CPU usage as percentage in range [0, 100].
 */
double total_CPU_time(cpu_stat_tot_idle_t* sum1, cpu_stat_tot_idle_t* sum2);


// ############################ MEMORY ################################ 

/**
 * @brief Memory statistics in bytes
 * 
 * mem_total: total memory
 * mem_free: free memory (may differ from "available")
 * mem_available: memory available to start new applications
 */
typedef struct {
    unsigned long long mem_total;
    unsigned long long mem_free;
    unsigned long long mem_available;
} mem_stat_t;

/**
 * @brief Retrieve system memory statistics.
 * @param mem Output structure to fill.
 * @return EXIT_SUCCESS on success, EXIT_FAILURE on failure
 * @note unit: kB
 */
int parse_mem(mem_stat_t *mem);


// ############################ DISK ################################ 

#define MAX_FS 256

/**
 * @brief Filesystem entry (mount + accounting).
 * 
 * Values correspond to fields tipically found in /proc/mounts.
 */
typedef struct {
    char mount[512];                    /**< Mount point path */
    char fstype[100];                   /**< Filesystem type (e.g., ext4, fuse.sshfs, ... */
    bool pseudo;                        /**< True if this is a pseudo filesystem */
    unsigned long long total_space;     /**< Total space in bytes */
    unsigned long long free_space;      /**< Free space in bytes */
    unsigned long long used_space;      /**< Used space in bytes */
} disk_t;

/**
 * @brief Container for an array of filesystem entries
 * 
 */
typedef struct {
    disk_t *v;      /**< Allocated array */
    size_t size;    /**< Number of entries */
} disks_container;

/**
 * @brief Disk space snapshot in bytes.
 * 
 */
typedef struct {
    unsigned long long total;
    unsigned long long free;
    unsigned long long used;
} disk_space;

/**
 * @brief Selected filesystem choice: accounting + mount pointer.
 * 
 * mount_point points inside the mount string stored in fsv.
 * Valid only as long as fsv (and its underlying storage) remains valid.
 */
typedef struct {
    disk_space space;
    const char *mount_point;
} disk_choice;

/**
 * @brief Checks if the filesystem type is virtual/pseudo.
 * @param s Filesystem type string.
 * @return true if the filesystem is pseudo, false otherwise.
 */
bool is_pseudo_fstype(const char *s);

/**
 * @brief Retrieve mounted filesystems from /proc/mounts.
 *
 * The returned container includes both real and pseudo filesystems; pseudo
 * ones are marked through disk_t::pseudo.
 * 
 * Error handling:
 * - terminates the program with exit(EXIT_FAILURE) on allocation/read/open/close errors.
 *
 * @return disks_container* allocated by the function. Caller must free it.
 * @warning Caller owns it and must free it
 */
disks_container* retrieve_fs(void);

/**
 * @brief Convert bytes into a human readable string.
 * @param bytes Quantity in bytes.
 * @param out Output buffer.
 * @param out_sz Size of output buffer.
 */
void human_readable(unsigned long long bytes, char* out, size_t out_sz);

/**
 * @brief Compute total/free/used space for a filesystem using statvfs.
 * @param fs Filesystem entry to update.
 * @param stats Pointer to a struct statvfs filled by statvfs().
 */
void compute_fs_space(disk_t *fs, struct statvfs *stats);

/**
 * @brief Picks the filesystem with the greatest total capacity for target_fstype.
 *
 * @param fsv Array/container of mountpoints.
 * @param target_fstype Filesystem type to evaluate.
 * @return Index of the best mount, or -1 if not found.
 */
int pick_best_mount_by_fstype(const disks_container *fsv, const char *target_fstype);

/**
 * @brief Choose the best mountpoint matching a filesystem type and compute space stats.
 *
 * Selection policy:
 * - filters out pseudo filesystems
 * - matches fs type against target_fstype
 * - picks the mount with maximum total capacity
 *
 * @param fsv Filesystems container.
 * @param target_fstype Filesystem type to evaluate (e.g., "ext4", "fuse.vr").
 * @return disk_choice with space fields filled; mount_point is NULL if nothing matches.
 */
disk_choice compute_choice_for_fstype(const disks_container *fsv, const char *target_fstype);


// ############################ PROCESSES ################################

/**
 * @brief Process information parsed from /proc/[PID]/state (pid, name, state).
 */
typedef struct {
    pid_t pid;
    char name[256];
    char state[256];
} proc_t;

/**
 * @brief Container for an array of processes and their count.
 */
typedef struct {
    proc_t *ps;
    size_t qty;
} proc_v_t;


/**
 * @brief Scan /proc/<pid>/status for processes within the range [0, MAX_PROCS).
*
 * For each PID successfully read, extracts:
 * - Name
 * - State
 * - Pid
 *
 * @return proc_v_t* allocated container:
 * - psv->ps is an allocated array of proc_t
 * - psv->qty is the number of valid entries
 * - If no process if found: returns a container where psv->qty == 0 and psv->ps == NULL.
 * 
 * Error handling:
 * - terminates the program with exit(EXIT_FAILURE) on allocation failures.
 *
 * @warning Caller must free the returned container with free_processes().
 * @note If no process is found, returns a proc_v_t* with psv->ps == NULL and psv->qty == 0.
 */
proc_v_t* retrieve_processes(void);


/** 
 * @brief Free a proc_v_t container.
 *
 * Frees:
 * - psv->ps (array di proc_t)
 * - psv (il container)
 *
 * @param psv Container to free. Safe to call with NULL.
 */
void free_processes(proc_v_t *psv);


// ############################ NETWORK ################################

#define MAX_IFACES 20

/**
 * @brief nterface statistics: received and transmitted bytes.
 */
typedef struct {
    char iface[64];
    unsigned long long rx_bytes;
    unsigned long long tx_bytes;
} net_t;

/**
 * @brief Container for a vector of interfaces plus count.
 * 
 */
typedef struct {
    net_t *vector;  /**< Allocated array */
    int count;      /**< Number of elements */
} net_v_t;

/**
 * @brief Computed network bandwidth between two samples.
 */
typedef struct {
    unsigned long long tot_received;
    unsigned long long tot_sent;
    double received_per_sec;
    double sent_per_sec;
} computed_net_bytes_t;

/**
 * @brief Parse /proc/net/dev and populate a provided container with interfaces and counters.
 *
 * Precondition:
 * - retrieved != NULL
 * - retrieved->vector points to an allocated array of at least MAX_IFACES net_t elements
 * - retrieved->count is initialized to 0 by the caller (typically in net_sample()).
 *
 * Postcondition:
 * - retrieved->count is updated with the number of parsed interfaces.
 *
 * Error handling:
 * - On open/read/close errors, terminates with exit(EXIT_FAILURE) (consistent with project style).
 *
 * @param retrieved Output container to populate.
 * @return retrieved->count The number of parsed interfaces.
 * @note Interface names in /proc/net/dev can contain characters up to ':' delimiter.
 */
void retrieve_netstat(net_v_t *retrieved);

/**
 * @brief Compute received/transmitted bandwidth between two samples.
 * @param net_sample1 First sample.
 * @param net_sample2 Second sample.
 * @return Allocated computed stats; NULL on failure.
 * @warning Caller owns returned memory and must free it.
 */
computed_net_bytes_t* compute_used_bandwidth(net_v_t *net_sample1, net_v_t *net_sample2);

 /**
 * @brief Generate a net container and populate its vector with network statistics.
 *
 * Error handling:
 * - terminates the program with exit(EXIT_FAILURE) on allocation/read/open errors.
 * @returns an allocated net_v_t* and it owns an allocated net_t vector inside (net_container->vector)
 * @warning Caller must release both via free_net().
 */
net_v_t* net_sample(void);

/**
 * @brief Free a net_v_t container.
 *
 * Frees:
 * - s1->vector (array di net_t)
 * - s1 (il container)
 *
 * @param s1 Container da liberare. Safe to call with NULL.
 */
void free_net(net_v_t* s1);

#endif