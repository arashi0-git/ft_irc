#include "Server.hpp"

ServerConfig::ServerConfig(int ac, char **av) {
    if (ac != 3)
        throw std::invalid_argument("Usage: ./ircserv <port> <password>");

    std::istringstream iss(av[1]);
    int port;
    if (!(iss >> port) || port <= 0 || port > 65535)
        throw std::invalid_argument("Invalid port number");
    
    _port = port;
    _password = av[2];
    if (_password.empty())
        throw std::invalid_argument("Password cannot be empty");
}

int ServerConfig::getPort() const {
    return _port;
}

const std::string &ServerConfig::getPassword() const {
    return _password;
}
