#include "Server.hpp"

void Server::disconnectClient(int fd) {
    close(fd);

    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (it->fd == fd) {
            _fds.erase(it);
            break;
        }
    }

    for (std::map<std::string, Channel>::iterator it = _channels.begin(); it != _channels.end(); ++it) {
        it->second.removeMember(fd);
        it->second.removeOperator(fd);
        it->second.removeInvite(fd);
    }

    _clients.erase(fd);
    _clientBuffer.erase(fd);
}

void Server::handleClient(int fd) {

    char buffer[1024];
    std::memset(buffer, 0, sizeof(buffer));

    ssize_t bytesReceived = recv(fd, buffer, sizeof(buffer), 0);
    if (bytesReceived < 0) {
        std::cerr << "recv failed on fd " << fd << std::endl;
        disconnectClient(fd);
        return;
    }

    if (bytesReceived == 0) {
        std::cout << "Client disconnected fd = " << fd << std::endl;
        disconnectClient(fd);
        return;
    }

    if (bytesReceived > 0) {
        _clientBuffer[fd].append(buffer, bytesReceived);

        size_t pos;
        while ((pos = _clientBuffer[fd].find('\n')) != std::string::npos) {
            std::string line = _clientBuffer[fd].substr(0, pos);
            _clientBuffer[fd].erase(0, pos + 1);
            if (!line.empty() && line[line.length() - 1] == '\r') {
                line.erase(line.length() - 1);
            }
            processCommand(fd, line);
        }
    }
}

void Server::acceptNewClient() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int newSocket = accept(_serverSocket, (struct sockaddr *)&addr, &len);
    if (newSocket < 0) {
        throw std::runtime_error("accept failed");
    }
    struct pollfd pfd;
    pfd.fd = newSocket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _fds.push_back(pfd);

    Client cli(newSocket);
    if (addr.sin_family == AF_INET) {
        cli.setHostname(std::string(inet_ntoa(addr.sin_addr))); // 例: "127.0.0.1"
    }
    _clients[newSocket] = cli;
    std::cout << "New client connected fd = " << newSocket << std::endl;
}

void Server::run() {
    while (g_signal) {
        //_fds.data() points at the first pollfd in that array (equivalent to &_fds[0])
        int pollresult = poll(_fds.data(), _fds.size(), -1);
        if (pollresult < 0 && g_signal != false) {
            throw std::runtime_error("poll failed");
        }
        for (size_t i = 0; i < _fds.size(); ++i) {
            // if poll() tells us this fd is ready for reading, then handle it in bit-mask.
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocket) {
                    acceptNewClient();
                } else {
                    handleClient(_fds[i].fd);
                }
            }
        }
    }
    for (int i = _fds.size() - 1; i >= 1; --i)
        if (_fds[i].fd >= 0)
            Server::disconnectClient(_fds[i].fd);
    if (_serverSocket >= 0)
        close(_serverSocket);
}

void Server::initializePoll() {
    struct pollfd pfd;
    // Set which FD to watch
    pfd.fd = _serverSocket;
    // Tell it what events you care about
    // POLLIN means “notify me when this socket is ready for reading.
    pfd.events = POLLIN;
    // reset: revents is where poll() tells you what actually happened on that FD
    pfd.revents = 0;
    _fds.push_back(pfd);
}
//AF_INET is IPv4, SOCK_STREAM says that its going to be TCP, SO_REUSEADDR server can reacces the same port
// INADDR_ANY server will accpet all connections from everywhere
void Server::setupSocket() {
    // create socket (AF_INET=IPv4, SOCK_STREAM=TCP, 0=default protocol)
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
        throw std::runtime_error("Failed to create socket");

    // set socket allow rebinding (socket_fd, edit socket setting, rebinding, opt=turn on, size)
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt failed");

    // structure for IPv4
    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_port = htons(_config.getPort());
    addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(_serverSocket, (struct sockaddr *)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");

    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl failed");

    if (listen(_serverSocket, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");
}

// constructor
Server::Server(const ServerConfig &config)
    : _config(config), _serverSocket(-1), _serverName("ft_irc.server") {
    setupSocket();
    initializePoll();
    std::cout << "Server is running on port " << _config.getPort() << std::endl;
}

bool Server::isNumeric(const std::string &str) const {
    if (str.empty())
        return false;
    for (size_t i = 0; i < str.length(); ++i) {
        if (!std::isdigit(str[i]))
            return false;
    }
    return true;
}

void Server::logCommand(const std::string &command, int fd, bool success) {
    std::string status = success ? "SUCCESS" : "FAILED";
    std::cout << "[" << command << "] fd=" << fd << " " << status << std::endl;
}

void Server::logMessage(const std::string &message) {
    std::cout << "[LOG] " << message << std::endl;
}
