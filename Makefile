CC?=gcc
SRCFILES=./src/audio.c ./src/kb.c ./src/mouse.c

./build/demo: ./build $(SRCFILES) ./examples/demo.c
	$(CC) -I ./include -O3 $(SRCFILES) ./examples/demo.c -o $@ -lm

./build/mandelbrot: ./build $(SRCFILES) ./examples/mandelbrot.c
	$(CC) -I ./include -O3 $(SRCFILES) ./examples/mandelbrot.c -o $@ -lm

./build:
	mkdir $@

all: ./build/demo ./build/mandelbrot

clean:
	rm -f ./build/*
