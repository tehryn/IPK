#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>

//#include <stdio.h>
//#include <stdlib.h>
#include <string.h>
#include <unistd.h>
//#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>

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
};

class Request {
private:
    size_t content_start = 1;
    size_t byte_read     = 0;
    bool set_head(string src);
    bool set_command(string src);
    bool set_path(string src);
    bool set_len(string src);
    bool proc_ld_data();
public:
    string command       = "";
    string path          = "";
    string head          = "";
    size_t content_len   =  0;
    string content       = "";
    bool   is_file       = false;
    void ld_data(int fd);
};

bool Request::set_head(string src) {
    this->content_start = src.find("\r\n\r\n");
    if (this->content_start != src.npos) {
        this->content_start += 4;
        this->head = src.substr(0, this->content_start);
        return false;
    }
    return true;
}

bool Request::set_command(string src) {
    size_t idx = src.find(' ');
    if (idx != src.npos) {
        this->command = src.substr(0, idx);
        return false;
    }
    return true;
}

bool Request::set_path(string src) {
    size_t idx = src.find(' ');
    if (idx != src.npos) {
        idx++;
        size_t idx2 = src.find(' ', idx);
        if (idx2 != src.npos) {
            this->path = src.substr(idx, idx2 - idx);
            size_t test = 0;
            while (( test = this->path.find("%20")) != this->path.npos ) {
                this->path.replace(test, 3, " ");
            }
            idx = this->path.size() - 12;
            if (this->path.substr(idx, 12) == "?type=folder") {
                this->is_file = false;
                this->path    = this->path.substr(0, idx);
            }
            else if (this->path.substr(idx + 2, 10) == "?type=file") {
                this->is_file = true;
                this->path    = this->path.substr(0, idx+2);
            }
            else {
                return true;
            }
            return false;
        }
    }
    return true;
}

bool Request::set_len(string src) {
    size_t idx  = src.find("Content-Length: ");
    size_t idx2 = src.find("\r\n", idx);
    if (idx == src.npos || idx2 == src.npos) {
        return true;
    }
    idx += 16;
    try {
        this->content_len = stoi(src.substr(idx, idx2-idx));
    } catch (exception &e) {
        return true;
    }
    return false;
}

void Request::ld_data(int fd) {
    char buffer[256];
    DEBUG_LINE("Reading socket");
    unsigned len;
    while ((len = recv(fd, buffer, 256, 0)) > 0) {
        this->byte_read += len;
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
        }
        if (this->set_head(this->content) == false) {
            break;
        }
    }

    DEBUG_LINE("Reading done");

    if (this->proc_ld_data() == true) {
        throw new invalid_argument("Socket has invalid header!");
    }

    DEBUG_INLINE("Command: ");
    DEBUG_LINE(this->command);
    DEBUG_INLINE("Path: ");
    DEBUG_LINE(this->path);
    DEBUG_INLINE("Is it file: ");
    DEBUG_LINE(to_string(this->is_file));
    DEBUG_LINE("Header:");
    DEBUG_LINE(this->head);
    DEBUG_INLINE("Content-Length: ");
    DEBUG_LINE(to_string(this->content_len));

    byte_read -= this->content_start;
    this->content = this->content.substr(this->content_start);

    DEBUG_INLINE("Bytes read from contentent: ");
    DEBUG_LINE(to_string(this->byte_read));
    DEBUG_LINE("Current content:");
    DEBUG_INLINE(this->content);
    DEBUG_LINE("--END--");
    DEBUG_INLINE("Lenght of content: ");
    DEBUG_LINE(to_string(this->content.size()));
    DEBUG_LINE("Loading rest of socket");

    while (this->byte_read < this->content_len) {
        len = recv(fd, buffer, 256, 0);
        this->byte_read += len;
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
        }
    }

    DEBUG_LINE("Loading done");
    DEBUG_INLINE("Lenght of content: ");
    DEBUG_LINE(to_string(this->content.size()));
}

bool Request::proc_ld_data() {
    if (set_command(this->head)) {
        return true;
    }
    else if (set_path(this->head)){
        return true;
    }
    if (this->command == "PUT" && this->is_file ) {
        if (set_len(this->head)) {
            return true;
        }
    }
    return false;
}


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

string put_on_file(Request *req, string path) {
    string message = "";
    ofstream fout(path, ios::binary);
    if (fout.is_open() == false) {
        DEBUG_LINE("Fail to open file");
        message = "404 Not Found";
        return message;
    }
    else {
        DEBUG_INLINE("Writing to: ");
        DEBUG_LINE(path);
        fout.write(req->content.data(), req->content.size());
        message = "200 OK";
        fout.close();
        return message;
    }
}

string put_on_folder(string path) {
    struct stat st;
    memset(&st, 0, sizeof(struct stat));
    string message;
    DEBUG_LINE("Checkinf if directory already exists");
    if (stat(path.c_str(), &st) == -1) {
        DEBUG_INLINE("Creating directory: ");
        DEBUG_LINE(path.c_str());
        mkdir(path.c_str(), 0700);
        message = "200 OK";
    }
    else {
        DEBUG_INLINE("Directory failed to create: ");
        DEBUG_LINE(path.c_str());
        message = "400 Bad Request";
    }
    return message;
}

string create_response(Request *req, string root_path){
    string message;
    string path = root_path + req->path;
    if (req->command == "PUT") {
        if (req->is_file) {
            message = put_on_file(req, path);
        }
        else {
            message = put_on_folder(path);
        }
    }
    else if (req->command == "GET") {
        if (req->is_file) {

        }
        else {

        }
    }
    else if (req->command == "DELETE") {
        if (req->is_file) {

        }
        else {

        }
    }
    else {

    }
    return message;
}

int main(int argc, char **argv) {
    Arguments *args;
    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return -1; // TODO
    }
    int sockfd,              // Soclet descriptor
        sockcomm,            // Socket descriptor
        portno = args->port; // Port number
    socklen_t clilen;        // Size of clien's addres
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
    Request req;
    string message;
    while (true) {
        clilen = sizeof(cli_addr);
        sockcomm = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sockcomm > 0) {
            req.ld_data(sockcomm);
            message = create_response(&req, args->root_folder);
            if (send(sockcomm, message.data(), message.size(), 0) < 0) {
                cerr << "Unable to send message.\n";
                return 1;
            }
        }
        close(sockcomm);
    }
    delete args;
    return 0;
}