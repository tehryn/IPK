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
#include <sys/socket.h>
#include <netdb.h> // hostent, gethostbyname

using namespace std;

/**
 * Class for storing program settings.
 */
class Arguments {
private:
    /// Tells if command argument was set.
    bool cmd_set    = false;
    /// Tells if remote path argument was set.
    bool r_path_set = false;
    /// Tells if local path argument was set.
    bool l_path_set = false;
public:
    /// When command is get, this vector will store content of input file.
    vector<char> *content = nullptr;
    /// When command is get, this variable stores lenght of readed file.
    size_t   file_len     = 0;
    /// Port where is host is binded.
    int      port         = 6677;
    /// Protocol, usless variable. Client.cpp does not use it.
    string   protocol     = "http://";
    /// Name of host.
    string   server       = "";
    /// Stores value of command argument.
    string   command      = "";
    /// Stores value of remote path argument.
    string   remote_path  = "";
    /// Stores value of local path (without protocol, host and port).
    string   local_path   = "./";
    /// This hold strinf that will be added to path when sending request
    /// (?type=file or ?type=folder)
    string   file_folder  = "";
    /**
     * Construct an Argument class. Throw invalid_argument exception on error.
     * @param argc Number of arguments.
     * @param argv Array of arguments.
     */
    Arguments(int argc, char **argv);
    /**
     * Destructor.
     */
    ~Arguments();
};

/**
 * Class for parsing responses.
 */
class Response {
private:
    /// Index where content starts (Http head ends).
    size_t content_start = 1;
    /// Number of bytes we have read from content.
    size_t byte_read     = 0;
    /**
     * Retrieve response header.
     * @param  src String where header is.
     * @return     True when there is no header, false on succes.
     */
    bool set_head(string src);
    /**
     * Retrieve response header.
     * @param  src String where header is.
     * @return     True when there is no header, false on succes.
     */
    bool set_len(string src);
    /**
     * If response is not 200 OK, sets output error.
     */
    void set_error();
public:
    /// Response header.
    string head          = "";
    /// Lenght of content.
    size_t content_len   =  0;
    /// Content itself.
    string content       = "";
    /// Output error (when response is not 200 OK).
    string error         = "";
    /**
     * Constructor
     * @param fd Socket descriptor.
     */
    Response(int fd);
    /**
     * Writes socket content to file.
     * @param  args Program settings (needed fo local path to file)
     * @return      true on error, false on succes.
     */
    bool write_to_file(Arguments *args);
};

Arguments::Arguments(int argc, char **argv) {
    vector<string> args(argv, argv + argc); // arguments into vector of string
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
                throw invalid_argument("ERROR: Invalid command\n");
            }
        }
        else if (this->r_path_set == false) {
            this->r_path_set = true;
            this->remote_path = args[i];
            size_t test = 0;
            /* replacing spaces */
            while (( test = this->remote_path.find(" ")) != this->remote_path.npos ) {
                this->remote_path.replace(test, 1, "%20");
            }
        }
        else if (this->l_path_set == false) {
            this->l_path_set = true;
            this->local_path = args[i];
        }
        else {
            throw invalid_argument("ERROR: Too many arguments\n");
        }
    }
    if (this->command == "PUT" and this->file_folder == "?type=file" and l_path_set == false) {
        throw invalid_argument("ERROR: Set local path when using put\n");
    }
    else if (this->cmd_set == false or this->r_path_set == false) {
        throw invalid_argument("ERROR: Set both command and remote path\n");
    }
    size_t index = this->remote_path.find("://");
    /* Retrieving protocol */
    if (index == this->remote_path.npos) {
        index = 0;
    }
    else {
        index += 3;
        this->protocol = this->remote_path.substr(0, index);
        this->remote_path.erase(0, index);
    }
    /* Retrieving port number */
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
    /* Detecting user path */
    index = this->remote_path.find("/");
    if (index == this->remote_path.npos) {
        throw invalid_argument("ERROR: Invalid user specified");
    }
    this->server      = this->remote_path.substr(0, index); // Retrieving hostname
    this->remote_path = this->remote_path.substr(index, this->remote_path.size()); // retrieving path to file/folder on server
    index = this->remote_path.find("/", 1);
    if (index == 1 || (index == (this->remote_path.size()-1) && this->command != "GET") || index == this->remote_path.npos) {
        throw invalid_argument("ERROR: Invalid user or remote path");
    }
    if (this->file_folder == "?type=file" && this->remote_path[this->remote_path.size()-1] == '/') {
        throw invalid_argument("Unable to use file method on directory");
    }
    if (this->command == "PUT" && this->file_folder == "?type=file") {
        ifstream file(this->local_path, ios::binary);
        if (!file.is_open()) {
            throw invalid_argument("ERROR: Invalid local path.");
        }
        file.seekg(0, ios::end);
        this->file_len = file.tellg();
        file.seekg(0, ios::beg);
        this->content = new vector<char>(this->file_len);
        file.read((char*) &(this->content[0][0]), this->file_len);
    }
    if(this->local_path[this->local_path.size()-1] == '/') {
        size_t last_slash, prev_slash;
        string substr = this->remote_path;
        while ((last_slash = substr.find("/")) != substr.npos) {
            prev_slash = last_slash;
            substr = substr.substr(last_slash+1, substr.size() - last_slash);
        }
        last_slash = prev_slash;
    }
}

