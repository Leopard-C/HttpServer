/**
 * @file logger.h
 * @brief 日志记录器.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2024-03-02
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_LOGGER_H_
#define IC_SERVER_LOGGER_H_
#include <inttypes.h>
#include <string>

#ifndef __STDC_FORMAT_MACROS
#  define __STDC_FORMAT_MACROS
#endif

namespace ic {
namespace server {

/**
 * @brief 日志级别.
 */
enum class LogLevel {
    kDebug = 1,
    kInfo = 2,
    kWarn = 3,
    kError = 4,
    kOff = 5
};

const char* to_string_long(LogLevel level);

const char* to_string_short(LogLevel level);

/**
 * @brief 日志记录上下文.
 */
struct LogContext {
    LogContext(const char* file_name, const char* func_name, int line_num)
        : file_name(file_name), func_name(func_name), line_num(line_num) {}
    /** 当前源文件名称 */
    const char* file_name;
    /** 当前函数名称 */
    const char* func_name;
    /** 行号 */
    int line_num;
};

/**
 * @brief 日志记录器(抽象类).
 */
class ILogger {
public:
    ILogger(LogLevel min_level) : min_level_(min_level) {}
    virtual ~ILogger() {};

    LogLevel min_level() const { return min_level_; }
    void set_min_level(LogLevel min_level) { min_level_ = min_level; }

    void Debug(const LogContext& context, const char* format, ...) const;
    void Info(const LogContext& context, const char* format, ...) const;
    void Warn(const LogContext& context, const char* format, ...) const;
    void Error(const LogContext& context, const char* format, ...) const;

    void Debug(const LogContext& context, const std::string& msg) const;
    void Info(const LogContext& context, const std::string& msg) const;
    void Warn(const LogContext& context, const std::string& msg) const;
    void Error(const LogContext& context, const std::string& msg) const;

protected:
    /**
     * @brief 日志记录接口，派生类重写该方法.
     * @param level 日志级别
     * @param context 当前上下文
     * @param msg 日志内容
     */
    virtual void Log(LogLevel level, const LogContext& context, const std::string& msg) const = 0;

protected:
    /** 忽略低于该级别的日志 */
    LogLevel min_level_{LogLevel::kInfo};
};

/**
 * @brief 默认的日志记录器，输出到控制台.
 */
class ConsoleLogger : public ILogger {
public:
    /**
     * @brief 构造函数.
     * 
     * @param min_level 只打印等于或高于该级别的日志
     * @param detailed_min_level 等于或高于该级别的日志，显示详细信息(当前文件、当前行、当前函数名等).
     */
    ConsoleLogger(LogLevel min_level, LogLevel detailed_min_level)
        : ILogger(min_level), detailed_min_level_(detailed_min_level) {}

protected:
    /**
     * @brief 日志记录.
     * @param level 日志级别
     * @param context 当前上下文
     * @param msg 日志内容
     */
    virtual void Log(LogLevel level, const LogContext& context, const std::string& msg) const override;

private:
    /**
     * @brief 等于或高于该级别的日志，显示详细信息(当前文件、当前行、当前函数名等).
     */
    LogLevel detailed_min_level_{LogLevel::kWarn};
};

} // namespace server
} // namespace ic

#define LOG_CTX LogContext(__FILE__, __func__, __LINE__)

#endif // IC_SERVER_LOGGER_H_
