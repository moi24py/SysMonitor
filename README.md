# System Monitoring Tool
## Status: Work in Progress

# Features
Monitor in real time::
- CPU (total usage and per core)
- RAM (total, used, available)
- Disk (total, used, available space)
- Active processes (PID, name, state)
- Network (used bandwidth)

# Project Structure
```shell
sysmonitor/
├── src/
│   ├── main.c          # Main logic and update loop
│   ├── cpu.c           # CPU monitoring functions
│   ├── mem.c           # RAM monitoring functions
│   ├── disk.c          # Disk monitoring functions
│   ├── proc.c          # Process monitoring functions
│   └── display.c       # Screen rendering functions
├── include/
│   ├── sysmonitor.h    # Header with definitions and prototypes
│   └── colors.h        # ANSI color definitions header
├── Makefile            # Automatic compilation
└── README.md           # Project information and instructions

```