#include <iostream>

#include "QgisUntwine.hpp"

#ifdef WIN32
#include "QgisUntwine_win.cpp"
#else
#include "QgisUntwine_unix.cpp"
#endif

namespace untwine
{

QgisUntwine::QgisUntwine(const std::string& untwinePath) : m_path(untwinePath), m_running(false),
    m_percent(0)
{}

bool QgisUntwine::start(const StringList& files, const std::string& outputDir,
    const Options& argOptions)
{
    if (m_running)
        return false;

    Options options(argOptions);
    if (files.size() == 0 || outputDir.empty())
        return false;

    std::string s;
    for (auto ti = files.begin(); ti != files.end(); ++ti)
    {
        s += *ti;
        if (ti + 1 != files.end())
            s += ", ";
    }
    options.push_back({"files", s});
    options.push_back({"output_dir", outputDir});

    return start(options);
}

int QgisUntwine::progressPercent() const
{
    readPipe();

    return m_percent;
}

std::string QgisUntwine::progressMessage() const
{
    readPipe();

    return m_progressMsg;
}

std::string QgisUntwine::errorMessage() const
{
    readPipe();

    return m_errorMsg;
}

} // namespace untwine
