#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <algorithm>

//#include <stdio.h>
#include <time.h>
//#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h> //inet_ntoa, inet_addr
//#include <sys/types.h> //TODO
#include <sys/socket.h>
//#include <netinet/in.h> //TODO
#include <netdb.h> // hostent, gethostbyname

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
    bool cmd_set    = false;
    bool r_path_set = false;
    bool l_path_set = false;
public:
    vector<char> *content = nullptr;
    size_t   file_len     = 0;
    int      port         = 6677;
    string   protocol     = "http://"; // TODO a tohle mi je k čemu???
    string   server       = "";
    string   command      = "";
    string   remote_path  = "";
    string   local_path   = "";
    string   file_folder  = "";
    Arguments(int argc, char **argv);
    ~Arguments();
};

Arguments::Arguments(int argc, char **argv) {
    vector<string> args(argv, argv + argc);
    for (int i = 1; i < argc; i++) {
        if (this->cmd_set == false) {
            this->cmd_set = true;
            if (args[i] == "del") {
                this->command = "DELETE";
                file_folder   = "?type=file";
            }
            else if (args[i] == "get") {
                this->command = "GET";
                file_folder   = "?type=file";
            }
            else if (args[i] == "put") {
                this->command = "PUT";
                file_folder   = "?type=file";
            }
            else if (args[i] == "lst") {
                this->command = "GET";
                file_folder   = "?type=folder";
            }
            else if (args[i] == "mkd") {
                this->command = "PUT";
                file_folder   = "?type=folder";
            }
            else if (args[i] == "rmd") {
                this->command = "DELETE";
                file_folder   = "?type=folder";
            }
            else {
                throw invalid_argument("Invalid command\n");
            }
        }
        else if (this->r_path_set == false) {
            this->r_path_set = true;
            this->remote_path = args[i];
            size_t test = 0;
            while (( test = this->remote_path.find(" ")) != this->remote_path.npos ) {
                this->remote_path.replace(test, 1, "%20");
            }
        }
        else if (this->l_path_set == false) {
            this->l_path_set = true;
            this->local_path = args[i];
        }
        else {
            throw invalid_argument("Too many arguments\n");
        }
    }
    if (this->command == "PUT" and this->file_folder == "?type=file" and l_path_set == false) {
        throw invalid_argument("Set local path when using put or mkd\n");
    }
    else if (this->cmd_set == false or this->r_path_set == false) {
        throw invalid_argument("Set both command and remote path\n");
    }
    size_t index = this->remote_path.find("://");
    if (index == this->remote_path.npos) {
        index = 0;
    }
    else {
        index += 3;
        this->protocol = this->remote_path.substr(0, index);
        this->remote_path.erase(0, index);
    }
    size_t index2 = this->remote_path.find("/", index);
    if (index2 == this->remote_path.npos) {
        index2 = this->remote_path.size();
    }
    size_t index3 = this->remote_path.find(':', index);
    if (index3 > index2) {
        this->port = 6677;
    }
    else {
        this->port = stoi(this->remote_path.substr(index3+1, index2));
        this->remote_path.erase(index3, index2-index3);
    }
    index = this->remote_path.find("/");
    if (index == this->remote_path.npos) {
        throw invalid_argument("Invalid user specified\n");
    }
    this->server      = this->remote_path.substr(0, index);
    this->remote_path = this->remote_path.substr(index, this->remote_path.size());
    index = this->remote_path.find("/", 1);
    if (index == 1 || index == (this->remote_path.size()-1) || index == this->remote_path.npos) {
        throw invalid_argument("Invalid user or remote path\n");
    }
    if (this->file_folder == "?type=file" && this->remote_path[this->remote_path.size()-1] == '/') {
        throw invalid_argument("Unable to use file method on directory\n");
    }
    if (this->command == "PUT" && this->file_folder == "?type=file") {
        ifstream file(this->local_path, ios::binary);
        file.seekg(0, ios::end);
        this->file_len = file.tellg();
        file.seekg(0, ios::beg);
        this->content = new vector<char>(this->file_len);
        file.read((char*) &(this->content[0][0]), this->file_len);
    }
    DEBUG_LINE("protocol: "+this->protocol);
    DEBUG_LINE("port: "+ to_string(this->port));
    DEBUG_LINE("remote path: "+this->remote_path);
    DEBUG_LINE("hostname: "+this->server);
    DEBUG_LINE("-------------------------------------");
}

Arguments::~Arguments() {
    if (this->content != nullptr) {
        delete this->content;
    }
}

string http_request(Arguments *args) {
    string request = args->command + " " + args->remote_path + args->file_folder + " HTTP/1.1\r\n";;
    char buf[128];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, 128, "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
    request += buf;
    request += "Accept-Encoding: identity\r\n";
    request += "Accept: application/json\r\n";
    if (args->command == "PUT" && args->file_folder == "?type=file") {
        request += "Content-Type: application/octet-stream\r\n";
        request += "Content-Length: " + to_string(args->file_len) + "\r\n";
    }
    request += "\r\n";
    if (args->content != nullptr) {
        for (size_t i = 0; i < args->file_len; i++) {
            request += args->content[0][i];
        }
    }
    return request;
}

int main(int argc, char **argv) {
    Arguments *args;
    const size_t SIZE = 256;
    char buffer[SIZE] = {0,};
    int sockcl;
    struct sockaddr_in serv_addr;

    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return 1; // TODO
    }
    catch (exception &e) {
        cerr << "Something bad happend...\n";
        DEBUG_LINE(e.what());
        return 1; // TODO
    }

    hostent *server = gethostbyname(args->server.c_str());
    if (server == NULL) {
        cerr << "Server not found\n";
        delete args;
        return 1; // TODO
    }
    sockcl = socket(AF_INET, SOCK_STREAM, 0);
    if (sockcl <= 0) {
        cerr << "Unable to create socket\n";
        delete args;
        return 1; //TODO
    }
    in_addr *address = (in_addr *) server->h_addr;
    char *ip_addres  = inet_ntoa(* address);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addres);
    serv_addr.sin_port = htons(args->port);
    DEBUG_LINE("Connecting...");
    if (connect(sockcl, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Unable to connect\n";
        close(sockcl);
        delete args;
        return 1;
    }
    DEBUG_LINE("Connected");
    string request = http_request(args);
    DEBUG_LINE("Sending message:");
    DEBUG_INLINE(request);
    DEBUG_LINE("--END--");
    if (send(sockcl, request.data(), request.size(), 0) < 0) {
        cerr << "Unable to send message\n";
        close(sockcl);
        delete args;
        return 1; // TODO
    }

    DEBUG_LINE("Server response:");
    while (recv(sockcl, buffer, SIZE, 0) > 0) {
        DEBUG_INLINE(buffer);
        bzero((char *) buffer, SIZE);
    }
    DEBUG_LINE("--END--");
    close(sockcl);
    delete args;
    return 0;
}
