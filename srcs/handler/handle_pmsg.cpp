#include "Server.hpp"

// PRIVMSG #channel :Hello everyone
// PRIVMSG Alice :Hey, are you there?

void Server::handlePrivMsg(int fd, std::istringstream &iss) {
    std::string target, message;
    iss >> target;
    std::getline(iss, message);

    if (!message.empty() && message[0] == ' ')
        message = message.substr(1);

    if (!message.empty() && message[0] == ':')
        message = message.substr(1);

    if (target.empty()) {
        sendError(fd, "411 :No recipient given");
        logCommand("PRIVMSG", fd, false);
        return;
    }

    if (message.empty()) {
        sendError(fd, "412 :No text to send");
        logCommand("PRIVMSG", fd, false);
        return;
    }

    if (target[0] == '#') {
        if (channels.find(target) == channels.end()) {
            sendError(fd, "401 " + target + " :No such channel");
            logCommand("PRIVMSG", fd, false);
            return;
        }
        Channel &channel = channels[target];
        if (!channel.hasMember(fd)) {
            sendError(fd, "404 :Cannot send to channel");
            logCommand("PRIVMSG", fd, false);
            return;
        }
        std::string sender = _clients[fd].getNickname();
        std::string prefix = ":" + sender + " PRIVMSG " + target + " :" + message + "\r\n";

        for (std::set<int>::iterator it = channel.getMembers().begin();
            it != channel.getMembers().end(); ++it) {
            if (*it != fd) {
                send(*it, prefix.c_str(), prefix.length(), 0);
            }
        }
        logCommand("PRIVMSG", fd, true);
    } else {
        int targetFd = -1;
        for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
            if (it->second.getNickname() == target) {
                targetFd = it->first;
                break;
            }
        }
        if (targetFd == -1) {
            sendError(fd, "401 " + target + " :No such nick");
            logCommand("PRIVMSG", fd, false);
            return;
        }
        std::string sender = _clients[fd].getNickname();
        std::string prefix = ":" + sender + " PRIVMSG " + target + " :" + message + "\r\n";
        send(targetFd, prefix.c_str(), prefix.length(), 0);
        logCommand("PRIVMSG", fd, true);
    }
}
