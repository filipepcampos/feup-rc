all: main

$(shell mkdir -p build)

main:
	gcc main.c -o ./build/main

test:
	./build/receiver /dev/ttyS11 &
	echo "Just a quick test message" | ./build/emitter /dev/ttyS10

clean:
	rm -rf ./build
