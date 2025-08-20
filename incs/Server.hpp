#ifndef SERVER_HPP
#define SERVER_HPP

#include "Channel.hpp"
#include "Client.hpp"
#include "ServerConfig.hpp"
#include <algorithm>
#include <cstring>
#include <fcntl.h>
#include <map>
#include <netinet/in.h>
#include <poll.h>
#include <sstream>
#include <stdexcept>
#include <string>
#include <sys/socket.h>
#include <unistd.h>
#include <vector>
extern bool g_signal;

class Server {
  private:
    ServerConfig _config;
    int _serverSocket;
    std::vector<struct pollfd> _fds;
    std::map<int, std::string> clientBuffer;
    std::map<std::string, Channel> channels;
    std::map<std::string, int> _nickToFd;
    std::map<int, Client> _clients;
    std::string _serverName;

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
    void handleMessageVerb(int fd, std::istringstream &iss, const std::string &verb);
    void handlePass(int fd, std::istringstream &iss);
    void handleKick(int fd, std::istringstream &iss);
    void handleTopic(int fd, std::istringstream &iss);
    void handleMode(int fd, std::istringstream &iss);
    void handleInvite(int fd, std::istringstream &iss);
    void handlePart(int fd, std::istringstream &iss);
    void handleHelp(int fd, std::istringstream &iss);
    void handlePing(int fd, std::istringstream &iss);
    void handleWho(int fd, std::istringstream &iss);
    void sendError(int fd, const std::string &message);
    void sendWelcome(int fd);
    bool canAuthenticate(const Client &client) const;
    void handleModeOperator(int fd, Channel &channel, const std::string &mode,
                            const std::string &target);
    void handleModeInviteOnly(int fd, Channel &channel, const std::string &mode);
    void handleModeTopic(int fd, Channel &channel, const std::string &mode);
    void handleModeKey(int fd, Channel &channel, const std::string &mode, const std::string &key);
    void handleModeLimit(int fd, Channel &channel, const std::string &mode,
                         const std::string &limit);
    bool isNumeric(const std::string &str) const;
    void sendNamesReply(int fd, const Channel &channel);
    void logCommand(const std::string &command, int fd, bool success);
    void logMessage(const std::string &message);
};

void signal_handler(int signal);
#endif
