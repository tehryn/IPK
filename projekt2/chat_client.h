#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <thread>
#include <chrono>

#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>
#include <csignal>


enum {
    SOCK = 0,
    STDOUT,
    STDERR,
    END
};

using namespace std;
template<typename T1>
void DEBUG_INLINE(T1 x) {
    cerr << x;
}

template<typename T1, typename T2>
void DEBUG_INLINE(T1 x, T2 y) {
    cerr << x << y;
}

template<typename T1>
void DEBUG_LINE(T1 x) {
    cerr << x << endl;
}

template<typename T1, typename T2>
void DEBUG_LINE(T1 x, T2 y) {
    cerr << x << y << endl;
}

void send_messages(int sockfd, string user);
void recv_messages(int sockfd);