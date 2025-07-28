#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name), _topic(""), _members(), _operators(){}

Channel::~Channel() {}

const std::string &Channel::getName() const {
    return _name;
}

void Channel::addMember(int fd) {
    _members.insert(fd);
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
    _operators.erase(fd);
}

bool Channel::hasMember(int fd) const {
    return _members.find(fd) != _members.end();
}

const std::set<int> &Channel::getMembers() const {
    return _members;
}

void Channel::addOperator(int fd) {
    _operators.insert(fd);
}

bool Channel::isOperator(int fd) const {
    return _operators.find(fd) != _operators.end();
}

void Channel::setTopic(const std::string &topic) {
    _topic = topic;
}

const std::string &Channel::getTopic() const {
    return _topic;
}