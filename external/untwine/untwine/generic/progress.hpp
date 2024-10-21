#include <unistd.h>
#include <cstdint>
#include <string>

namespace untwine
{
namespace os
{

inline bool writeMessage(int fd, int32_t msgId, uint32_t percent, const std::string& message)
{
    bool err = false;
    err = (::write(fd, &msgId, sizeof(msgId)) == -1);
    err |= (::write(fd, &percent, sizeof(percent)) == -1);
    uint32_t ssize = (uint32_t)message.size();
    err |= (::write(fd, &ssize, sizeof(ssize)) == -1);
    err |= (::write(fd, message.data(), ssize) == -1);
    if (err)
        ::close(fd);
    return err;
}

inline bool writeErrorMessage(int fd, int32_t msgId, const std::string& message)
{
    bool err = false;
    err = (::write(fd, &msgId, sizeof(msgId)) == -1);
    uint32_t ssize = (uint32_t)message.size();
    err |= (::write(fd, &ssize, sizeof(ssize)) == -1);
    err |= (::write(fd, message.data(), ssize) == -1);
    if (err)
        ::close(fd);
    return err;
}

} // namespace os
} // namespace untwine
