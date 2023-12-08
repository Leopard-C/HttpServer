#include "util/thread.h"
#ifdef _WIN32
#  include <Windows.h>
#elif defined(__linux__)
#  include <sys/syscall.h>
#  include <unistd.h>
#else
#  include <thread>
#endif

namespace ic {
namespace server {
namespace util {

static size_t _thread_id() {
#ifdef _WIN32
    return static_cast<size_t>(::GetCurrentThreadId());
#elif defined(__linux__)
    return static_cast<size_t>(::syscall(SYS_gettid));
#else
    return static_cast<size_t>(std::hash<std::thread::id>()(std::this_thread::get_id()));
#endif
}

size_t thread_id() {
    static thread_local const size_t tid = _thread_id();
    return tid;
}

} // namespace util
} // namespace server
} // namespace ic
