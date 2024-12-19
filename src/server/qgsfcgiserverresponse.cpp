/***************************************************************************
                          qgsfcgiserverresponse.cpp

  Define response wrapper for fcgi response
  -------------------
  begin                : 2017-01-03
  copyright            : (C) 2017 by David Marteau
  email                : david dot marteau at 3liz dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsfcgiserverresponse.h"
#include "qgsmessagelog.h"
#include <fcgi_stdio.h>
#include <QDebug>

#include "qgslogger.h"

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
#include <sys/types.h>
#include <sys/socket.h>

//
// QgsFCGXStreamData copied from libfcgi FCGX_Stream_Data
//
typedef struct QgsFCGXStreamData
{
  unsigned char *buff;      /* buffer after alignment */
  int bufflen;              /* number of bytes buff can store */
  unsigned char *mBuff;     /* buffer as returned by Malloc */
  unsigned char *buffStop;  /* reader: last valid byte + 1 of entire buffer.
                               * stop generally differs from buffStop for
                               * readers because of record structure.
                               * writer: buff + bufflen */
  int type;                 /* reader: FCGI_PARAMS or FCGI_STDIN
                               * writer: FCGI_STDOUT or FCGI_STDERR */
  int eorStop;              /* reader: stop stream at end-of-record */
  int skip;                 /* reader: don't deliver content bytes */
  int contentLen;           /* reader: bytes of unread content */
  int paddingLen;           /* reader: bytes of unread padding */
  int isAnythingWritten;    /* writer: data has been written to ipcFd */
  int rawWrite;             /* writer: write data without stream headers */
  FCGX_Request *reqDataPtr; /* request data not specific to one stream */
} QgsFCGXStreamData;
#endif

QgsSocketMonitoringThread::QgsSocketMonitoringThread( bool *isResponseFinished, QgsFeedback *feedback )
  : mIsResponseFinished( isResponseFinished )
  , mFeedback( feedback )
  , mIpcFd( -1 )
{
  setObjectName( "FCGI socket monitor" );
  Q_ASSERT( mIsResponseFinished );
  Q_ASSERT( mFeedback );

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
  if ( FCGI_stdout && FCGI_stdout->fcgx_stream && FCGI_stdout->fcgx_stream->data )
  {
    QgsFCGXStreamData *stream = static_cast<QgsFCGXStreamData *>( FCGI_stdin->fcgx_stream->data );
    if ( stream && stream->reqDataPtr )
    {
      mIpcFd = stream->reqDataPtr->ipcFd;
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "FCGI_stdin stream data is null! Socket monitoring disable." ),
                                 QStringLiteral( "FCGIServer" ),
                                 Qgis::MessageLevel::Warning );
    }
  }
  else
  {
    QgsMessageLog::logMessage( QStringLiteral( "FCGI_stdin is null! Socket monitoring disable." ),
                               QStringLiteral( "FCGIServer" ),
                               Qgis::MessageLevel::Warning );
  }
#endif
}

void QgsSocketMonitoringThread::run( )
{
  if ( mIpcFd < 0 )
  {
    QgsMessageLog::logMessage( QStringLiteral( "Socket monitoring disabled: no socket fd!" ),
                               QStringLiteral( "FCGIServer" ),
                               Qgis::MessageLevel::Warning );
    return;
  }

#if defined(Q_OS_UNIX) && !defined(Q_OS_ANDROID)
  char c;
  while ( !*mIsResponseFinished )
  {
    const ssize_t x = recv( mIpcFd, &c, 1, MSG_PEEK | MSG_DONTWAIT ); // see https://stackoverflow.com/a/12402596
    if ( x < 0 )
    {
      // Ie. we are still connected but we have an 'error' as there is nothing to read
      QgsDebugMsgLevel( QStringLiteral( "FCGIServer: remote socket still connected. errno: %1" ).arg( errno ), 5 );
    }
    else
    {
      // socket closed, nothing can be read
      QgsDebugMsgLevel( QStringLiteral( "FCGIServer: remote socket has been closed! errno: %1" ).arg( errno ), 2 );
      mFeedback->cancel();
      break;
    }

    QThread::msleep( 333L );
  }

  if ( *mIsResponseFinished )
  {
    QgsDebugMsgLevel( QStringLiteral( "FCGIServer: socket monitoring quits normally." ), 2 );
  }
  else
  {
    QgsDebugMsgLevel( QStringLiteral( "FCGIServer: socket monitoring quits: no more socket." ), 2 );
  }
#endif
}



