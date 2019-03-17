all: server client

server: src/server.cpp
	g++ -std=c++11 -o server src/server.cpp -lpthread
client: src/client.cpp
	g++ -std=c++11 -o client src/client.cpp -lpthread

clean:
	rm -f *.o client server
