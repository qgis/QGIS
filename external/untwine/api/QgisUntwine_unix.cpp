#include <iostream>

#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#include <sys/wait.h>

#include "QgisUntwine.hpp"

namespace untwine
{

bool QgisUntwine::start(Options& options)
{
    int fd[2];
    int ret = ::pipe(fd);
    (void)ret;

    m_pid = ::fork();

    // Child
    if (m_pid == 0)
    {
        // Close file descriptors other than the stdin/out/err and our pipe.
        // There may be more open than FD_SETSIZE, but meh.
        for (int i = STDERR_FILENO + 1; i < FD_SETSIZE; ++i)
            if (i != fd[1])
                close(i);

        // Add the FD for the progress output
        options.push_back({"progress_fd", std::to_string(fd[1])});

        for (Option& op : options)
            op.first = "--" + op.first;

        std::vector<const char *> argv;
        argv.push_back(m_path.data());
        for (const Option& op : options)
        {
            argv.push_back(op.first.data());
            argv.push_back(op.second.data());
        }
        argv.push_back(nullptr);
        if (::execv(m_path.data(), const_cast<char *const *>(argv.data())) != 0)
        {
            std::cerr << "Couldn't start untwine '" << m_path << "'.\n";
            exit(-1);
        }
    }
    // Parent
    else
    {
        close(fd[1]);
        m_progressFd = fd[0];
        // Don't block attempting to read progress.
        ::fcntl(m_progressFd, F_SETFL, O_NONBLOCK);
        m_running = true;
    }
    return true;
}

bool QgisUntwine::stop()
{
    if (!m_running)
        return false;
    ::kill(m_pid, SIGINT);
    (void)waitpid(m_pid, nullptr, 0);
    m_pid = 0;
    return true;
}

// Called when the child has stopped.
void QgisUntwine::childStopped()
{
    m_running = false;
}

bool QgisUntwine::running()
{
    if (m_running && (::waitpid(m_pid, nullptr, WNOHANG) != 0))
        childStopped();
    return m_running;
}

namespace
{

uint32_t readString(int fd, std::string& s)
{
    uint32_t ssize;

    // Loop while there's nothing to read.  Generally this shouldn't loop.
    while (true)
    {
        ssize_t numRead = read(fd, &ssize, sizeof(ssize));
        if (numRead == -1 && errno != EAGAIN)
            continue;
        else if (numRead == sizeof(ssize))
            break;
        else
            return -1; // Shouldn't happen.
    }

    // Loop reading string
    char buf[80];
    std::string t;
    while (ssize)
    {
        ssize_t toRead = (std::min)((size_t)ssize, sizeof(buf));
        ssize_t numRead = read(fd, buf, toRead);
        if (numRead == 0 || (numRead == -1 && errno != EAGAIN))
            return -1;
        if (numRead > 0)
        {
            ssize -= numRead;
            t += std::string(buf, numRead);
        }
    }
    s = std::move(t);
    return 0;
}

} // unnamed namespace

void QgisUntwine::readPipe() const
{
    int32_t msgId;

    // Read messages until the pipe has been drained.
    while (true)
    {
        ssize_t size = read(m_progressFd, &msgId, sizeof(msgId));
        // If we didn't read the full size, just return.
        if (size != sizeof(m_percent))
            return;

        if (msgId == ProgressMsg)
        {
            ssize_t size = read(m_progressFd, &m_percent, sizeof(m_percent));
            // If we didn't read the full size, just return.
            if (size != sizeof(m_percent))
                break;

            // Read the string, waiting as necessary.
            if (readString(m_progressFd, m_progressMsg) != 0)
                break;
        }
        else if (msgId == ErrorMsg)
        {
            // Read the error string, waiting as necessary.
            if (readString(m_progressFd, m_errorMsg) != 0)
                break;
        }
        else
            break;
    }
}

} // namespace untwine
