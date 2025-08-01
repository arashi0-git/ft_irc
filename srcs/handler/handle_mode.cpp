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
        return;
    }

    bool modeRequiresTarget = (mode == "+o" || mode == "-o");

    if (modeRequiresTarget && target.empty()) {
        sendError(fd, "461 :Not enough parameters");
        return;
    }


    if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        return;
    }

    if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        return;
    }

    Channel &channel = _channels[channelName];

    if (!channel.hasMember(fd)){
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }

    char modechar = mode[1];
    switch (modechar) {
        case 'o':
            handleModeOperator(fd, channel, mode, target);
            break;
        case 'i':
            handleModeInviteOnly(fd, channel, mode);
            break;
        case 't':
            handleModeTopic(fd, channel, mode);
            break;
        case 'k':
            handleModeKey(fd, channel, mode, target);
            break;
        case 'l':
            handleModeLimit(fd, channel, mode, target);
            break;
        default:
            sendError(fd, "472 " + std::string(1, modeChar) + " :is unknown mode char to me");
            break;
    }
}