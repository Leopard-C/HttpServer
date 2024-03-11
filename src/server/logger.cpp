#include "server/logger.h"
#include "server/util/format_time.h"
#include <cstdarg>
#include <cstdio>
#include <cstring>

namespace ic {
namespace server {

const char* to_string_long(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug: return "DEBUG";
        case LogLevel::kInfo: return "INFORMATION";
        case LogLevel::kWarn: return "WARNING";
        case LogLevel::kError:
        default: return "ERROR";
    }
}

const char* to_string_short(LogLevel level) {
    switch (level) {
        case LogLevel::kDebug: return "DBG";
        case LogLevel::kInfo: return "INF";
        case LogLevel::kWarn: return "WRN";
        case LogLevel::kError:
        default: return "ERR";
    }
}

#define LOG_FORMAT_STR(level) \
    if (level < min_level_) {\
        return;\
    }\
    va_list args, args_copy;\
    va_start(args, format);\
    va_copy(args_copy, args);\
    constexpr int INIT_BUFF_LEN = 127;\
    std::string msg(INIT_BUFF_LEN, '\0');\
    int len = vsnprintf(&msg[0], INIT_BUFF_LEN + 1, format, args_copy);\
    va_end(args_copy);\
    if (len > INIT_BUFF_LEN) {\
        msg.resize(len, '\0');\
        vsnprintf(&msg[0], len + 1, format, args);\
    }\
    va_end(args);\
    Log(level, context, msg)

void ILogger::Debug(const LogContext& context, const char* format, ...) const {
    LOG_FORMAT_STR(LogLevel::kDebug);
}

void ILogger::Info(const LogContext& context, const char* format, ...) const {
    LOG_FORMAT_STR(LogLevel::kInfo);
}

void ILogger::Warn(const LogContext& context, const char* format, ...) const {
    LOG_FORMAT_STR(LogLevel::kWarn);
}

void ILogger::Error(const LogContext& context, const char* format, ...) const {
    LOG_FORMAT_STR(LogLevel::kError);
}

void ILogger::Debug(const LogContext& context, const std::string& msg) const {
    if (min_level_ <= LogLevel::kDebug) {
        Log(LogLevel::kDebug, context, msg);
    }
}

void ILogger::Info(const LogContext& context, const std::string& msg) const {
    if (min_level_ <= LogLevel::kInfo) {
        Log(LogLevel::kInfo, context, msg);
    }
}

void ILogger::Warn(const LogContext& context, const std::string& msg) const {
    if (min_level_ <= LogLevel::kWarn) {
        Log(LogLevel::kWarn, context, msg);
    }
}

void ILogger::Error(const LogContext& context, const std::string& msg) const {
    if (min_level_ <= LogLevel::kError) {
        Log(LogLevel::kError, context, msg);
    }
}

/**
 * @brief 日志记录.
 * @param level 日志级别
 * @param context 当前上下文
 * @param msg 日志内容
 */
void ConsoleLogger::Log(LogLevel level, const LogContext& context, const std::string& msg) const {
    auto now = std::chrono::system_clock::now();
    if (level < detailed_min_level_) {
        printf("%s [%s] %s\n", util::format_time_ms(now).c_str(), to_string_short(level), msg.c_str());
    }
    else {
        printf("%s [%s] %s <%s> <%s:%d>\n", util::format_time_ms(now).c_str(),
            to_string_short(level), msg.c_str(), context.func_name, context.file_name, context.line_num);
    }
}

} // namespace server
} // namespace ic
