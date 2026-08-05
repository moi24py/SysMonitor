#ifndef _DISPLAY_H
#define _DISPLAY_H

#include <ncurses.h>
#include "sysmonitor.h"

typedef struct Display Display;

/**
 * @brief Initialize ncurses and create UI windows.
 * @return Pointer to a Display instance, or NULL on failure.
 * @note display_update must be called repeatedly in the main loop.
 */
Display* display_create(void);

/**
 * @brief Render a UI update tick and handle user input.
 * @param d Display instance from display_create().
 * @note display_update also reads non-blocking keyboard input for scrolling.
 * @warning This function performs periodic polling internally (e.g., CPU/network ~2s).
 */
void display_update(Display* d);

/**
 * @brief Destroy the UI and restore the terminal state.
 * @param d Display instance to destroy.
 */
void display_destroy(Display* d);

#endif