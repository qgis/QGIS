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

//
// QgsFcgiServerResponse
//

QgsFcgiServerResponse::QgsFcgiServerResponse( QgsServerRequest::Method method )
  : mMethod( method )
{
  mBuffer.open( QIODevice::ReadWrite );
  setDefaultHeaders();
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
    QgsMessageLog::logMessage( "Cannot send error after headers written" );
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
    QgsMessageLog::logMessage( "finish() called twice" );
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
