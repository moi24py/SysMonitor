#include <stdio.h>

#ifndef _SYSMONITOR_H
#define _SYSMONITOR_H

/* ### CPU ### */
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
void get_CPU_usage();

// Function that retrieves memory statistics
int parse_mem(mem_stat_t *mem);

// Prints memory statistics
void print_mem(mem_stat_t *mem);

// Retrieves memory statistics and prints them
void get_memory_usage();

#endif