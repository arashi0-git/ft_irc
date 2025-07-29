#include "Server.hpp"

// MODE #channel +/-o target
// MODE #channel +i/-i

void Server::handleMode(int fd, std::istringstream &iss) {
    std::string channelName, mode, target;
    iss >> channelName >> mode >> target;

    if (channelName.empty() || mode.empty()) {
        sendError(fd, "461 :Not enough parameters");
        return;
    }

    bool modeRequiresTarget = (mode == "+o" || mode == "-o");

    if (modeRequiresTarget && target.empty()) {
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
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    std::map<std::string, int>::iterator it = _nickToFd.find(target);
    if (it == _nickToFd.end()) {
        sendError(fd, "401 " + target + " :No such nick");
        return;
    }

    int targetFd = it->second;
    if (!channel.hasMember(targetFd)) {
        sendError(fd, "441 " + target + " " + channelName + " :They aren't on that channel");
        return;
    }

    if (mode.size() != 2 || (mode != "+o" && mode != "-o")) {
        sendError(fd, "472 " + (mode.size() > 1 ? std::string(1, mode[1]) : mode) + " :is unknown mode char to me");
        return;
    }

    if (mode.size() == 2 && mode == '+i') {
        channel.setInviteOnly(true);

        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " +i\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
    else if (mode.size() == 2 && mode == '-i') {
        channel.setInviteOnly(false);
        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " -i\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
    else if (mode.size() == 2 && mode == '+o') {
        if (channel.isOperator(targetFd)) {
            sendError(fd, "482 " + channelName + " :Target is already an operator");
            return;
        }
        channel.addOperator(targetFd);
        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " " + mode + " " + target + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
    else if (mode.size() == 2 && mode == '-o') {
        if (!channel.isOperator(targetFd)) {
            sendError(fd, "482 " + channelName + " :Target is not an operator");
            return;
        }
        channel.removeOperator(targetFd);
        std::string msg = ":" + _clients[fd].getNickname() + " MODE " + channelName + " " + mode + " " + target + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
    }
}