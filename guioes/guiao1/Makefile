all: emitter receiver

$(shell mkdir -p build)

emitter:
	gcc emitter.c -o build/emitter

receiver:
	gcc receiver.c -o build/receiver

test:
	./build/receiver /dev/ttyS11 &
	echo "Just a quick test message" | ./build/emitter /dev/ttyS10

clean:
	rm -rf ./build
