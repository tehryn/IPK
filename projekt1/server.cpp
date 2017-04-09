#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include <fstream>
#include <csignal>

#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/stat.h>
#include <dirent.h>

using namespace std;

/**
 * Class that stores program setings
 */
class Arguments {
private:
    /// Identifies if root folder was set.
    bool set_root_folder = false;
    /// Identifies if port number was set.
    bool set_port        = false;
public:
    /// Path to root folder
    string   root_folder = ".";
    /// Port number
    unsigned port        = 6677;
    /**
     * Construct an Argument class. Throws invalid_argument exception on error.
     * @param argc Number of arguments.
     * @param argv Array of arguments.
     */
    Arguments(int argc, char **argv);
};

/**
 * Class for parsing Requests
 */
class Request {
private:
    /// Index where content of message stars.
    size_t content_start = 1;
    /// Number of bytes that was read from content.
    size_t byte_read     = 0;
    /**
     * Retrieve request header.
     * @param  src String where header is.
     * @return     True when there is no header, false on succes.
     */
    bool set_head(string src);
    /**
     * Retrieve command from HTTP header.
     * @param  src HTTP header
     * @return     true on error, false on succes.
     */
    bool set_command(string src);
    /**
     * Retrieve path to file/directory from HTTP header
     * @param  src HTTP header
     * @return     true on succes, false on error.
     */
    bool set_path(string src);
    /**
     * Retrieve value of Content-Length from HTTP header.
     * @param  src HTTP header
     * @return     true on error, false on succes.
     */
    bool set_len(string src);
    /**
     * Procces loaded data.
     * @return true when data are invalid, false on succes.
     */
    bool proc_ld_data();
public:
    /// Stores value of command (GET, PUT or DELETE).
    string command       = "";
    /// Stores value of path to file/directory.
    string path          = "";
    /// Stores HTTP head.
    string head          = "";
    /// Lenght of content.
    size_t content_len   =  0;
    /// Content itself.
    string content       = "";
    /// Tells if we are working with file (true) or folder (false).
    bool   is_file       = false;
    /**
     * Loads data from socket.
     * @param fd Socket descriptor.
     */
    void ld_data(int fd);
};

bool Request::set_head(string src) {
    this->content_start = src.find("\r\n\r\n"); // end of http head
    if (this->content_start != src.npos) {
        this->content_start += 4;
        this->head = src.substr(0, this->content_start); // stores head
        return false;
    }
    return true;
}

bool Request::set_command(string src) {
    size_t idx = src.find(' '); // command is first part of head, so space
    if (idx != src.npos) {      // seperates it from rest of head.
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
            idx = this->path.size() - 12; // 12 is lenght of "?type=folder"
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
            size_t test = 0;
            /* Replacing %20 back for spaces */
            while (( test = this->path.find("%20")) != this->path.npos ) {
                this->path.replace(test, 3, " ");
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
    idx += 16; // 16 is lenght of "Content-Length: "
    try {
        this->content_len = stoi(src.substr(idx, idx2-idx));
    } catch (exception &e) {
        return true;
    }
    return false;
}

void Request::ld_data(int fd) {
    char buffer[256]; // for storing bytes from socket
    unsigned len;     // for storing number of loaded bytes
    while ((len = recv(fd, buffer, 256, 0)) > 0) {
        this->byte_read += len;
        /* Load data to string, no worry of zero character when using push_back*/
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
        }
        /* Check if we already loaded http head */
        if (this->set_head(this->content) == false) {
            break; // we do not know how big socket is, neet to parse head first
        }
    }
    /* Parsing head */
    if (this->proc_ld_data() == true) {
        throw new invalid_argument("ERROR: Socket has invalid header!");
    }
    /* We probably loaded some bytes from content... but how many?? */
    byte_read -= this->content_start;
    /* Seperating contetn from header */
    this->content = this->content.substr(this->content_start);
    /* Reading rest of socket */
    while (this->byte_read < this->content_len) {
        len = recv(fd, buffer, 256, 0);
        this->byte_read += len;
        for (unsigned i = 0; i < len; i++) {
            this->content.push_back(buffer[i]);
        }
    }
}

bool Request::proc_ld_data() {
    if (set_command(this->head)) { // setting command
        return true;
    }
    else if (set_path(this->head)){ // setting path
        return true;
    }
    if (this->command == "PUT" && this->is_file ) { // setting content_len
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
            /* We will need to join 2 strings, so this will evoid of multiple '/' in path */
            if (this->root_folder[this->root_folder.size() - 1] == '/') {
                this->root_folder.resize(this->root_folder.size() - 1);
            }
        }
        else if (args[i] == "-p" and this->set_port == false and (i+1) < argc) {
             this->set_port    = true;
             this->port = stoi(args[++i]);
        }
        else {
            throw invalid_argument("ERROR: Arguments of program are invalid");
        }
    }
}

