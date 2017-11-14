/***************************************************************************
                          qgsfcgiserverrequest.cpp

  Define response wrapper for fcgi request
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
#include "qgsfcgiserverrequest.h"
#include "qgslogger.h"
#include "qgsserverlogger.h"
#include "qgsmessagelog.h"
#include <fcgi_stdio.h>

#include <QDebug>


QgsFcgiServerRequest::QgsFcgiServerRequest()
{
  mHasError  = false;

  // Rebuild the full URL

  // Get the REQUEST_URI from the environment
  QUrl url;
  QString uri = getenv( "REQUEST_URI" );
  if ( uri.isEmpty() )
  {
    uri = getenv( "SCRIPT_NAME" );
  }

  url.setUrl( uri );

  // Check if host is defined
  if ( url.host().isEmpty() )
  {
    url.setHost( getenv( "SERVER_NAME" ) );
  }

  // Port ?
  if ( url.port( -1 ) == -1 )
  {
    QString portString = getenv( "SERVER_PORT" );
    if ( !portString.isEmpty() )
    {
      bool portOk;
      int portNumber = portString.toInt( &portOk );
      if ( portOk && portNumber != 80 )
      {
        url.setPort( portNumber );
      }
    }
  }

  // scheme
  if ( url.scheme().isEmpty() )
  {
    QString( getenv( "HTTPS" ) ).compare( QLatin1String( "on" ), Qt::CaseInsensitive ) == 0
    ? url.setScheme( QStringLiteral( "https" ) )
    : url.setScheme( QStringLiteral( "http" ) );
  }

  // XXX OGC paremetrs are passed with the query string
  // we override the query string url in case it is
  // defined independently of REQUEST_URI
  const char *qs = getenv( "QUERY_STRING" );
  if ( qs )
  {
    url.setQuery( qs );
  }

#ifdef QGISDEBUG
  qDebug() << "fcgi query string: " << url.query();
#endif

  QgsServerRequest::Method method = GetMethod;

  // Get method
  const char *me = getenv( "REQUEST_METHOD" );

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

  // Output debug infos
  QgsMessageLog::MessageLevel logLevel = QgsServerLogger::instance()->logLevel();
  if ( logLevel <= QgsMessageLog::INFO )
  {
    printRequestInfos();
  }
}

QByteArray QgsFcgiServerRequest::data() const
{
  return mData;
}

// Read post put data
void QgsFcgiServerRequest::readData()
{
  // Check if we have CONTENT_LENGTH defined
  const char *lengthstr = getenv( "CONTENT_LENGTH" );
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

void QgsFcgiServerRequest::printRequestInfos()
{
  QgsMessageLog::logMessage( QStringLiteral( "******************** New request ***************" ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  if ( getenv( "REMOTE_ADDR" ) )
  {
    QgsMessageLog::logMessage( "REMOTE_ADDR: " + QString( getenv( "REMOTE_ADDR" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_HOST" ) )
  {
    QgsMessageLog::logMessage( "REMOTE_HOST: " + QString( getenv( "REMOTE_HOST" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_USER" ) )
  {
    QgsMessageLog::logMessage( "REMOTE_USER: " + QString( getenv( "REMOTE_USER" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "REMOTE_IDENT" ) )
  {
    QgsMessageLog::logMessage( "REMOTE_IDENT: " + QString( getenv( "REMOTE_IDENT" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "CONTENT_TYPE" ) )
  {
    QgsMessageLog::logMessage( "CONTENT_TYPE: " + QString( getenv( "CONTENT_TYPE" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "AUTH_TYPE" ) )
  {
    QgsMessageLog::logMessage( "AUTH_TYPE: " + QString( getenv( "AUTH_TYPE" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_USER_AGENT" ) )
  {
    QgsMessageLog::logMessage( "HTTP_USER_AGENT: " + QString( getenv( "HTTP_USER_AGENT" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_PROXY" ) )
  {
    QgsMessageLog::logMessage( "HTTP_PROXY: " + QString( getenv( "HTTP_PROXY" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "HTTPS_PROXY" ) )
  {
    QgsMessageLog::logMessage( "HTTPS_PROXY: " + QString( getenv( "HTTPS_PROXY" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "NO_PROXY" ) )
  {
    QgsMessageLog::logMessage( "NO_PROXY: " + QString( getenv( "NO_PROXY" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
  if ( getenv( "HTTP_AUTHORIZATION" ) )
  {
    QgsMessageLog::logMessage( "HTTP_AUTHORIZATION: " + QString( getenv( "HTTP_AUTHORIZATION" ) ), QStringLiteral( "Server" ), QgsMessageLog::INFO );
  }
}
