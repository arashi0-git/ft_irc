#include "Server.hpp"

bool Server::canAuthenticate(const Client &client) const {
    return client.hasPasswordReceived() && !client.getNickname().empty() && !client.getUsername().empty() && !client.getRealname().empty();
}

void Server::sendWelcome(int fd) {
    const Client &client = _clients[fd];
    std::string msg = ":irc.local 001 " + client.getNickname() + " : Welcome to the IRC server " + client.getNickname() + "\r\n";
    send(fd, msg.c_str(), msg.length(), 0);
}

void Server::handlePass(int fd, std::istringstream &iss) {
    std::string password;
    iss >> password;
    
    if (password.empty()) {
        sendError(fd, "461 :Not enough parameters");
        return;
    }

    if (_clients[fd].hasPasswordReceived()) {
        sendError(fd, "462 :You may not reregister");
        return;
    }

    if (password != _config.getPassword()) {
        sendError(fd, "464 :Password incorrect");
        return;
    }

    _clients[fd].setPasswordReceived(true);

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
        return;
    }

    if (realname[0] == ':')
        realname = realname.substr(1);

    if (_clients[fd].isAuthenticated()) {
        sendError(fd, "462 :You may not reregister");
        return;
    }

    _clients[fd].setUsername(username);
    _clients[fd].setRealname(realname);

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
        return;
    }

    if (_nickToFd.find(nickname) != _nickToFd.end() && _nickToFd[nickname] != fd) {
        sendError(fd, "433 :Nickname is already in use");
        return;
    }

    std::string oldNick = _clients[fd].getNickname();
    if (!oldNick.empty())
        _nickToFd.erase(oldNick);

    _clients[fd].setNickname(nickname);
    _nickToFd[nickname] = fd;

    if (canAuthenticate(_clients[fd])) {
        _clients[fd].setAuthenticated(true);
        sendWelcome(fd);
    }
}
