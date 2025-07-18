#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <string>
#include <sstream>
#include <iostream>

class ServerConfig {
    private:
        int _port;
        std::string _password;
    
        public:
            ServerConfig(int ac, char **av);
            int getPort() const;
            const std::string &getPassword() const;
};


#endif