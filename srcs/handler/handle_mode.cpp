#include "Server.hpp"

// MODE #channel +o/-o target
// MODE #channel +i/-i
// MODE #channel +l/-l limit
// MODE #channel +t/-t
// MODE #channel +k/-k key

void Server::handleMode(int fd, std::istringstream &iss) {
    std::string channelName, mode, target;
    iss >> channelName >> mode >> target;

    if (channelName.empty() || mode.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("MODE", fd, false);
        return;
    }

    bool modeRequiresTarget = (mode == "+o" || mode == "-o");

    if (modeRequiresTarget && target.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("MODE", fd, false);
        return;
    }

    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        logCommand("MODE", fd, false);
        return;
    }

    if (channels.find(channelName) == channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        logCommand("MODE", fd, false);
        return;
    }

    Channel &channel = channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        logCommand("MODE", fd, false);
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        logCommand("MODE", fd, false);
        return;
    }

    char modechar = mode[1];
    switch (modechar) {
    case 'o':
        handleModeOperator(fd, channel, mode, target);
        logCommand("MODE", fd, true);
        break;
    case 'i':
        handleModeInviteOnly(fd, channel, mode);
        logCommand("MODE", fd, true);
        break;
    case 't':
        handleModeTopic(fd, channel, mode);
        logCommand("MODE", fd, true);
        break;
    case 'k':
        handleModeKey(fd, channel, mode, target);
        logCommand("MODE", fd, true);
        break;
    case 'l':
        handleModeLimit(fd, channel, mode, target);
        logCommand("MODE", fd, true);
        break;
    default:
        sendError(fd, "472 " + std::string(1, modechar) + " :is unknown mode char to me");
        logCommand("MODE", fd, false);
        break;
    }
}
