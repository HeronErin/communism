CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib -I../protocol
RM      = rm -f


SRC   = $(wildcard *.c)
OBJS= $(SRC:.c=.o)


default: server

protocol:
	make -C ../protocol


server: protocol $(OBJS)
	$(CC) $(CFLAGS) -o server $(OBJS) ../protocol/protocol.o
run: server
	./server
clean:
	$(RM) server $(OBJS)
	make -C ../protocol clean
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@