sand: main.c
	gcc main.c -o sand -lncurses

run: sand
	./sand

