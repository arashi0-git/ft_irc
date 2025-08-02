# IRC プロトコル実装詳細

## 実装済みコマンド
- **PASS**: パスワード認証
- **NICK**: ニックネーム設定
- **USER**: ユーザー情報設定
- **JOIN**: チャンネル参加
- **PART**: チャンネル退出
- **PRIVMSG**: プライベートメッセージ送信
- **KICK**: ユーザーをチャンネルから追放（オペレーター専用）
- **INVITE**: ユーザーをチャンネルに招待（オペレーター専用）
- **TOPIC**: チャンネルトピック変更・表示（オペレーター専用）
- **MODE**: チャンネルモード変更（オペレーター専用）

## チャンネルモード
- **i**: 招待専用チャンネル（invite-only）
- **t**: トピック変更をオペレーターに制限
- **k**: チャンネルキー（パスワード）設定
- **o**: オペレーター権限付与・剥奪
- **l**: ユーザー数制限設定

## エラーコード（RFC準拠）
- **401**: No such nick
- **403**: No such channel
- **441**: They aren't on that channel
- **442**: You're not on that channel
- **461**: Not enough parameters
- **476**: Invalid channel name
- **482**: You're not channel operator

## プロトコル仕様
- メッセージ終端: `\r\n`
- チャンネル名: `#`で始まる
- 非ブロッキングI/O使用
- poll()による多重化I/O
- TCP/IP通信