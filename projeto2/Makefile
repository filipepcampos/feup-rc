CC=gcc

EXEC = main
OBJ = ./src/url_parser.o ./src/queue.o

.SILENT: clean
.INTERMEDIATE: $(OBJ)

all: build_dir $(EXEC)

build_dir:
	mkdir -p build 

$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) ./src/$@.c -o ./build/$@

clean:
	-rm -rf ./build/*

