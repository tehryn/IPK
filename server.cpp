#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#ifdef DEBUG
#define DEBUG_LINE(x)   (cout << x << endl)
#define DEBUG_INLINE(x) (cout << x)
#else
#define DEBUG_LINE(x) ;
#define DEBUG_INLINE(x) ;
#endif

using namespace std;

class Arguments {
private:
    bool set_root_folder = false;
    bool set_port        = false;
public:
    string   root_folder = ".";
    unsigned port        = 6677;
    Arguments(int argc, char **argv);
    void ld_data(int fd);
};

class Request {
private:
    size_t content_start = 1;
    void set_head(string src) {
        this->content_start = src.find("\r\n\r\n") + 5;
        this->head = src.substr(0, this->content_start - 5);
    }

    void set_command(string src) {
        this->command = src.substr(0, src.find(' '));
    }

    void set_path(string src) {
        size_t tmp = src.find(' ') + 1;
        this->path = src.substr(tmp, src.find(' ', tmp));
    }
    void proc_ld_data();
public:
    string command       = "";
    string path          = "";
    string head          = "";
    size_t content_len   =  0;
    vector<char> content;
    void ld_data(int fd);
};

void Request::ld_data(int fd) {
/*    size_t idx = message.find("\r\n\r\n");
    if (idx == message.npos) {
        throw invalid_argument("HTTP head is missing.\n");
    }
    this->set_head(message);
    this->set_command(message);
    this->set_path(message);*/
    char buffer[256];
    unsigned len = 0;
    DEBUG_LINE("Reading socket:");
    while ((len = recv(fd, buffer, 256, 0)) > 0) {
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
            DEBUG_INLINE(buffer[i]);
        }
    }
    DEBUG_LINE("--END--");
    return;
}
/*
Request::Request(int fd) {

}
*/

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
    const int SIZE = 4096;
    int sockfd,              // Soclet descriptor
        sockcomm,            // Socket descriptor
        portno = args->port; // Port number
    socklen_t clilen;        // Size of clien's addres
    char      buffer[SIZE] = {};

    struct sockaddr_in serv_addr = {},
                        cli_addr  = {};

    /*
    struct sockaddr_in6 serv_addr = {0,},
                        cli_addr  = {0,};
     */
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "Could not open socket\n";
        return -1; // TODO
    }
    int optval = 1; // TODO k cemu to je?
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(portno);

    /*
    serv_addr.sin6_family = AF_INET6;
    serv_addr.sin6_addr = in6addr_any;
    serv_addr.sin6_port = htons(portno);
    */
    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Unable to bind\n";
        return -1; // TODO
    }

    if (listen(sockfd, 1) < 0) {
        cerr << "Unable to listen\n";
        return -1;
    }
//    size_t index = 0;
//    string head, command, path;
    Request req;
    while (true) {
        clilen = sizeof(cli_addr);
        sockcomm = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sockcomm > 0) {
            req.ld_data(sockcomm);
            DEBUG_LINE(req.head);
            DEBUG_LINE(req.command);
            DEBUG_LINE(req.path);
            if (send(sockcomm, buffer, strlen(buffer), 0) < 0) {
                cerr << "Unable to send message.\n";
                return 1;
            }
        }
        close(sockcomm);
    }
    delete args;
    return 0;
}