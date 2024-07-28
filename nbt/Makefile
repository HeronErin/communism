CC      = gcc
CFLAGS  = -g -Ilib -fPIC -I../zlib
RM      = rm -f

default: NBT

lib: nbt.o

nbt.o: lib/nbt.c ../zlib/libz.a
	$(CC) $(CFLAGS) -c -o nbt.o lib/nbt.c

NBT: nbt.o
	$(CC) $(CFLAGS) -o NBT main.c nbt.o ../zlib/libz.a

run: clean NBT
	./NBT

../zlib/libz.a:
	cd ../zlib && sh ./configure
	make static -C ../zlib -j 4

clean:
	-$(RM) NBT nbt.o
