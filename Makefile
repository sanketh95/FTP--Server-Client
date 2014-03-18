CC = g++

all: client server

client: client.cpp client.hpp
	$(CC) -o client client.cpp client.hpp

server: server.cpp server.hpp
	$(CC) -o server server.cpp server.hpp

clean:
	rm -f server client

strip:
	strip server client
