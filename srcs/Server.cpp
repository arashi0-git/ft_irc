#include "Server.hpp"
void Server::disconnectClient(int fd) {
    close(fd);

    for (std::vector<struct pollfd>::iterator it = _fds.begin(); it != _fds.end(); ++it) {
        if (it->fd == fd) {
            _fds.erase(it);
            break;
        }
    }

    _clients.erase(fd);
    clientBuffer.erase(fd);
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
        clientBuffer[fd].append(buffer, bytesReceived);

        size_t pos;
        while ((pos = clientBuffer[fd].find('\n')) != std::string::npos) {
            std::string line = clientBuffer[fd].substr(0, pos);
            clientBuffer[fd].erase(0, pos + 1);
            if (!line.empty() && line[line.length() - 1] == '\r')
                line.erase(line.length() - 1);
            processCommand(fd, line);
        }
    }
}

void Server::acceptNewClient() {
    int newSocket = accept(_serverSocket, NULL, NULL);
    if (newSocket < 0) {
        throw std::runtime_error("accept failed");
    }
    struct pollfd pfd;
    pfd.fd = newSocket;
    pfd.events = POLLIN;
    pfd.revents = 0;
    _fds.push_back(pfd);
    _clients[newSocket] = Client(newSocket);
    std::cout << "New client connected fd = " << newSocket << std::endl;
}

void Server::run() {
    while (g_signal){
        int pollresult = poll(_fds.data(), _fds.size(), -1);
        if (pollresult < 0 && g_signal != false) {
            throw std::runtime_error("poll failed");
        }
        for (size_t i = 0; i < _fds.size(); ++i) {
            if (_fds[i].revents & POLLIN) {
                if (_fds[i].fd == _serverSocket) {
                    acceptNewClient();
                } else {
                    handleClient(_fds[i].fd);
                }
            }
        }
    }
    for (size_t i = 1; i < _fds.size(); ++i)
        if (_fds[i].fd >= 0)
            close(_fds[i].fd);
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
