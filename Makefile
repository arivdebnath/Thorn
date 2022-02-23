CC = gcc
CFLAGS = -std=c99 -Wall -g
parsing: parsing.o mpc.o
	$(CC) $(CFLAGS) parsing.o mpc.o -ledit -lm -o parsing

parsing.o: parsing.c
	$(CC) $(CFLAGS) -c parsing.c -ledit -lm

mpc.o: mpc.c mpc.h
	$(CC) $(CFLAGS) -c mpc.c -lm

clean:
	rm -f *.o