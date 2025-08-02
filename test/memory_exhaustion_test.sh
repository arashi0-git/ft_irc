#!/bin/bash

echo "=== メモリ枯渇攻撃テスト ==="

# valgrindでサーバー起動
echo "1. valgrindでサーバー起動中..."
valgrind --leak-check=full --show-leak-kinds=all --track-fds=yes --log-file=memory_exhaustion.log ./ircserv 8088 memtest &
VALGRIND_PID=$!

sleep 3

echo "2. 大量接続攻撃テスト（100接続）..."

# 大量のクライアント接続を同時に作成
for i in {1..100}; do
    (echo -e "PASS memtest\r\nNICK user$i\r\nUSER user$i 0 * :User $i\r\nJOIN #flood\r\nPRIVMSG #flood :Flooding $i\r\n"; sleep 10) | nc 127.0.0.1 8088 &
done

echo "3. 大量チャンネル作成攻撃..."

# 単一クライアントで大量のチャンネルを作成
(
    echo -e "PASS memtest\r\nNICK flooduser\r\nUSER flooduser 0 * :Flood User\r\n"
    for i in {1..500}; do
        echo -e "JOIN #channel$i\r\n"
        echo -e "PRIVMSG #channel$i :Channel flood $i\r\n"
    done
    sleep 5
) | nc 127.0.0.1 8088 &

echo "4. 大量メッセージ送信攻撃..."

# 継続的な大量メッセージ送信
(
    echo -e "PASS memtest\r\nNICK spammer\r\nUSER spammer 0 * :Spammer\r\nJOIN #spam\r\n"
    for i in {1..1000}; do
        echo -e "PRIVMSG #spam :Spam message number $i with some additional text to increase memory usage\r\n"
    done
    sleep 5
) | nc 127.0.0.1 8088 &

echo "5. 長時間実行中... (30秒)"
sleep 30

echo "6. メモリ使用量確認..."
ps aux | grep ircserv | grep -v grep | awk '{print "PID: " $2 ", Memory: " $4 "%, VSZ: " $5 "KB, RSS: " $6 "KB"}'

echo "7. 接続終了とサーバー停止..."
kill $VALGRIND_PID
sleep 5

if [ -f memory_exhaustion.log ]; then
    echo "=== メモリ枯渇攻撃テスト結果 ==="
    grep -A 10 "HEAP SUMMARY" memory_exhaustion.log
    echo ""
    grep -A 5 "LEAK SUMMARY" memory_exhaustion.log
    echo ""
    echo "エラー確認:"
    grep -A 3 "ERROR SUMMARY" memory_exhaustion.log || echo "エラーなし"
else
    echo "valgrindログファイルが見つかりません"
fi

echo "=== メモリ枯渇攻撃テストケース完了 ==="