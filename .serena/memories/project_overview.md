# ft_irc プロジェクト概要

## プロジェクトの目的
- C++98標準を使用したIRCサーバーの実装
- クライアント間のリアルタイムメッセージング機能を提供
- チャンネルベースの通信とプライベートメッセージに対応
- RFC準拠のIRCプロトコルを実装

## 技術スタック
- **言語**: C++98
- **コンパイラ**: c++
- **コンパイルフラグ**: -Wall -Wextra -Werror -std=c++98
- **ネットワーク**: TCP/IP (IPv4/IPv6)
- **システムコール**: socket, bind, listen, accept, poll, send, recv等

## 実行方法
```bash
# コンパイル
make

# 実行
./ircserv <port> <password>
```

## 機能
- 複数クライアントの同時接続
- ユーザー認証（パスワード、ニックネーム、ユーザー名）
- チャンネル参加・退出
- プライベートメッセージ送受信
- チャンネルオペレーター機能（KICK, INVITE, TOPIC, MODE）
- チャンネルモード設定（invite-only, topic制限, key, operator権限, ユーザー数制限）