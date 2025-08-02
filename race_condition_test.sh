#!/bin/bash

echo "=== 競合状態（レースコンディション）テスト ==="

# valgrindでサーバー起動
echo "1. valgrindでサーバー起動中..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=race_condition.log ./ircserv 8087 racetest &
VALGRIND_PID=$!

sleep 3

echo "2. 同時接続・切断テスト（レースコンディション）..."

# 複数のクライアントが同時に接続・切断を繰り返す
for i in {1..50}; do
    (
        # 高速接続・切断
        echo -e "PASS racetest\r\nNICK race$i\r\nUSER race$i 0 * :Race User $i\r\nJOIN #race\r\nQUIT :Bye\r\n" | nc 127.0.0.1 8087 &
        sleep 0.1
        kill $! 2>/dev/null  # 強制切断でレースコンディションを誘発
    ) &
done

echo "3. 同一チャンネルへの同時JOIN・PART攻撃..."

# 同じチャンネルに複数ユーザーが同時参加・離脱
for i in {1..20}; do
    (
        echo -e "PASS racetest\r\nNICK concurrent$i\r\nUSER concurrent$i 0 * :Concurrent $i\r\n"
        for j in {1..10}; do
            echo -e "JOIN #racechannel\r\nPART #racechannel\r\n"
        done
        sleep 2
    ) | nc 127.0.0.1 8087 &
done

echo "4. 同時ニックネーム変更攻撃..."

# 複数クライアントが同時に同じニックネームに変更を試行
TARGET_NICK="popular_name"
for i in {1..15}; do
    (
        echo -e "PASS racetest\r\nNICK initial$i\r\nUSER initial$i 0 * :Initial $i\r\n"
        sleep 1
        echo -e "NICK $TARGET_NICK\r\n"  # 全員が同じニックネームに変更を試みる
        sleep 2
    ) | nc 127.0.0.1 8087 &
done

echo "5. 高頻度メッセージ送信（タイミング攻撃）..."

# 複数クライアントが同一チャンネルに高頻度でメッセージ送信
for i in {1..10}; do
    (
        echo -e "PASS racetest\r\nNICK spammer$i\r\nUSER spammer$i 0 * :Spammer $i\r\nJOIN #spam\r\n"
        for j in {1..100}; do
            echo -e "PRIVMSG #spam :High frequency message $j from spammer$i\r\n"
        done
    ) | nc 127.0.0.1 8087 &
done

echo "6. ファイルディスクリプタ枯渇攻撃..."

# 大量の接続を短時間で作成（ファイルディスクリプタ制限テスト）
for i in {1..200}; do
    (echo -e "PASS racetest\r\nNICK fd$i\r\nUSER fd$i 0 * :FD Test $i\r\n"; sleep 15) | nc 127.0.0.1 8087 &
done

echo "7. レースコンディション実行中... (20秒)"
sleep 20

echo "8. 結果確認..."
kill $VALGRIND_PID
sleep 5

if [ -f race_condition.log ]; then
    echo "=== レースコンディションテスト結果 ==="
    grep -A 10 "HEAP SUMMARY" race_condition.log
    echo ""
    grep -A 5 "LEAK SUMMARY" race_condition.log
    echo ""
    echo "エラー確認:"
    grep -A 3 "ERROR SUMMARY" race_condition.log || echo "メモリエラーなし"
    echo ""
    echo "レースコンディション関連エラー:"
    grep -i "race\|deadlock\|corruption" race_condition.log || echo "レースコンディションエラーなし"
else
    echo "valgrindログファイルが見つかりません"
fi

echo "=== レースコンディションテスト完了 ==="