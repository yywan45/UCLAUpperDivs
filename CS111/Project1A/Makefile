main: lab1a.c
	gcc -pthread -o lab1a -g lab1a.c -Wall
clean:
	rm -rf lab1a *.o *.out *.tar.gz *~
dist:
	rm -rf lab1a-804638846.tar.gz
	tar -zcf lab1a-804638846.tar.gz lab1a.c Makefile README
