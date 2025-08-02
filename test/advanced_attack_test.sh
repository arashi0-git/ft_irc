#!/bin/bash

echo "=== 高度なセキュリティ攻撃テスト ==="

# valgrindでサーバー起動
echo "1. valgrindでサーバー起動中..."
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=attack_test.log ./ircserv 8089 attacktest &
VALGRIND_PID=$!

sleep 3

echo "2. 巨大文字列攻撃テスト..."

# 10KB, 100KB, 1MBの巨大文字列攻撃
echo "  2.1. 10KB文字列攻撃..."
HUGE_STRING_10K=$(printf 'A%.0s' {1..10240})
(echo -e "PASS attacktest\r\nNICK $HUGE_STRING_10K\r\n"; sleep 2) | nc 127.0.0.1 8089 &

sleep 3

echo "  2.2. 100KB文字列攻撃..."
HUGE_STRING_100K=$(printf 'B%.0s' {1..102400})
(echo -e "PASS attacktest\r\nUSER $HUGE_STRING_100K 0 * :$HUGE_STRING_100K\r\n"; sleep 2) | nc 127.0.0.1 8089 &

sleep 3

echo "  2.3. 1MB文字列攻撃..."
# 1MBは大きすぎるので分割して送信
(
    echo -e "PASS attacktest\r\nNICK megauser\r\nUSER megauser 0 * :Mega User\r\nJOIN #test\r\n"
    for i in {1..1000}; do
        LARGE_MSG=$(printf 'X%.0s' {1..1024})
        echo -e "PRIVMSG #test :$LARGE_MSG\r\n"
    done
) | nc 127.0.0.1 8089 &

sleep 5

echo "3. 数値オーバーフロー攻撃テスト..."

# 巨大なポート番号での接続試行
echo "  3.1. 巨大ポート番号攻撃..."
(echo -e "PASS attacktest\r\nNICK overflow1\r\nUSER overflow1 4294967295 * :Overflow User\r\n"; sleep 2) | nc 127.0.0.1 8089 &

# 負の数値攻撃
echo "  3.2. 負の数値攻撃..."
(echo -e "PASS attacktest\r\nNICK overflow2\r\nUSER overflow2 -2147483648 * :Negative User\r\n"; sleep 2) | nc 127.0.0.1 8089 &

sleep 3

echo "4. 制御文字・不正文字攻撃テスト..."

# NULL文字攻撃
echo "  4.1. NULL文字攻撃..."
printf "PASS attacktest\r\nNICK null\0user\r\nUSER nulluser 0 * :Null User\r\n" | nc 127.0.0.1 8089 &

# 改行文字攻撃
echo "  4.2. 改行文字攻撃..."
(echo -e "PASS attacktest\r\nNICK new\nline\r\nUSER newline 0 * :New Line User\r\n"; sleep 2) | nc 127.0.0.1 8089 &

# バイナリデータ攻撃
echo "  4.3. バイナリデータ攻撃..."
(printf "\x00\x01\x02\x03\xFF\xFE\xFD\xFC"; echo -e "PASS attacktest\r\nNICK binary\r\n"; sleep 2) | nc 127.0.0.1 8089 &

sleep 5

echo "5. SQLインジェクション類似攻撃..."
(echo -e "PASS attacktest\r\nNICK 'OR 1=1--\r\nUSER '; DROP TABLE users; -- 0 * :SQL Injection\r\n"; sleep 2) | nc 127.0.0.1 8089 &

sleep 3

echo "6. 攻撃テスト結果確認..."
kill $VALGRIND_PID
sleep 3

if [ -f attack_test.log ]; then
    echo "=== 攻撃テスト結果 ==="
    grep -A 10 "HEAP SUMMARY" attack_test.log
    echo ""
    grep -A 5 "LEAK SUMMARY" attack_test.log
    echo ""
    echo "エラー確認:"
    grep -A 3 "ERROR SUMMARY" attack_test.log
else
    echo "valgrindログファイルが見つかりません"
fi

echo "=== 高度攻撃テスト完了 ==="