CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib
RM      = rm -f


SRC   = $(wildcard *.c)
OBJS= $(SRC:.c=.o)


default: server

server: $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS)
run: server
	./server
clean:
	$(RM) server $(OBJS)
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@