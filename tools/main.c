#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdbool.h>
#include <signal.h>
#include <ncurses.h>

#include "sysmonitor.h"
#include "display.h"

static volatile bool running = true;

static void handle_signal(int sig) {
    (void)sig;
    running = false;
}

int main(void) {
    signal(SIGINT, handle_signal);

    Display *d = display_create();
    if (!d) {
        fprintf(stderr, "Errore: impossibile inizializzare il display\n");
        return EXIT_FAILURE;
    }

    while (running) {
        // Un solo refresh “frequente”
        display_update(d);

        // Piccola pausa per non consumare CPU
        napms(50); // ~20 FPS, scroll molto reattivo
    }

    display_destroy(d);
    return EXIT_SUCCESS;
}
