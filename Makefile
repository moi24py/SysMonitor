all: build/cpu.o build/mem.o
	gcc -std=c99 -Wall -Wextra -pedantic -O2 -g -I include build/cpu.o build/mem.o tools/main.c -o build/main

build/cpu.o: src/cpu.c include/sysmonitor.h
	gcc -c -std=c99 src/cpu.c -Wall -Wextra -pedantic -O2 -g -I include -o build/cpu.o

build/mem.o: src/mem.c include/sysmonitor.h
	gcc -c -std=c99 src/mem.c -Wall -Wextra -pedantic -O2 -g -I include -o build/mem.o

clean:
	rm -r build && mkdir build