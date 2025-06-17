all: main client
main:main.cpp
	g++ -std=c++11 -o $@ $^ -g
client:client.cpp
	g++ -std=c++11 -o $@ $^

.PHONY:clean
clean: 
	rm -f main client