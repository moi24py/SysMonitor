#include <stdio.h>
#include <stdbool.h>
#include <sys/statvfs.h>
#include <sys/types.h>

#ifndef _SYSMONITOR_H
#define _SYSMONITOR_H

// ############################## CPU ################################ 

#define MAX_CPUS 256

// Struct that stores CPU measurement values
typedef struct cpu_stat{
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

// Struct that stores active and idle CPU data
typedef struct {
    unsigned long long total;
    unsigned long long idle;
} cpu_stat_tot_idle_t;

// Struct that stores memory statistics
typedef struct mem_stat{
    unsigned long long mem_total;
    unsigned long long mem_free;
    unsigned long long mem_available;
} mem_stat_t;

// Retrieves CPU measurement data
int parse_stats(cpu_stat_t *cpu_stat);

// Prints CPU statistics
void print_stats(cpu_stat_t* stats);

// Computes time values: total and idle
cpu_stat_tot_idle_t* sum_total(cpu_stat_t *cpu_stats);

// Computes total CPU utilization (all cores)
double total_CPU_time(cpu_stat_tot_idle_t* sum1, cpu_stat_tot_idle_t* sum2);

// Function that extracts CPU measurement data and computes utilization in percentage
void get_CPU_usage(void);

// ############################ MEMORY ################################ 

// Function that retrieves memory statistics
int parse_mem(mem_stat_t *mem);

// Prints memory statistics
void print_mem(mem_stat_t *mem);

// Retrieves memory statistics and prints them
void get_memory_usage(void);

// ############################ DISK ################################ 

#define MAX_FS 256

// Struct that stores filesystem name
typedef struct disk{
    char mount[512];
    char fstype[32];
    bool pseudo;
    unsigned long long total_space;
    unsigned long long free_space;
    unsigned long long used_space;
} disk_t;

// Checks if the filesystem type is virtual
bool is_pseudo_fstype(char *s);

// Retrieves mounted filesystems (pseudo-fs excluded)
disk_t* retrieve_fs(void);

// Asks the user if pseudo-filesystems should be shown
bool want_overlay(void);

// Prints an array of disk_t structures that store filesystem data
void print_fs_stats(disk_t *fs, int overlay);

// Human readable bytes
char* human_readable(const unsigned long long bytes);

// Prompts the user to decide whether to include pseudo-filesystems
bool want_overlay(void);

// Computes a filesystem total, free and used space
void compute_fs_space(disk_t *fs, struct statvfs *stats);

// Prints a filesystem total space, free space, used space
void print_fs_space(disk_t *fs, struct statvfs *stats, size_t i);

// Retrieves and prints disk stats
void get_disk_stats(void);

// ############################ PROCESS ################################

// Struct that stores process pid, name and state
typedef struct {
    pid_t pid;
    char name[256];
    char state[256];
} proc_t;

// Struct that stores an array of processes and their quantity 
typedef struct {
    proc_t *ps; // dinamic array
    size_t qty;
} proc_v_t;

// Retrieves processes PID, name and status
// Returns an array of struct proc_t
proc_v_t* retrieve_processes(void);

// Prints a proc_t struc
void print_processes(const proc_v_t *ps);

// Retrieves and prints processes
void get_proc(void);

// Free
void free_processes(proc_v_t *psv);

#endif