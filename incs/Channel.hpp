#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <set>
#include <string>

class Channel {
  private:
    std::string _name;
    std::string _topic;
    std::set<int> _members;
    std::set<int> _operators;
    std::set<int> _invited;
    bool _inviteOnly;
    bool _topicSet;
    bool _hasKey;
    std::string _key;
    bool _hasLimit;
    size_t _userLimit;

  public:
    Channel();
    Channel(const std::string &name);
    ~Channel();

    const std::string &getName() const;

    void addMember(int fd);
    void removeMember(int fd);
    void removeOperator(int fd);
    bool hasMember(int fd) const;
    const std::set<int> &getMembers() const;

    void addOperator(int fd);
    bool isOperator(int fd) const;
    bool hasOperator() const;

    void setTopic(const std::string &topic);
    void setTopicFlag(bool flag);
    bool isTopic() const;
    const std::string &getTopic() const;

    void invite(int fd);
    bool isInvited(int fd) const;
    void removeInvite(int fd);

    void setInviteOnly(bool flag);
    bool isInviteOnly() const;

    void setKey(const std::string &key);
    void removeKey();
    bool hasKey() const;
    const std::string &getKey() const;

    void setLimit(size_t limit);
    void removeLimit();
    bool hasLimit() const;
    size_t getLimit() const;
};

#endif
