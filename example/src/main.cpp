#ifdef _WIN32
#  include <Windows.h>
#else
#  include <signal.h>
#  include <unistd.h>
#endif
#include <log/logger.h>
#include "app/app.h"
#include "singleton/singleton.h"

static void StopApplication() {
    auto application = ic::Singleton<Application>::GetInstance();
    if (application) {
        application->Stop();
    }
}

/**
 * @brief 处理Ctrl+C事件.
 */
#ifdef _WIN32
static BOOL ConsoleCtrlHandler(DWORD fwdCtrlType) {
    switch (fwdCtrlType) {
        case CTRL_C_EVENT:
            StopApplication();
            return TRUE;
        case CTRL_CLOSE_EVENT:
            StopApplication();
            return TRUE;
        default:
            return FALSE;
    }
}
#else // Linux
static void CatchCtrlC(int sig) {
    StopApplication();
}
#endif

int main() {
    srand((unsigned int)time(NULL));

    /* 1. 创建并初始化应用程序 */
    auto application = ic::Singleton<Application>::Instance();
    if (!application->Init()) {
        return 1;
    }

#ifdef _WIN32
    SetConsoleCtrlHandler((PHANDLER_ROUTINE)ConsoleCtrlHandler, TRUE);
#else
    signal(SIGINT, CatchCtrlC);
#endif

    /* 2. 启动应用程序 */
    application->Run();

    /* 3. 销毁应用程序 */
    ic::Singleton<Application>::DesInstance();
    return 0;
}
