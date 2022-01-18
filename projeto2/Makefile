CC=gcc

EXEC = download
OBJ = ./src/url_parser.o ./src/queue.o ./src/server_commands.o

.SILENT: clean
.INTERMEDIATE: $(OBJ)

all: build_dir $(EXEC)

build_dir:
	mkdir -p build 

$(EXEC) : $(OBJ)
	$(CC) $(CFLAGS) $(OBJ) ./src/$@.c -o ./build/$@

clean:
	-rm -rf ./build

