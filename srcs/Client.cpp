#include "Client.hpp"

Client::Client() : _fd(-1), _hasPasswordReceived(false), _authenticated(false) {}

Client::Client(int fd) {
    _fd = fd;
    _authenticated = false;
    _hasPasswordReceived = false;
}

Client::~Client() {}

int Client::getFd() const { return _fd; }

const std::string &Client::getNickname() const { return _nickname; }

void Client::setNickname(const std::string &nick) { _nickname = nick; }

const std::string &Client::getUsername() const { return _username; }
void Client::setUsername(const std::string &username) { _username = username; }

const std::string &Client::getRealname() const { return _realname; }

void Client::setRealname(const std::string &realname) { _realname = realname; }

const std::string &Client::getHostname() const { return _hostname; }

void Client::setHostname(const std::string &hostname) { _hostname = hostname; }

void Client::clearBuffer() { _buffer.clear(); }

bool Client::isAuthenticated() const { return _authenticated; }

void Client::setAuthenticated(bool authenticated) { _authenticated = authenticated; }

void Client::setPasswordReceived(bool value) { _hasPasswordReceived = value; }

bool Client::hasPasswordReceived() const { return _hasPasswordReceived; }
