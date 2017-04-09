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

int main(int argc, char **argv) {
    const int mutex_number = END - SOCK;
    struct sockaddr_in serv_addr;
    int sockfd;
    string IP, user;
    if (parse_arguments(argc, argv, &user, &IP)) {
        cerr << "Invalid arguments" << endl;
        return 1;
    }
    DEBUG_LINE(IP, user);
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
    (*(mutex_vec[SOCK])).unlock();
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


    (*(mutex_vec[SOCK])).lock();
    if (send(sockfd, hello_world.data(), hello_world.size(), 0) < 0) {
        cerr << "ERROR: Unable to send message to server" << endl;
    }
    (*(mutex_vec[SOCK])).unlock();

    while(true) {
        cin.clear();
        getline(cin, message);
        message = user + ": " + message + "\r\n";
        (*(mutex_vec[SOCK])).lock();
        if (send(sockfd, message.data(), message.size(), 0) < 0) {
            cerr << "ERROR: Unable to send message to server" << endl;
        }
        (*(mutex_vec[SOCK])).unlock();
    }


}
void recv_messages(int sockfd) {
    cout << sockfd << endl;
}