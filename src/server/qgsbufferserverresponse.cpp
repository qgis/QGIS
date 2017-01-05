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

#include "qgsbufferserverresponse.h"
#include "qgslogger.h"
#include "qgsmessagelog.h"

#include <QDebug>

//
// QgsBufferServerResponse
//

QgsBufferServerResponse::QgsBufferServerResponse()
{
  mBuffer.open( QIODevice::ReadWrite );
  mHeadersWritten = false;
  mFinished       = false;
  mReturnCode     = 200;
}

QgsBufferServerResponse::~QgsBufferServerResponse()
{

}

void QgsBufferServerResponse::clearHeader( const QString& key )
{
  if ( !mHeadersWritten )
    mHeaders.remove( key );
}

void QgsBufferServerResponse::setHeader( const QString& key, const QString& value )
{
  if ( ! mHeadersWritten )
    mHeaders.insert( key, value );
}

void QgsBufferServerResponse::setReturnCode( int code )
{
  mReturnCode = code;
}

QString QgsBufferServerResponse::getHeader( const QString& key ) const
{
  return mHeaders.value( key );
}

QList<QString> QgsBufferServerResponse::headerKeys() const
{
  return mHeaders.keys();
}

bool QgsBufferServerResponse::headersWritten() const
{
  return mHeadersWritten;
}

void QgsBufferServerResponse::sendError( int code,  const QString& message )
{
  if ( mHeadersWritten )
  {
    QgsMessageLog::logMessage( "Cannot send error after headers written" );
    return;
  }

  clear();
  setReturnCode( code );
  setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/plain; charset=utf-8" ) );
  write( message );
  finish();
}

QIODevice* QgsBufferServerResponse::io()
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

  if ( !mHeadersWritten )
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
  if ( ! mHeadersWritten )
  {
    mHeadersWritten = true;
  }

  mBuffer.seek( 0 );
  QByteArray& ba = mBuffer.buffer();
  mBody.append( ba );
  ba.clear();
}


void QgsBufferServerResponse::clear()
{
  mHeaders.clear();
  mBuffer.seek( 0 );
  mBuffer.buffer().clear();
}


//QgsBufferServerRequest
//
QgsBufferServerRequest::QgsBufferServerRequest( const QString& url, Method method, QByteArray* data )
    : QgsServerRequest( url, method )
{
  if ( data )
  {
    mData = *data;
  }
}

QgsBufferServerRequest::QgsBufferServerRequest( const QUrl& url, Method method, QByteArray* data )
    : QgsServerRequest( url, method )
{
  if ( data )
  {
    mData = *data;
  }
}

QgsBufferServerRequest::~QgsBufferServerRequest()
{
}



