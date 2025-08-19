#ifndef CLIENT_HPP
#define CLIENT_HPP

#include <arpa/inet.h>
#include <string>

class Client {
  private:
    int _fd;
    std::string _nickname;
    std::string _username;
    std::string _hostname;
    std::string _realname;
    std::string _buffer;
    bool _hasPasswordReceived;
    bool _authenticated;

  public:
    Client();
    Client(int fd);
    ~Client();

    int getFd() const;
    std::string &getBuffer();
    const std::string &getNickname() const;
    void setNickname(const std::string &nick);

    const std::string &getUsername() const;
    void setUsername(const std::string &username);

    const std::string &getRealname() const;
    void setRealname(const std::string &realname);

    const std::string &getHostname() const;
    void setHostname(const std::string &hostname);

    void clearBuffer();

    bool isAuthenticated() const;
    void setAuthenticated(bool authenticated);

    void setPasswordReceived(bool value);
    bool hasPasswordReceived() const;
};

#endif