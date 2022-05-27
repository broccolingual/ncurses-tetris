CC = gcc
CFLAG = -c -Wall

turtle: turtle.o init.o random.o score.o utils.o
	$(CC) -o turtle turtle.o init.o random.o score.o utils.o -lncurses

turtle.o: turtle.c turtle.h block.h field.h
	$(CC) $(CFLAG) turtle.c

init.o: init.c
	$(CC) $(CFLAG) init.c

random.o: random.c block.h
	$(CC) $(CFLAG) random.c

score.o: score.c
	$(CC) $(CFLAG) score.c

utils.o: utils.c
	$(CC) $(CFLAG) utils.c

clean:
	rm *.o
