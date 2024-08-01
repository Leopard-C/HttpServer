/* Visual Leak Detector 内存泄露检测工具  */
#ifdef _WIN32
#  define _USE_VLD 0
#  if _USE_VLD == 1
#    pragma comment(lib, "D:/CPP_INCLUDE/vld-2.5.1/shared/x64/lib/vld.lib")
#    include <D:/CPP_INCLUDE/vld-2.5.1/shared/x64/include/vld.h>
#  endif // _USE_VLD
#endif // _WIN32

#ifdef _WIN32
#  include <Windows.h>
#else
#  include <signal.h>
#  include <unistd.h>
#endif
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
#endif // _WIN32

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
