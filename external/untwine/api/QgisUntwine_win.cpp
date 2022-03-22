#include <iostream>
#include <algorithm>

#include "QgisUntwine.hpp"

namespace untwine
{

bool QgisUntwine::start(Options& options)
{
    HANDLE handle[2];
    SECURITY_ATTRIBUTES pipeAttr;
    pipeAttr.nLength = sizeof(SECURITY_ATTRIBUTES);
    pipeAttr.bInheritHandle = TRUE;
    pipeAttr.lpSecurityDescriptor = NULL;

    CreatePipe(&handle[0], &handle[1], &pipeAttr, 0);

    // Set the read end to no-wait.
    DWORD mode = PIPE_NOWAIT;
    SetNamedPipeHandleState(handle[0], &mode, NULL, NULL);

    size_t xhandle = reinterpret_cast<size_t>(handle[1]);
    options.push_back({"progress_fd", std::to_string(xhandle)});
    std::string cmdline;
    cmdline += m_path + " ";
    for (const Option& op : options)
        cmdline += "--" + op.first + " \"" + op.second + "\" ";

    PROCESS_INFORMATION processInfo;
    STARTUPINFO startupInfo;

    ZeroMemory(&processInfo, sizeof(PROCESS_INFORMATION));
    ZeroMemory(&startupInfo, sizeof(STARTUPINFO));
    startupInfo.cb = sizeof(STARTUPINFO);
    /**
    startupInfo.hStdInput = GetStdHandle(STD_INPUT_HANDLE);
    startupInfo.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    startupInfo.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    **/

    std::vector<char> ncCmdline(cmdline.begin(), cmdline.end());
    ncCmdline.push_back((char)0);
    bool ok = CreateProcessA(m_path.c_str(), ncCmdline.data(),
        NULL, /* process attributes */
        NULL, /* thread attributes */
        TRUE, /* inherit handles */
        CREATE_NO_WINDOW, /* creation flags */
        NULL, /* environment */
        NULL, /* current directory */
        &startupInfo, /* startup info */
        &processInfo /* process information */
    );
    if (ok)
    {
        m_pid = processInfo.hProcess;
        m_progressFd = handle[0];
        m_running = true;
    }
    return ok;
}

bool QgisUntwine::stop()
{
    if (!m_running)
        return false;
    TerminateProcess(m_pid, 1);
    WaitForSingleObject(m_pid, INFINITE);
    childStopped();
    m_pid = 0;
    return true;
}

// Called when the child has stopped.
void QgisUntwine::childStopped()
{
    m_running = false;
    CloseHandle(m_progressFd);
    CloseHandle(m_pid);
}

bool QgisUntwine::running()
{
    if (m_running && WaitForSingleObject(m_pid, 0) != WAIT_TIMEOUT)
        childStopped();
    return m_running;
}


namespace
{

int readString(HANDLE h, std::string& s)
{
    uint32_t ssize;

    // Loop while there's nothing to read.  Generally this shouldn't loop.
    while (true)
    {
        DWORD numRead;
        bool ok = ReadFile(h, &ssize, sizeof(ssize), &numRead, NULL);
        // EOF or nothing to read.
        if (numRead == 0 && GetLastError() == ERROR_NO_DATA)
            continue;
        else if (numRead == sizeof(ssize))
            break;
        else
            return -1;
    }

    // Loop reading string
    char buf[80];
    std::string t;
    while (ssize)
    {
        DWORD numRead;
        DWORD toRead = (std::min)((size_t)ssize, sizeof(buf));
        ReadFile(h, buf, toRead, &numRead, NULL);
        if (numRead == 0 && GetLastError() == ERROR_NO_DATA)
            continue;
        if (numRead <= 0)
            return -1;
        ssize -= numRead;
        t += std::string(buf, numRead);
    }
    s = std::move(t);
    return 0;
}

} // unnamed namespace

void QgisUntwine::readPipe() const
{
    // Read messages until the pipe has been drained.
    while (true)
    {
        DWORD numRead;
	uint32_t msgId;

        ReadFile(m_progressFd, &msgId, sizeof(msgId), &numRead, NULL);
        if (numRead != sizeof(msgId))
            return;

        if (msgId == ProgressMsg)
        {
            // Read the percent value.
            ReadFile(m_progressFd, &m_percent, sizeof(m_percent), &numRead, NULL);
            if (numRead != sizeof(m_percent))
                return;

            // Read the string, waiting as necessary.
            if (readString(m_progressFd, m_progressMsg) != 0)
                break;
        }
        else if (msgId == ErrorMsg)
        {
            // Read the string, waiting as necessary.
            if (readString(m_progressFd, m_errorMsg) != 0)
                break;
        }
    }
}

} // namespace untwine
