CC=gcc
CFLAGS_OBJS=-Wall -O3 -c
CFLAGS_TARGET=-Wall -O3 -s

all: udptx udprx

udptx: udptx.o udp.o
	$(CC) $(CFLAGS_TARGET) -o $@ $^

udprx: udprx.o udp.o
	$(CC) $(CFLAGS_TARGET) -o $@ $^

%.o: %.c
	$(CC) $(CFLAGS_OBJS) -o $@ $<

clean:
	rm -f udprx.o udptx.o udp.o
