#ifndef CHANNEL_HPP
#define CHANNEL_HPP

#include <string>
#include <set>

class Channel {
    private:
        std::string _name;
        std::string _topic;
        std::set<int> _members;
        std::set<int> _operators;
        std::set<int> _invited;
        bool _inviteOnly;
    public:
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
        const std::string &getTopic() const;

        void invite(int fd);
        bool isInvited(int fd) const;

        void setInviteOnly(bool flag);
        bool isInviteOnly() const;

};

#endif