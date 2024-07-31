CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib
RM      = rm -f


SRC   = $(wildcard serverbound/*.c clientbound/*.c)
OBJS= $(SRC:.c=.o)


default: packets.o

packets.o: $(OBJS)
	ar rcs packets.o $(OBJS)

clean:
	$(RM) packets.o $(OBJS)
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@
