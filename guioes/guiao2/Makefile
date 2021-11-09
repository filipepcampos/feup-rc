CC=gcc
CFLAGS=-Wall

all: receiver emitter

core.o :
	$(CC) $(CFLAGS) -c core.c

rcv :
	$(CC) $(CFLAGS) -c core.o rcv.c

emitter : core.o rcv.o
	$(CC) $(CFLAGS) core.o rcv.o emitter.c -o ./build/emitter

receiver : core.o rcv.o
	$(CC) $(CFLAGS) core.o rcv.o receiver.c -o ./build/receiver

test:
	./build/receiver /dev/ttyS1 &
	./build/emitter /dev/ttyS0

clean:
	-rm -rf ./build/*
	-rm *.o
