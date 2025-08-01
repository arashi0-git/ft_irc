#include "Channel.hpp"

Channel::Channel(const std::string &name)
    : _name(name),
    _topic(""),
    _members(),
    _operators(),
    _invited(),
    _inviteOnly(false),
    _topicSet(false),
    _hasKey(false),
    _key(""),
    _hasLimit(false),
    _userLimit(0) {}

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

void Channel::removeOperator(int fd) {
    _operators.erase(fd);
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

void Channel::setTopicFlag(bool flag) {
    _topicSet = flag;
}

bool Channel::isTopic() const {
    return _topicSet;
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

void Channel::setKey(const std::string &key) {
    _key = key;
}

void Channel::removeKey() {
    _key.clear();
}

bool Channel::hasKey() const {
    return !_key.empty();
}

const std::string &Channel::getKey() const {
    return _key;
}

void Channel::setLimit(size_t limit) {
    _userLimit = limit;
    _hasLimit = true;
}

void Channel::removeLimit() {
    _userLimit = 0;
    _hasLimit = false;
}

bool Channel::hasLimit() const {
    return _hasLimit;
}

size_t Channel::getLimit() const {
    return _userLimit;
}
