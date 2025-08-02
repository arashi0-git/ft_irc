# コードスタイルと規約

## 命名規約
- **クラス名**: PascalCase (例: `Server`, `Channel`, `Client`, `ServerConfig`)
- **メンバ変数**: アンダースコアプレフィックス (例: `_port`, `_password`, `_clients`)
- **メソッド名**: camelCase (例: `getPort`, `setNickname`, `handleClient`)
- **定数**: 大文字 (例: `SOMAXCONN`)

## ファイル構造
- **ヘッダーファイル**: `incs/` ディレクトリ (.hpp)
- **実装ファイル**: `srcs/` ディレクトリ (.cpp)
- **ハンドラー**: `srcs/handler/` ディレクトリに機能別に分離

## コーディング規約
- C++98標準準拠
- STLコンテナの使用 (`std::map`, `std::vector`, `std::string`, `std::set`)
- イテレータの明示的な型指定
- メモリ管理: RAII パターンの使用
- エラーハンドリング: 例外処理 (`try-catch`, `throw`)

## 設計パターン
- **単一責任の原則**: 各クラスは明確な責務を持つ
- **データカプセル化**: プライベートメンバ変数とpublicアクセサメソッド
- **コマンドパターン**: ハンドラー関数による機能分離
- **ファクトリーパターン**: 設定オブジェクトによる初期化