string put_on_file(Request *req, string path) {
    string message = "";
    DIR* dir = opendir(path.c_str());
    if (dir) {
        message = "Not a file.";
        closedir(dir);
        return message;
    } else if( access(path.c_str(), F_OK) != -1 ) {
        message = "Already exists.";
        return message;
    }
    ofstream fout(path, ios::binary);
    if (fout.is_open() == false) {
        message = "File not found.";
        return message;
    }
    else {
        fout.write(req->content.data(), req->content.size());
        message = "200 OK";
        fout.close();
        return message;
    }
}

string put_on_folder(string path) {
    string message;
    if (mkdir(path.c_str(), 0700) < 0) {
        if (errno == ENOENT) {
            message = "Directory not found.";
        }
        else if (errno == ENOTDIR) {
            message =  "Not a directory.";
        }
        else if (errno == EEXIST) {
            message =  "Already exists.";
        }
        else {
            message = "Unknown error.";
        }
    }
    else {
        message = "200 OK";
    }
    return message;
}

string get_on_foleder (string root_path, vector<char> **content) {
    string message = "";
    DIR *dir;
    struct dirent *ent;
    *content = new vector<char>(0);
    if ((dir = opendir(root_path.c_str())) != nullptr) {
        unsigned size = 0;
        while ((ent = readdir(dir)) != nullptr) {
            if (ent->d_name[0] == '.') {
                continue;
            }
            size = strlen(ent->d_name);
            for (unsigned i = 0; i<size; i++) {
                (*content)->push_back(ent->d_name[i]);
            }
            (*content)->push_back('\n');
        }
        message = "200 OK";
        closedir(dir);
    }
    else if (errno == ENOENT){
        message = "Directory not found.";
    }
    else if (errno == ENOTDIR) {
        message = "Not a directory.";
    }
    else {
        message = "Unknown error.";
    }
    return message;
}

string get_on_file (string path, vector<char> **content) {
    string message = "";
    DIR* dir = opendir(path.c_str());
    if (dir) {
        message = "Not a file.";
        closedir(dir);
        return message;
    }
    size_t len;
    ifstream file(path.c_str(), ios::binary);
    if (file.is_open()) {
        file.seekg(0, ios::end);
        len = file.tellg();
        file.seekg(0, ios::beg);
        *content = new vector<char>(len);
        file.read((char*) (&content[0][0][0]), len); // You need to accept it and keep going...
        message = "200 OK";
    }
    else {
        message = "File not found.";
    }
    return message;
}

string del_on_file(string path) {
    string message = "";
    if (unlink(path.c_str()) == 0) {
        message = "200 OK";
    }
    else if (errno == ENOENT || errno == ENOTDIR){
        message = "File not found.";
    }
    else if (errno == EISDIR) {
        message = "Not a file.";
    }
    else {
        message = "Unknown error.";
    }
    return message;
}

string del_on_folder(string path) {
    string message = "";
    if (rmdir(path.c_str()) == 0) {
        message = "200 OK";
    }
    else if (errno == ENOENT){
        message = "Directory not found.";
    }
    else if (errno == ENOTDIR) {
        message = "Not a directory.";
    }
    else if (errno == ENOTEMPTY) {
        message = "Directory not empty.";
    }
    else {
        message = "Unknown error.";
    }
    return message;
}

