/***************************************************************************
                          qgsbufferserverresponse.cpp

  Define response wrapper for buffer response
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

#include "qgsbufferserverresponse.h"
#include "qgsmessagelog.h"

//
// QgsBufferServerResponse
//

QgsBufferServerResponse::QgsBufferServerResponse()
{
  mBuffer.open( QIODevice::ReadWrite );
}

void QgsBufferServerResponse::removeHeader( const QString &key )
{
  if ( !mHeadersSent )
    mHeaders.remove( key );
}

void QgsBufferServerResponse::setHeader( const QString &key, const QString &value )
{
  if ( ! mHeadersSent )
    mHeaders.insert( key, value );
}

void QgsBufferServerResponse::setStatusCode( int code )
{
  mStatusCode = code;
}

QString QgsBufferServerResponse::header( const QString &key ) const
{
  return mHeaders.value( key );
}

bool QgsBufferServerResponse::headersSent() const
{
  return mHeadersSent;
}

void QgsBufferServerResponse::sendError( int code,  const QString &message )
{
  if ( mHeadersSent )
  {
    QgsMessageLog::logMessage( "Cannot send error after headers sent" );
    return;
  }

  clear();
  setStatusCode( code );
  setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/plain; charset=utf-8" ) );
  write( message );
  finish();
}

QIODevice *QgsBufferServerResponse::io()
{
  return &mBuffer;
}

void QgsBufferServerResponse::finish()
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
      mHeaders.insert( QStringLiteral( "Content-Length" ), QStringLiteral( "%1" ).arg( mBuffer.pos() ) );
    }
  }
  flush();
  mFinished = true;
}

void QgsBufferServerResponse::flush()
{
  if ( ! mHeadersSent )
  {
    mHeadersSent = true;
  }

  mBuffer.seek( 0 );
  QByteArray &ba = mBuffer.buffer();
  mBody.append( ba );
  ba.clear();
}


void QgsBufferServerResponse::clear()
{
  mHeaders.clear();
  mBuffer.seek( 0 );
  mBuffer.buffer().clear();
}


QByteArray QgsBufferServerResponse::data() const
{
  return mBuffer.data();
}


void QgsBufferServerResponse::truncate()
{
  mBuffer.seek( 0 );
  mBuffer.buffer().clear();
}
