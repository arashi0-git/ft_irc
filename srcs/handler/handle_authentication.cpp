#include "Server.hpp"

// toCheck
bool Server::canAuthenticate(const Client &client) const {
    return client.hasPasswordReceived() && !client.getNickname().empty() &&
           !client.getUsername().empty() && !client.getRealname().empty();
}

void printAuthStatus(Client &cl, int fd) {
    std::cout << "[AUTH] fd=" << fd << " (PASS:" << (cl.hasPasswordReceived() ? "✔" : "✘")
              << " NICK:" << (cl.getNickname().empty() ? "✘" : cl.getNickname())
              << " USER:" << (cl.getUsername().empty() ? "✘" : cl.getUsername())
              << " REALNAME:" << (cl.getRealname().empty() ? "✘" : cl.getRealname()) << ")"
              << std::endl;
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
        sendError(fd, "462 :You may not register");
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
    } else {
        printAuthStatus(_clients[fd], fd);
    }
}

void Server::handleUser(int fd, std::istringstream &iss) {
    Client &cl = _clients[fd];

    // 462: already registered //todo todo
    if (cl.hasUserReceived()) {
        sendError(fd, "462 :You may not reregister");
        logCommand("USER", fd, false);
        return;
    }

    // USER <username> <mode> <unused> :<realname...>
    std::string username, mode, unused;
    if (!(iss >> username >> mode >> unused)) {
        sendError(fd, "461 USER :Not enough parameters");
        logCommand("USER", fd, false);
        return;
    }


    // Grab trailing realname (may contain spaces)
    std::string realname;
    std::getline(iss, realname);
    if (!realname.empty() && realname[0] == ' ')
        realname.erase(0, 1);
    if (!realname.empty() && realname[0] == ':')
        realname.erase(0, 1);

    if (realname.empty()) {
        sendError(fd, "461 USER :Not enough parameters");
        logCommand("USER", fd, false);
        return;
    }

    cl.setUsername(username);
    cl.setRealname(realname);
    cl.setUserReceived(true);
    logCommand("USER", fd, true);

    if (canAuthenticate(cl)) {
        cl.setAuthenticated(true);
        sendWelcome(fd);
    }
    printAuthStatus(cl, fd);
}

bool isValidNickname(const std::string &nick) {
    if (nick.empty() || nick.size() > 9)
        return false;
    char first = nick[0];
    if (!isalpha(first) && strchr("[]\\`^{}_", -1) == NULL)
        return false;
    for (size_t i = 1; i < nick.size(); i++) {
        char c = nick[i];
        if (!isalnum(c) && strchr("[]\\`^{}_", -1) == NULL)
            return false;
    }
    return true;
}

void Server::handleNick(int fd, std::istringstream &iss) {
    std::string nickname;
    iss >> nickname;

    if (nickname.empty()) {
        sendError(fd, "431 :No nickname given");
        logCommand("NICK", fd, false);
        return;
    }

    if (!isValidNickname(nickname)) {
        sendError(fd, "432 * " + nickname + " :Erroneous nickname");
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
    } else {
        printAuthStatus(_clients[fd], fd);
    }
}
