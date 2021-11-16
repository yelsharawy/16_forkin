all: main.o
	gcc -o forkin main.o

main.o: main.c
	gcc -c main.c

clean:
	-rm ./*.o
	-rm ./forkin*

run:
	./forkin*

