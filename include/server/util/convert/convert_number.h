/**
 * @file convert_number.h
 * @brief 字符串转数字.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present Jinbao Chen.
 */
#ifndef IC_SERVER_UTIL_CONVERT_NUMBER_H_
#define IC_SERVER_UTIL_CONVERT_NUMBER_H_
#include <limits>
#include <string>
#include <stdexcept>

#define UTIL_CONV_ERR_INVALID_STRING() \
    if (err_msg) { *err_msg = "invalid string"; }\
    return false

#define UTIL_CONV_ERR_OVERFLOW() \
    if (err_msg) { *err_msg = "overflow"; }\
    return false

namespace ic {
namespace server {
namespace util {
namespace conv {

namespace _internal {

bool trim(const std::string& str, int* start, int* end, bool* is_negative);
bool is_valid_floating_point(const std::string& str, const int start, const int end);

template<typename T, typename std::enable_if<std::is_same<T, float>::value, int>::type* = nullptr>
inline void str_to_floating_point(const std::string& in, T* out) {
    *out = std::stof(in, NULL);
}

template<typename T, typename std::enable_if<std::is_same<T, double>::value, int>::type* = nullptr>
inline void str_to_floating_point(const std::string& in, T* out) {
    *out = std::stod(in, NULL);
}

template<typename T>
bool convert_negative_integer(const std::string& in, const int start, const int end, T* out, std::string* err_msg) {
    constexpr T min_limit = std::numeric_limits<T>::min();
    constexpr T min_limit_div_10 = min_limit / 10;
    T val = 0;
    T digit;
    for (int i = start; i <= end; ++i) {
        if (in[i] >= '0' && in[i] <= '9') {
            digit = in[i] - '0';
            if (val >= min_limit_div_10) {
                val *= 10;
                if (val >= min_limit + digit) {
                    val -= digit;
                    continue;
                }
            }
            UTIL_CONV_ERR_OVERFLOW();
        }
        UTIL_CONV_ERR_INVALID_STRING();
    }
    *out = val;
    return true;
}

template<typename T>
bool convert_positive_integer(const std::string& in, const int start, const int end, T* out, std::string* err_msg) {
    constexpr T max_limit = std::numeric_limits<T>::max();
    constexpr T max_limit_div_10 = max_limit / 10;
    T val = 0;
    T digit;
    for (int i = start; i <= end; ++i) {
        if (in[i] >= '0' && in[i] <= '9') {
            digit = in[i] - '0';
            if (val <= max_limit_div_10) {
                val *= 10;
                if (val <= max_limit - digit) {
                    val += digit;
                    continue;
                }
            }
            UTIL_CONV_ERR_OVERFLOW();
        }
        UTIL_CONV_ERR_INVALID_STRING();
    }
    *out = val;
    return true;
}

} // namespace _internal

/**
 * @brief 将字符串转为数字，判定标准比较严格，牺牲了部分性能.
 * 
 * @param[in]  in 输入字符串
 * @param[out] out 转换结果
 * @param[out] err_msg 出错信息
 * @retval true 转换成功
 * @retval false 转换失败（空字符串、无效字符、溢出等情况）
 */
template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_signed<T>::value, int>::type* = nullptr>
bool convert_number(const std::string& in, T* out, std::string* err_msg = nullptr) {
    int start, end;
    bool is_negative;
    if (!_internal::trim(in, &start, &end, &is_negative)) {
        UTIL_CONV_ERR_INVALID_STRING();
    }
    return is_negative ?
        _internal::convert_negative_integer<T>(in, start, end, out, err_msg) :
        _internal::convert_positive_integer<T>(in, start, end, out, err_msg);
}

template<typename T, typename std::enable_if<std::is_integral<T>::value && std::is_unsigned<T>::value, int>::type* = nullptr>
bool convert_number(const std::string& in, T* out, std::string* err_msg = nullptr) {
    int start, end;
    bool is_negative;
    if (!_internal::trim(in, &start, &end, &is_negative) || is_negative) {
        UTIL_CONV_ERR_INVALID_STRING();
    }
    return _internal::convert_positive_integer<T>(in, start, end, out, err_msg);
}

template<typename T, typename std::enable_if<std::is_floating_point<T>::value, int>::type* = nullptr>
bool convert_number(const std::string& in, T* out, std::string* err_msg = nullptr) {
    int start, end;
    bool is_negative;
    if (!_internal::trim(in, &start, &end, &is_negative) || !_internal::is_valid_floating_point(in, start, end)) {
        UTIL_CONV_ERR_INVALID_STRING();
    }
    try {
        _internal::str_to_floating_point(in, out);
        return true;
    }
    catch (std::invalid_argument) {
        UTIL_CONV_ERR_INVALID_STRING();
    }
    catch (std::out_of_range) {
        UTIL_CONV_ERR_OVERFLOW();
    }
    catch (std::exception ex) {
        if (err_msg) { *err_msg = ex.what(); }
        return false;
    }
}

} // namespace conv
} // namespace util
} // namespace server
} // namespace ic

#undef UTIL_CONV_ERR_INVALID_STRING
#undef UTIL_CONV_ERR_OVERFLOW

#endif // IC_SERVER_UTIL_CONVERT_NUMBER_H_
