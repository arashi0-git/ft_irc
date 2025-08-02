#!/bin/bash

echo "=== ft_irc 基本動作テスト ==="

# 色付き出力用
GREEN='\033[0;32m'
RED='\033[0;31m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# テスト結果記録
TESTS_PASSED=0
TESTS_FAILED=0

# テスト関数
test_result() {
    if [ $1 -eq 0 ]; then
        echo -e "${GREEN}✅ PASS${NC}: $2"
        ((TESTS_PASSED++))
    else
        echo -e "${RED}❌ FAIL${NC}: $2"
        ((TESTS_FAILED++))
    fi
}

echo "1. コンパイルテスト..."
make re > /dev/null 2>&1
test_result $? "make re (リビルド)"

if [ ! -f "./ircserv" ]; then
    echo -e "${RED}❌ 実行ファイル ircserv が見つかりません。コンパイルに失敗しました。${NC}"
    exit 1
fi

echo ""
echo "2. サーバー起動テスト..."

# 引数なしでの起動テスト（エラーが期待される）
./ircserv > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1
if kill -0 $SERVER_PID 2>/dev/null; then
    kill $SERVER_PID
    test_result 1 "引数なしでの起動（エラーが期待される）"
else
    test_result 0 "引数なしでの起動（適切にエラー終了）"
fi

# 不正な引数での起動テスト
./ircserv abc def > /dev/null 2>&1 &
SERVER_PID=$!
sleep 1
if kill -0 $SERVER_PID 2>/dev/null; then
    kill $SERVER_PID
    test_result 1 "不正な引数での起動（エラーが期待される）"
else
    test_result 0 "不正な引数での起動（適切にエラー終了）"
fi

# 正常な起動テスト
echo ""
echo "3. 正常サーバー起動テスト（ポート6667）..."
./ircserv 6667 testpass > server_output.log 2>&1 &
SERVER_PID=$!
sleep 2

if kill -0 $SERVER_PID 2>/dev/null; then
    test_result 0 "サーバー起動（ポート6667、パスワード: testpass）"
else
    test_result 1 "サーバー起動失敗"
    echo "サーバーログを確認してください:"
    cat server_output.log
    exit 1
fi

echo ""
echo "4. 基本接続テスト..."

# netcatでの接続テスト
(echo -e "PASS testpass\r\nNICK testuser\r\nUSER testuser 0 * :Test User\r\nQUIT\r\n"; sleep 3) | nc 127.0.0.1 6667 > client_output.log 2>&1 &
CLIENT_PID=$!
sleep 4

# クライアント接続結果確認
if grep -q "001.*Welcome" client_output.log; then
    test_result 0 "クライアント認証・接続"
else
    test_result 1 "クライアント認証・接続失敗"
    echo "クライアント出力:"
    cat client_output.log
fi

echo ""
echo "5. チャンネル機能テスト..."

# チャンネル参加・メッセージ送信テスト
(echo -e "PASS testpass\r\nNICK channeluser\r\nUSER channeluser 0 * :Channel User\r\nJOIN #testchannel\r\nPRIVMSG #testchannel :Hello Channel!\r\nPART #testchannel\r\nQUIT\r\n"; sleep 5) | nc 127.0.0.1 6667 > channel_test.log 2>&1 &
sleep 6

# チャンネル機能結果確認
if grep -q "JOIN #testchannel" channel_test.log && grep -q "366.*End of /NAMES list" channel_test.log; then
    test_result 0 "チャンネル参加・退出"
else
    test_result 1 "チャンネル参加・退出失敗"
    echo "チャンネルテスト出力:"
    cat channel_test.log
fi

echo ""
echo "6. 複数クライアント接続テスト..."

# 複数クライアント同時接続
(echo -e "PASS testpass\r\nNICK user1\r\nUSER user1 0 * :User One\r\nJOIN #multiclient\r\nPRIVMSG #multiclient :User1 here\r\n"; sleep 8) | nc 127.0.0.1 6667 > user1.log 2>&1 &
(echo -e "PASS testpass\r\nNICK user2\r\nUSER user2 0 * :User Two\r\nJOIN #multiclient\r\nPRIVMSG #multiclient :User2 here\r\n"; sleep 8) | nc 127.0.0.1 6667 > user2.log 2>&1 &

sleep 10

# 複数クライアント結果確認
USER1_SUCCESS=0
USER2_SUCCESS=0

if grep -q "001.*Welcome" user1.log && grep -q "JOIN #multiclient" user1.log; then
    USER1_SUCCESS=1
fi

if grep -q "001.*Welcome" user2.log && grep -q "JOIN #multiclient" user2.log; then
    USER2_SUCCESS=1
fi

if [ $USER1_SUCCESS -eq 1 ] && [ $USER2_SUCCESS -eq 1 ]; then
    test_result 0 "複数クライアント同時接続"
else
    test_result 1 "複数クライアント同時接続失敗"
fi

echo ""
echo "7. 異常切断テスト..."

# 突然の切断テスト
(echo -e "PASS testpass\r\nNICK disconnectuser\r\nUSER disconnectuser 0 * :Disconnect User\r\nJOIN #disconnect\r\n"; sleep 2) | nc 127.0.0.1 6667 &
DISCONNECT_PID=$!
sleep 3
kill -9 $DISCONNECT_PID 2>/dev/null

# サーバーがまだ動作しているか確認
sleep 2
if kill -0 $SERVER_PID 2>/dev/null; then
    test_result 0 "異常切断後のサーバー安定性"
else
    test_result 1 "異常切断でサーバーがクラッシュ"
fi

echo ""
echo "8. サーバー終了..."
kill $SERVER_PID 2>/dev/null
sleep 2

echo ""
echo "=== テスト結果サマリー ==="
echo -e "${GREEN}成功: $TESTS_PASSED${NC}"
echo -e "${RED}失敗: $TESTS_FAILED${NC}"
echo "総テスト数: $((TESTS_PASSED + TESTS_FAILED))"

if [ $TESTS_FAILED -eq 0 ]; then
    echo -e "${GREEN}🎉 全てのテストが成功しました！${NC}"
    EXIT_CODE=0
else
    echo -e "${YELLOW}⚠️  一部のテストが失敗しました。ログファイルを確認してください。${NC}"
    EXIT_CODE=1
fi

echo ""
echo "生成されたログファイル:"
echo "- server_output.log (サーバー出力)"
echo "- client_output.log (クライアント接続テスト)"
echo "- channel_test.log (チャンネル機能テスト)"
echo "- user1.log, user2.log (複数クライアントテスト)"

echo ""
echo "=== 基本動作テスト完了 ==="

exit $EXIT_CODE