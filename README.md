# Tool di monitoraggio di sistema
## status: work in progress

# Funzionalità
Monitora in tempo reale:
- CPU (uso totale e per core)
- RAM (totale, usata, libera)
- Disco (spazio totale, usato, libero)
- Processi attivi (PID, nome, stato)
- Rete (bandwidth usata)

# Struttura del progetto
```c
sysmonitor/
├── src/
│   ├── main.c          # Logica principale e loop di aggiornamento
│   ├── cpu.c           # Funzioni per monitoraggio CPU
│   ├── mem.c           # Funzioni per monitoraggio RAM
│   ├── disk.c          # Funzioni per monitoraggio Disco
│   ├── proc.c          # Funzioni per monitoraggio Processi
│   └── display.c       # Funzioni per visualizzazione a schermo
├── include/
│   ├── sysmonitor.h    # Header con definizioni e prototipi
│   └── colors.h        # Header per colori ANSI
├── Makefile            # Compilazione automatica
└── README.md           # Informazioni ed istruzioni

```