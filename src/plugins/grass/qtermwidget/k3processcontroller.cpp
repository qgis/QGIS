/* This file is part of the KDE libraries
    Copyright (C) 1997 Christian Czezakte (e9025461@student.tuwien.ac.at)

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

#include "k3processcontroller.h"
#include "k3process.h"

//#include <config.h>

#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>

#include <QtCore/QSocketNotifier>

class K3ProcessController::Private
{
  public:
    Private()
        : needcheck( false ),
        notifier( 0 )
    {
    }

    ~Private()
    {
      delete notifier;
    }

    int fd[2];
    bool needcheck;
    QSocketNotifier *notifier;
    QList<K3Process*> kProcessList;
    QList<int> unixProcessList;
    static struct sigaction oldChildHandlerData;
    static bool handlerSet;
    static int refCount;
    static K3ProcessController* instance;
};

K3ProcessController *K3ProcessController::Private::instance = 0;
int K3ProcessController::Private::refCount = 0;

void K3ProcessController::ref()
{
  if ( !Private::refCount )
  {
    Private::instance = new K3ProcessController;
    setupHandlers();
  }
  Private::refCount++;
}

void K3ProcessController::deref()
{
  Private::refCount--;
  if ( !Private::refCount )
  {
    resetHandlers();
    delete Private::instance;
    Private::instance = 0;
  }
}

K3ProcessController* K3ProcessController::instance()
{
  /*
   * there were no safety guards in previous revisions, is that ok?
  if ( !Private::instance ) {
      ref();
  }
   */

  return Private::instance;
}

K3ProcessController::K3ProcessController()
    : d( new Private )
{
  if ( pipe( d->fd ) )
  {
    perror( "pipe" );
    abort();
  }

  fcntl( d->fd[0], F_SETFL, O_NONBLOCK ); // in case slotDoHousekeeping is called without polling first
  fcntl( d->fd[1], F_SETFL, O_NONBLOCK ); // in case it fills up
  fcntl( d->fd[0], F_SETFD, FD_CLOEXEC );
  fcntl( d->fd[1], F_SETFD, FD_CLOEXEC );

  d->notifier = new QSocketNotifier( d->fd[0], QSocketNotifier::Read );
  d->notifier->setEnabled( true );
  QObject::connect( d->notifier, SIGNAL( activated( int ) ),
                    SLOT( slotDoHousekeeping() ) );
}

K3ProcessController::~K3ProcessController()
{
#ifndef Q_OS_MAC
  /* not sure why, but this is causing lockups */
  close( d->fd[0] );
  close( d->fd[1] );
#else
//FIXME: why does close() freeze up destruction?
#endif

  delete d;
}


extern "C"
{
  static void theReaper( int num )
  {
    K3ProcessController::theSigCHLDHandler( num );
  }
}

#ifdef Q_OS_UNIX
struct sigaction K3ProcessController::Private::oldChildHandlerData;
#endif
bool K3ProcessController::Private::handlerSet = false;

void K3ProcessController::setupHandlers()
{
  if ( Private::handlerSet )
    return;
  Private::handlerSet = true;

#ifdef Q_OS_UNIX
  struct sigaction act;
  sigemptyset( &act.sa_mask );

  act.sa_handler = SIG_IGN;
  act.sa_flags = 0;
  sigaction( SIGPIPE, &act, 0L );

  act.sa_handler = theReaper;
  act.sa_flags = SA_NOCLDSTOP;
  // CC: take care of SunOS which automatically restarts interrupted system
  // calls (and thus does not have SA_RESTART)
#ifdef SA_RESTART
  act.sa_flags |= SA_RESTART;
#endif
  sigaction( SIGCHLD, &act, &Private::oldChildHandlerData );

  sigaddset( &act.sa_mask, SIGCHLD );
  // Make sure we don't block this signal. gdb tends to do that :-(
  sigprocmask( SIG_UNBLOCK, &act.sa_mask, 0 );
#else
  //TODO: win32
#endif
}

