#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include "../include/sysmonitor.h"

int main(){

    while(true){
        get_CPU_usage();
        get_memory_usage();
        
        sleep(3);
    }
    
    return 0;
}