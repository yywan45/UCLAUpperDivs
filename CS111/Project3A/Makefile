SHELL = /bin/bash
CC = gcc
CFLAGS = -g -Wall

default: lab3a.c
	$(CC) $(CFLAGS) lab3a.c -o lab3a

output: lab3a disk-image
	@./lab3a disk-image

clean:
	@rm -rf lab3a *.o *.csv *.tar.gz *~

dist: README Makefile lab3a.c
	@tar -cvzf lab3a-804638846.tar.gz README Makefile lab3a.c
