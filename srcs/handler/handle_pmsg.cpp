#include "Server.hpp"

// PRIVMSG #channel :Hello everyone
// PRIVMSG Alice :Hey, are you there?

static std::string trim(const std::string &s) {
    const std::string ws = " \t\r\n";
    // 指定した文字集合に含まれない最初の文字の位置
    std::string::size_type b = s.find_first_not_of(ws);
    if (b == std::string::npos)
        return "";
    // 指定した集合に含まれない 最後の文字の位置
    std::string::size_type e = s.find_last_not_of(ws);
    return s.substr(b, e - b + 1);
}

static std::vector<std::string> splitByComma(const std::string &targets) {
    std::vector<std::string> out;
    std::string part;
    part.reserve(targets.size());

    for (std::string::size_type i = 0; i < targets.size(); ++i) {
        char c = targets[i];
        if (c == ',') {
            std::string t = trim(part);
            if (!t.empty())
                out.push_back(t);
            part.clear();
        } else {
            part += c;
        }
    }
    std::string t = trim(part);
    if (!t.empty())
        out.push_back(t);

    return out;
}

// 共通処理：verb は "PRIVMSG" または "NOTICE"
void Server::handleMessageVerb(int fd, std::istringstream &iss, const std::string &verb) {
    Client &fromCli = _clients[fd];
    if (!fromCli.isAuthenticated()) {
        sendError(fd, "451 :You have not registered");
        logCommand(verb, fd, false);
        return;
    }

    std::string targetsToken, message;
    iss >> targetsToken;
    std::getline(iss, message);
    if (!message.empty() && message[0] == ' ')
        message.erase(0, 1);
    if (!message.empty() && message[0] == ':')
        message.erase(0, 1);

    if (targetsToken.empty()) {
        sendError(fd, std::string("411 :No recipient given (") + verb + ")");
        logCommand(verb, fd, false);
        return;
    }
    if (message.empty()) {
        sendError(fd, "412 :No text to send");
        logCommand(verb, fd, false);
        return;
    }

    std::vector<std::string> list = splitByComma(targetsToken);
    bool delivered = false;

    for (size_t i = 0; i < list.size(); ++i) {
        const std::string target = list[i];

        if (!target.empty() && target[0] == '#') {
            // ---- チャンネル宛 ----
            std::map<std::string, Channel>::iterator chit = channels.find(target);
            if (chit == channels.end()) {
                sendError(fd, "403 " + target + " :No such channel");
                continue;
            }
            Channel &channel = chit->second;

            if (!channel.hasMember(fd)) {
                sendError(fd, "404 " + channel.getName() + " :Cannot send to channel");
                continue;
            }

            const std::string sender = _clients[fd].getNickname();
            const std::string line =
                ":" + sender + " " + verb + " " + target + " :" + message + "\r\n";

            const std::set<int> &mem = channel.getMembers();
            for (std::set<int>::const_iterator it = mem.begin(); it != mem.end(); ++it) {
                if (*it != fd)
                    send(*it, line.c_str(), line.size(), 0); // 送信者へはエコーしない
            }
            delivered = true;
        } else {
            // ---- ニック宛 ----
            int targetFd = -1;
            for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end();
                 ++it) {
                if (it->second.getNickname() == target) {
                    targetFd = it->first;
                    break;
                }
            }
            if (targetFd == -1) {
                sendError(fd, "401 " + target + " :No such nick");
                continue;
            }

            const std::string sender = _clients[fd].getNickname();
            const std::string line =
                ":" + sender + " " + verb + " " + target + " :" + message + "\r\n";
            send(targetFd, line.c_str(), line.size(), 0);
            delivered = true;
        }
    }

    logCommand(verb, fd, delivered);
}
