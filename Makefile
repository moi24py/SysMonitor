all: build/cpu.o
	gcc tools/main.c -o build/main -Wall -Wextra -pedantic -O2 -g -std=c99 -I include build/cpu.o

build/cpu.o: src/cpu.c include/sysmonitor.h
	gcc -c -std=c99 src/cpu.c -Wall -Wextra -pedantic -O2 -g -I include -o build/cpu.o

clean:
	rm -r build && mkdir build