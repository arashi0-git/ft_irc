#include "Server.hpp"

void Server::sendError(int fd, const std::string &message) {
    std::string response = ":" + _serverName + " " + message + "\r\n";
    send(fd, response.c_str(), response.length(), 0);
}

void Server::processCommand(int fd, const std::string &line) {
    std::istringstream iss(line);
    std::string command;
    iss >> command;
    for (size_t i = 0; i < command.length(); ++i) {
        command[i] = std::toupper(static_cast<unsigned char>(command[i]));
    }

    if ((command == "NICK" || command == "USER") && !_clients[fd].hasPasswordReceived()) {
        sendError(fd, "464 :Password required");
        return;
    }

    if (command == "NICK")
        handleNick(fd, iss);
    else if (command == "USER")
        handleUser(fd, iss);
    else if (command == "PRIVMSG")
        handlePrivMsg(fd, iss);
    else if (command == "JOIN")
        handleJoin(fd, iss);
    else if (command == "PASS")
        handlePass(fd, iss);
    else if (command == "KICK")
        handleKick(fd, iss);
    else if (command == "TOPIC")
        handleTopic(fd, iss);
    else if (command == "MODE")
        handleMode(fd, iss);
    else if (command == "INVITE")
        handleInvite(fd, iss);
    else if (command == "PART")
        handlePart(fd, iss);
    else
        sendError(fd,"421 " + command +  " :Unknown command");
}
