#include "Server.hpp"
#include <cstdlib>

void Server::handleModeOperator(int fd, Channel &channel, const std::string &mode,
                                const std::string &target) {
    std::map<std::string, int>::iterator it = _nickToFd.find(target);
    if (it == _nickToFd.end()) {
        sendError(fd, "401 " + target + " :No such nick/channel");
        return;
    }

    int targetFd = it->second;
    if (!channel.hasMember(targetFd)) {
        sendError(fd, "441 " + target + " " + channel.getName() + " :They aren't on that channel");
        return;
    }

    if (mode == "+o") {
        if (channel.isOperator(targetFd)) {
            sendError(fd, "482 " + channel.getName() + " :You're not channel operator");
            return;
        }
        channel.addOperator(targetFd); // set ignores duplicates.
    } else if (mode == "-o") {
        if (!channel.isOperator(targetFd)) {
            sendError(fd, "482 " + channel.getName() + " :You're not channel operator");
            return;
        }
        channel.removeOperator(targetFd); // if fd doesn't exist, nothing happens
    } else {
        sendError(fd, "472 " + mode + ":is unknown mode char to me");
        return;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode +
                      " " + target + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}

void Server::handleModeInviteOnly(int fd, Channel &channel, const std::string &mode) {
    if (mode == "+i") {
        channel.setInviteOnly(true);
    } else if (mode == "-i") {
        channel.setInviteOnly(false);
    } else {
        sendError(fd, "472 " + mode + ":is unknown mode char to me");
        return;
    }
    std::string msg =
        ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}

void Server::handleModeTopic(int fd, Channel &channel, const std::string &mode) {
    if (mode == "+t") {
        channel.setTopicFlag(true);
    } else if (mode == "-t") {
        channel.setTopicFlag(false);
    } else {
        sendError(fd, "472 " + mode + ":is unknown mode char to me");
        return;
    }
    std::string msg =
        ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}

void Server::handleModeKey(int fd, Channel &channel, const std::string &mode,
                           const std::string &key) {
    if (mode == "+k") {
        if (channel.hasKey()) {
            sendError(fd, "467 " + channel.getName() + " :Channel key already set");
            return;
        }
        if (key.empty()) {
            sendError(fd, "461 " + channel.getName() + " :Not enough parameters");
            return;
        }
        channel.setKey(key);
    } else if (mode == "-k") {
        channel.removeKey();
    } else {
        sendError(fd, "472 " + mode + " :is unknown mode char to me");
        return;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode;
    if (mode == "+k")
        msg += " *";
    msg += "\r\n";

    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}

void Server::handleModeLimit(int fd, Channel &channel, const std::string &mode,
                             const std::string &limit) {
    if (mode == "+l") {
        if (limit.empty() || !isNumeric(limit)) {
            sendError(fd, "461 " + channel.getName() + " :Not enough parameters");
            return;
        }
        channel.setLimit(std::atoi(limit.c_str()));
    } else if (mode == "-l") {
        channel.removeLimit();
    } else {
        sendError(fd, "472 " + mode + " :is unknown mode char to me");
        return;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode;
    if (mode == "+l")
        msg += " " + limit;
    msg += "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
}
