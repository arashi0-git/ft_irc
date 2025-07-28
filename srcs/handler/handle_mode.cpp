#include "Server.hpp"

// MODE #channel +/-o target

void Sever::handleMode(int fd, std::istringstream &iss) {
    std::string channelName, mode, target;
    iss >> channelName >> mode >> target;

    if (channelName.empty() || mode.empty() || target.empty()) {
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

    Channel &channel = _channels[channelName];

    if (!channel.hasMember(fd)){
        sendError(fd, "482 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    int targetFd = -1;
    for (std::set<int>::iterator it = channel.getMember().begin(); it != channel.getMember().end(); ++it) {
        if (channel.getMember(*it) == target)
            targetFd = *it;
        else
            sendError(fd, "482 " + target + " :No such a member in channel");
    }
    if (mode == '+o') {
        channel.addOperator(targetFd);
        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " " + mode + " " + target + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
    else if (mode == '-o') {
        channel.removeOperator(targetFd);
        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " " + mode + " " + target + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
    else
        sendError(fd, "476 " + _clients.getNickname(fd) + " " + mode + " :No such a mode");
}