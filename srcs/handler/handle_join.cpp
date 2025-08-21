#include "Server.hpp"

// irssi: /QUOTE JOIN #pen

void Server::sendNamesReply(int fd, const Channel &channel) {
    std::string reply =
        ":" + _serverName + " 353 " + _clients[fd].getNickname() + " = " + channel.getName() + " :";
    for (std::set<int>::const_iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        std::string nick = _clients[*it].getNickname();
        if (channel.isOperator(*it))
            reply += "@" + nick + " ";
        else
            reply += nick + " ";
    }
    reply += "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);

    std::string endreply = ":" + _serverName + " 366 " + _clients[fd].getNickname() + " " +
                           channel.getName() + " :End of /NAMES list.\r\n";
    send(fd, endreply.c_str(), endreply.length(), 0);
}

void Server::handleJoin(int fd, std::istringstream &iss) {
    std::string channelName, key;
    iss >> channelName >> key;

    if (!_clients[fd].isAuthenticated()) {
        sendError(fd, "451 " + _clients[fd].getNickname() + " :You have not registered");
        logCommand("JOIN", fd, false);
        return;
    }

    if (channelName.empty()) {
        sendError(fd, "461 " + _clients[fd].getNickname() + " JOIN :Not enough parameters");
        logCommand("JOIN", fd, false);
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + _clients[fd].getNickname() + " " + channelName +
                          " :Invalid channel name (Usage: JOIN <#channel>)");
        logCommand("JOIN", fd, false);
        return;
    }

    if (_channels.find(channelName) == _channels.end()) {
        _channels[channelName] = Channel(channelName);
    }
    Channel &channel = _channels[channelName];

    if (channel.hasMember(fd)) {
        sendError(fd, "442 " + _clients[fd].getNickname() + " " + channelName +
                          " :You are already on that channel");
        logCommand("JOIN", fd, false);
        return;
    }

    if (channel.hasLimit() && channel.getMembers().size() >= channel.getLimit()) {
        sendError(fd, "471 " + _clients[fd].getNickname() + " " + channelName +
                          " :Cannot join channel (+l)");
        logCommand("JOIN", fd, false);
        return;
    }

    if (channel.isInviteOnly() && !channel.isInvited(fd)) {
        sendError(fd, "473 " + _clients[fd].getNickname() + " " + channelName +
                          " :Cannot join channel (+i)");
        logCommand("JOIN", fd, false);
        return;
    }

    if (channel.hasKey() && channel.getKey() != key) {
        sendError(fd, "475 " + _clients[fd].getNickname() + " " + channelName +
                          " :Cannot join channel (+k)");
        logCommand("JOIN", fd, false);
        return;
    }

    channel.addMember(fd);

    if (!channel.hasOperator() || channel.getMembers().size() == 1) {
        channel.addOperator(fd);
    }

    std::string nick = _clients[fd].getNickname();

    // hostname
    // const std::string nick = _clients[fd].getNickname() + "!" + _clients[fd].getUsername() + "@"
    // + _clients[fd].getHostname();

    std::string reply = ":" + nick + " JOIN :" + channelName + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);

    if (channel.getTopic().empty()) {
        std::string reply331 =
            ":" + _serverName + " 331 " + nick + " " + channelName + " :No topic is set\r\n";
        send(fd, reply331.c_str(), reply331.length(), 0);
    } else {
        std::string reply332 = ":" + _serverName + " 332 " + nick + " " + channelName + " :" +
                               channel.getTopic() + "\r\n";
        send(fd, reply332.c_str(), reply332.length(), 0);
    }
    sendNamesReply(fd, channel);
    logCommand("JOIN", fd, true);
}
