all : main

main : 20121625.o shell.o assemble.o createTable.o 
	gcc -o 20121625.out 20121625.o shell.o assemble.o createTable.o -Wall

20121625.o : 20121625.c
	gcc -c 20121625.c -Wall

shell.o : shell.c
	gcc -c shell.c -Wall

assemble.o : assemble.c
	gcc -c assemble.c -Wall

createTable.o : createTable.c
	gcc -c createTable.c -Wall

clear : 
	rm -f *.o 2012625.out
