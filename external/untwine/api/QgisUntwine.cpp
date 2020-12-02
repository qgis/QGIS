#ifndef _WIN32
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/errno.h>
#endif

#include "QgisUntwine.hpp"

namespace untwine
{

  QgisUntwine::QgisUntwine( const std::string &untwinePath ) : m_path( untwinePath ), m_running( false ),
    m_percent( 0 )
  {}

  bool QgisUntwine::start( const StringList &files, const std::string &outputDir,
                           const Options &argOptions )
  {
    if ( m_running )
      return false;

    Options options( argOptions );
    if ( files.size() == 0 || outputDir.empty() )
      return false;

    std::string s;
    for ( const std::string &t : files )
      s += t + " ";
    options.push_back( {"files", s} );
    options.push_back( {"output_dir", outputDir} );
#ifdef _WIN32
    PHANDLE[2] phandle;
    SECURITY_ATTRIBUITES pipeAttr;
    pipeAttr.nLength = sizeof( SECURITY_ATTRIBUTES );
    pipeAttr.InheritHandle = TRUE;
    ipipeAttr.lpSecurityDescriptor = NULL;

    CreatePipe( phandle[0], phandle[1], &pipeAttr, 0 );
    SetHandleInformation( *( pHandle[0] ), HANDLE_FLAG_INHERIT, 0 );

    options.push_back( {"progress_fd", std::to_string( fd[0] )} );
    std::string cmdline;
    cmdline += m_path + " ";
    for ( const Options &op : options )
      cmdline += "--" + op.first + " \"" + op.second + "\" ";

    PROCESS_INFORMATION processInfo;
    STARTUPINFO startupInfo;

    ZeroMemory( &processInfo, sizeof( PROCESS_INFORMATION ) );
    ZeroMemory( &startupInfo, sizeof( STARTUPINFO ) );
    startupInfo.cb = sizeof( STARTUPINFO );

    CreateProcessA( m_path.c_str(), cmdline.c_str(),
                    NULL, /* process attributes */
                    NULL, /* thread attributes */
                    FALSE, /* inherit handles */
                    CREATE_NO_WINDOW, /* creation flags */
                    NULL, /* environment */
                    NULL, /* current directory */
                    &startupInfo, /* startup info */
                    &processInfo /* process information */
                  );
//ABELL?
//    CloseHandle(processInfo.hProcess);
    m_pid = processInfo.hProcess;
//    CloseHandle(processInfo.hThread);
    CloseHandle( *( phandle[1] ) );

#else
    int fd[2];
    int ret = ::pipe( fd );

    m_pid = ::fork();

    // Child
    if ( m_pid == 0 )
    {
      // Close file descriptors other than the stdin/out/err and our pipe.
      // There may be more open than FD_SETSIZE, but meh.
      for ( int i = STDERR_FILENO + 1; i < FD_SETSIZE; ++i )
        if ( i != fd[1] )
          close( i );

      // Add the FD for the progress output
      options.push_back( {"progress_fd", std::to_string( fd[1] )} );

      for ( Option &op : options )
        op.first = "--" + op.first;

      std::vector<const char *> argv;
      argv.push_back( m_path.data() );
      for ( const Option &op : options )
      {
        argv.push_back( op.first.data() );
        argv.push_back( op.second.data() );
      }
      argv.push_back( nullptr );
      ::execv( m_path.data(), const_cast<char *const *>( argv.data() ) );
    }

    // Parent
    else
    {
      close( fd[1] );
      m_progressFd = fd[0];
      // Don't block attempting to read progress.
      ::fcntl( m_progressFd, F_SETFL, O_NONBLOCK );
      m_running = true;
    }
#endif
    return true;
  }

  bool QgisUntwine::stop()
  {
    if ( !m_running )
      return false;
#ifdef _WIN32
    TerminateProcess( m_pid, 1 );
    WaitForSingleObject( m_pid, INFINITE );
    CloseHandle( m_pid );
#else
    ::kill( m_pid, SIGINT );
    ( void )waitpid( m_pid, nullptr, 0 );
#endif
    m_running = false;
    m_pid = 0;
    return true;
  }

  bool QgisUntwine::running()
  {
#ifdef _WIN32
    if ( m_running && WaitForSingleObject( m_pid, 0 ) != WAIT_TIMEOUT )
      m_running = false;
#else
    if ( m_running && ( ::waitpid( m_pid, nullptr, WNOHANG ) != 0 ) )
      m_running = false;
#endif
    return m_running;
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

  namespace
  {

    int readString( int fd, std::string &s )
    {
      int ssize;

      // Loop while there's nothing to read.  Generally this shouldn't loop.
      while ( true )
      {
        ssize_t numRead = read( fd, &ssize, sizeof( ssize ) );
        // EOF or nothing to read.
        if ( numRead == 0 || ( numRead == -1 && errno != EAGAIN ) )
          return -1; // Shouldn't happen.
        if ( numRead > 0 )
          break;
      }

      // Loop reading string
      char buf[80];
      std::string t;
      while ( ssize )
      {
        ssize_t toRead = ( std::min )( ( size_t )ssize, sizeof( buf ) );
        ssize_t numRead = read( fd, buf, toRead );
        if ( numRead == 0 || ( numRead == -1 && errno != EAGAIN ) )
          return -1;
        if ( numRead > 0 )
        {
          ssize -= numRead;
          t += std::string( buf, numRead );
        }
      }
      s = std::move( t );
      return 0;
    }

  } // unnamed namespace

  void QgisUntwine::readPipe() const
  {
    // Read messages until the pipe has been drained.
    while ( true )
    {
      ssize_t size = read( m_progressFd, &m_percent, sizeof( m_percent ) );
      // EOF or nothing to read.
      if ( size == 0 || ( size == -1 && errno == EAGAIN ) )
        return;

      // Read the string, waiting as necessary.
      if ( readString( m_progressFd, m_progressMsg ) != 0 )
        break;
    }
  }

} // namespace untwine
