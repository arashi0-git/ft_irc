#include <csignal>

bool g_signal = true;
void signal_handler(int signal)
{
    (void)signal;
    g_signal = false;
}