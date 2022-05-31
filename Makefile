CC = gcc
CFLAG = -c -Wall
TARGET = turtle
SRCS = turtle.c init.c random.c score.c utils.c memory.c
OBJS = $(SRCS:%.c=%.o)
HDS = turtle.h block.h field.h init.h utils.h random.h score.h memory.h
LIBS = -lncurses

$(TARGET): $(OBJS)
	$(CC) -o $@ $(OBJS) $(LIBS)

$(OBJS): $(SRCS) $(HDS)
	$(CC) $(CFLAG) $(SRCS)

clean:
	rm -f $(OBJS) $(TARGET)
