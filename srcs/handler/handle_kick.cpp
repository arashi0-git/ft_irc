#include "Server.hpp"

// KICK #channel user :reason

void Server::handleKick(int fd, std::istringstream &iss) {
    std::string channelName, user, reason;
    iss >> channelName >> user;
    getline(iss, reason);

    if (!reason.empty() && reason[0] == ' ') {
        reason.erase(0, 1);
    }
    if (!reason.empty() && reason[0] == ':') {
        reason.erase(0, 1);
    }

    std::string sender = _clients[fd].getNickname();
    if (sender.empty())
        sender = "*";

    if (channelName.empty() || user.empty()) {
        sendError(fd, "461 " + sender + " KICK :Not enough parameters");
        logCommand("KICK", fd, false);
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + sender + " " + channelName +
                          " :Invalid channel name (Usage: JOIN <#channel>)");
        logCommand("KICK", fd, false);
        return;
    }

    if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + sender + " " + channelName + " :No such channel");
        logCommand("KICK", fd, false);
        return;
    }

    Channel &channel = _channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + sender + " " + channelName + " :You're not on that channel");
        logCommand("KICK", fd, false);
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + sender + " " + channelName + " :You're not channel operator");
        logCommand("KICK", fd, false);
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
        sendError(fd, "401 " + sender + " " + user + " :No such nick/channel");
        logCommand("KICK", fd, false);
        return;
    }

    if (!channel.hasMember(userFd)) {
        sendError(fd, "441 " + sender + " " + user + " " + channelName +
                          " :They aren't on that channel");
        logCommand("KICK", fd, false);
        return;
    }

    if (reason.empty())
        reason = "Kicked by " + _clients[fd].getNickname();
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        std::string msg = ":" + _clients[fd].getNickname() + " KICK " + channelName + " " + user +
                          " :" + reason + "\r\n";
        send(*it, msg.c_str(), msg.length(), 0);
    }

    channel.removeMember(userFd);
    channel.removeInvite(userFd);
    channel.removeOperator(userFd);

    if (channel.getMembers().size() == 0) {
        _channels.erase(channelName);
    }
    logCommand("KICK", fd, true);
}
