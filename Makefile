mycipher: mycipher.o
	gcc -o mycipher mycipher.o
mycipher.o: mycipher.c
	gcc -c mycipher.c

clean:
	rm -f mycipher *.o *~
