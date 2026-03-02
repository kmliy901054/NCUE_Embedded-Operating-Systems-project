HW4: main.o input.o calculate.o
	$(CC) main.o input.o calculate.o -o HW4

input.o: input.c
	$(CC) -c input.c

calculate.o: calculate.c
	$(CC) -c calculate.c

main.o: main.c
	$(CC) -c main.c

clean:
	rm -f *.o HW4
	
