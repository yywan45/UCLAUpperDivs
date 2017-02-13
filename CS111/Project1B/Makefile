default:
	gcc -pthread -lmcrypt client.c -o client
	gcc -pthread -lmcrypt server.c -o server

clean:
	rm ./client ./server ./*.txt *~ *tar.gz

dist:
	tar -cvzf lab1b-804638846.tar.gz client.c server.c Makefile README my.key 
