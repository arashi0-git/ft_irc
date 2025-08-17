#include "Server.hpp"

void Server::sendError(int fd, const std::string &message) {
    std::string response = ":" + _serverName + " " + message + "\r\n";
    send(fd, response.c_str(), response.length(), 0);
}

// yuhi handlePing
void Server::handlePing(int fd, std::istringstream &iss) {
    std::string token;
    // PING <token> or PING :<token>
    if (!(iss >> token) || token.empty()) {
        // 409 ERR_NOORIGIN
        sendError(fd, "409 :No origin specified");
        return;
    }
    // 先頭が ':' なら外す
    if (!token.empty() && token[0] == ':')
        token.erase(0, 1);

    std::string reply = ":" + _serverName + " PONG " + _serverName + " :" + token + "\r\n";
    send(fd, reply.c_str(), reply.length(), 0);
}

void Server::handleWho(int fd, std::istringstream &iss) {
    std::string mask;
    iss >> mask;
    const std::string me = _clients[fd].getNickname();

    // 引数なし: 空で終端だけ返す（Unknownにしない）
    if (mask.empty()) {
        std::string end = ":" + _serverName + " 315 " + me + " * :End of /WHO list.\r\n";
        send(fd, end.c_str(), end.size(), 0);
        return;
    }

    // チャンネル指定のWHOだけ対応（それ以外は終端だけ返す）
    if (mask[0] == '#') {
        std::map<std::string, Channel>::iterator it = channels.find(mask);
        if (it != channels.end()) {
            Channel &ch = it->second;

            // ★ ここが ch.members() の代わり。getMembers() を回す
            const std::set<int> &mem = ch.getMembers();
            for (std::set<int>::const_iterator mit = mem.begin(); mit != mem.end(); ++mit) {
                int mfd = *mit;
                Client &u = _clients[mfd];

                // 352 <me> <channel> <user> <host> <server> <nick> H :0 <realname>
                std::string line = ":" + _serverName + " 352 " + me + " " + mask + " " +
                                   u.getUsername() + " " + u.getHostname() + " " + _serverName +
                                   " " + u.getNickname() + " H :0 " + u.getRealname() + "\r\n";
                send(fd, line.c_str(), line.size(), 0);
            }
        }
        // チャンネルが無い or 居ない場合も必ず 315 で閉じる
        std::string end = ":" + _serverName + " 315 " + me + " " + mask + " :End of /WHO list.\r\n";
        send(fd, end.c_str(), end.size(), 0);
        return;
    }

    // それ以外（ニック等のマスク）は最小対応: 終端だけ返す
    std::string end = ":" + _serverName + " 315 " + me + " " + mask + " :End of /WHO list.\r\n";
    send(fd, end.c_str(), end.size(), 0);
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
    else if (command == "HELP")
        handleHelp(fd, iss);
    else if (command == "CAP") // yuhi
        return;
    // QUIT condition prevent server sending any 421 error message to a closed client
    else if (command == "PING" || command == "QUIT")
        handlePing(fd, iss);
    else if (command == "WHO")
        handleWho(fd, iss);
    else
        sendError(fd, "421 " + command + " :Unknown command");
}
