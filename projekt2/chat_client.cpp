#include <iostream>
#include <string>
#include <vector>
#include <thread>
#include <csignal>

#include <string.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <unistd.h>

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

/// Socket descriptor
int sockfd;

/// Tells if we sent message "user logged in"
bool message_sent = false;

/// User name
string user;

/// Thread for sending messages
thread *thread_send = NULL;

/// Thread for receiving messages
thread *thread_recv = NULL;

/**
 * Parse program atguments.
 * @param  argc Number of arguments.
 * @param  argv Array of program arguments.
 * @param  user In this string will be stored username.
 * @param  IP   In this string will be stored IP addres.
 * @return      Returns true when arguments are invalid, otherwise returns false.
 */
bool parse_arguments(int argc, char **argv, string *user, string *IP) {
    if (argc != 5) {
        return true;
    }
    else {
        if (strcmp(argv[1], "-u") == 0 && strcmp(argv[3],"-i") == 0) {
            *user  = string(argv[2]);
            *IP    = string(argv[4]);
        }
        else if (strcmp(argv[1],"-i") == 0 && strcmp(argv[3],"-u") == 0) {
            *IP    = string(argv[2]);
            *user    = string(argv[4]);
        }
        else {
            return true;
        }
        return false;
    }
}

/**
 * Signal handler. Sends last message to server, if needed and free allocated
 * memory.
 * @param signum [description]
 */
void quit(int signum) {
    if (thread_recv != NULL) {
        (*thread_recv).detach(); // Need to detach thread before calling destructor
        delete thread_recv;      // Deleting thread
    }
    if (thread_send != NULL) {
        (*thread_send).detach(); // Need to detach thread before calling destructor
        delete thread_send;      // Deleting thread
    }
    if (message_sent) {
        user += " logged out\r\n";                 // setting last message
        send(sockfd, user.data(), user.size(), 0); // sending logout info to server
    }
    exit(signum); // Exiting...
}

/**
 * Loads messages from stdin and send them to server.
 * @param sockfd Socket descriptor
 * @param user   User name
 */
void send_messages(int sockfd, string user) {
    string hello_world   = user + " logged in\r\n"; // Log in info message
    string message;                                 // String for storing messages

    // Sending "user: logged in\r\n" to server
    if (send(sockfd, hello_world.data(), hello_world.size(), 0) < 0) {
        cerr << "ERROR: Unable to send message to server" << endl;
    }
    message_sent = true; // First message was probably sent
    // Loading messages from stdin
    while(true) {
        cin.clear();           // Clearing stdin (in case of previous error)
        getline(cin, message); // Reading new message
        if (message == "") {   // Empty message will not be sent
            continue;
        }
        message = user + ": " + message + "\r\n"; // Adding user and CLRF to message
        // Sending message
        if (send(sockfd, message.data(), message.size(), 0) < 0) {
            cerr << "ERROR: Unable to send message to server" << endl;
        }
    }
}

/**
 * Prints messages from server on stdout.
 * @param sockfd Socket descriptor
 */
void recv_messages(int sockfd) {
    char message[256] ={0,}; // buffer for storing messages
    // Waiting for messages from server
    while (recv(sockfd, message, 255, 0)) {
        cout << message << flush; // printing message to stdout.
        bzero(message, 256);      // setting whole content of buffer to 0
    }
}

int main(int argc, char **argv) {
    signal(SIGTERM, quit);        // Expecting SIGTERM signal
    signal(SIGINT, quit);         // Expecting SIGINT signal (ctrl + c)
    struct sockaddr_in serv_addr; // Server addres
    string IP;                    // Server IP
    // Parsing arguments
    if (parse_arguments(argc, argv, &user, &IP)) {
        cerr << "Invalid arguments" << endl;
        return 1;
    }
    // Setting server addres
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    serv_addr.sin_port = htons(21011);

    // opening socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        cerr << "ERROR: Unable to create socket" << endl;
        return 1;
    }

    // connecting to server
    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR: Unable to connect" << endl;
        close(sockfd);
        return 1;
    }
    // Creating new threads and sending them to their functions
    thread_send = new thread(send_messages, sockfd, user);
    thread_recv = new thread(recv_messages, sockfd);
    pause(); // Waiting for signal
    return 0;
}