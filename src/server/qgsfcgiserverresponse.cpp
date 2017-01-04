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
#include "qgslogger.h"
#include "qgsmessagelog.h"
#include <fcgi_stdio.h>

#include <QDebug>

//
// QgsFcgiServerResponse
//

QgsFcgiServerResponse::QgsFcgiServerResponse( QgsServerRequest::Method method )
{
  mBuffer.open( QIODevice::ReadWrite );
  mHeadersWritten = false;
  mFinished       = false;
  mMethod         = method;
}

QgsFcgiServerResponse::~QgsFcgiServerResponse()
{

}

void QgsFcgiServerResponse::clearHeader( const QString& key )
{
  mHeaders.remove( key );
}

void QgsFcgiServerResponse::setHeader( const QString& key, const QString& value )
{
  mHeaders.insert( key, value );
}

void QgsFcgiServerResponse::setReturnCode( int code )
{
  // fcgi applications must return HTTP status in header
  mHeaders.insert( QStringLiteral( "Status" ), QStringLiteral( " %1" ).arg( code ) );
}

void QgsFcgiServerResponse::sendError( int code,  const QString& message )
{
  if ( mHeadersWritten )
  {
    QgsMessageLog::logMessage( "Cannot send error after headers written" );
    return;
  }

  clear();
  setDefaultHeaders();
  setReturnCode( code );
  setHeader( QStringLiteral( "Content-Type" ), QStringLiteral( "text/html;charset=utf-8" ) );
  write( QStringLiteral( "<html><body>%1</body></html>" ).arg( message ) );
  finish();
}

QIODevice* QgsFcgiServerResponse::io()
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

void QgsFcgiServerResponse::flush()
{
  if ( ! mHeadersWritten )
  {
    // Send all headers
    QMap<QString, QString>::const_iterator it;
    for ( it = mHeaders.constBegin(); it != mHeaders.constEnd(); ++it )
    {
      fputs( it.key().toUtf8(), FCGI_stdout );
      fputs( ": ", FCGI_stdout );
      fputs( it.value().toUtf8(), FCGI_stdout );
      fputs( "\n" , FCGI_stdout );
    }
    fputs( "\n", FCGI_stdout );
    mHeadersWritten = true;
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
    QByteArray& ba = mBuffer.buffer();
    size_t count   = fwrite(( void* )ba.data(), ba.size(), 1, FCGI_stdout );
#ifdef QGISDEBUG
    qDebug() << QStringLiteral( "Sent %1 blocks of %2 bytes" ).arg( count ).arg( ba.size() );
#else
    Q_UNUSED( count );
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
}

void QgsFcgiServerResponse::setDefaultHeaders()
{
  setHeader( QStringLiteral( "Server" ), QStringLiteral( " Qgis FCGI server - QGis version %1" ).arg( Qgis::QGIS_VERSION ) );
}


//
// QgsFcgiServerRequest
//

QgsFcgiServerRequest::QgsFcgiServerRequest()
{
  mHasError  = false;

  // Get the REQUEST_URI from the environment
  QUrl url;
  const char* uri = getenv( "REQUEST_URI" );
  if ( uri )
  {
    url.setUrl( uri );
  }
  // XXX OGC paremetrs are passed with the query string
  // we override the query string url in case it is
  // defined independently of REQUEST_URI
  const char* qs = getenv( "QUERY_STRING" );
  if ( qs )
  {
    url.setQuery( qs );
  }

#ifdef QGISDEBUG
  qDebug() << "fcgi query string: " << url.query();
#endif

  QgsServerRequest::Method method = GetMethod;

  // Get method
  const char* me = getenv( "REQUEST_METHOD" );

  if ( me )
  {
    if ( strcmp( me, "POST" ) == 0 )
    {
      method = PostMethod;
    }
    else if ( strcmp( me, "PUT" ) == 0 )
    {
      method = PutMethod;
    }
    else if ( strcmp( me, "DELETE" ) == 0 )
    {
      method = DeleteMethod;
    }
    else if ( strcmp( me, "HEAD" ) == 0 )
    {
      method = HeadMethod;
    }
  }

  if ( method == PostMethod || method == PutMethod )
  {
    // Get post/put data
    readData();
  }

  setUrl( url );
  setMethod( method );
}

QgsFcgiServerRequest::~QgsFcgiServerRequest()
{

}

QByteArray QgsFcgiServerRequest::data() const
{
  return mData;
}

// Read post put data
void QgsFcgiServerRequest::readData()
{
  // Check if we have CONTENT_LENGTH defined
  const char* lengthstr = getenv( "CONTENT_LENGTH" );
  if ( lengthstr )
  {
#ifdef QGISDEBUG
    qDebug() << "fcgi: reading " << lengthstr << " bytes from stdin";
#endif
    bool success = false;
    int length = QString( lengthstr ).toInt( &success );
    if ( success )
    {
      // XXX This not efficiont at all  !!
      for ( int i = 0; i < length; ++i )
      {
        mData.append( getchar() );
      }
    }
    else
    {
      QgsMessageLog::logMessage( "fcgi: Failed to parse CONTENT_LENGTH",
                                 QStringLiteral( "Server" ), QgsMessageLog::CRITICAL );
      mHasError = true;
    }
  }
  else
  {
    QgsMessageLog::logMessage( "fcgi: No POST data" );
  }
}

