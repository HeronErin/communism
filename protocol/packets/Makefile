CC      = gcc

# provide debug info, then include dirs
CFLAGS  = -g -I../cJSON -I../zlib
RM      = rm -f


SRC   = $(wildcard serverbound/*.c clientbound/*.c)
OBJS= $(SRC:.c=.o)


default: packetBundle.o

packetBundle.o: $(OBJS)
	ar rcs packetBundle.o $(OBJS)

clean:
	$(RM) packetBundle.o $(OBJS)
%.o : %.c
	$(CC) $(CFLAGS) -c  $< -o $@
