CC = gcc
CFLAG = -c -Wall

tetris: tetris.o init.o random.o score.o
	$(CC) -o tetris tetris.o init.o random.o score.o -lncurses

tetris.o: tetris.c tetris.h block.h field.h
	$(CC) $(CFLAG) tetris.c

init.o: init.c
	$(CC) $(CFLAG) init.c

random.o: random.c block.h
	$(CC) $(CFLAG) random.c

score.o: score.c
	$(CC) $(CFLAG) score.c

clean:
	rm *.o
