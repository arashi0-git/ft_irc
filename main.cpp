#include "Server.hpp"
#include "ServerConfig.hpp"
#include "csignal"

int main(int ac, char **av) {
    signal(SIGINT, signal_handler);
    try {
        ServerConfig config(ac, av);
        Server server(config);
        server.run();
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    return 0;
}