string create_response(Request *req, string root_path){
    string message;
    string error = "";
    char buf[128];
    time_t now = time(0);
    struct tm tm = *gmtime(&now);
    strftime(buf, 128, "\r\nDate: %a, %d %b %Y %H:%M:%S %Z\r\n", &tm);
    string response = buf;
    vector<char> *content = nullptr;
    string path = root_path + req->path;
    if (req->command == "PUT") {
        if (req->is_file) {
            message = put_on_file(req, path);
            if (message != "200 OK") {
                error  = "{\n\t\"Error\":\"";
                error += message;
                error += "\"\n}";
            }
        }
        else {
            message = put_on_folder(path);
            if (message != "200 OK") {
                if (message != "200 OK") {
                    error  = "{\n\t\"Error\":\"";
                    error += message;
                    error += "\"\n}";
                }
            }
        }
    }
    else if (req->command == "GET") {
        if (req->is_file) {
            message = get_on_file(path, &content);
            if (message != "200 OK") {
                error  = "{\n\t\"Error\":\"";
                error += message;
                error += "\"\n}";
            }
        }
        else {
            message = get_on_foleder(path, &content);
            if (message != "200 OK") {
                error  = "{\n\t\"Error\":\"";
                error += message;
                error += "\"\n}";
            }
        }
    }
    else if (req->command == "DELETE") {
        if (req->is_file) {
            message = del_on_file(path);
            if (message != "200 OK") {
                error  = "{\n\t\"Error\":\"";
                error += message;
                error += "\"\n}";
            }
        }
        else {
            message = del_on_folder(path);
            if (message != "200 OK") {
                error  = "{\n\t\"Error\":\"";
                error += message;
                error += "\"\n}";
            }
        }
    }
    else {
        message = "";
        return message;
    }
    if (error != "") {
        if (error.find("not found") == error.npos) {
            message = "400 Bad Request";
        }
        else {
            message = "404 Not Found";
        }
    }
    if (req->command == "GET") {
        response += "Content-Type: application/octet-stream\r\n";
    }
    else {
        response += "Content-Type: application/json\r\n";
    }
    response += "Content-Length: ";
    if (content != nullptr) {
        response += to_string((content->size() + error.size()));
    }
    else {
        response += to_string(error.size()); // Content-Length 0
    }
    response += "\r\nContent-Encoding: identity\r\n";
    response = "HTTP/1.1 " + message + response + "\r\n";
    if (content != nullptr) {
        for (size_t i = 0; i < content->size(); i++) {
            response += content[0][i];
        }
        delete content;
    }
    response += error;
    return response;
}

Request *req;
Arguments *args;
int sockfd, sockcomm;
void quit(int signum) {
    if (req != nullptr) {
        delete req;
        close(sockcomm);
    }
    if (args != nullptr) {
        delete args;
        close(sockfd);
    }
    exit(signum);
}

int main(int argc, char **argv) {
    signal(SIGTERM, quit);
    signal(SIGINT, quit);
    int sockfd,              // Soclet descriptor
        sockcomm,            // Socket descriptor
        portno; // Port number
    socklen_t clilen;        // Size of clien's addres
    struct sockaddr_in serv_addr = {},
                       cli_addr  = {};

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        cerr << "ERROR: Could not open socket" << endl;
        return 1; // TODO
    }
    try {
        args = new Arguments(argc, argv);
    } catch (invalid_argument& e) {
        cerr << e.what() << endl;
        return -1; // TODO
    }
    portno = args->port;
    int optval = 1; // TODO k cemu to je?
    setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR,(const void *)&optval , sizeof(int));

    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family      = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port        = htons(portno);

    if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR: Unable to bind" << endl;
        return 1; // TODO
    }

    if (listen(sockfd, 1) < 0) {
        cerr << "ERROR: Unable to listen" << endl;
        return 1;
    }
    string message;
    while (true) {
        req = new Request;
        clilen = sizeof(cli_addr);
        sockcomm = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (sockcomm > 0) {
            req->ld_data(sockcomm);
            message = create_response(req, args->root_folder);
            if (send(sockcomm, message.data(), message.size(), 0) < 0) {
                cerr << "ERROR: Unable to send message." << endl;
                return 1;
            }
        }
        delete req;
        req = nullptr;
        close(sockcomm);
    }
    delete args;
    return 0;
}