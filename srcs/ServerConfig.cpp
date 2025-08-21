#include "Server.hpp"

ServerConfig::ServerConfig(int ac, char **av) {
    if (ac != 3)
        throw std::invalid_argument("Usage: ./ircserv <port> <password>");

    // it provides an input stream interface over a std::string
    std::istringstream iss(av[1]);
    int port;

    // iss >> port parses text from a string stream (iss) into a number variable (port).
    if (!(iss >> port) || !(iss.eof()) || port <= 0 || port > 65535)
        throw std::invalid_argument("Invalid port number");

    _port = port;
    _password = av[2];
    if (_password.empty())
        throw std::invalid_argument("Password cannot be empty");
}

int ServerConfig::getPort() const { return _port; }

const std::string &ServerConfig::getPassword() const { return _password; }
