CC=gcc
CFLAGS=-Wall

EXEC = receiver emitter
OBJ = ./src/core.o ./src/rcv.o ./src/util.o ./src/config.o ./src/logger.o

.SILENT: clean
.INTERMEDIATE: $(OBJ)

all: build_dir $(EXEC)

build_dir:
	mkdir -p build 

$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) ./src/$@.c -o ./build/$@

clean:
	-rm -rf ./build/*

test:
	./build/receiver /dev/ttyS1 &
	./build/emitter /dev/ttyS0
