CC?=gcc
SRCFILES=./src/audio.c ./src/kb.c ./src/mouse.c

demo: $(SRCFILES) ./examples/demo.c
	$(CC) -I ./include -O3 $^ -o $@ -lm

clean:
	rm -f demo
