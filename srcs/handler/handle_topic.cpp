#include "Server.hpp"

// TOPIC #channel :new topic

void Server::handleTopic(int fd, std::istringstream &iss) {
    std::string channelName, topic;
    iss >> channelName;
    getline(iss, topic);

    if (channelName.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("TOPIC", fd, false);
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name (Usage: JOIN <#channel>)");
        logCommand("TOPIC", fd, false);
        return;
    }

    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        logCommand("TOPIC", fd, false);
        return;
    }

    Channel &channel = channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel"); 
        logCommand("TOPIC", fd, false);
        return;
    }

    if (topic.empty()) {
        if (channel.getTopic().empty()) {
            std::string msg = ":" + _serverName + " 331 " + _clients[fd].getNickname() + " " +
                              channelName + " :No topic is set\r\n";
            send(fd, msg.c_str(), msg.length(), 0);
        } else {
            std::string msg2 = ":" + _serverName + " 332 " + _clients[fd].getNickname() + " " +
                               channelName + " :" + channel.getTopic() + "\r\n";
            send(fd, msg2.c_str(), msg2.length(), 0);
        }
        logCommand("TOPIC", fd, true);
    } else {
        if (channel.isTopic() && !channel.isOperator(fd)) {
            sendError(fd, "482 " + channelName + " :You're not channel operator");
            logCommand("TOPIC", fd, false);
            return;
        }
        if (!topic.empty() && topic[0] == ':')
            topic = topic.substr(1);
        channel.setTopic(topic);
        std::string msg =
            ":" + _clients[fd].getNickname() + " TOPIC " + channelName + ": " + topic + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin();
             it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
        logCommand("TOPIC", fd, true);
    }
}
