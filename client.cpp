#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

using namespace std;

class Arguments {
public:
    string command;
    string remote_path;
    string local_path;
    Arguments(int argc, char **argv);
};

Arguments::Arguments(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        if (args[i] == "-r") {
        }
        else if (args[i] == "-p") {
        }
        else {
            throw invalid_argument("Arguments of program are invalid\n");
        }
    }
}