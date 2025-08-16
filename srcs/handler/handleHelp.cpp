#include "Server.hpp"

void Server::handleHelp(int fd, std::istringstream &) {
    std::string response = "Available commands:\r\n";
    response += "PASS <password>\r\n";
    response += "NICK <nickname>\r\n";
    response += "USER <username> <hostname> <servername> :<realname>\r\n";
    response += "JOIN <#channel> [key]\r\n";
    response += "PART <#channel> [reason]\r\n";
    response += "PRIVMSG <target> :<message>\r\n";
    response += "NOTICE <target> :<message>\r\n";
    response += "TOPIC <#channel> [:<topic>]\r\n";
    response += "MODE <#channel> <+/-mode> [parameters]\r\n";
    response += "KICK <#channel> <nickname> [reason]\r\n";
    response += "INVITE <nickname> <#channel>\r\n";
    response += "HELP\r\n";
    
    send(fd, response.c_str(), response.length(), 0);
    logCommand("HELP", fd, true);
}
