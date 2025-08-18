#include "Server.hpp"

// toCheck
bool Server::canAuthenticate(const Client &client) const {
    return client.hasPasswordReceived() && !client.getNickname().empty() &&
           !client.getUsername().empty() && !client.getRealname().empty();
}

void Server::sendWelcome(int fd) {
    const Client &client = _clients[fd];
    std::string msg = ":" + _serverName + " 001 " + client.getNickname() +
                      " : Welcome to the IRC server " + client.getNickname() + "\r\n";
    send(fd, msg.c_str(), msg.length(), 0);
}

void Server::handlePass(int fd, std::istringstream &iss) {
    std::string password;
    iss >> password;

    if (password.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("PASS", fd, false);
        return;
    }

    if (_clients[fd].hasPasswordReceived()) {
        sendError(fd, "462 :You may not reregister");
        logCommand("PASS", fd, false);
        return;
    }

    if (password != _config.getPassword()) {
        sendError(fd, "464 :Password incorrect");
        logCommand("PASS", fd, false);
        return;
    }

    _clients[fd].setPasswordReceived(true);
    logCommand("PASS", fd, true);

    if (canAuthenticate(_clients[fd])) {
        _clients[fd].setAuthenticated(true);
        sendWelcome(fd);
    }
}

void Server::handleUser(int fd, std::istringstream &iss) {
    std::string username;
    std::string hostname;
    std::string servername;
    std::string realname;

    iss >> username >> hostname >> servername >> realname;

    if (username.empty() || realname.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("USER", fd, false);
        return;
    }

    if (realname[0] == ':')
        realname = realname.substr(1);

    if (_clients[fd].isAuthenticated()) {
        sendError(fd, "462 :You may not reregister");
        logCommand("USER", fd, false);
        return;
    }

    _clients[fd].setUsername(username);
    _clients[fd].setRealname(realname);
    logCommand("USER", fd, true);

    if (canAuthenticate(_clients[fd])) {
        _clients[fd].setAuthenticated(true);
        sendWelcome(fd);
    }
}

void Server::handleNick(int fd, std::istringstream &iss) {
    std::string nickname;
    iss >> nickname;

    if (nickname.empty()) {
        sendError(fd, "431 :No nickname given");
        logCommand("NICK", fd, false);
        return;
    }

    if (_nickToFd.find(nickname) != _nickToFd.end() && _nickToFd[nickname] != fd) {
        sendError(fd, "433 :Nickname is already in use");
        logCommand("NICK", fd, false);
        return;
    }

    std::string oldNick = _clients[fd].getNickname();
    if (!oldNick.empty())
        _nickToFd.erase(oldNick);

    _clients[fd].setNickname(nickname);
    _nickToFd[nickname] = fd;
    logCommand("NICK", fd, true);

    if (canAuthenticate(_clients[fd])) {
        _clients[fd].setAuthenticated(true);
        sendWelcome(fd);
    }
}
