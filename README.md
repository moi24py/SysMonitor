# System Monitoring Tool

# Overview
Real-time system monitoring tool for GNU/Linux.

It displays:
- CPU usage (total + per core)
- RAM usage (total/used/available)
- Disk usage (total/used/available)
- Active processes (PID, name, state)
- Network bandwidth (used bandwidth)

## Screenshot
![System monitor output](images/output.png)

# Requirements
- Ubuntu 24.10 (tested)
- Uses `ncurses` for terminal rendering.

# Build
`make` to compile the project
`make clean` to regenerate `/build`

# Run
- To run the project: `./build/main`
- To exit: press Ctrl + C (`SIGINT`)

# How it works
`tools/main.c` runs a loops that calls `display_update(d)` and waits ~50ms with `napms`, while `display_update()` handles the input and metrics polling.
The program periodically:
1. Reads system data from:
   - `/proc/stat` (CPU utilization)
   - `/proc/meminfo` (RAM usage)
   - `/proc/net/dev` (network throughput)
   - `/proc/mounts` (disk mount points)
   - `/proc/[PID]/status` (process details: state/name/etc.)
2. Computes derived metrics (percentages, deltas for bandwidth).
3. Renders the results using ncurses windows in the terminal.

# Project Structure
```shell
sysmonitor/
├── src/
│   ├── cpu.c           # CPU monitoring functions
│   ├── mem.c           # RAM monitoring functions
│   ├── disk.c          # Disk monitoring functions
│   ├── proc.c          # Process monitoring functions
│   ├── network.c       # Used bandwidth
│   └── display.c       # Screen rendering functions
├── include/
│   ├── display.h       # Used bandwidth
│   └── sysmonitor.h    # Header with definitions and prototypes
├── tools/
│   └── main.c          # Main logic and update loop
├── build/              # Object files and executable
├── Makefile            # Automatic compilation
└── README.md           # Project information and instructions

```

# References
- [C reference](https://en.cppreference.com/c)

- [ncurses tutorial](https://github.com/mcdaniel/curses_tutorial) by professor Patrick McDaniel