#include "Channel.hpp"

Channel::Channel(const std::string &name) : _name(name), _topic(""), _members(), _operators(), _invited(), _inviteOnly(false){}

Channel::~Channel() {}

const std::string &Channel::getName() const {
    return _name;
}

void Channel::addMember(int fd) {
    _members.insert(fd);
}

void Channel::removeMember(int fd) {
    _members.erase(fd);
}

void Channel::removeOperator(int fd) {
    _members.erase(fd);
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

bool Channel::hasOperator() const {
    return !_operators.empty();
}

void Channel::setTopic(const std::string &topic) {
    _topic = topic;
}

const std::string &Channel::getTopic() const {
    return _topic;
}

void Channel::invite(int fd) {
    _invited.insert(fd);
}

bool Channel::isInvited(int fd) const {
    return _invited.count(fd);
}

void Channel::setInviteOnly(bool flag) {
    _inviteOnly = flag;
}

bool Channel::isInviteOnly() const {
    return _inviteOnly;
}