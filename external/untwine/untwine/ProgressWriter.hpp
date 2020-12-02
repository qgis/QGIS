#pragma once

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

private:
    static std::mutex s_mutex;
    int m_progressFd;
    double m_percent; // Current percent.
    double m_increment; // Current increment.
};

} //namespace untwine
