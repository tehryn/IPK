#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
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
    int    port        = 6677;
    string protocol    = "http://";
    string server      = "";
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
                this->command = "DEL";
            }
            else if (args[i] == "get") {
                this->command = "GET";
            }
            else if (args[i] == "put") {
                this->command = "PUT";
            }
            else if (args[i] == "lst") {
                this->command = "LST";
            }
            else if (args[i] == "mkd") {
                this->command = "MKD";
            }
            else if (args[i] == "rmd") {
                this->command = "RMD";
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
        index = this->remote_path.size();
    }
    this->server      = this->remote_path.substr(0, index);
    this->remote_path = this->remote_path.substr(index, this->remote_path.size());
    cout << "protocol: " << this->protocol << "\n"; // TODO
    cout << "port: " << this->port << "\n";
    cout << "remote path: " <<this->remote_path << "\n";
    cout << "server:" << this->server << "\n\n\n";
}

string http_request(Arguments *args) {
    string request = args->command + " " + args->remote_path + " HTTP/1.1\r\n";;
    char buf[256];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, 256, "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
    request += buf;
    request += "Accept-Encoding: identity, gzip, deflate\r\n";
    request += "Accept: text/html\r\n";
    if (args->command == "PUT") {
        request += "Content-Type: TODO\r\n";
        request += "Content-Length: TODO\r\n";
    }
    request += "\r\n";
    return request.c_str();
}

int main(int argc, char **argv) {
    Arguments *args;
    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return 1; // TODO
    }

    hostent *server = gethostbyname(args->server.c_str());
    if (server == NULL) {
        cerr << "Server not found\n";
        delete args;
        return 1; // TODO
    }

    const size_t SIZE = 1024;
    char buffer[1024] = {0,};
    int sockcl;
    struct sockaddr_in serv_addr;
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
    cout << "Connecting...\n"; // TODO
    if (connect(sockcl, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "Unable to connect\n";
        close(sockcl);
        delete args;
        return 1;
    }
    cout << "Connected\n"; // TODO
    string request = http_request(args);
    const char *message = request.c_str();
    cout << "Sending meesage:\n" << message; // TODO
    if (send(sockcl, message, strlen(message), 0) < 0) {
        cerr << "Unable to send message\n";
        close(sockcl);
        delete args;
        return 1; // TODO
    }

    if (recv(sockcl, buffer, SIZE, 0) < 0) {
        cout << "No response\n";
    }
    cout << buffer;
    close(sockcl);
    delete args;
    return 0;
}