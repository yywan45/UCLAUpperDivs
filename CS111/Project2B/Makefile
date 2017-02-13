all: lab2_list.c SortedList.c
	gcc -Wall -g -pthread -o lab2_list lab2_list.c SortedList.c
#	-./lab2_list.sh

lab2b: lab2_list.c
	gcc -Wall -g -pthread lab2_list.c -o lab2b_list

SortedList: SortedList.c
	gcc -Wall -g -pthread SortedList.c -o SortedList

clean:
	rm -f SortedList lab2_list *~ *.o *.png *.csv *.tar.gz

dist:
	tar -cvzf lab2b-804638846.tar.gz *.c *.sh *.png *.h *.csv Makefile README.txt
