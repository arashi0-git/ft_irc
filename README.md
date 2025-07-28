# ft_ircで学んだこと

## socket()

* 通信のエンドポイントを作成し、エンドポイントを表すソケット記述子を返す
* 異なるタイプのソケットで異なる通信サービスが展開できる

```
int socket(int *domain, int type, int protocol);
```

## domain

* 要求するアドレス・ドメイン。AF_INET,AF_INET6,AF_UNIX,AF_RAWのいずれか

## type

* 作成するソケットのタイプ。SOCKET_STREAM,SOCKET_DGRAM,SOCKET_RAWのいずれか

## protocol

* 要求済みのプロトコル。0,IPOOROTO_UDP,IPPROT_TCPのいずれか

## bind()

* ソケットにIPアドレスとポート番号を割り当てる関数
* クライアントからの接続を受け付ける場所（ポート）を指定できる
* bind()を使ってポート〇〇〇〇番で受け付けるとOSに伝える
* 指定した関数をラップしたstd::functionをつくる

```
# include <iostream>

void test_function(int a, int b) {
    printf("a=%d, b=%d\n", a, b);
}

int main(int argc, const char * argv[]) {
    auto func1 = std::bind(test_function, std::placeholders::_1, std::placeholders::_2);
    func1(1, 2);
    // -> a=1, b=2

    auto func2 = std::bind(test_function, std::placeholders::_1, 9);
    func2(1);
    // -> a=1, b=9

    return 0;
}
```

## listen()

* ソケットを”待ち受け状態”に設定するために使う

## accept()

* クライアントからの接続要求を受け入れるために使う。

```C++
void Server::setupSocket() {
    _serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (_serverSocket < 0)
        throw std::runtime_error("Failed to create socket");
    
    int opt = 1;
    if (setsockopt(_serverSocket, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) < 0)
        throw std::runtime_error("setsockopt failed");

    struct sockaddr_in addr;
    std::memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(_config.getPort());

    if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");

    if (fcntl(_serverSocket, F_SETFL, O_NONBLOCK) < 0)
        throw std::runtime_error("fcntl failed");

    if (listen(_serverSocket, SOMAXCONN) < 0)
        throw std::runtime_error("listen failed");
}
```

## ```setupSocket()```の仕事

```setupSocket()```ではOSに対して、「このソケットは、OS上でポート何番を使いTCPサーバーとして接続待ちをする」ことをOSに伝える関数。

### ```socket()```関数で新しいTCPソケットを作成

    * TCPソケット：TCPプロトコル（信頼性のある通信）を使うためのソケット
    * TCP：Transmission Control Protocol（伝送制御プロトコル）インターネットなどのIPネットワーク上で信頼性の高いデータ通信を確率するためのプロトコル
        通信するときに使う約束事<https://wa3.i-3-i.info/word19.html>
    * ソケット：ネットワーク通信を行うための出入口（エンドポイント）

* AF_INET:IPv4用
* SOCK_STREAM:TCPプロトコル
```socket(AF_INNT, SOCK_STREAM, 0)```はIPv4+TCPで通信するソケットを作成

### setsockopt()でソケットオプションの設定（再バインドを許可）

* SO_REUSEADDR:サーバー再起動時にアドレス使用中エラーを防ぐ
* 直前に落ちたサーバーがポートを占領していても再バインドできるようになる

```C++
int opt = 1;
setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

* ```socket_fd```：設定対象のソケットファイルディスクリプタ
* ```SOL_SOCKET```：ソケットレベルのオプションを設定するという意味
* ```SO_REUSEADDR```：アドレス再利用オプション
* ```&opt```：オプションの値へのポインタ(1=有効)
* ```sizeof(opt)```：値のサイズ（バイト数）

### ✅サーバーでの```SO_REUSEADDR```の目的

* サーバー再起動時にbind()が失敗しないよにするために使われる
* サーバーを強制終了・再起動すると、ポートがしばらく「TIME_WAIT」状態にで残る
* その状態で```bind()```を呼ぶと

```C++
bind: Address already in use
```

とエラーになる
「TIME_WAIT」状態でもすぐにバインド可

### IRCサーバーでTCPソケットを使う理由

* 文字が順番通りに届く
* 途中でかけたり、重複しない
* 双方向のやり取りが必要
->TCPが適している

### ```bind()```引数

```C++
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```

* ```sockfd```：```socket()```で作成したソケットのfd
* ```addr```：```sockaddr_in```のポインタ（バインド先の情報）
* ```addlen```：```addr```構造体のサイズ(sizeof(addr))

### ```fcntl()```

```C++
fcntl(fd, F_SETFL, O_NONBLOCK);
```

* ```fd```をノンブロッキングモードにする
* ```read()```や```recv()```などがデータがない時にもブロックせずにすぐに戻るようになります
* サーバーが複数クライアントを同時に処理するには、どのクライアントも待たせずに、常に全体を監視できる必要があるため

### ```listen()```：「このソケットでクライアントからの接続を待ち受ける」ことをOSに伝える関数

```C++
int listen(int sockfd, int backlog);
```

* ```sockfd()```：接続待ち状態にするソケット(socket() + bind()済みのもの)
* ```backlog```：OSに「キューの長さ（＝待ちの行列の数）を伝える
* ```backlog```はOSが一時的にためておける接続要求の最大数を意味する
* ```SOMAXCONN```はシステムが定義している最大数

### ✅ソケットを接続要求待ち状態(passive mode)にする

* **「接続窓口をOSが開く処理」**

### ```poll()```

* ```poll()```：複数のファイルディスクリプタを同時に監視し、それらにイベント（読み込み、書き込み、切断など）が発生した可動化を検出するための関数

```C++
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

* fds：監視対象のファイルディスクリプタの配列（複数ある）
* nfds：監視対象の数（fdsの要素数）
* timeout：タイムアウト時間-1にすると無限秒
* 返り値：>0->イベントが発生したfdの数、0->タイムアウト何も起こらなかった、<0->エラー

### TCP通信はストリーム型（区切りなし）

* サーバーは「どこまでが一つのメッセージなのか」を自分で判断する必要がある
* \r\nまたは\nで終わるのが一般的

```C++
if (!channel.hasMember(fd)) {
        sendError(fd, "442 " + channelName + " :You're not on that channel");
        return;
    }
```

* 呼び出し元（fd）がそのチャンネルのメンバーかどうか

```C++
if (_channels.find(channelName) == _channels.end()) {
        sendError(fd, "403 " + channelName + " :No such channel");
        return;
    }
```

* チャンネルが存在するか

```C++
if (!channel.isOperator(fd)) {
        sendError(fd, "482 " + channelName + " :You're not channel operator");
        return;
    }
```

* チャンネルのオペレーターか

```C++
int userFd = -1;
    for (std::map<int, Client>::iterator it = _clients.begin(); it != _clients.end(); ++it) {
        if (it->second.getNickname() == user) {
            userFd = it->first;
            break;
        }
    }
    if (userFd == -1) {
        sendError(fd, "401 " + user + " :No such nick");
        return;
    }
```

* チャンネル内のユーザーのfdを取得

```C++
if (channelName[0] != '#') {
        sendError(fd, "476 " + channelName + " :Invalid channel name");
        return;
    }
```

* channel名の先頭に"#"がついてるかどうか

```C++
std::string msg = ":" + _clients[fd].getNickname() + " TOPIC " + channelName + ": " + topic + "\r\n";
        for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
            send(*it, msg.c_str(), msg.length(), 0);
        }
```

* channel内のmemberになにかメッセージを送る
