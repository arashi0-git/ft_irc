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

    if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        return;
    }

    if (!_channels[channelName].hasMember(fd)){
        sendError(fd, "482 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    if (topic.empty()) {
        
    }
}