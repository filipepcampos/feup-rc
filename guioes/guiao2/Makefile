CC=gcc
CFLAGS=-Wall

all: receiver emitter

core.o :
	$(CC) $(CFLAGS) -c core.c

emitter : core.o
	$(CC) $(CFLAGS) core.o emitter.c -o ./build/emitter

receiver : core.o
	$(CC) $(CFLAGS) core.o receiver.c -o ./build/receiver

test:
	./build/receiver /dev/ttyS1 &
	./build/emitter /dev/ttyS0

clean:
	rm -rf ./build/*
	rm *.o
