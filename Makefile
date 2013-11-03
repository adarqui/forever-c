all:
	gcc -Wall -O3 forever-c.c -o forever-c

install:
	mv forever-c /usr/local/bin

clean:
	rm -f forever-c
