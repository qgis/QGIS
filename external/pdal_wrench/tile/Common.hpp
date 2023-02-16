#pragma once

#ifdef _WIN32
#include <Windows.h>
#endif

#include <stdexcept>

using PointCount = uint64_t;

class  FatalError : public std::runtime_error
{
public:
    inline FatalError(std::string const& msg) : std::runtime_error(msg)
        {}
};

namespace untwine
{

// We check both _WIN32 and _MSC_VER to deal with MinGW, which doesn't support the special
// Windows wide character interfaces for streams.
#if defined(_WIN32) && defined(_MSC_VER)
inline std::wstring toNative(const std::string& in)
{
    if (in.empty())
        return std::wstring();

    int len = MultiByteToWideChar(CP_UTF8, 0, in.data(), in.length(), nullptr, 0);
    std::wstring out(len, 0);
    if (MultiByteToWideChar(CP_UTF8, 0, in.data(), in.length(), out.data(), len) == 0)
    {
        char buf[200] {};
        len = FormatMessageA(0, 0, GetLastError(), 0, buf, 199, 0);
        throw FatalError("Can't convert UTF8 to UTF16: " + std::string(buf, len));
    }
    return out;
}

inline std::string fromNative(const std::wstring& in)
{
    if (in.empty())
        return std::string();

    int len = WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), nullptr, 0, nullptr, nullptr);
    std::string out(len, 0);
    if (WideCharToMultiByte(CP_UTF8, 0, in.data(), in.length(), out.data(), len, nullptr, nullptr) == 0)
    {
        char buf[200] {};
        len = FormatMessageA(0, 0, GetLastError(), 0, buf, 199, 0);
        throw FatalError("Can't convert UTF16 to UTF8: " + std::string(buf, len));
    }
    return out;
}
#else
inline std::string toNative(const std::string& in)
{
    return in;
}

inline std::string fromNative(const std::string& in)
{
    return in;
}
#endif

}
