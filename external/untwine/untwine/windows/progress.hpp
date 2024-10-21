#include <Windows.h>

namespace untwine
{
namespace os
{

inline bool writeMessage(int fd, int32_t msgId, uint32_t percent, const std::string& message)
{
    DWORD numWritten;
    HANDLE h = reinterpret_cast<HANDLE>((intptr_t)fd);
    WriteFile(h, &msgId, sizeof(msgId), &numWritten, NULL);
    WriteFile(h, &percent, sizeof(percent), &numWritten, NULL);
    uint32_t ssize = (uint32_t)message.size();
    WriteFile(h, &ssize, sizeof(ssize), &numWritten, NULL);
    WriteFile(h, message.data(), ssize, &numWritten, NULL);
    return true;
}

inline bool writeErrorMessage(int fd, int32_t msgId, const std::string& message)
{
    DWORD numWritten;
    HANDLE h = reinterpret_cast<HANDLE>((intptr_t)fd);
    WriteFile(h, &msgId, sizeof(msgId), &numWritten, NULL);
    uint32_t ssize = (uint32_t)message.size();
    WriteFile(h, &ssize, sizeof(ssize), &numWritten, NULL);
    WriteFile(h, message.data(), ssize, &numWritten, NULL);
    return true;
}

} // namespace os
} // namespace untwine
