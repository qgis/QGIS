/*
   This file is part of the KDE libraries
   Copyright (C) 1997 Christian Czezatke (e9025461@student.tuwien.ac.at)

    Rewritten for QT4 by e_k <e_k at users.sourceforge.net>, Copyright (C)2008

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "k3process.h"
//#include <config.h>

#include "k3processcontroller.h"
#include "kpty.h"

#ifdef __osf__
#define _OSF_SOURCE
#include <float.h>
#endif

#ifdef _AIX
#define _ALL_SOURCE
#endif

#include <sys/socket.h>
#include <sys/ioctl.h>

#include <sys/types.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/stat.h>
#include <sys/wait.h>

#ifdef HAVE_SYS_SELECT_H
#include <sys/select.h>
#endif

#include <errno.h>
#include <assert.h>
#include <fcntl.h>
#include <time.h>
#include <stdlib.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>

#include <QtCore/QMap>
#include <QtCore/QFile>
#include <QtCore/QSocketNotifier>

//#include <kdebug.h>
//#include <kstandarddirs.h>
//#include <kuser.h>


//////////////////
// private data //
//////////////////

class K3ProcessPrivate
{
  public:
    K3ProcessPrivate() :
        usePty( K3Process::NoCommunication ),
        addUtmp( false ), useShell( false ),
        pty( 0 ),
        priority( 0 )
    {
    }

    K3Process::Communication usePty;
  bool addUtmp : 1;
  bool useShell : 1;

    KPty *pty;

    int priority;

    QMap<QString, QString> env;
    QString wd;
    QByteArray shell;
    QByteArray executable;
};

/////////////////////////////
// public member functions //
/////////////////////////////

K3Process::K3Process( QObject* parent )
    : QObject( parent ),
    run_mode( NotifyOnExit ),
    runs( false ),
    pid_( 0 ),
    status( 0 ),
    keepPrivs( false ),
    innot( 0 ),
    outnot( 0 ),
    errnot( 0 ),
    communication( NoCommunication ),
    input_data( 0 ),
    input_sent( 0 ),
    input_total( 0 ),
    d( new K3ProcessPrivate )
{
  K3ProcessController::ref();
  K3ProcessController::instance()->addKProcess( this );


  out[0] = out[1] = -1;
  in[0] = in[1] = -1;
  err[0] = err[1] = -1;
}

void
K3Process::setEnvironment( const QString &name, const QString &value )
{
  d->env.insert( name, value );
}

void
K3Process::setWorkingDirectory( const QString &dir )
{
  d->wd = dir;
}

void
K3Process::setupEnvironment()
{
  QMap<QString, QString>::Iterator it;
  for ( it = d->env.begin(); it != d->env.end(); ++it )
  {
    setenv( QFile::encodeName( it.key() ).data(),
            QFile::encodeName( it.value() ).data(), 1 );
  }
  if ( !d->wd.isEmpty() )
  {
    ( void ) chdir( QFile::encodeName( d->wd ).data() );
  }
}

void
K3Process::setRunPrivileged( bool keepPrivileges )
{
  keepPrivs = keepPrivileges;
}

bool
K3Process::runPrivileged() const
{
  return keepPrivs;
}

bool
K3Process::setPriority( int prio )
{
  if ( runs )
  {
    if ( setpriority( PRIO_PROCESS, pid_, prio ) )
      return false;
  }
  else
  {
    if ( prio > 19 || prio < ( geteuid() ? getpriority( PRIO_PROCESS, 0 ) : -20 ) )
      return false;
  }
  d->priority = prio;
  return true;
}

K3Process::~K3Process()
{
  if ( run_mode != DontCare )
    kill( SIGKILL );
  detach();

  delete d->pty;
  delete d;

  K3ProcessController::instance()->removeKProcess( this );
  K3ProcessController::deref();
}

void K3Process::detach()
{
  if ( runs )
  {
    K3ProcessController::instance()->addProcess( pid_ );
    runs = false;
    pid_ = 0; // close without draining
    commClose(); // Clean up open fd's and socket notifiers.
  }
}

void K3Process::setBinaryExecutable( const char *filename )
{
  d->executable = filename;
}

K3Process &K3Process::operator<<( const QStringList& args )
{
  QStringList::ConstIterator it = args.begin();
  for ( ; it != args.end() ; ++it )
    arguments.append( QFile::encodeName( *it ) );
  return *this;
}

K3Process &K3Process::operator<<( const QByteArray& arg )
{
  return operator<< ( arg.data() );
}

K3Process &K3Process::operator<<( const char* arg )
{
  arguments.append( arg );
  return *this;
}

K3Process &K3Process::operator<<( const QString& arg )
{
  arguments.append( QFile::encodeName( arg ) );
  return *this;
}

void K3Process::clearArguments()
{
  arguments.clear();
}

bool K3Process::start( RunMode runmode, Communication comm )
{
  if ( runs )
  {
    qDebug() << "Attempted to start an already running process" << endl;
    return false;
  }

  uint n = arguments.count();
  if ( n == 0 )
  {
    qDebug() << "Attempted to start a process without arguments" << endl;
    return false;
  }
  char **arglist;
  QByteArray shellCmd;
  if ( d->useShell )
  {
    if ( d->shell.isEmpty() )
    {
      qDebug() << "Invalid shell specified" << endl;
      return false;
    }

    for ( uint i = 0; i < n; i++ )
    {
      shellCmd += arguments[i];
      shellCmd += ' '; // CC: to separate the arguments
    }

    arglist = static_cast<char **>( malloc( 4 * sizeof( char * ) ) );
    arglist[0] = d->shell.data();
    arglist[1] = ( char * ) "-c";
    arglist[2] = shellCmd.data();
    arglist[3] = 0;
  }
  else
  {
    arglist = static_cast<char **>( malloc(( n + 1 ) * sizeof( char * ) ) );
    for ( uint i = 0; i < n; i++ )
      arglist[i] = arguments[i].data();
    arglist[n] = 0;
  }

  run_mode = runmode;

  if ( !setupCommunication( comm ) )
  {
    qDebug() << "Could not setup Communication!" << endl;
    free( arglist );
    return false;
  }

  // We do this in the parent because if we do it in the child process
  // gdb gets confused when the application runs from gdb.
#ifdef HAVE_INITGROUPS
  struct passwd *pw = geteuid() ? 0 : getpwuid( getuid() );
#endif

  int fd[2];
  if ( pipe( fd ) )
    fd[0] = fd[1] = -1; // Pipe failed.. continue

  // we don't use vfork() because
  // - it has unclear semantics and is not standardized
  // - we do way too much magic in the child
  pid_ = fork();
  if ( pid_ == 0 )
  {
    // The child process

    close( fd[0] );
    // Closing of fd[1] indicates that the execvp() succeeded!
    fcntl( fd[1], F_SETFD, FD_CLOEXEC );

    if ( !commSetupDoneC() )
      qDebug() << "Could not finish comm setup in child!" << endl;

    // reset all signal handlers
    struct sigaction act;
    sigemptyset( &act.sa_mask );
    act.sa_handler = SIG_DFL;
    act.sa_flags = 0;
    for ( int sig = 1; sig < NSIG; sig++ )
      sigaction( sig, &act, 0L );

    if ( d->priority )
      setpriority( PRIO_PROCESS, 0, d->priority );

    if ( !runPrivileged() )
    {
      setgid( getgid() );
#ifdef HAVE_INITGROUPS
      if ( pw )
        initgroups( pw->pw_name, pw->pw_gid );
#endif
      if ( geteuid() != getuid() )
        setuid( getuid() );
      if ( geteuid() != getuid() )
        _exit( 1 );
    }

    setupEnvironment();

    if ( runmode == DontCare || runmode == OwnGroup )
      setsid();

    const char *executable = arglist[0];
    if ( !d->executable.isEmpty() )
      executable = d->executable.data();
    execvp( executable, arglist );

    char resultByte = 1;
    ( void ) write( fd[1], &resultByte, 1 );
    _exit( -1 );
  }
  else if ( pid_ == -1 )
  {
    // forking failed

    // commAbort();
    pid_ = 0;
    free( arglist );
    return false;
  }
  // the parent continues here
  free( arglist );

  if ( !commSetupDoneP() )
    qDebug() << "Could not finish comm setup in parent!" << endl;

  // Check whether client could be started.
  close( fd[1] );
  for ( ;; )
  {
    char resultByte;
    int n = ::read( fd[0], &resultByte, 1 );
    if ( n == 1 )
    {
      // exec() failed
      close( fd[0] );
      waitpid( pid_, 0, 0 );
      pid_ = 0;
      commClose();
      return false;
    }
    if ( n == -1 )
    {
      if ( errno == EINTR )
        continue; // Ignore
    }
    break; // success
  }
  close( fd[0] );

  runs = true;
  switch ( runmode )
  {
    case Block:
      for ( ;; )
      {
        commClose(); // drain only, unless obsolete reimplementation
        if ( !runs )
        {
          // commClose detected data on the process exit notifification pipe
          K3ProcessController::instance()->unscheduleCheck();
          if ( waitpid( pid_, &status, WNOHANG ) != 0 ) // error finishes, too
          {
            commClose(); // this time for real (runs is false)
            K3ProcessController::instance()->rescheduleCheck();
            break;
          }
          runs = true; // for next commClose() iteration
        }
        else
        {
          // commClose is an obsolete reimplementation and waited until
          // all output channels were closed (or it was interrupted).
          // there is a chance that it never gets here ...
          waitpid( pid_, &status, 0 );
          runs = false;
          break;
        }
      }
      // why do we do this? i think this signal should be emitted _only_
      // after the process has successfully run _asynchronously_ --ossi
      emit processExited( this );
      break;
    default: // NotifyOnExit & OwnGroup
      input_data = 0; // Discard any data for stdin that might still be there
      break;
  }
  return true;
}



bool K3Process::kill( int signo )
{
  if ( runs && pid_ > 0 && !::kill( run_mode == OwnGroup ? -pid_ : pid_, signo ) )
    return true;
  return false;
}



bool K3Process::isRunning() const
{
  return runs;
}



pid_t K3Process::pid() const
{
  return pid_;
}

#ifndef timersub
# define timersub(a, b, result) \
  do { \
    (result)->tv_sec = (a)->tv_sec - (b)->tv_sec; \
    (result)->tv_usec = (a)->tv_usec - (b)->tv_usec; \
    if ((result)->tv_usec < 0) { \
      --(result)->tv_sec; \
      (result)->tv_usec += 1000000; \
    } \
  } while (0)
#endif

bool K3Process::wait( int timeout )
{
  if ( !runs )
    return true;

#ifndef __linux__
  struct timeval etv;
#endif
  struct timeval tv, *tvp;
  if ( timeout < 0 )
    tvp = 0;
  else
  {
#ifndef __linux__
    gettimeofday( &etv, 0 );
    etv.tv_sec += timeout;
#else
    tv.tv_sec = timeout;
    tv.tv_usec = 0;
#endif
    tvp = &tv;
  }

  int fd = K3ProcessController::instance()->notifierFd();
  for ( ;; )
  {
    fd_set fds;
    FD_ZERO( &fds );
    FD_SET( fd, &fds );

#ifndef __linux__
    if ( tvp )
    {
      gettimeofday( &tv, 0 );
      timersub( &etv, &tv, &tv );
      if ( tv.tv_sec < 0 )
        tv.tv_sec = tv.tv_usec = 0;
    }
#endif

    switch ( select( fd + 1, &fds, 0, 0, tvp ) )
    {
      case -1:
        if ( errno == EINTR )
          break;
        // fall through; should happen if tvp->tv_sec < 0
      case 0:
        K3ProcessController::instance()->rescheduleCheck();
        return false;
      default:
        K3ProcessController::instance()->unscheduleCheck();
        if ( waitpid( pid_, &status, WNOHANG ) != 0 ) // error finishes, too
        {
          processHasExited( status );
          K3ProcessController::instance()->rescheduleCheck();
          return true;
        }
    }
  }
  return false;
}



bool K3Process::normalExit() const
{
  return ( pid_ != 0 ) && !runs && WIFEXITED( status );
}


bool K3Process::signalled() const
{
  return ( pid_ != 0 ) && !runs && WIFSIGNALED( status );
}


bool K3Process::coreDumped() const
{
#ifdef WCOREDUMP
  return signalled() && WCOREDUMP( status );
#else
  return false;
#endif
}


int K3Process::exitStatus() const
{
  return WEXITSTATUS( status );
}


int K3Process::exitSignal() const
{
  return WTERMSIG( status );
}


bool K3Process::writeStdin( const char *buffer, int buflen )
{
  // if there is still data pending, writing new data
  // to stdout is not allowed (since it could also confuse
  // kprocess ...)
  if ( input_data != 0 )
    return false;

  if ( communication & Stdin )
  {
    input_data = buffer;
    input_sent = 0;
    input_total = buflen;
    innot->setEnabled( true );
    if ( input_total )
      slotSendData( 0 );
    return true;
  }
  else
    return false;
}

void K3Process::suspend()
{
  if ( outnot )
    outnot->setEnabled( false );
}

void K3Process::resume()
{
  if ( outnot )
    outnot->setEnabled( true );
}

bool K3Process::closeStdin()
{
  if ( communication & Stdin )
  {
    communication = communication & ~Stdin;
    delete innot;
    innot = 0;
    if ( !( d->usePty & Stdin ) )
      close( in[1] );
    in[1] = -1;
    return true;
  }
  else
    return false;
}

bool K3Process::closeStdout()
{
  if ( communication & Stdout )
  {
    communication = communication & ~Stdout;
    delete outnot;
    outnot = 0;
    if ( !( d->usePty & Stdout ) )
      close( out[0] );
    out[0] = -1;
    return true;
  }
  else
    return false;
}

bool K3Process::closeStderr()
{
  if ( communication & Stderr )
  {
    communication = communication & ~Stderr;
    delete errnot;
    errnot = 0;
    if ( !( d->usePty & Stderr ) )
      close( err[0] );
    err[0] = -1;
    return true;
  }
  else
    return false;
}

bool K3Process::closePty()
{
  if ( d->pty && d->pty->masterFd() >= 0 )
  {
#ifndef Q_OS_MAC
    if ( d->addUtmp )
      d->pty->logout();
#endif
    d->pty->close();
    return true;
  }
  else
    return false;
}

void K3Process::closeAll()
{
  closeStdin();
  closeStdout();
  closeStderr();
  closePty();
}

/////////////////////////////
// protected slots         //
/////////////////////////////



void K3Process::slotChildOutput( int fdno )
{
  if ( !childOutput( fdno ) )
    closeStdout();
}


void K3Process::slotChildError( int fdno )
{
  if ( !childError( fdno ) )
    closeStderr();
}


void K3Process::slotSendData( int )
{
  if ( input_sent == input_total )
  {
    innot->setEnabled( false );
    input_data = 0;
    emit wroteStdin( this );
  }
  else
  {
    int result = ::write( in[1], input_data + input_sent, input_total - input_sent );
    if ( result >= 0 )
    {
      input_sent += result;
    }
    else if (( errno != EAGAIN ) && ( errno != EINTR ) )
    {
      qDebug() << "Error writing to stdin of child process" << endl;
      closeStdin();
    }
  }
}

void K3Process::setUseShell( bool useShell, const char *shell )
{
  d->useShell = useShell;
  if ( shell && *shell )
    d->shell = shell;
  else
// #ifdef NON_FREE // ... as they ship non-POSIX /bin/sh
#if !defined(__linux__) && !defined(__FreeBSD__) && !defined(__NetBSD__) && !defined(__OpenBSD__) && !defined(__GNU__) && !defined(__DragonFly__)
    // Solaris POSIX ...
    if ( !access( "/usr/xpg4/bin/sh", X_OK ) )
      d->shell = "/usr/xpg4/bin/sh";
    else
      // ... which links here anyway
      if ( !access( "/bin/ksh", X_OK ) )
        d->shell = "/bin/ksh";
      else
        // dunno, maybe superfluous?
        if ( !access( "/usr/ucb/sh", X_OK ) )
          d->shell = "/usr/ucb/sh";
        else
#endif
          d->shell = "/bin/sh";
}

void K3Process::setUsePty( Communication usePty, bool addUtmp )
{
  d->usePty = usePty;
  d->addUtmp = addUtmp;
  if ( usePty )
  {
    if ( !d->pty )
      d->pty = new KPty;
  }
  else
  {
    delete d->pty;
    d->pty = 0;
  }
}

KPty *K3Process::pty() const
{
  return d->pty;
}

QString K3Process::quote( const QString &arg )
{
  QChar q( '\'' );
  return QString( arg ).replace( q, "'\\''" ).prepend( q ).append( q );
}


//////////////////////////////
// private member functions //
//////////////////////////////


void K3Process::processHasExited( int state )
{
  // only successfully run NotifyOnExit processes ever get here

  status = state;
  runs = false; // do this before commClose, so it knows we're dead

  commClose(); // cleanup communication sockets

  if ( run_mode != DontCare )
    emit processExited( this );
}



int K3Process::childOutput( int fdno )
{
  if ( communication & NoRead )
  {
    int len = -1;
    emit receivedStdout( fdno, len );
    errno = 0; // Make sure errno doesn't read "EAGAIN"
    return len;
  }
  else
  {
    char buffer[1025];
    int len;

    len = ::read( fdno, buffer, 1024 );

    if ( len > 0 )
    {
      buffer[len] = 0; // Just in case.
      emit receivedStdout( this, buffer, len );
    }
    return len;
  }
}

int K3Process::childError( int fdno )
{
  char buffer[1025];
  int len;

  len = ::read( fdno, buffer, 1024 );

  if ( len > 0 )
  {
    buffer[len] = 0; // Just in case.
    emit receivedStderr( this, buffer, len );
  }
  return len;
}


int K3Process::setupCommunication( Communication comm )
{
  // PTY stuff //
  if ( d->usePty )
  {
    // cannot communicate on both stderr and stdout if they are both on the pty
    if ( !( ~( comm & d->usePty ) & ( Stdout | Stderr ) ) )
    {
      qWarning() << "Invalid usePty/communication combination (" << d->usePty << "/" << comm << ")" << endl;
      return 0;
    }
    if ( !d->pty->open() )
      return 0;

    int rcomm = comm & d->usePty;
    int mfd = d->pty->masterFd();
    if ( rcomm & Stdin )
      in[1] = mfd;
    if ( rcomm & Stdout )
      out[0] = mfd;
    if ( rcomm & Stderr )
      err[0] = mfd;
  }

  communication = comm;

  comm = comm & ~d->usePty;
  if ( comm & Stdin )
  {
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, in ) )
      goto fail0;
    fcntl( in[0], F_SETFD, FD_CLOEXEC );
    fcntl( in[1], F_SETFD, FD_CLOEXEC );
  }
  if ( comm & Stdout )
  {
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, out ) )
      goto fail1;
    fcntl( out[0], F_SETFD, FD_CLOEXEC );
    fcntl( out[1], F_SETFD, FD_CLOEXEC );
  }
  if ( comm & Stderr )
  {
    if ( socketpair( AF_UNIX, SOCK_STREAM, 0, err ) )
      goto fail2;
    fcntl( err[0], F_SETFD, FD_CLOEXEC );
    fcntl( err[1], F_SETFD, FD_CLOEXEC );
  }
  return 1; // Ok
fail2:
  if ( comm & Stdout )
  {
    close( out[0] );
    close( out[1] );
    out[0] = out[1] = -1;
  }
fail1:
  if ( comm & Stdin )
  {
    close( in[0] );
    close( in[1] );
    in[0] = in[1] = -1;
  }
fail0:
  communication = NoCommunication;
  return 0; // Error
}



int K3Process::commSetupDoneP()
{
  int rcomm = communication & ~d->usePty;
  if ( rcomm & Stdin )
    close( in[0] );
  if ( rcomm & Stdout )
    close( out[1] );
  if ( rcomm & Stderr )
    close( err[1] );
  in[0] = out[1] = err[1] = -1;

  // Don't create socket notifiers if no interactive comm is to be expected
  if ( run_mode != NotifyOnExit && run_mode != OwnGroup )
    return 1;

  if ( communication & Stdin )
  {
    fcntl( in[1], F_SETFL, O_NONBLOCK | fcntl( in[1], F_GETFL ) );
    innot =  new QSocketNotifier( in[1], QSocketNotifier::Write, this );
    Q_CHECK_PTR( innot );
    innot->setEnabled( false ); // will be enabled when data has to be sent
    QObject::connect( innot, SIGNAL( activated( int ) ),
                      this, SLOT( slotSendData( int ) ) );
  }

  if ( communication & Stdout )
  {
    outnot = new QSocketNotifier( out[0], QSocketNotifier::Read, this );
    Q_CHECK_PTR( outnot );
    QObject::connect( outnot, SIGNAL( activated( int ) ),
                      this, SLOT( slotChildOutput( int ) ) );
    if ( communication & NoRead )
      suspend();
  }

  if ( communication & Stderr )
  {
    errnot = new QSocketNotifier( err[0], QSocketNotifier::Read, this );
    Q_CHECK_PTR( errnot );
    QObject::connect( errnot, SIGNAL( activated( int ) ),
                      this, SLOT( slotChildError( int ) ) );
  }

  return 1;
}



int K3Process::commSetupDoneC()
{
  int ok = 1;
  if ( d->usePty & Stdin )
  {
    if ( dup2( d->pty->slaveFd(), STDIN_FILENO ) < 0 ) ok = 0;
  }
  else if ( communication & Stdin )
  {
    if ( dup2( in[0], STDIN_FILENO ) < 0 ) ok = 0;
  }
  else
  {
    int null_fd = open( "/dev/null", O_RDONLY );
    if ( dup2( null_fd, STDIN_FILENO ) < 0 ) ok = 0;
    close( null_fd );
  }
  struct linger so;
  memset( &so, 0, sizeof( so ) );
  if ( d->usePty & Stdout )
  {
    if ( dup2( d->pty->slaveFd(), STDOUT_FILENO ) < 0 ) ok = 0;
  }
  else if ( communication & Stdout )
  {
    if ( dup2( out[1], STDOUT_FILENO ) < 0 ||
         setsockopt( out[1], SOL_SOCKET, SO_LINGER, ( char * )&so, sizeof( so ) ) )
      ok = 0;
    if ( communication & MergedStderr )
    {
      if ( dup2( out[1], STDERR_FILENO ) < 0 )
        ok = 0;
    }
  }
  if ( d->usePty & Stderr )
  {
    if ( dup2( d->pty->slaveFd(), STDERR_FILENO ) < 0 ) ok = 0;
  }
  else if ( communication & Stderr )
  {
    if ( dup2( err[1], STDERR_FILENO ) < 0 ||
         setsockopt( err[1], SOL_SOCKET, SO_LINGER, ( char * )&so, sizeof( so ) ) )
      ok = 0;
  }

  // don't even think about closing all open fds here or anywhere else

  // PTY stuff //
  if ( d->usePty )
  {
    d->pty->setCTty();
#ifndef Q_OS_MAC
    if ( d->addUtmp )
      d->pty->login( getenv( "USER" ), getenv( "DISPLAY" ) );
#endif
  }

  return ok;
}



void K3Process::commClose()
{
  closeStdin();

  if ( pid_ ) // detached, failed, and killed processes have no output. basta. :)
  {
    // If both channels are being read we need to make sure that one socket
    // buffer doesn't fill up whilst we are waiting for data on the other
    // (causing a deadlock). Hence we need to use select.

    int notfd = K3ProcessController::instance()->notifierFd();

    while (( communication & ( Stdout | Stderr ) ) || runs )
    {
      fd_set rfds;
      FD_ZERO( &rfds );
      struct timeval timeout, *p_timeout;

      int max_fd = 0;
      if ( communication & Stdout )
      {
        FD_SET( out[0], &rfds );
        max_fd = out[0];
      }
      if ( communication & Stderr )
      {
        FD_SET( err[0], &rfds );
        if ( err[0] > max_fd )
          max_fd = err[0];
      }
      if ( runs )
      {
        FD_SET( notfd, &rfds );
        if ( notfd > max_fd )
          max_fd = notfd;
        // If the process is still running we block until we
        // receive data or the process exits.
        p_timeout = 0; // no timeout
      }
      else
      {
        // If the process has already exited, we only check
        // the available data, we don't wait for more.
        timeout.tv_sec = timeout.tv_usec = 0; // timeout immediately
        p_timeout = &timeout;
      }

      int fds_ready = select( max_fd + 1, &rfds, 0, 0, p_timeout );
      if ( fds_ready < 0 )
      {
        if ( errno == EINTR )
          continue;
        break;
      }
      else if ( !fds_ready )
        break;

      if (( communication & Stdout ) && FD_ISSET( out[0], &rfds ) )
        slotChildOutput( out[0] );

      if (( communication & Stderr ) && FD_ISSET( err[0], &rfds ) )
        slotChildError( err[0] );

      if ( runs && FD_ISSET( notfd, &rfds ) )
      {
        runs = false; // hack: signal potential exit
        return; // don't close anything, we will be called again
      }
    }
  }

  closeStdout();
  closeStderr();

  closePty();
}



///////////////////////////
// CC: Class K3ShellProcess
///////////////////////////

K3ShellProcess::K3ShellProcess( const char *shellname ):
    K3Process(), d( 0 )
{
  setUseShell( true, shellname ? shellname : getenv( "SHELL" ) );
}

K3ShellProcess::~K3ShellProcess()
{
}

QString K3ShellProcess::quote( const QString &arg )
{
  return K3Process::quote( arg );
}

bool K3ShellProcess::start( RunMode runmode, Communication comm )
{
  return K3Process::start( runmode, comm );
}


//#include "moc_k3process.cpp"
