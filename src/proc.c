#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/sysmonitor.h"

#define MAX_PROCS 5000

// Retrieves processes PID, name and status
// Returns an array of struct proc_t
proc_t* retrieve_processes(size_t *ps_qty){
   
    proc_t *ps = (proc_t*) malloc( sizeof(proc_t)*MAX_PROCS );
    if (ps == NULL){
        perror("malloc()");
        exit(EXIT_FAILURE);
    }
   
    char path[512];
    size_t count = 0;
    for (size_t i = 0; i < MAX_PROCS; i++){
        // cat /proc/1/status
        int n = snprintf(path, sizeof(path), "/proc/%zu/status", i);
        if (n < 0 || (size_t)n >= sizeof(path)) printf("Errore snprintf\n");
        
        FILE *fp = fopen(path, "r");
        if (fp){
            char line[512];
            bool got_name = false, got_state = false, got_pid = false;
            int temp;
            while ( fgets(line, sizeof(line), fp) != NULL ){
                if (sscanf(line, "Name: %255s", (ps+count)->name) == 1) got_name = true;
                else if (sscanf(line, "State: %255s", (ps+count)->state) == 1) got_state = true;
                else if (sscanf(line, "Pid: %d", &temp) == 1){
                    got_pid = true;
                    (ps+count)->pid = (pid_t)temp;
                }
            }
            fclose(fp);
            if (got_name && got_state && got_pid && count < MAX_PROCS) count++;
        }
    }
    *ps_qty = count;
    return ps;
}

// Prints a proc_t struc
void print_processes(proc_t *ps, size_t ps_qty){
    for(size_t i = 0; i < ps_qty; i++)
        printf("%s %s %d\n", (ps+i)->name, (ps+i)->state, (int)(ps+i)->pid);
}

// Retrieves and prints processes
void get_proc(void){
    size_t ps_qty;
    proc_t * ps = retrieve_processes(&ps_qty);
    print_processes(ps, ps_qty);
}