void K3ProcessController::resetHandlers()
{
  if ( !Private::handlerSet )
    return;
  Private::handlerSet = false;

#ifdef Q_OS_UNIX
  sigset_t mask, omask;
  sigemptyset( &mask );
  sigaddset( &mask, SIGCHLD );
  sigprocmask( SIG_BLOCK, &mask, &omask );

  struct sigaction act;
  sigaction( SIGCHLD, &Private::oldChildHandlerData, &act );
  if ( act.sa_handler != theReaper )
  {
    sigaction( SIGCHLD, &act, 0 );
    Private::handlerSet = true;
  }

  sigprocmask( SIG_SETMASK, &omask, 0 );
#else
  //TODO: win32
#endif
  // there should be no problem with SIGPIPE staying SIG_IGN
}

// the pipe is needed to sync the child reaping with our event processing,
// as otherwise there are race conditions, locking requirements, and things
// generally get harder
void K3ProcessController::theSigCHLDHandler( int arg )
{
  int saved_errno = errno;

  char dummy = 0;
  if( ::write( instance()->d->fd[1], &dummy, 1 ) < 0 )
    perror( "write failed" );

#ifdef Q_OS_UNIX
  if ( Private::oldChildHandlerData.sa_handler != SIG_IGN &&
       Private::oldChildHandlerData.sa_handler != SIG_DFL )
  {
    Private::oldChildHandlerData.sa_handler( arg ); // call the old handler
  }
#else
  //TODO: win32
#endif

  errno = saved_errno;
}

int K3ProcessController::notifierFd() const
{
  return d->fd[0];
}

void K3ProcessController::unscheduleCheck()
{
  char dummy[16]; // somewhat bigger - just in case several have queued up
  if ( ::read( d->fd[0], dummy, sizeof( dummy ) ) > 0 )
    d->needcheck = true;
}

void
K3ProcessController::rescheduleCheck()
{
  if ( d->needcheck )
  {
    d->needcheck = false;
    char dummy = 0;
    if( ::write( d->fd[1], &dummy, 1 ) < 0 )
      perror( "write failed" );
  }
}

void K3ProcessController::slotDoHousekeeping()
{
  char dummy[16]; // somewhat bigger - just in case several have queued up
  if( ::read( d->fd[0], dummy, sizeof( dummy ) ) < 0 )
    perror( "read failed" );

  int status;
again:
  QList<K3Process*>::iterator it( d->kProcessList.begin() );
  QList<K3Process*>::iterator eit( d->kProcessList.end() );
  while ( it != eit )
  {
    K3Process *prc = *it;
    if ( prc->runs && waitpid( prc->pid_, &status, WNOHANG ) > 0 )
    {
      prc->processHasExited( status );
      // the callback can nuke the whole process list and even 'this'
      if ( !instance() )
        return;
      goto again;
    }
    ++it;
  }
  QList<int>::iterator uit( d->unixProcessList.begin() );
  QList<int>::iterator ueit( d->unixProcessList.end() );
  while ( uit != ueit )
  {
    if ( waitpid( *uit, 0, WNOHANG ) > 0 )
    {
      uit = d->unixProcessList.erase( uit );
      deref(); // counterpart to addProcess, can invalidate 'this'
    }
    else
      ++uit;
  }
}

bool K3ProcessController::waitForProcessExit( int timeout )
{
#ifdef Q_OS_UNIX
  for ( ;; )
  {
    struct timeval tv, *tvp;
    if ( timeout < 0 )
      tvp = 0;
    else
    {
      tv.tv_sec = timeout;
      tv.tv_usec = 0;
      tvp = &tv;
    }

    fd_set fds;
    FD_ZERO( &fds );
    FD_SET( d->fd[0], &fds );

    switch ( select( d->fd[0] + 1, &fds, 0, 0, tvp ) )
    {
      case -1:
        if ( errno == EINTR )
          continue;
        // fall through; should never happen
      case 0:
        return false;
      default:
        slotDoHousekeeping();
        return true;
    }
  }
#else
  //TODO: win32
  return false;
#endif
}

void K3ProcessController::addKProcess( K3Process* p )
{
  d->kProcessList.append( p );
}

void K3ProcessController::removeKProcess( K3Process* p )
{
  d->kProcessList.removeAll( p );
}

void K3ProcessController::addProcess( int pid )
{
  d->unixProcessList.append( pid );
  ref(); // make sure we stay around when the K3Process goes away
}

//#include "moc_k3processcontroller.cpp"
