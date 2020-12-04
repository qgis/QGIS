#pragma once

#include <map>
#include <string>
#include <vector>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace untwine
{

class QgisUntwine
{
public:
    using Option = std::pair<std::string, std::string>;
    using Options = std::vector<Option>;
    using StringList = std::vector<std::string>;

    QgisUntwine(const std::string& untwinePath);

    bool start(const StringList& files, const std::string& outputDir,
        const Options& argOptions = Options());
    bool stop();
    bool running();
    int progressPercent() const;
    std::string progressMessage() const;

private:
    std::string m_path;
    mutable bool m_running;
    mutable int m_percent;
    mutable std::string m_progressMsg;
#ifndef _WIN32
    pid_t m_pid;
    int m_progressFd;
#else
    HANDLE m_pid;
    HANDLE m_progressFd;
#endif

    bool start(Options& options);
    void readPipe() const;
    void childStopped();
};

} // namespace untwine
