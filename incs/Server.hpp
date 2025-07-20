#ifndef SEVER_HPP
#define SEVER_HPP

#include "ServerConfig.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <poll.h>

std::vector<struct pollfd> _fds;

class Server {
    private:
        ServerConfig _config;
        int _serverSocket;
    
    public:
        Server(const ServerConfig &config);
        void run();
        void setupSocket();
        void initializePoll();
};

#endif