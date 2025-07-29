#include "Server.hpp"

// PART #channel :comment

void Server::handlePart(int fd, std::istringstream &iss) {
    std::string channelName, comment;
    iss >> channelName;
    getline(iss, comment);
    if (!comment.empty() && comment[0] == ' ')
        comment = comment.substr(1);
    if (!comment.empty() && comment[0] == ':')
        comment = comment.substr(1);

    if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        return;
    }

    Channel &channel = _channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    channel.removeMember(fd);

    if (channel.isOperator(fd))
        channel.removeOperator(fd);

    std::string msg = ":" + _clients[fd].getNickname() + " PART " + channelName;
    if (!comment.empty())
        msg += " :" + comment;
    msg += "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    send(fd, msg.c_str(), msg.length(), 0);

    if (channel.getMembers().empty()) {
        _channels.erase(channelName);
    }
}