Arguments::~Arguments() {
    if (this->content != nullptr) {
        delete this->content;
    }
}

bool Response::set_head(string src) {
    this->content_start = src.find("\r\n\r\n");
    if (this->content_start != src.npos) {
        this->content_start += 4;
        this->head = src.substr(0, this->content_start);
        return false;
    }
    return true;
}

bool Response::set_len(string src) {
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

void Response::set_error() {
    size_t idx = 0;
    size_t idx2 = 0;
    if (this->head.substr(9, 6) != "200 OK") {
        idx = this->content.find("\"Error\":\"") + 9;
        idx2 = this->content.find("\"", idx);
        this->error = this->content.substr(idx, idx2-idx);
    }
}

bool Response::write_to_file(Arguments *args) {
    ofstream fout(args->local_path, ios::binary);
    if (!fout.is_open()) {
        return true;
    }
    else {
        cout << args->local_path;
        fout.write(this->content.data(), this->content.size());
        fout.close();
        return false;
    }
}

Response::Response(int fd) {
    char buffer[256];
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
    if (this->set_len(this->head)) {
        throw new invalid_argument("ERROR: No Content-Length specified!");
    }

    byte_read -= this->content_start;
    this->content = this->content.substr(this->content_start);

    while (this->byte_read < this->content_len) {
        len = recv(fd, buffer, 256, 0);
        this->byte_read += len;
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
        }
    }
    this->set_error();
}

string http_request(Arguments *args) {
    string request = args->command + " " + args->remote_path + args->file_folder + " HTTP/1.1\r\n";
    char buf[128];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, 128, "Date: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
    request += buf;
    request += "Accept-Encoding: identity\r\n";
    if (args->command == "GET") {
        request += "Accept: application/octet-stream\r\n";
    }
    else {
        request += "Accept: application/json\r\n";
    }
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
    int sockcl;
    struct sockaddr_in serv_addr;

    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what();
        return 1; // TODO
    }
    catch (exception &e) {
        cerr << "ERROR: Something bad happend...\n";
        return 1; // TODO
    }

    hostent *server = gethostbyname(args->server.c_str());
    if (server == NULL) {
        cerr << "ERROR: Server not found\n";
        delete args;
        return 1; // TODO
    }
    sockcl = socket(AF_INET, SOCK_STREAM, 0);
    if (sockcl <= 0) {
        cerr << "ERROR: Unable to create socket\n";
        delete args;
        return 1; //TODO
    }
    in_addr *address = (in_addr *) server->h_addr;
    char *ip_addres  = inet_ntoa(* address);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(ip_addres);
    serv_addr.sin_port = htons(args->port);
    if (connect(sockcl, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR: Unable to connect\n";
        close(sockcl);
        delete args;
        return 1;
    }
    string request = http_request(args);
    if (send(sockcl, request.data(), request.size(), 0) < 0) {
        cerr << "ERROR: Unable to send message\n";
        close(sockcl);
        delete args;
        return 1; // TODO
    }

    Response resp(sockcl);
    if (resp.error != "") {
        cerr << resp.error;
    }
    else {
        if(args->command == "GET" and args->file_folder == "?type=file") {
            if (resp.write_to_file(args)) {
                cerr << "ERROR: Unable to open local file.";
                close(sockcl);
                delete args;
                return 1;
            }
        }
        else {
            cout << resp.content;
        }
    }
    close(sockcl);
    delete args;
    return 0;
}
