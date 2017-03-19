all: ftrest ftrestd

ftrestd: server.o
	g++ -std=c++11 -o ftrestd server.o

ftrest: client.o
	g++ -std=c++11 -o ftrest client.o

client.o: client.cpp
	g++ -std=c++11 -c client.cpp -o client.o

server.o: server.cpp
	g++ -std=c++11 -c server.cpp -o server.o

.PHONY: debug clean
debug:
	g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrest client.cpp -DDEBUG
	g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrestd server.cpp -DDEBUG

clean:
	rm client.o server.o

odevzdani:
	tar -czf xmatej52.tar.gz Makefile server.cpp client.cpp readme.md
	mv xmatej52.tar.gz xmatej52.tgz
