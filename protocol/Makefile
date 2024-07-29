CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib
RM      = rm -f


SRC   = $(wildcard *.c)
OBJS= $(SRC:.c=.o)


default: protocol.o

protocol.o: $(OBJS)
	ar rcs protocol.o $(OBJS)

clean:
	$(RM) protocol.o $(OBJS)
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@
