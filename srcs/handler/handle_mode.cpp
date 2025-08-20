#include "Server.hpp"

// MODE #channel +o/-o target
// MODE #channel +i/-i
// MODE #channel +l/-l limit
// MODE #channel +t/-t
// MODE #channel +k/-k key

// void Server::handleMode(int fd, std::istringstream &iss) {
//     std::string channelName, mode, target;
//     iss >> channelName >> mode >> target;

//     if (channelName.empty() || mode.empty()) {
//         sendError(fd, "461 :Not enough parameters");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     bool modeRequiresTarget = (mode == "+o" || mode == "-o");

//     if (modeRequiresTarget && target.empty()) {
//         sendError(fd, "461 :Not enough parameters");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     if (channelName[0] != '#') {
//         sendError(fd, "476 " + channelName + " :Invalid channel name");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     if (_channels.find(channelName) == _channels.end()) {
//         sendError(fd, "403 " + channelName + " :No such channel");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     Channel &channel = _channels[channelName];

//     if (!channel.hasMember(fd)) {
//         sendError(fd, "442 " + channelName + " :You're not on that channel");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     if (!channel.isOperator(fd)) {
//         sendError(fd, "482 " + channelName + " :You're not channel operator");
//         logCommand("MODE", fd, false);
//         return;
//     }

//     char modechar = mode[1];
//     switch (modechar) {
//     case 'o':
//         handleModeOperator(fd, channel, mode, target);
//         logCommand("MODE", fd, true);
//         break;
//     case 'i':
//         handleModeInviteOnly(fd, channel, mode);
//         logCommand("MODE", fd, true);
//         break;
//     case 't':
//         handleModeTopic(fd, channel, mode);
//         logCommand("MODE", fd, true);
//         break;
//     case 'k':
//         handleModeKey(fd, channel, mode, target);
//         logCommand("MODE", fd, true);
//         break;
//     case 'l':
//         handleModeLimit(fd, channel, mode, target);
//         logCommand("MODE", fd, true);
//         break;
//     default:
//         sendError(fd, "472 " + std::string(1, modechar) + " :is unknown mode char to me");
//         logCommand("MODE", fd, false);
//         break;
//     }
// }

// yuhi
void Server::handleMode(int fd, std::istringstream &iss) {
    std::string target, mode, param;
    iss >> target >> mode >> param;

    // target すら無い → パラメータ不足
    if (target.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("MODE", fd, false);
        return;
    }

    // =========================
    //  ユーザ MODE の最小対応
    //  MODE <nick> [+i/-i] | MODE <nick>
    // =========================
    if (target[0] != '#') {
        Client &cli = _clients[fd];
        const std::string nick = cli.getNickname();

        // 自分以外のユーザMODE変更は不可
        if (target != nick) {
            // 502 ERR_USERSDONTMATCH
            sendError(fd, "502 :Cannot change mode for other users");
            logCommand("MODE", fd, false);
            return;
        }

        // 照会だけ（MODE <nick>）
        if (mode.empty()) {
            // 221 RPL_UMODEIS（中身は空でもOK）
            sendError(fd, "221 " + nick + " :");
            logCommand("MODE", fd, true);
            return;
        }

        // 最低限：+i / -i だけ受理（内部で保持しないならそのまま 221 を返せばOK）
        if (mode == "+i" || mode == "-i") {
            // 例: cli.setInvisible(mode == "+i"); // 実装があれば
            sendError(fd, "221 " + nick + " :");
            logCommand("MODE", fd, true);
            return;
        }

        // それ以外のユーザMODEフラグは未対応
        // 501 ERR_UMODEUNKNOWNFLAG
        sendError(fd, "501 :Unknown MODE flag");
        logCommand("MODE", fd, false);
        return;
    }

    // =========================
    //  ここからチャンネル MODE（既存ロジックを活かす）
    // =========================
    const std::string &channelName = target;

    // モード指定なしの照会（MODE #chan）→ 最低限 324 を返しておく（空でも可）
    if (mode.empty()) {
        const std::string nick = _clients[fd].getNickname();
        // 324 RPL_CHANNELMODEIS <nick> #chan <modes>
        // いまはモード文字列を空にしておく（必要なら Channel から組み立て）
        sendError(fd, "324 " + nick + " " + channelName + " :");
        logCommand("MODE", fd, true);
        return;
    }

    bool modeRequiresTarget = (mode == "+o" || mode == "-o");

    if (modeRequiresTarget && param.empty()) {
        sendError(fd, "461 :Not enough parameters");
        logCommand("MODE", fd, false);
        return;
    }

    if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        logCommand("MODE", fd, false);
        return;
    }

    Channel &channel = _channels[channelName];

    if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        logCommand("MODE", fd, false);
        return;
    }

    // yuhi ban list
    const std::string me = _clients[fd].getNickname();
    if (mode == "b" && param.empty()) {
        // MODE #chan b ＝ Banリスト照会 → 空リスト終端だけ返して静かに終わる
        sendError(fd, "368 " + me + " " + channelName + " :End of channel ban list");
        logCommand("MODE", fd, true);
        return;
    }

    if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        logCommand("MODE", fd, false);
        return;
    }

    // 1文字目は +/-, 2文字目がモード文字の想定（例: "+i", "-o"）
    char modechar = (mode.size() > 1) ? mode[1] : '\0';
    bool success = false;
    switch (modechar) {
    case 'o':
        success = handleModeOperator(fd, channel, mode, param);
        break;
    case 'i':
        success = handleModeInviteOnly(fd, channel, mode);
        break;
    case 't':
        success = handleModeTopic(fd, channel, mode);
        break;
    case 'k':
        success = handleModeKey(fd, channel, mode, param);
        break;
    case 'l':
        success = handleModeLimit(fd, channel, mode, param);
        logCommand("MODE", fd, true);
        break;
    default:
        sendError(fd, "472 " + std::string(1, modechar) + " :is unknown mode char to me");
        logCommand("MODE", fd, false);
        break;
    }
    logCommand("MODE", fd, success);
}
