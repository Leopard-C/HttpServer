#include "string_view.h"
#include "util/string/memmem.h"

#define _IC_MIN(a,b) ((a) < (b) ? (a) : (b))

namespace ic {
namespace server {

const size_t StringView::npos = static_cast<size_t>(-1);

StringView::StringView()
    : begin_(""), size_(0)
{
}

StringView::StringView(const char* begin, size_t size)
    : begin_(begin), size_(size)
{
}

void StringView::Assign(const char* begin, size_t size) {
    begin_ = begin;
    size_ = size;
}

/**
 * @brief 字符串等号比较.
 */
bool StringView::operator==(const char* str) const {
    return (strlen(str) == size_) && memcmp(begin_, str, size_) == 0;
}

bool StringView::operator==(const std::string& str) const {
    return (str.size() == size_) && memcmp(begin_, str.c_str(), size_) == 0;
}

bool StringView::operator==(const StringView& rhs) const {
    return (rhs.size_ == size_) && memcmp(begin_, rhs.begin_, size_) == 0;
}

/**
 * @brief 字符串比较大小.
 */
int StringView::Compare(size_t start, const char* str, size_t n) const {
    if (n == 0) {
        return 0;
    }
    if (start >= size_) {
        return 0 - str[0];
    }
    size_t end = _IC_MIN(n, size_ - start);
    int result = 0;
    for (size_t i = 0; i < end; ++i) {
        if ((result = begin_[start + i] - str[i]) != 0) {
            return result;
        }
    }
    return (n > size_ - start) ? (0 - str[size_ - start]) : 0;
}

int StringView::CompareNoCase(size_t start, const char* str, size_t n) const {
    if (n == 0) {
        return 0;
    }
    if (start >= size_) {
        return 0 - tolower(str[0]);
    }
    size_t end = _IC_MIN(n, size_ - start);
    int result = 0;
    for (size_t i = 0; i < end; ++i) {
        if ((result = tolower(begin_[start + i]) - tolower(str[i])) != 0) {
            return result;
        }
    }
    return (n > size_ - start) ? (0 - tolower(str[size_ - start])) : 0;
}

/**
 * @brief 字符串查找.
 */
size_t StringView::Find(size_t start, size_t end, const char* needle, size_t needle_len) const {
    end = _IC_MIN(end, size_);
    if (start < end) {
        const char* result = (const char*)util::memmem(begin_ + start, end - start, needle, needle_len);
        if (result) {
            return result - begin_;
        }
    }
    return std::string::npos;
}

size_t StringView::FindFirstNotOf(size_t start, size_t end, const char* chars, size_t chars_len) const {
    end = _IC_MIN(end, size_);
    if (start < end) {
        size_t j;
        for (size_t i = start; i < end; ++i) {
            for (j = 0; j < chars_len; ++j) {
                if (begin_[i] == chars[j]) {
                    break;
                }
            }
            if (j == chars_len) {
                return i;
            }
        }
    }
    return std::string::npos;
}

size_t StringView::FindLastNotOf(size_t start, size_t end, const char* chars, size_t chars_len) const {
    end = _IC_MIN(end, size_);
    if (start < end) {
        size_t j;
        for (int i = static_cast<int>(end) - 1, istart = static_cast<int>(start); i >= istart; --i) {
            for (j = 0; j < chars_len; ++j) {
                if (begin_[i] == chars[j]) {
                    break;
                }
            }
            if (j == chars_len) {
                return i;
            }
        }
    }
    return std::string::npos;
}

/**
 * @brief 字符串截取.
 */
StringView StringView::SubStr(size_t start, size_t length/* = std::string::npos*/) const {
    start = _IC_MIN(start, size_);
    length = _IC_MIN(length, size_ - start);
    return StringView(begin_ + start, length);
}

/**
 * @brief 去除左侧空白字符.
 */
StringView& StringView::TrimLeft(const char* spaces/* = " "*/) {
    size_t pos = FindFirstNotOf(spaces);
    pos = _IC_MIN(pos, size_);
    begin_ += pos;
    size_ -= pos;
    return *this;
}

/**
 * @brief 去除右侧空白字符.
 */
StringView& StringView::TrimRight(const char* spaces/* = " "*/) {
    size_t pos = FindLastNotOf(spaces);
    size_ = pos + 1;
    return *this;
}

/**
 * @brief 字符串分隔.
 */
std::vector<StringView> StringView::Split(const char* delimiter, size_t delimiter_len, bool remove_empty_entries/* = false*/) const {
    std::vector<StringView> result;
    size_t start = 0;
    size_t pos = start - delimiter_len;
    StringView elem;
    do {
        start = pos + delimiter_len;
        pos = Find(start, delimiter, delimiter_len);
        elem = SubStr(start, pos - start);
        if (!elem.empty() || !remove_empty_entries) {
            result.push_back(elem);
        }
    } while(pos != std::string::npos);
    return result;
}

} // namespace server
} // namespace ic
