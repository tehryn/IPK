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
private:
    bool set_root_folder = false;
    bool set_port        = false;
public:
    string   root_folder = ".";
    unsigned port        = 80;
    Arguments(int argc, char **argv);
};

Arguments::Arguments(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        if (args[i] == "-r" and this->set_root_folder == false and (i+1) < argc) {
            this->set_root_folder = true;
            this->root_folder     = args[++i];
        }
        else if (args[i] == "-p" and this->set_port == false and (i+1) < argc) {
             this->set_port    = true;
             this->port = stoi(args[++i]);
        }
        else {
            throw invalid_argument("Arguments of program are invalid\n");
        }
    }
}

int main(int argc, char **argv) {
    Arguments *args;
    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return -1; // TODO
    }
    const int SIZE = 1024;
    int sockfd,              // File descriptor
        sockcomm,            // File descriptor
        portno = args->port; // Port number
    socklen_t clilen;        // Size of clien's addres
    char      buffer[SIZE] = {};

    struct sockaddr_in6 serv_addr = {},
                        cli_addr  = {};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        cerr << "Could not open socket\n";
        return -1; // TODO
    }
    int optval = 1; // TODO k cemu to je?
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(portno);
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Unable to bind\n";
        return -1; // TODO
    }

    if (listen(sockfd, 1) < 0) {
        cerr << "Unable to listen\n";
        return -1;
    }
    while (true) {
        clilen = sizeof(cli_addr);
        sockcomm = accept (sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sockcomm > 0) {
            while (recv(sockcomm, buffer, SIZE, 0) > 0) {
                send(sockcomm, buffer, strlen(buffer), 0);
            }
        }
        close(sockcomm);
/*        clilen = sizeof(cli_addr);
        bzero((char *) buffer, SIZE);
        len = recvfrom(sockfd, buffer, SIZE, 0, (struct sockaddr *) &cli_addr, &clilen);
        if (len < 0) {
            cerr << "Unable to recive message";
            return -1; // TODO
        }
        //TODO
        len = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *) &cli_addr, clilen);
        if (len < 0) {
            cerr << "Unable to send message";
        }*/

    }
    return 0;
}