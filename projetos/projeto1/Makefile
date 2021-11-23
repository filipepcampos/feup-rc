CC=gcc
#CFLAGS=-Wall
CFLAGS=

EXEC = application
OBJ = ./src/core.o ./src/datalink_receiver.o ./src/config.o ./src/logger.o ./src/datalink_emitter.o ./src/interface.o

.SILENT: clean
.INTERMEDIATE: $(OBJ)

all: build_dir $(EXEC)

build_dir:
	mkdir -p build 

$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) ./src/$@.c -o ./build/$@

clean:
	-rm -rf ./build/*

