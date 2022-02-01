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
#include "qgsserverlogger.h"
#include "qgsmessagelog.h"
#include "qgsstringutils.h"
#include <fcgi_stdio.h>
#include <QDebug>

QgsFcgiServerRequest::QgsFcgiServerRequest()
{
  // Get the REQUEST_URI from the environment
  QString uri = getenv( "REQUEST_URI" );

  if ( uri.isEmpty() )
  {
    uri = getenv( "SCRIPT_NAME" );
  }

  QUrl url;
  url.setUrl( uri );
  fillUrl( url );
  // Store the URL before the server rewrite that could have been set in QUERY_STRING
  setOriginalUrl( url );

  const QString qs = getenv( "QUERY_STRING" );
  const QString questionMark = qs.isEmpty() ? QString() : QChar( '?' );
  const QString extraPath = QStringLiteral( "%1%2%3" ).arg( getenv( "PATH_INFO" ) ).arg( questionMark ).arg( qs );

  QUrl baseUrl;
  if ( uri.endsWith( extraPath ) )
  {
    baseUrl.setUrl( uri.left( uri.length() -  extraPath.length() ) );
  }
  else
  {
    baseUrl.setUrl( uri );
  }
  fillUrl( baseUrl );
  setBaseUrl( url );

  // OGC parameters are passed with the query string, which is normally part of
  // the REQUEST_URI, we override the query string url in case it is defined
  // independently of REQUEST_URI
  if ( ! qs.isEmpty() )
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
    else if ( strcmp( me, "PATCH" ) == 0 )
    {
      method = PatchMethod;
    }
  }

  if ( method == PostMethod || method == PutMethod )
  {
    // Get post/put data
    readData();
  }

  setUrl( url );
  setMethod( method );

  // Fill the headers dictionary
  for ( const auto &headerKey : qgsEnumMap<QgsServerRequest::RequestHeader>().values() )
  {
    const QString headerName = QgsStringUtils::capitalize(
                                 QString( headerKey ).replace( QLatin1Char( '_' ), QLatin1Char( ' ' ) ), Qgis::Capitalization::TitleCase
                               ).replace( QLatin1Char( ' ' ), QLatin1Char( '-' ) );
    const char *result = getenv( QStringLiteral( "HTTP_%1" ).arg( headerKey ).toStdString().c_str() );
    if ( result && strlen( result ) > 0 )
    {
      setHeader( headerName, result );
    }
  }

  // Output debug infos
  const Qgis::MessageLevel logLevel = QgsServerLogger::instance()->logLevel();
  if ( logLevel <= Qgis::MessageLevel::Info )
  {
    printRequestInfos( url );
  }
}

void QgsFcgiServerRequest::fillUrl( QUrl &url ) const
{
  // Check if host is defined
  if ( url.host().isEmpty() )
  {
    url.setHost( getenv( "SERVER_NAME" ) );
  }

  // Port ?
  if ( url.port( -1 ) == -1 )
  {
    const QString portString = getenv( "SERVER_PORT" );
    if ( !portString.isEmpty() )
    {
      bool portOk;
      const int portNumber = portString.toInt( &portOk );
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
    bool success = false;
    int length = QString( lengthstr ).toInt( &success );
    // Note: REQUEST_BODY is not part of CGI standard, and it is not
    // normally passed by any CGI web server and it is implemented only
    // to allow unit tests to inject a request body and simulate a POST
    // request
    const char *request_body  = getenv( "REQUEST_BODY" );
    if ( success && request_body )
    {
      QString body( request_body );
      body.truncate( length );
      mData.append( body.toUtf8() );
      length = 0;
    }
#ifdef QGISDEBUG
    qDebug() << "fcgi: reading " << lengthstr << " bytes from " << ( request_body ? "REQUEST_BODY" : "stdin" );
#endif
    if ( success )
    {
      // XXX This not efficient at all  !!
      for ( int i = 0; i < length; ++i )
      {
        mData.append( getchar() );
      }
    }
    else
    {
      QgsMessageLog::logMessage( "fcgi: Failed to parse CONTENT_LENGTH",
                                 QStringLiteral( "Server" ), Qgis::MessageLevel::Critical );
      mHasError = true;
    }
  }
  else
  {
    QgsMessageLog::logMessage( "fcgi: No POST data" );
  }
}

void QgsFcgiServerRequest::printRequestInfos( const QUrl &url )
{
  QgsMessageLog::logMessage( QStringLiteral( "******************** New request ***************" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );

  const QStringList envVars
  {
    QStringLiteral( "SERVER_NAME" ),
    QStringLiteral( "REQUEST_URI" ),
    QStringLiteral( "SCRIPT_NAME" ),
    QStringLiteral( "PATH_INFO" ),
    QStringLiteral( "HTTPS" ),
    QStringLiteral( "REMOTE_ADDR" ),
    QStringLiteral( "REMOTE_HOST" ),
    QStringLiteral( "SERVER_PORT" ),
    QStringLiteral( "QUERY_STRING" ),
    QStringLiteral( "REMOTE_USER" ),
    QStringLiteral( "REMOTE_IDENT" ),
    QStringLiteral( "CONTENT_TYPE" ),
    QStringLiteral( "REQUEST_METHOD" ),
    QStringLiteral( "AUTH_TYPE" ),
    QStringLiteral( "HTTP_PROXY" ),
    QStringLiteral( "NO_PROXY" ),
    QStringLiteral( "QGIS_PROJECT_FILE" ),
    QStringLiteral( "QGIS_SERVER_IGNORE_BAD_LAYERS" ),
    QStringLiteral( "QGIS_SERVER_SERVICE_URL" ),
    QStringLiteral( "QGIS_SERVER_WMS_SERVICE_URL" ),
    QStringLiteral( "QGIS_SERVER_WFS_SERVICE_URL" ),
    QStringLiteral( "QGIS_SERVER_WMTS_SERVICE_URL" ),
    QStringLiteral( "QGIS_SERVER_WCS_SERVICE_URL" ),
    QStringLiteral( "SERVER_PROTOCOL" )
  };

  QgsMessageLog::logMessage( QStringLiteral( "Request URL: %2" ).arg( url.url() ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );

  QgsMessageLog::logMessage( QStringLiteral( "Environment:" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( QStringLiteral( "------------------------------------------------" ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
  for ( const auto &envVar : envVars )
  {
    if ( getenv( envVar.toStdString().c_str() ) )
    {
      QgsMessageLog::logMessage( QStringLiteral( "%1: %2" ).arg( envVar ).arg( QString( getenv( envVar.toStdString().c_str() ) ) ), QStringLiteral( "Server" ), Qgis::MessageLevel::Info );
    }
  }

  qDebug() << "Headers:";
  qDebug() << "------------------------------------------------";
  for ( const auto &headerName : headers().keys() )
  {
    qDebug() << headerName << ": " << headers().value( headerName );
  }
}

QString QgsFcgiServerRequest::header( const QString &name ) const
{
  // Get from internal dictionary
  QString result = QgsServerRequest::header( name );

  // Or from standard environment variable
  // https://tools.ietf.org/html/rfc3875#section-4.1.18
  if ( result.isEmpty() )
  {
    result = qgetenv( QStringLiteral( "HTTP_%1" ).arg(
                        name.toUpper().replace( QLatin1Char( '-' ), QLatin1Char( '_' ) ) ).toStdString().c_str() );
  }
  return result;
}
