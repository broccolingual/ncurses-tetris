CC = gcc
CFLAG = -c -Wall
TARGET = turtle
SRCS = turtle.c init.c random.c score.c utils.c
OBJS = $(SRCS:%.c=%.o)
HDS = turtle.h block.h field.h
LIBS = -lncurses

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(OBJS): $(SRCS) $(HDS)
	$(CC) $(CFLAG) $(SRCS)

clean:
	rm -f $(OBJS) $(TARGET)
