/**
 * @file string_view.h
 * @brief 简易版的字符串视图.
 * @author Leopard-C (leopard.c@outlook.com)
 * @date 2023-11-29
 * 
 * @copyright Copyright (c) 2023-present, Jinbao Chen.
 */
#ifndef IC_SERVER_STRING_VIEW_H_
#define IC_SERVER_STRING_VIEW_H_
#include <string>
#include <vector>
#include <string.h>

namespace ic {
namespace server {

class StringView {
public:
    StringView();
    StringView(const char* begin, size_t size);

    bool empty() const { return size_ == 0; }
    size_t size() const { return size_; }
    size_t length() const { return size_; }
    const char* data() const { return begin_; }
    const char* begin() const { return begin_; }
    const char* end() const { return begin_ + size_; }
    char front() const { return begin_[0]; }
    char back() const { return begin_[size_ - 1]; }

public:
    void Assign(const char* begin, size_t size);
    std::string ToString() const { return std::string(begin_, size_); }
    bool operator==(const char* str) const;
    bool operator==(const std::string& str) const;
    bool operator==(const StringView& rhs) const;
    const char& operator[](size_t idx) const { return *(begin_ + idx); }

    /**
     * @brief 字符串比较.
     */
    int Compare(const char* str) const
        { return Compare(0, str, strlen(str)); }
    int Compare(const char* str, size_t n) const
        { return Compare(0, str, n); }
    int Compare(size_t start, const char* str, size_t n) const;

    int CompareNoCase(const char* str) const
        { return CompareNoCase(0, str, strlen(str)); }
    int CompareNoCase(const char* str, size_t n) const
        { return CompareNoCase(0, str, n); }
    int CompareNoCase(size_t start, const char* str, size_t n) const;

    /**
     * @brief 查找子串.
     */
    size_t Find(const char* needle) const
        { return Find(0, size_, needle, strlen(needle)); }
    size_t Find(const char* needle, size_t needle_len) const
        { return Find(0, size_, needle, needle_len); }
    size_t Find(size_t start, const char* needle, size_t needle_len) const
        { return Find(start, size_, needle, needle_len); }
    size_t Find(size_t start, size_t end, const char* needle, size_t needle_len) const;

    size_t FindFirstNotOf(const char* chars) const
        { return FindFirstNotOf(0, size_, chars, strlen(chars)); }
    size_t FindFirstNotOf(const char* chars, size_t chars_len) const
        { return FindFirstNotOf(0, size_, chars, chars_len); }
    size_t FindFirstNotOf(size_t start, const char* chars, size_t chars_len) const
        { return FindFirstNotOf(start, size_, chars, chars_len); }
    size_t FindFirstNotOf(size_t start, size_t end, const char* chars, size_t chars_len) const;

    size_t FindLastNotOf(const char* chars) const
        { return FindLastNotOf(0, size_, chars, strlen(chars)); }
    size_t FindLastNotOf(const char* chars, size_t chars_len) const
        { return FindLastNotOf(0, size_, chars, chars_len); }
    size_t FindLastNotOf(size_t start, const char* chars, size_t chars_len) const
        { return FindLastNotOf(start, size_, chars, chars_len); }
    size_t FindLastNotOf(size_t start, size_t end, const char* chars, size_t chars_len) const;

    /**
     * @brief 字符串截取.
     */
    StringView SubStr(size_t start, size_t length = std::string::npos) const;

    /**
     * @brief 去除空白字符.
     */
    StringView& Trim(const char* spaces = " ")
        { return TrimLeft(spaces), TrimRight(spaces); }
    StringView& TrimLeft(const char* spaces = " ");
    StringView& TrimRight(const char* spaces = " ");

    /**
     * @brief 字符串分隔.
     * 
     * @param delimiter 分隔符(字符串)
     * @param delimiter_len 分隔字符串长度
     * @param remove_empty_entries 是否移除分隔后的空字符串
     */
    std::vector<StringView> Split(const char* delimiter, size_t delimiter_len, bool remove_empty_entries = false) const;

public:
    static const size_t npos;

private:
    const char* begin_;
    size_t size_;
};

} // namespace server
} // namespace ic

#endif // IC_SERVER_STRING_VIEW_H_
