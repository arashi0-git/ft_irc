#include "Server.hpp"

// INVITE <nickname> <channel>

void Server::handleInvite(int fd, std::istringstream &iss) {
    std::string nickName, channelName;
    iss >> nickName >> channelName;

    if (nickName.empty() || channelName.empty()) {
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

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    std::map<std::string, int>::iterator it = _nickToFd.find(nickName);
    if (it == _nickToFd.end()) {
        sendError(fd, "401 " + nickName + " :No such nick");
        return;
    }

    if (channel.hasMember(it->second)) {
        sendError(fd, "443 " + nickName + " " +channelName + " :is already on channel");
        return;
    }

    channel.invite(it->second);
    std::string msg = ":" + _clients[fd].getNickname() + " INVITE " + nickName + " :" + channelName + "\r\n";
    send(it->second, msg.c_str(), msg.length(), 0);
}