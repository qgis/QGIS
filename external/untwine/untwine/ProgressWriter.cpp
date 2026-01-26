#include <cmath>
#include <string>
#include <mutex>

#include "ProgressWriter.hpp"
#include "progress.hpp"

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
    if (!os::writeMessage(m_fd, msgId, percent, message))
        m_fd = -1;
}

void ProgressWriter::writeErrorMessage(const std::string& message)
{
    if (m_fd < 0)
    {
        std::cerr << "Untwine Error: " << message << "\n";
        return;
    }

    const int32_t msgId = 1001;
    if (!os::writeErrorMessage(m_fd, msgId, message))
        m_fd = -1;
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
