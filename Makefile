all: ftrest ftrestd

ftrestd: server.cpp
	g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrestd server.cpp

ftrest: client.cpp
		g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrest client.cpp

.PHONY: debug
debug: client.cpp server.cpp
		g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrest client.cpp -DDEBUG
		g++ -Wall -std=c++11 -pedantic -lm -g -Wextra -pedantic -o ftrestd server.cpp -DDEBUG
