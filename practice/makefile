all: client server
client:client.cc
	g++ -std=c++11 -o $@ $^
server:server.cc
	g++ -std=c++11 -o $@ $^
.PHONY:clean
clean:
	rm -f client server