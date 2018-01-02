#compiler
CC = g++

CFLAGS = -g -Wall -c

compile : server client

server: server.o
	g++ server.o -o server -lpthread

client: client.o
	g++ client.o -o client

server.o: server.cpp
	g++ -c -lpthread server.cpp 

client.o: client.cpp
	g++ -c client.cpp

clean:
	rm -rf *.o compile
