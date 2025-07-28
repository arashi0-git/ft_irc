#include "Server.hpp"

// TOPIC #channel :new topic

void Server::handleTopic(int fd, std::istringstream &iss) {
    std::string channelName, topic;
    iss >> channelName;
    getline(iss, topic);

    if (channelName.empty()) {
        sendError(fd, "461 :Not enough parameters");
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        return;
    }

    
}