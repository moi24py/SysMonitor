all: build/cpu.o build/mem.o build/disk.o build/proc.o build/network.o build/display.o
	gcc -std=c99 -Wall -Wextra -pedantic -O2 -g -I include build/cpu.o build/mem.o build/disk.o build/proc.o build/network.o build/display.o tools/main.c -lncursesw -o build/main

build/cpu.o: src/cpu.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/cpu.c -Wall -Wextra -pedantic -O2 -g -I include -o build/cpu.o

build/mem.o: src/mem.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/mem.c -Wall -Wextra -pedantic -O2 -g -I include -o build/mem.o

build/disk.o: src/disk.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/disk.c -Wall -Wextra -pedantic -O2 -g -I include -o build/disk.o

build/proc.o: src/disk.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/proc.c -Wall -Wextra -pedantic -O2 -g -I include -o build/proc.o

build/network.o: src/network.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/network.c -Wall -Wextra -pedantic -O2 -g -I include -o build/network.o

build/display.o: src/display.c include/sysmonitor.h include/display.h
	gcc -c -std=c99 src/display.c -Wall -Wextra -pedantic -O2 -g -I include -o build/display.o

clean:
	rm -r build && mkdir build