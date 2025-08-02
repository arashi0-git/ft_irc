# コードベース構造

## ディレクトリ構成
```
ft_irc/
├── incs/           # ヘッダーファイル
│   ├── Server.hpp
│   ├── Client.hpp
│   ├── Channel.hpp
│   └── ServerConfig.hpp
├── srcs/           # 実装ファイル
│   ├── Server.cpp
│   ├── Client.cpp
│   ├── Channel.cpp
│   ├── ServerConfig.cpp
│   └── handler/    # コマンドハンドラー
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
├── main.cpp        # エントリーポイント
├── Makefile        # ビルド設定
└── README.md       # 技術ドキュメント

## 主要クラス
- **Server**: メインサーバークラス、ソケット管理、クライアント処理
- **Client**: クライアント情報管理（fd, nickname, username等）
- **Channel**: チャンネル管理（メンバー, オペレーター, モード設定）
- **ServerConfig**: サーバー設定（ポート、パスワード）

## データ構造
- `std::map<int, Client> _clients`: fd→クライアント情報のマッピング
- `std::map<std::string, Channel> channels`: チャンネル名→チャンネル情報
- `std::map<std::string, int> _nickToFd`: ニックネーム→fdの高速検索
- `std::vector<struct pollfd> _fds`: poll()用ファイルディスクリプタ配列