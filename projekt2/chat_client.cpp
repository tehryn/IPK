#include "chat_client.h"
/** @var mutex vector of mutexes that provide datarace*/
std::vector<std::mutex *> mutex_vec;

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
int sockfd;
string user;
void quit(int signum) {
    user += " logged out\r\n";
    send(sockfd, user.data(), user.size(), 0);
    exit(signum);
}

int main(int argc, char **argv) {
    signal(SIGTERM, quit);
    signal(SIGINT, quit);
    const int mutex_number = END - SOCK;
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

    mutex_vec.resize(mutex_number); // resize vector of mutexes
    for(unsigned i = 0; i < mutex_number; i++){
        mutex *new_mutex = new std::mutex();
        mutex_vec[i] = new_mutex;
        (*(mutex_vec[i])).lock();
    }
    thread thread_send(send_messages, sockfd, user);
    thread thread_recv(recv_messages, sockfd);
    thread_recv.join();
    thread_send.join();
    return 0;
}

void send_messages(int sockfd, string user) {
    string hello_world   = user + " logged in\r\n";
    string goodbye_world = user + " logged out\r\n";
    string message;
//    DEBUG_LINE("SENDER: sending hello_world");

    if (send(sockfd, hello_world.data(), hello_world.size(), 0) < 0) {
        cerr << "ERROR: Unable to send message to server" << endl;
    }
//    (*(mutex_vec[SOCK])).unlock();
//    DEBUG_LINE("SENDER: message sent and SOCK unlocked");
    while(true) {
        cin.clear();
//        DEBUG_LINE("SENDER: Reading STDIN");
        getline(cin, message);
//        DEBUG_LINE("SENDER: Read: ", message);
        if (message == "") {
            continue;
        }
        message = user + ": " + message + "\r\n";
//        DEBUG_LINE("SENDER: Waiting to sent message");
//        (*(mutex_vec[SOCK])).lock();
//        DEBUG_LINE("SENDER: Sending message");
        if (send(sockfd, message.data(), message.size(), 0) < 0) {
            cerr << "ERROR: Unable to send message to server" << endl;
        }
//        DEBUG_LINE("SENDER: Unlocking...");
//        (*(mutex_vec[SOCK])).unlock();
    }


}
void recv_messages(int sockfd) {
    char message[256] ={0,};
//    DEBUG_LINE("RECV: Waiting on 1st lock...");
//    (*(mutex_vec[SOCK])).lock();
//    DEBUG_LINE("RECV: Waiting to message");
//    while (true) {
//        if (errno == EWOULDBLOCK) {
//            (*(mutex_vec[SOCK])).unlock();
//            sleep(1);
//            (*(mutex_vec[SOCK])).lock();
//            continue;
//        }
//        else if (errno) {
//            break;
//        }
//        DEBUG_LINE("RECV: Here comes the message:");
        while (recv(sockfd, message, 256, 0)) {
            cout << message << flush;
            bzero(message, 256);
        }
//        DEBUG_LINE("RECV: Unlocking....");
//        (*(mutex_vec[SOCK])).unlock();
//        DEBUG_LINE("RECV: Waiting....");
//        (*(mutex_vec[SOCK])).lock();
//        DEBUG_LINE("RECV: Reading message");
//    }
//    DEBUG_LINE("Out of cycle");
//    (*(mutex_vec[SOCK])).unlock();
}