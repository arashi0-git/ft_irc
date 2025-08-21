#include "Server.hpp"
#include <cstdlib>

bool Server::handleModeOperator(int fd, Channel &channel, const std::string &mode,
                                const std::string &target) {
    const std::string nick = _clients[fd].getNickname();
    std::map<std::string, int>::iterator it = _nickToFd.find(target);
    if (it == _nickToFd.end()) {
        sendError(fd, "401 " + nick + " " + target + " :No such nick/channel");
        return false;
    }

    int targetFd = it->second;
    if (!channel.hasMember(targetFd)) {
        sendError(fd, "441 " + nick + " " + target + " " + channel.getName() + " :They aren't on that channel");
        return false;
    }

    if (mode == "+o") {
        if (channel.isOperator(targetFd)) {
            return true;
        }
        channel.addOperator(targetFd); // set ignores duplicates.
    } else if (mode == "-o") {
        if (!channel.isOperator(targetFd)) {
            return true;
        }
        channel.removeOperator(targetFd); // if fd doesn't exist, nothing happens
    } else {
        char ch = (mode.size() > 1) ? mode[1] : (mode.empty() ? '?' : mode[0]);
        sendError(fd, "472 " + nick + " " + std::string(1, ch) + " :is unknown mode char to me");
        return false;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode +
                      " " + target + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    return true;
}

bool Server::handleModeInviteOnly(int fd, Channel &channel, const std::string &mode) {
    const std::string nick = _clients[fd].getNickname();
    if (mode == "+i") {
        channel.setInviteOnly(true);
    } else if (mode == "-i") {
        channel.setInviteOnly(false);
    } else {
        char ch = (mode.size() > 1) ? mode[1] : (mode.empty() ? '?' : mode[0]);
        sendError(fd, "472 " + nick + " " + std::string(1, ch) + " :is unknown mode char to me");
        return false;
    }
    std::string msg =
        ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    return true;
}

bool Server::handleModeTopic(int fd, Channel &channel, const std::string &mode) {
    const std::string nick = _clients[fd].getNickname();
    if (mode == "+t") {
        channel.setTopicFlag(true);
    } else if (mode == "-t") {
        channel.setTopicFlag(false);
    } else {
        char ch = (mode.size() > 1) ? mode[1] : (mode.empty() ? '?' : mode[0]);
        sendError(fd, "472 " + nick + " " + std::string(1, ch) + " :is unknown mode char to me");
        return false;
    }
    std::string msg =
        ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode + "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    return true;
}

bool Server::handleModeKey(int fd, Channel &channel, const std::string &mode,
                           const std::string &key) {
    const std::string nick = _clients[fd].getNickname();
    if (mode == "+k") {
        if (channel.hasKey()) {
            sendError(fd, "467 " + nick + " " + channel.getName() + " :Channel key already set");
            return false;
        }
        if (key.empty()) {
            sendError(fd, "461 " + nick + " MODE :Not enough parameters");
            return false;
        }
        channel.setKey(key);
    } else if (mode == "-k") {
        channel.removeKey();
    } else {
        char ch = (mode.size() > 1) ? mode[1] : (mode.empty() ? '?' : mode[0]);
        sendError(fd, "472 " + nick + " " + std::string(1, ch) + " :is unknown mode char to me");
        return false;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode;
    if (mode == "+k")
        msg += " *";
    msg += "\r\n";

    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    return true;
}

bool Server::handleModeLimit(int fd, Channel &channel, const std::string &mode,
                             const std::string &limit) {
    const std::string nick = _clients[fd].getNickname();
    if (mode == "+l") {
        if (limit.empty() || !isNumeric(limit)) {
            sendError(fd, "461 " + nick + " MODE :Not enough parameters");
            return false;
        }
        channel.setLimit(std::atoi(limit.c_str()));
    } else if (mode == "-l") {
        channel.removeLimit();
    } else {
        char ch = (mode.size() > 1) ? mode[1] : (mode.empty() ? '?' : mode[0]);
        sendError(fd, "472 " + nick + " " + std::string(1, ch) + " :is unknown mode char to me");
        return false;
    }

    std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channel.getName() + " " + mode;
    if (mode == "+l")
        msg += " " + limit;
    msg += "\r\n";
    for (std::set<int>::iterator it = channel.getMembers().begin();
         it != channel.getMembers().end(); ++it) {
        send(*it, msg.c_str(), msg.length(), 0);
    }
    return true;
}
