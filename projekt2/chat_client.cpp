#include "chat_client.h"

bool parse_arguments(int argc, char **argv, string *user, string *IP) {
    if (argc != 5) {
        return true;
    }
    else {
        if (strcmp(argv[1], "-u") == 0) {
            *user  = string(argv[2]);
            *IP    = string(argv[4]);
        }
        else if (strcmp(argv[1],"-i") == 0) {
            *IP    = string(argv[2]);
            *user    = string(argv[4]);
        }
        else {
            return true;
        }
        return false;
    }
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
 * Signal handler. Sends last message to server, if needed and free allocated
 * memory.
 * @param signum [description]
 */
void quit(int signum) {
    if (message_sent) {
        user += " logged out\r\n";
        send(sockfd, user.data(), user.size(), 0);
    }
    if (thread_recv != NULL) {
        (*thread_recv).detach();
        delete thread_recv;
    }
    if (thread_send != NULL) {
        (*thread_send).detach();
        delete thread_send;
    }
    exit(signum);
}

int main(int argc, char **argv) {
    signal(SIGTERM, quit);
    signal(SIGINT, quit);
    struct sockaddr_in serv_addr;
    string IP;
    if (parse_arguments(argc, argv, &user, &IP)) {
        cerr << "Invalid arguments" << endl;
        return 1;
    }
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = inet_addr(IP.c_str());
    serv_addr.sin_port = htons(21011);

    // opening socket
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        cerr << "ERROR: Unable to create socket" << endl;
        return 1;
    }

    if (connect(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
        cerr << "ERROR: Unable to connect" << endl;
        close(sockfd);
        return 1;
    }
    thread_send = new thread(send_messages, sockfd, user);
    thread_recv = new thread(recv_messages, sockfd);
    pause();
    return 0;
}

/**
 * Loads messages from stdin and send them to server.
 * @param sockfd Socket descriptor
 * @param user   User name
 */
void send_messages(int sockfd, string user) {
    string hello_world   = user + " logged in\r\n";
    string goodbye_world = user + " logged out\r\n";
    string message;

    if (send(sockfd, hello_world.data(), hello_world.size(), 0) < 0) {
        cerr << "ERROR: Unable to send message to server" << endl;
    }
    message_sent = true;
    while(true) {
        cin.clear();
        getline(cin, message);
        if (message == "") {
            continue;
        }
        message = user + ": " + message + "\r\n";
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
    char message[256] ={0,};
        while (recv(sockfd, message, 255, 0)) {
            cout << message << flush;
            bzero(message, 256);
        }
}