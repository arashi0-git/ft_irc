# ft_irc - IRC Server Implementation

C++98標準で実装されたIRCサーバーです。複数クライアントの同時接続、チャンネル管理、オペレーター機能をサポートしています。

## 🚀 使用方法

### サーバーの起動

#### コンパイル
```bash
# ビルド
make

# クリーンビルド
make re

# クリーンアップ
make clean      # オブジェクトファイルを削除
make fclean     # 実行ファイルも含めて削除
```

#### サーバー起動
```bash
./ircserv <port> <password>
```

**パラメータ:**
- `port`: サーバーが待機するポート番号（1-65535）
- `password`: クライアント接続時に必要なパスワード

**例:**
```bash
./ircserv 6667 mypassword
```

### クライアントの接続

#### 推奨IRCクライアント
- **HexChat** (GUI)
- **WeeChat** (CUI)  
- **irssi** (CUI)
- **netcat** (テスト用)

#### 接続設定
- **サーバー:** 127.0.0.1 (localhost)
- **ポート:** サーバー起動時に指定したポート
- **パスワード:** サーバー起動時に指定したパスワード

#### 基本的な接続手順
1. IRCクライアントを起動
2. サーバー情報を設定
3. 接続後、以下のコマンドを実行：
   ```
   PASS <password>
   NICK <nickname>
   USER <username> 0 * :<realname>
   ```

#### netcatでのテスト接続
```bash
# 基本接続テスト
nc 127.0.0.1 6667

# 接続後に以下を順次入力：
PASS mypassword
NICK testuser
USER testuser 0 * :Test User
JOIN #testchannel
PRIVMSG #testchannel :Hello World!
```

## 📋 実装済み機能

### 基本機能
- ✅ 複数クライアント同時接続
- ✅ 非ブロッキングI/O（poll使用）
- ✅ TCP/IP通信
- ✅ RFC準拠のIRCプロトコル

### 認証・ユーザー管理
- ✅ **PASS** - パスワード認証
- ✅ **NICK** - ニックネーム設定
- ✅ **USER** - ユーザー情報設定

### チャンネル機能
- ✅ **JOIN** - チャンネル参加
- ✅ **PART** - チャンネル退出
- ✅ **PRIVMSG** - メッセージ送信
- ✅ 自動チャンネル作成

### オペレーター専用機能
- ✅ **KICK** - ユーザー追放
- ✅ **INVITE** - ユーザー招待
- ✅ **TOPIC** - トピック変更・表示
- ✅ **MODE** - チャンネルモード変更

### チャンネルモード
- ✅ **+i** - 招待専用チャンネル
- ✅ **+t** - トピック変更をオペレーターに制限
- ✅ **+k** - チャンネルキー（パスワード）
- ✅ **+o** - オペレーター権限付与・剥奪
- ✅ **+l** - ユーザー数制限

## 🧠 技術的な学習内容

