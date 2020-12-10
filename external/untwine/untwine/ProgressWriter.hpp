#pragma once

#include <mutex>

#include "../untwine/Common.hpp"

namespace untwine
{

class ProgressWriter
{
public:
    ProgressWriter(int fd);

    /// Set the increment to use on the next call to setIncrement.
    void setIncrement(double increment);
    /// Set the absolute percentage to use for the next setIncrement.
    void setPercent(double percent);

    /// Write a message using the current increment.
    void writeIncrement(const std::string& message);
    /// Write a message and set the current percentage.
    void write(double percent, const std::string& message);

    void update(PointCount numProcessed);
    // Utility fields
    PointCount m_total;
    PointCount m_threshold;
    PointCount m_current;

private:
    std::mutex m_mutex;
    int m_progressFd;
    double m_percent; // Current percent.
    double m_increment; // Current increment.

    void writeMessage(uint32_t percent, const std::string& message);
};

} //namespace untwine
