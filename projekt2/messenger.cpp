#include <iostream>
#include <string>
#include <vector>

#include <string.h>

//#define DEBUG_INLINE(x) (cerr << (x))
//#define DEBUG_LINE(x) (cerr << (x) << endl)


/*template void DEBUG_LINE(T1 x, T2 y="") {
    cerr << x << y << endl;
}*/

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

string parse_arguments(int argc, char **argv, int *port) {
    if (argc != 5) {
        return NULL;
    }
    else {
        *port = -1;
        string IP;
        if (strcmp(argv[1], "-i") == 0) {
            *port = atoi(argv[2]);
            IP    = string(argv[4]);
        }
        else if (strcmp(argv[1],"-p") == 0) {
            IP    = string(argv[2]);
            *port = atoi(argv[4]);
        }
        else {
            return "";
        }
        return IP;
    }
}

int main(int argc, char **argv) {
    int port = -1;
    string IP = parse_arguments(argc, argv, &port);
    DEBUG_LINE(IP);
    DEBUG_LINE(port);
}