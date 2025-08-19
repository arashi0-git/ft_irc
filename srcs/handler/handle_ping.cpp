
#include "Server.hpp"
#include <cstdlib>

/*
PING is a keep-alive and latency check. Either the client or the server sends
PING :<token> (or PING <server>).
The receiver must immediately reply with
PONG <serverName> :<token>
so the sender knows the connection is alive and can match the reply to the original ping.
*/
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