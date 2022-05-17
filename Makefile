CC = gcc
CFLAG = -c -Wall

tetris: tetris.o init.o random.o
	$(CC) -o tetris tetris.o init.o random.o -lncurses

tetris.o: tetris.c tetris.h block.h
	$(CC) $(CFLAG) tetris.c

init.o: init.c
	$(CC) $(CFLAG) init.c

random.o: random.c block.h
	$(CC) $(CFLAG) random.c

clean:
	rm *.o