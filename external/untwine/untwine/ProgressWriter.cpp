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

std::mutex ProgressWriter::s_mutex;

ProgressWriter::ProgressWriter(int fd) : m_progressFd(fd), m_percent(0.0), m_increment(.1)
{}

void ProgressWriter::setIncrement(double increment)
{
    if (!m_progressFd)
        return;
    std::unique_lock<std::mutex> lock(s_mutex);

    m_increment = increment;
}

void ProgressWriter::setPercent(double percent)
{
    if (!m_progressFd)
        return;
    std::unique_lock<std::mutex> lock(s_mutex);

    m_percent = (std::max)(0.0, ((std::min)(1.0, percent)));
}

void ProgressWriter::writeIncrement(const std::string& message)
{
    if (!m_progressFd)
        return;
    std::unique_lock<std::mutex> lock(s_mutex);

    m_percent += m_increment;
    m_percent = (std::min)(1.0, m_percent);

    uint32_t percent = (uint32_t)std::round(m_percent * 100.0);
    writeMessage(percent, message);
}

void ProgressWriter::write(double percent, const std::string& message)
{
    if (!m_progressFd)
        return;
    std::unique_lock<std::mutex> lock(s_mutex);

    m_percent = (std::min)(0.0, ((std::max)(1.0, percent)));

    uint32_t ipercent = (uint32_t)std::round(m_percent * 100.0);
    writeMessage(ipercent, message);
}

void ProgressWriter::writeMessage(uint32_t percent, const std::string& message)
{
#ifndef _WIN32
    ::write(m_progressFd, &percent, sizeof(percent));
    uint32_t ssize = (uint32_t)message.size();
    ::write(m_progressFd, &ssize, sizeof(ssize));
    ::write(m_progressFd, message.data(), ssize);
#else
    DWORD numWritten;
    HANDLE h = reinterpret_cast<HANDLE>((intptr_t)m_progressFd);
    WriteFile(h, &percent, sizeof(percent), &numWritten, NULL);
    uint32_t ssize = (uint32_t)message.size();
    WriteFile(h, &ssize, sizeof(ssize), &numWritten, NULL);
    WriteFile(h, message.data(), ssize, &numWritten, NULL);
#endif
}

} // namespace untwine
