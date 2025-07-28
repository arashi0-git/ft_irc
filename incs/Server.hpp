#ifndef SERVER_HPP
#define SERVER_HPP

#include "ServerConfig.hpp"
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <stdexcept>
#include <poll.h>
#include <map>
#include <string>
#include <vector>
#include <sstream>
#include "Client.hpp"
#include "Channel.hpp"

class Server {
    private:
        ServerConfig _config;
        int _serverSocket;
        std::vector<struct pollfd> _fds;
        std::map<int, std::string> clientBuffer;
        std::map<std::string, Channel> channels;
    
    public:
        Server(const ServerConfig &config);
        void run();
        void setupSocket();
        void initializePoll();
        void acceptNewClient();
        void handleClient(int fd);
        void disconnectClient(int fd);
        void processCommand(int fd, const std::string &command);
        void handleNick(int fd, std::istringstream &iss);
        void handleUser(int fd, std::istringstream &iss);
        void handleJoin(int fd, std::istringstream &iss);
        void handlePrivMsg(int fd, std::istringstream &iss);
        void handlePass(int fd, std::istringstream &iss);
        void handleKick(int fd, std::istringstream &iss);
        void handleTopic(int fd, std::istringstream &iss);
        void handleMode(int fd, std::istringstream &iss);
        void handleInvite(int fd, std::istringstream &iss);
        void sendError(int fd, const std::string &message);
        void sendWelcome(int fd);
        bool canAuthenticate(const Client &client) const;
};

#endif