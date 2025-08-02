# 推奨コマンド

## ビルドコマンド
```bash
# 全体をビルド
make

# クリーンビルド
make re

# オブジェクトファイル削除
make clean

# 実行ファイルも含めて削除
make fclean
```

## 実行コマンド
```bash
# サーバー起動
./ircserv <port> <password>

# 例: ポート6667、パスワード"secret"で起動
./ircserv 6667 secret
```

## テストコマンド
```bash
# netcatでの基本テスト
nc -C 127.0.0.1 6667

# 分割送信テスト（subject.txt記載）
# ctrl+Dで'com', 'man', 'd\n'を順次送信
nc -C 127.0.0.1 6667
```

## IRC クライアントでのテスト
- HexChat, WeeChat, irssi等のIRCクライアントを使用
- サーバー: 127.0.0.1, ポート: 設定したポート番号
- パスワード: 設定したパスワード

## デバッグ用コマンド
```bash
# プロセス確認
ps aux | grep ircserv

# ポート使用状況確認
netstat -tuln | grep <port>

# ログ出力（必要に応じて）
./ircserv 6667 secret 2>&1 | tee server.log
```