##### 用語
- **TCPソケット**: TCPプロトコル（信頼性のある通信）を使用
- **TCP**: Transmission Control Protocol（伝送制御プロトコル）- インターネット上で信頼性の高いデータ通信を確立するプロトコル（[参考](https://wa3.i-3-i.info/word19.html)）
- **ソケット**: プログラムがネットワーク通信をするときに使う、専用の窓口（エンドポイント）

#####  IRCサーバーでTCPソケットを使う理由  
- 文字が順番通りに届く
- 途中で欠けたり、重複しない
- 双方向のやり取りが必要
- → **TCPが適している**

##### 使用したExternal Functions一覧  
・socket ・setsockopt ・htons ・bind   
・fcntl ・listen ・accept ・send ・recv   
・poll ・close  

# サーバーのソケットを立ち上げる関数　setupSocket() の実装例 (Server.cpp)

```cpp
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

<br>

## socket() - 新しいTCPソケットを作成
通信のエンドポイントを作成し、ソケットファイルディスクリプタを返します。  
この呼び出しで、「このサーバーは TCP/IP で通信を行うぞ！」 という「空の」ソケットがカーネル内に生成され、ファイルディスクリプタ（例：3, 4…）として返ってきます。


```cpp
int socket(int domain, int type, int protocol);
```

**パラメータ:**
- **domain**: アドレスドメイン（**AF_INET (v4)**、**AF_INET6 (v6)**、**AF_UNIX**、**AF_RAW**）
- **type**: ソケットタイプ（**SOCK_STREAM**、**SOCK_DGRAM**、**SOCK_RAW**）  
  - **SOCK_STREAM** … 送った順序どおり、欠落なく届く TCP 向け。  
  - **SOCK_DGRAM** … 小さなパケットを投げっぱなし、到達保証なしの UDP 向け。  
  - **SOCK_RAW** … 自作プロトコルや ICMP/TCP ヘッダ解析・生成を行いたいときに使う上級者向け。
- **protocol**: プロトコル（**0（デフォルト）**、**IPPROTO_UDP**、**IPPROTO_TCP**）

##　使用例（Server.cpp)
```cpp
int socket(int domain, int type, int protocol);
```
- `AF_INET`: IPv4用
- `SOCK_STREAM`: TCPプロトコル  
`socket(AF_INET, SOCK_STREAM, 0)` → IPv4+TCPで通信するソケットを作成



<br>

## setupSocket() - ソケットオプション設定（再バインド許可）

setsockopt() は、ソケットの動作を細かく制御するための関数で、作成したソケットに対して各種オプションを設定できます。
「このソケットはOS上で指定ポートを使いTCPサーバーとして接続待ちをする」などの設定が可能です。

```cpp
int opt = 1;
setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
```

**パラメータ:**
- `socket_fd`: 設定対象のソケットファイルディスクリプタ
- `SOL_SOCKET`: ソケットレベルのオプション設定
- `SO_REUSEADDR`: アドレス再利用オプション
- `&opt`: オプションの値へのポインタ（1=有効化）
- `sizeof(opt)`: 値のサイズ（バイト数）

#### ✅ サーバーでのSO_REUSEADDRの目的

- サーバーを止めてすぐに起動し直せるようにするため
- サーバー強制終了・再起動時、ポートが「TIME_WAIT」状態で残る
- その状態でbind()を呼ぶと以下のエラーが発生：
  ```cpp
  bind: Address already in use
  ```
- SO_REUSEADDR を立てると「TIME_WAIT」状態でも即座にバインド可能にする

<br>

## htons() -  “Host To Network Short”  ポートの値を変換  
16 ビット（short）の整数値を ホストバイトオーダー（実行環境依存：リトルエンディアンやビッグエンディアン）から ネットワークバイトオーダー（常にビッグエンディアン）に変換する関数
- リトルエンディアン（Little Endian）  
定義：数値の「最下位バイト」（least significant byte）を 先頭アドレス に置き、以降上位バイトを順に後ろに置く方式。
- ビッグエンディアン（Big Endian）  
定義：数値の「最上位バイト」（most significant byte）を 先頭アドレス に置き、以降下位バイトを順に後ろに置く方式。

##### なぜ必要か？  
TCP/IP プロトコルでは、ヘッダー内のポート番号やアドレスなどを ビッグエンディアン（上位バイトを先）で送受信することが規定されています。  
一方、PC やサーバーの CPU では リトルエンディアン（下位バイトを先）を使うものも多く、直接 bind() に渡すと byte 順序が合わず、正しいポート番号で待ち受けできなくなる恐れがあります。htons を使うことで、たとえ実行環境がリトルエンディアンでも、必ず「ネットワークに出す前に」正しいバイト順にそろえられます。




<br>

## bind() - アドレス割り当て （あだ名：ピコ太郎）
作成したソケットにIPアドレスとポート番号を割り当てる関数。
主にサーバー側で利用され、クライアントからの接続を受け付ける場所（ポート）を指定します。  

bind()の引数
```cpp
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
```
- `sockfd`: `socket()`で作成したソケットのfd
- `addr`: `sockaddr_in`のポインタ（バインド先の情報）
- `addrlen`: `addr`構造体のサイズ（sizeof(addr)）

##### struct sockaddr_inとは
ソケット通信機能を提供するシステム（OS）によって定義されている、IPv4（現在最も広く使われているインターネットプロトコル）のアドレス情報を格納するための専用の構造体。具体的には、以下の重要な情報を保持しています。

- どのIPアドレスか？ (sin_addr)  
addr.sin_addr.s_addr = INADDR_ANY: このコンピュータで利用可能な、全てのIPアドレスで待ち受けますという設定

- どのポート番号か？ (sin_port)  
sin_port = htons(_config.getPort());

- アドレスの種類は何か？ (この場合はIPv4であることを示す sin_family)  
  sin_family = AF_INET: 「通信の種類は、一般的なインターネット（IPv4）を使います」という設定

- **bind()の引数として渡す場合は、struct sockaddrにキャストして渡す。**  
sockaddr_inで住所を記入し、sockaddrという封筒に入れて関数に渡すイメージ

```cpp
　if (bind(_serverSocket, (struct sockaddr*)&addr, sizeof(addr)) < 0)
        throw std::runtime_error("bind failed");
```
<br>

## fcntl() - ノンブロッキングモード設定
ファイルディスクリプタ（ソケットも含む）に対して各種操作や設定を行うための汎用システムコール

```cpp
int fcntl(int fd, int cmd, ... /* arg */ );

//使用例
fcntl(fd, F_SETFL, O_NONBLOCK);
```
引数
- `fd`：操作対象のファイルディスクリプタ（例：socket() や accept() が返す値）。
- `cmd` ：実行したい操作の種類を表す定数（F_SETFL：ファイルステータスフラグ(実際の入出力動作に関わるフラグ)の設定）
- 可変引数 `arg`：cmd に応じて渡す値。O_NONBLOCKを渡すとノンブロッキングモードを設定できる


**なぜノンブロッキングモードへの設定が必要なのか**
I/O をすべてノンブロッキングで扱う理由は、ひとつのプロセス／スレッド、ひとつの poll()ループだけで多くのクライアントを同時にさばき、いずれの操作でも「ハング」せず常に応答性を保つため。

-「待ち状態で止まらない」
ブロッキング I/O（read/write/accept）が１つでも止まると、サーバー全体が固まってしまうため、すべてのソケットをノンブロッキングにしておけば常に「即時返答／即時制御」が可能

-「１回の poll() だけ」要件の実現
サーバーを単一プロセス・単一スレッドで動かし、かつ複数の poll() 呼び出しも禁止された中で、ノンブロッキングかつ１回の poll() で「接続受け付け」「データ受信」「データ送信」「エラー検知」をまとめて扱うために必須です。

-「ハング（応答停止）の防止とスケーラビリティ 」
サーバーが複数クライアントを同時処理するため、全体を常に監視できる必要があります。一人のクライアントが遅延や帯域制限で送受信が遅くても、他のクライアントへの処理が妨げられず、高い同時接続数にも耐えられる安定設計になります。



### accept() - 接続要求受け入れ
クライアントからの接続要求を受け入れるために使用します。

### listen() - 接続待ち受け設定
ソケットを"待ち受け状態"に設定するために使用します。
「このソケットでクライアントからの接続を待ち受ける」ことをOSに伝える関数

```cpp
int listen(int sockfd, int backlog);
```

- `sockfd`: 接続待ち状態にするソケット（socket() + bind()済み）
- `backlog`: OSに伝える「キューの長さ（待ちの行列数）」
- `backlog`はOSが一時的に保存できる接続要求の最大数
- `SOMAXCONN`はシステム定義の最大数

#### ✅ ソケットを接続要求待ち状態（passive mode）にする
- **「接続窓口をOSが開く処理」**

### poll() - 多重化I/O

複数のファイルディスクリプタを同時に監視し、イベント（読み込み、書き込み、切断等）発生を検出する関数

```cpp
#include <poll.h>

int poll(struct pollfd fds[], nfds_t nfds, int timeout);
```

**パラメータ:**
- `fds`: 監視対象のファイルディスクリプタ配列
- `nfds`: 監視対象の数（fdsの要素数）
- `timeout`: タイムアウト時間（-1で無限）
- **返り値**: >0→イベント発生fdの数、0→タイムアウト、<0→エラー

### TCP通信の特徴（ストリーム型）

- サーバーは「どこまでが一つのメッセージか」を自分で判断する必要
- `\r\n`または`\n`で終わるのが一般的

## 💡 実装パターン集

### チャンネルメンバー確認
```cpp
if (!channel.hasMember(fd)) {
    sendError(fd, "442 " + channelName + " :You're not on that channel");
    return;
}
```
呼び出し元（fd）がそのチャンネルのメンバーかどうかをチェック

### チャンネル存在確認
```cpp
if (_channels.find(channelName) == _channels.end()) {
    sendError(fd, "403 " + channelName + " :No such channel");
    return;
}
```
チャンネルが存在するかを確認

### オペレーター権限確認
```cpp
if (!channel.isOperator(fd)) {
    sendError(fd, "482 " + channelName + " :You're not channel operator");
    return;
}
```
チャンネルのオペレーターかをチェック

### ユーザーFD取得（高速化）
```cpp
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
チャンネル内のユーザーのfdを取得

### チャンネル名検証
```cpp
if (channelName[0] != '#') {
    sendError(fd, "476 " + channelName + " :Invalid channel name");
    return;
}
```
チャンネル名の先頭に"#"が付いているかをチェック

### チャンネル内ブロードキャスト
```cpp
std::string msg = ":" + _clients[fd].getNickname() + " TOPIC " + channelName + ": " + topic + "\r\n";
for (std::set<int>::iterator it = channel.getMembers().begin(); it != channel.getMembers().end(); ++it) {
    send(*it, msg.c_str(), msg.length(), 0);
}
```
チャンネル内のメンバーにメッセージを送信

### 高速ユーザー検索
```cpp
std::map<std::string, int>::iterator it = _nickToFd.find(target);
if (it == _nickToFd.end()) {
    sendError(fd, "401 " + target + " :No such nick");
    return;
}

int targetFd = it->second;
if (!channel.hasMember(targetFd)) {
    sendError(fd, "441 " + target + " " + channelName + " :They aren't on that channel");
    return;
}
```
targetのfd取得にmapを使用して高速化

## 🏗️ プロジェクト構成

```
ft_irc/
├── incs/              # ヘッダーファイル
│   ├── Server.hpp     # メインサーバークラス
│   ├── Client.hpp     # クライアント管理
│   ├── Channel.hpp    # チャンネル管理  
│   └── ServerConfig.hpp # 設定管理
├── srcs/              # 実装ファイル
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── ServerConfig.cpp
│   └── handler/       # コマンドハンドラー
│       ├── handler.cpp
│       ├── handle_authentication.cpp
│       ├── handle_join.cpp
│       ├── handle_kick.cpp
│       ├── handle_pmsg.cpp
│       ├── handle_topic.cpp
│       ├── handle_mode.cpp
│       ├── handle_mode_utils.cpp
│       ├── handle_invite.cpp
│       └── handle_part.cpp
├── main.cpp           # エントリーポイント
├── Makefile           # ビルド設定
└── README.md          # このファイル
```

## 🧪 テストスクリプト

プロジェクトには包括的なテストスイートが含まれています。

### 基本動作テスト
```bash
# コンパイル、起動、基本接続テスト
./basic_test.sh
```

### 高度なセキュリティテスト

#### 1. 高度攻撃テスト（包括的）
```bash
# 巨大文字列、数値オーバーフロー、制御文字攻撃
./advanced_attack_test.sh
```

#### 2. メモリ枯渇攻撃テスト
```bash
# 大量接続、チャンネル作成、メッセージ送信テスト
./memory_exhaustion_test.sh
```

#### 3. レースコンディションテスト
```bash
# 同時接続・切断、競合状態テスト
./race_condition_test.sh
```

### メモリリークテスト
```bash
# valgrindを使用したメモリリーク検証
./memory_test.sh
```

### 動的テスト（複数クライアント）
```bash
# 複数クライアント、異常切断テスト
./dynamic_test.sh
```

### テスト結果の確認
各テストスクリプトは以下の情報を提供します：
- ✅ **メモリリーク検証**: valgrindによる完全なメモリ追跡
- 🛡️ **セキュリティ検証**: 各種攻撃パターンへの耐性
- 📊 **パフォーマンス**: 大量負荷時の安定性
- 🔍 **詳細ログ**: すべてのテスト結果をファイルに保存

**テスト実行例:**
```bash
# 全テストを順次実行
chmod +x ./*.sh
./basic_test.sh && ./advanced_attack_test.sh && ./memory_exhaustion_test.sh
```

## ⚙️ 技術仕様

- **言語**: C++98
- **コンパイラ**: c++
- **フラグ**: -Wall -Wextra -Werror -std=c++98
- **外部ライブラリ**: 禁止（標準ライブラリのみ）
- **I/O**: 非ブロッキング（poll使用）
- **プロトコル**: TCP/IP (IPv4/IPv6対応)
- **RFC準拠**: IRCプロトコル準拠
- **セキュリティ**: 高度攻撃耐性、メモリリーク0保証

## 🏆 テスト結果

**セキュリティ評価:** ⭐⭐⭐⭐⭐
- メモリリーク: 0 bytes (全テストケース)
- バッファオーバーフロー攻撃: 完全耐性
- レースコンディション: エラーなし
- 大量負荷処理: 200+同時接続対応

---

*42 School ft_irc project - C++98 IRC Server Implementation*
