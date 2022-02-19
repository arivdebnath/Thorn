parsing: parsing.o mpc.o
	gcc -std=c99 -Wall parsing.o mpc.o -ledit -lm -o parsing

parsing.o: parsing.c
	gcc -std=c99 -Wall -c parsing.c -ledit -lm

mpc.o: mpc.c mpc.h
	gcc -std=c99 -Wall -c mpc.c -lm

clean:
	rm *.o