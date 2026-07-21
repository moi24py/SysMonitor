#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "../include/sysmonitor.h"

#define MAX_PROCS 5000

// Retrieves processes PID, name and status
// Returns an array of struct proc_t
proc_v_t* retrieve_processes(void){
   
    // Array that stores processes
    proc_v_t *psv = (proc_v_t*) malloc( sizeof(*psv));
    if (psv == NULL){
        perror("malloc(proc_v_t)");
        exit(EXIT_FAILURE);
    }

    psv->qty = 0; // Inizializes the count of stored process information
    psv->ps = malloc(MAX_PROCS * sizeof(*psv->ps));
    if (psv->ps == NULL){
        perror("malloc(proc_t array)");
        free(psv);
        exit(EXIT_FAILURE);
    }
    
    char path[512];  // Filepath string
    /* On Linux, processes are represented under /proc.
    To retrieve a process PID, name and state, read /proc/<pid>/status */
    for (size_t pid = 0; pid < MAX_PROCS; pid++){
        // Concatenates pid with path
        int n = snprintf(path, sizeof(path), "/proc/%zu/status", pid);
        if (n < 0 || (size_t)n >= sizeof(path)) printf("Errore snprintf\n");
        
        FILE *fp = fopen(path, "r"); // Opens /proc/<pid>/status
        if (fp){
            bool got_name = false, got_state = false, got_pid = false;
            char line[512];
            // Temporarily stores the PID as an int. After retrieval, it is casted to pid_t and written to the struct proct_t pid field
            int temp_pid;
            // Reads /proc/<pid>/status and parses it line by line to get name, state and pid
            while ( fgets(line, sizeof(line), fp) != NULL ){
                if (sscanf(line, "Name: %255s", psv->ps[psv->qty].name) == 1) got_name = true;
                else if (sscanf(line, "State: %255s", psv->ps[psv->qty].state) == 1) got_state = true;
                else if (sscanf(line, "Pid: %d", &temp_pid) == 1){
                    got_pid = true;
                    psv->ps[psv->qty].pid = (pid_t)temp_pid;
                }
            }
            fclose(fp);
            // If the retrieval operation was successful, increment the count of stored process information
            if (got_name && got_state && got_pid)
                if (psv->qty < MAX_PROCS) psv->qty++;
            
        }
    }

    // If no valid process has been found, free vector memory
    if (psv->qty == 0){
        free(psv->ps);
        psv->ps = NULL;
        return psv;
    }

    // Reallocatea the array to the exact size; if realloc fails, keeps the original pointer
    proc_t *resized = realloc(psv->ps, psv->qty * sizeof(*psv->ps));
    if (resized == NULL) return psv;
    psv->ps = resized;

    return psv;
}

// Prints a proc_t struc
void print_processes(const proc_v_t *psv){
    for(size_t i = 0; i < psv->qty; i++){
        printf("PID: %-5d    Name: %-50s State: %-10s\n",
            (int)psv->ps[i].pid,
            psv->ps[i].name,
            psv->ps[i].state
        );
    }
}

// Frees the allocated memory
void free_processes(proc_v_t *psv) {
    if (!psv) return;
    free(psv->ps); // frees vector
    free(psv);     // frees container
}

// Retrieves, prints and frees stored processes vector and information
void get_proc(void){
    proc_v_t * procs_vector = retrieve_processes();
    print_processes(procs_vector);
    free_processes(procs_vector);
}