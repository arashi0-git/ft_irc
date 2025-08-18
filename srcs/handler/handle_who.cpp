

#include "Server.hpp"
#include <cstdlib>

/*
WHOは、見えているユーザの一覧と属性を取得するコマンドです。
クライアント（irssiなど）は、チャンネル内のメンバーや特定ユーザの詳細（user/host/権限/在席など）を知るために使います。
*/
void Server::handleWho(int fd, std::istringstream &iss) {
    std::string mask;
    iss >> mask;
    const std::string me = _clients[fd].getNickname();

    // 引数なし: 空で終端だけ返す（Unknownにしない）
    if (mask.empty()) {
        std::string end = ":" + _serverName + " 315 " + me + " * :End of /WHO list.\r\n";
        send(fd, end.c_str(), end.size(), 0);
        return;
    }

    // チャンネル指定のWHOだけ対応（それ以外は終端だけ返す）
    if (mask[0] == '#') {
        std::map<std::string, Channel>::iterator it = channels.find(mask);
        if (it != channels.end()) {
            Channel &ch = it->second;

            // ★ ここが ch.members() の代わり。getMembers() を回す
            const std::set<int> &mem = ch.getMembers();
            for (std::set<int>::const_iterator mit = mem.begin(); mit != mem.end(); ++mit) {
                int mfd = *mit;
                Client &u = _clients[mfd];

                // 352 <me> <channel> <user> <host> <server> <nick> H :0 <realname>
                std::string line = ":" + _serverName + " 352 " + me + " " + mask + " " +
                                   u.getUsername() + " " + u.getHostname() + " " + _serverName +
                                   " " + u.getNickname() + " H :0 " + u.getRealname() + "\r\n";
                send(fd, line.c_str(), line.size(), 0);
            }
        }
        // チャンネルが無い or 居ない場合も必ず 315 で閉じる
        std::string end = ":" + _serverName + " 315 " + me + " " + mask + " :End of /WHO list.\r\n";
        send(fd, end.c_str(), end.size(), 0);
        return;
    }

    // それ以外（ニック等のマスク）は最小対応: 終端だけ返す
    std::string end = ":" + _serverName + " 315 " + me + " " + mask + " :End of /WHO list.\r\n";
    send(fd, end.c_str(), end.size(), 0);
}
