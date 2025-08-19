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

    if (command == "PASS") {
        handlePass(fd, iss);
        return;
    }

    if ((command == "NICK" || command == "USER") && !_clients[fd].hasPasswordReceived()) {
        sendError(fd, "464 :Password required");
        return;
    }

    if (command == "NICK") {
        handleNick(fd, iss);
        return;
    } else if (command == "USER") {
        handleUser(fd, iss);
        return;
    } else if (command == "HELP") {
        handleHelp(fd, iss);
        return;
    } else if (command == "PING") {
        handlePing(fd, iss);
        return;
    }
    // CAP : Client Capability Negotiation
    // QUIT condition prevent server sending any 421 error message to a closed client
    else if (command == "CAP" || command == "QUIT")
        return;

    if (!_clients[fd].isAuthenticated()) {
        sendError(fd, "451 :You have not registered");
        logCommand(command, fd, false);
        return;
    }

    if (command == "PRIVMSG")
        handleMessageVerb(fd, iss, "PRIVMSG");
    else if (command == "NOTICE")
        handleMessageVerb(fd, iss, "NOTICE");
    else if (command == "JOIN")
        handleJoin(fd, iss);
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
    else if (command == "WHO")
        handleWho(fd, iss);
    else
        sendError(fd, "421 " + command + " :Unknown command");
}
