LIBRARY_PATH=../include
all:fibo.c main.c
	gcc -c fibo.c fibo.o -I ../include
	gcc -c main.c main.o -I ../include
	gcc fibo.o main.o -o ../fibo

clean:
	rm *.o
