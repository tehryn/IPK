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
#include <netdb.h>

using namespace std;

class Arguments {
private:
    bool cmd_set    = false;
    bool r_path_set = false;
    bool l_path_set = false;
public:
    string command     = "";
    string remote_path = "";
    string local_path  = "";
    Arguments(int argc, char **argv);
};

Arguments::Arguments(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        if (this->cmd_set == false) {
            this->cmd_set = true;
            if (args[i] == "del") {
                this->command = "del";
            }
            else if (args[i] == "get") {
                this->command = "get";
            }
            else if (args[i] == "put") {
                this->command = "put";
            }
            else if (args[i] == "lst") {
                this->command = "lst";
            }
            else if (args[i] == "mkd") {
                this->command = "mkd";
            }
            else if (args[i] == "rmd") {
                this->command = "rmd";
            }
            else {
                throw invalid_argument("Invalid command\n");
            }
        }
        else if (this->r_path_set == false) {
            this->r_path_set = true;
            this->remote_path = args[i];
        }
        else if (this->l_path_set == false) {
            this->l_path_set = true;
            this->local_path = args[i];
        }
        else {
            throw invalid_argument("Too many arguments\n");
        }
    }
    if (this->command == "put" and l_path_set == false) {
        throw invalid_argument("Set local path when using put\n");
    }
    else if (this->cmd_set == false or this->r_path_set == false) {
        throw invalid_argument("Set both command and remote path\n");
    }
}

int main(int argc, char **argv) {
    Arguments *args;
    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return 1; // TODO
    }

    hostent *server = gethostbyname(args->remote_path.c_str());
    if (server == NULL) {
        cerr << "Could not connect to server\n";
        return 1; // TODO
    }
    int sockcl,
        len;
    struct sockaddr_in6 serv_addr;
    sockcl = socket(AF_INET, SOCK_STREAM, 0);
    serv_addr.sin6_family = AF_INET6; //TODO
    if (sockcl <= 0) {
        cerr << "Unable to create socket\n";
        return 1; //TODO
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    bcopy((char *)server->h_addr, (char *)&serv_addr.sin6_addr, server->h_length);
//    if (connect(sockcl, ));


    return 0;
}