//
// QgsFcgiServerResponse
//
QgsFcgiServerResponse::QgsFcgiServerResponse( QgsServerRequest::Method method )
  : mMethod( method )
  , mFeedback( new QgsFeedback )
{
  mBuffer.open( QIODevice::ReadWrite );
  setDefaultHeaders();

  mSocketMonitoringThread = std::make_unique<QgsSocketMonitoringThread>( &mFinished, mFeedback.get() );
  mSocketMonitoringThread->start();
}

QgsFcgiServerResponse::~QgsFcgiServerResponse()
{
  mFinished = true;
  mSocketMonitoringThread->exit();
  mSocketMonitoringThread->wait();
}

void QgsFcgiServerResponse::removeHeader( const QString &key )
{
  mHeaders.remove( key );
}

void QgsFcgiServerResponse::setHeader( const QString &key, const QString &value )
{
  mHeaders.insert( key, value );
}

QString QgsFcgiServerResponse::header( const QString &key ) const
{
  return mHeaders.value( key );
}

bool QgsFcgiServerResponse::headersSent() const
{
  return mHeadersSent;
}

void QgsFcgiServerResponse::setStatusCode( int code )
{
  // fcgi applications must return HTTP status in header
  mHeaders.insert( QStringLiteral( "Status" ), QStringLiteral( " %1" ).arg( code ) );
  // Store the code to make it available for plugins
  mStatusCode = code;
}

void QgsFcgiServerResponse::sendError( int code,  const QString &message )
{
  if ( mHeadersSent )
  {
    QgsMessageLog::logMessage( "Cannot send error after headers written",
                               QStringLiteral( "FCGIServer" ),
                               Qgis::MessageLevel::Warning );
    return;
  }

  clear();
  setStatusCode( code );
  setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html;charset=utf-8" ) );
  write( QStringLiteral( "<html><body>%1</body></html>" ).arg( message ) );
  finish();
}

QIODevice *QgsFcgiServerResponse::io()
{
  return &mBuffer;
}

void QgsFcgiServerResponse::finish()
{
  if ( mFinished )
  {
    QgsMessageLog::logMessage( "finish() called twice",
                               QStringLiteral( "FCGIServer" ),
                               Qgis::MessageLevel::Warning );
    return;
  }

  if ( mFeedback->isCanceled() )
  {
    clear(); // we clear all buffers as the socket is dead
    FCGI_stdout->fcgx_stream->wasFCloseCalled = true; // avoid sending FCGI end protocol as the socket is dead
    mFinished = true;
    return;
  }

  if ( !mHeadersSent )
  {
    if ( ! mHeaders.contains( "Content-Length" ) )
    {
      mHeaders.insert( QStringLiteral( "Content-Length" ), QString::number( mBuffer.pos() ) );
    }
  }
  flush();
  mFinished = true;
}

void QgsFcgiServerResponse::flush()
{
  if ( ! mHeadersSent )
  {
    // Send all headers
    QMap<QString, QString>::const_iterator it;
    for ( it = mHeaders.constBegin(); it != mHeaders.constEnd(); ++it )
    {
      fputs( it.key().toUtf8(), FCGI_stdout );
      fputs( ": ", FCGI_stdout );
      fputs( it.value().toUtf8(), FCGI_stdout );
      fputs( "\n", FCGI_stdout );
    }
    fputs( "\n", FCGI_stdout );
    mHeadersSent = true;
  }

  mBuffer.seek( 0 );
  if ( mMethod == QgsServerRequest::HeadMethod )
  {
    // Ignore data for head method as we only
    // write headers for HEAD requests
    mBuffer.buffer().clear();
  }
  else if ( mBuffer.bytesAvailable() > 0 )
  {
    QByteArray &ba = mBuffer.buffer();
    const size_t count   = fwrite( ( void * )ba.data(), ba.size(), 1, FCGI_stdout );
#ifdef QGISDEBUG
    qDebug() << QStringLiteral( "Sent %1 blocks of %2 bytes" ).arg( count ).arg( ba.size() );
#else
    Q_UNUSED( count )
#endif
    // Reset the internal buffer
    ba.clear();
  }
}


void QgsFcgiServerResponse::clear()
{
  mHeaders.clear();
  mBuffer.seek( 0 );
  mBuffer.buffer().clear();

  // Restore default headers
  setDefaultHeaders();
}


QByteArray QgsFcgiServerResponse::data() const
{
  return mBuffer.data();
}


void QgsFcgiServerResponse::truncate()
{
  mBuffer.seek( 0 );
  mBuffer.buffer().clear();
}


void QgsFcgiServerResponse::setDefaultHeaders()
{
  setHeader( QStringLiteral( "Server" ), QStringLiteral( " QGIS FCGI server - QGIS version %1" ).arg( Qgis::version() ) );
}
