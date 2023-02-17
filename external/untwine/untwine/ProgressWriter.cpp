#include <cmath>
#include <string>
#include <mutex>

#ifndef _WIN32
#include <unistd.h>
#else
#include <Windows.h>
#endif

#include "ProgressWriter.hpp"

namespace untwine
{

ProgressWriter::ProgressWriter() : m_fd(-1), m_debug(false), m_percent(0.0), m_increment(.01)
{}

void ProgressWriter::init(int fd, bool debug)
{
    m_fd = fd;
    m_debug = debug;
}

void ProgressWriter::setIncrement(double increment)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_increment = increment;
}

void ProgressWriter::setPercent(double percent)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_percent = (std::max)(0.0, ((std::min)(1.0, percent)));
}

void ProgressWriter::writeIncrement(const std::string& message)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    writeIncrementRaw(message);
}

// Unlocked version - obtain lock before calling.
void ProgressWriter::writeIncrementRaw(const std::string& message)
{
    m_percent += m_increment;
    m_percent = (std::min)(1.0, m_percent);

    uint32_t percent = (uint32_t)std::round(m_percent * 100.0);
    writeMessage(percent, message);
}

void ProgressWriter::write(double percent, const std::string& message)
{
    std::unique_lock<std::mutex> lock(m_mutex);

    m_percent = (std::min)(0.0, ((std::max)(1.0, percent)));

    uint32_t ipercent = (uint32_t)std::round(m_percent * 100.0);
    writeMessage(ipercent, message);
}

void ProgressWriter::writeMessage(uint32_t percent, const std::string& message)
{
    if (m_debug)
        std::cout << "Untwine progress (" << percent << "% done): " << message << "\n";
    if (m_fd < 0)
        return;

    const int32_t msgId = 1000;
#ifndef _WIN32
    bool err = false;
    err = (::write(m_fd, &msgId, sizeof(msgId)) == -1);
    err |= (::write(m_fd, &percent, sizeof(percent)) == -1);
    uint32_t ssize = (uint32_t)message.size();
    err |= (::write(m_fd, &ssize, sizeof(ssize)) == -1);
    err |= (::write(m_fd, message.data(), ssize) == -1);
    if (err)
    {
        ::close(m_fd);
        m_fd = -1;
    }
#else
    DWORD numWritten;
    HANDLE h = reinterpret_cast<HANDLE>((intptr_t)m_fd);
    WriteFile(h, &msgId, sizeof(msgId), &numWritten, NULL);
    WriteFile(h, &percent, sizeof(percent), &numWritten, NULL);
    uint32_t ssize = (uint32_t)message.size();
    WriteFile(h, &ssize, sizeof(ssize), &numWritten, NULL);
    WriteFile(h, message.data(), ssize, &numWritten, NULL);
#endif
}

void ProgressWriter::writeErrorMessage(const std::string& message)
{
    if (m_fd < 0)
    {
        std::cerr << "Untwine Error: " << message << "\n";
        return;
    }

    const int32_t msgId = 1001;
#ifndef _WIN32
    bool err = false;
    err = (::write(m_fd, &msgId, sizeof(msgId)) == -1);
    uint32_t ssize = (uint32_t)message.size();
    err |= (::write(m_fd, &ssize, sizeof(ssize)) == -1);
    err |= (::write(m_fd, message.data(), ssize) == -1);
    if (err)
    {
        ::close(m_fd);
        m_fd = -1;
    }
#else
    DWORD numWritten;
    HANDLE h = reinterpret_cast<HANDLE>((intptr_t)m_fd);
    WriteFile(h, &msgId, sizeof(msgId), &numWritten, NULL);
    uint32_t ssize = (uint32_t)message.size();
    WriteFile(h, &ssize, sizeof(ssize), &numWritten, NULL);
    WriteFile(h, message.data(), ssize, &numWritten, NULL);
#endif
}

// Determine the point increment and reset the counters.
void ProgressWriter::setPointIncrementer(PointCount total, int totalClicks)
{
    assert(totalClicks <= 100);
    assert(totalClicks > 0);

    m_current = 0;
    if (total < ChunkSize)
        m_pointIncrement = total;
    else
        m_pointIncrement = total / totalClicks;
    m_nextClick = m_pointIncrement;
}

// Write a message if the threshold has been reached.
void ProgressWriter::update(PointCount count)
{
    if (m_fd < 0)
        return;

    std::unique_lock<std::mutex> lock(m_mutex);

    m_current += count;
    while (m_current >= m_nextClick)
    {
        m_nextClick += m_pointIncrement;

        writeIncrementRaw("Processed " + std::to_string(m_current) + " points");
    }
}

} // namespace untwine
