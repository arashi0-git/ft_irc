#include "Server.hpp"

// KICK #channel user :reason

void Server::handleKick(int fd, std::istringstream &iss) {
    std::string channelName, user, reason;
    iss >> channelName >> user >> reason;

    if (channelName.empty() || user.empty()) {
        sendError(fd, "461 :Not enough parameters");
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        return;
    }

    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        return;
    }

    Channel &channel = channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    int userFd = -1;
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == user) {
            userFd = it->first;
            break;
        }
    }
    if (userFd == -1) {
        sendError(fd, "401 " + user + " :No such nick");
        return;
    }

    if (!channel.hasMember(userFd)) {
        sendError(fd, "441 " + user + " " + channelName + " :They aren't on that channel");
        return;
    }

    if (reason.empty())
        reason = "Kicked by " + _clients[fd].getNickname();
    for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
        std::string msg = ":" + _clients[fd].getNickname() + " KICK " + channelName + " " + user + " :" + reason +"\r\n";
        send(*it, msg.c_str(), msg.length(), 0);
    }

    channel.removeMember(userFd);
    
    if (channel.getMembers().size() == 0) {
        channels.erase(channelName);
    }
}