CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib
RM      = rm -f


SRC   = $(wildcard *.c)
OBJS= $(SRC:.c=.o)


default: protocol.o

packetso:
	make -C packets

protocol.o: packetso $(OBJS)
	ar rcsT protocol.o $(OBJS) packets/packetBundle.o

clean:
	$(RM) protocol.o $(OBJS)
	make -C packets clean
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@
