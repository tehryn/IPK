all: chat_client

chat_client: chat_client.o
	g++ -std=c++11 -o chat_client chat_client.o -lpthread

chat_client.o: chat_client.cpp
	g++ -std=c++11 -c chat_client.cpp -o chat_client.o -lpthread

.PHONY: debug clean
debug:
	g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o chat_client chat_client.cpp -DDEBUG -lpthread

clean:
	rm chat_client.o

odevzdani:
	tar -czf xmatej52.tar.gz Makefile chat_client.cpp ipk_chat.pdf
	mv xmatej52.tar.gz xmatej52.tgz

