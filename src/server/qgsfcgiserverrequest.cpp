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

#include "qgsfcgiserverrequest.h"

#include "qgis.h"
#include "qgsmessagelog.h"
#include "qgsserverlogger.h"
#include "qgsstringutils.h"

#include <QDebug>

#include <fcgi_stdio.h>

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
  const QString extraPath = u"%1%2%3"_s.arg( getenv( "PATH_INFO" ) ).arg( questionMark ).arg( qs );

  QUrl baseUrl;
  if ( uri.endsWith( extraPath ) )
  {
    baseUrl.setUrl( uri.left( uri.length() - extraPath.length() ) );
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
  if ( !qs.isEmpty() )
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

  if ( method == PostMethod || method == PutMethod || method == PatchMethod )
  {
    // Get post/put/patch data
    readData();
  }

  setUrl( url );
  setMethod( method );

  // Fill the headers dictionary
  for ( const auto &headerKey : qgsEnumMap<QgsServerRequest::RequestHeader>() )
  {
    const QString headerName = QgsStringUtils::capitalize(
                                 QString( headerKey ).replace( '_'_L1, ' '_L1 ), Qgis::Capitalization::TitleCase
    )
                                 .replace( ' '_L1, '-'_L1 );
    const char *result = getenv( u"HTTP_%1"_s.arg( headerKey ).toStdString().c_str() );
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
    QString( getenv( "HTTPS" ) ).compare( "on"_L1, Qt::CaseInsensitive ) == 0
      ? url.setScheme( u"https"_s )
      : url.setScheme( u"http"_s );
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
    const int length = QString( lengthstr ).toInt( &success );
    if ( !success || length < 0 )
    {
      QgsMessageLog::logMessage( "fcgi: Invalid CONTENT_LENGTH", u"Server"_s, Qgis::MessageLevel::Critical );
      mHasError = true;
    }
    else
    {
      // Note: REQUEST_BODY is not part of CGI standard, and it is not
      // normally passed by any CGI web server and it is implemented only
      // to allow unit tests to inject a request body and simulate a POST
      // request
      const char *requestBody = getenv( "REQUEST_BODY" );

#ifdef QGISDEBUG
      qDebug() << "fcgi: reading " << lengthstr << " bytes from " << ( requestBody ? "REQUEST_BODY" : "stdin" );
#endif

      if ( requestBody )
      {
        const size_t requestBodyLength = strlen( requestBody );
        const int actualLength = static_cast<int>( std::min<size_t>( length, requestBodyLength ) );
        if ( static_cast<size_t>( actualLength ) < requestBodyLength )
        {
          QgsMessageLog::logMessage( "fcgi: CONTENT_LENGTH is larger than actual length of REQUEST_BODY", u"Server"_s, Qgis::MessageLevel::Critical );
          mHasError = true;
        }
        mData = QByteArray::fromRawData( requestBody, actualLength );
      }
      else
      {
        mData.resize( length );
        const int actualLength = static_cast<int>( fread( mData.data(), 1, length, stdin ) );
        if ( actualLength < length )
        {
          mData.resize( actualLength );
          QgsMessageLog::logMessage( "fcgi: CONTENT_LENGTH is larger than actual length of stdin", u"Server"_s, Qgis::MessageLevel::Critical );
          mHasError = true;
        }
      }
    }
  }
  else
  {
    QgsMessageLog::logMessage( "fcgi: No POST/PUT/PATCH data" );
  }
}

void QgsFcgiServerRequest::printRequestInfos( const QUrl &url ) const
{
  QgsMessageLog::logMessage( u"******************** New request ***************"_s, u"Server"_s, Qgis::MessageLevel::Info );

  const QStringList envVars {
    u"SERVER_NAME"_s,
    u"REQUEST_URI"_s,
    u"SCRIPT_NAME"_s,
    u"PATH_INFO"_s,
    u"HTTPS"_s,
    u"REMOTE_ADDR"_s,
    u"REMOTE_HOST"_s,
    u"SERVER_PORT"_s,
    u"QUERY_STRING"_s,
    u"REMOTE_USER"_s,
    u"REMOTE_IDENT"_s,
    u"CONTENT_TYPE"_s,
    u"REQUEST_METHOD"_s,
    u"AUTH_TYPE"_s,
    u"HTTP_PROXY"_s,
    u"NO_PROXY"_s,
    u"QGIS_PROJECT_FILE"_s,
    u"QGIS_SERVER_IGNORE_BAD_LAYERS"_s,
    u"QGIS_SERVER_SERVICE_URL"_s,
    u"QGIS_SERVER_WMS_SERVICE_URL"_s,
    u"QGIS_SERVER_WFS_SERVICE_URL"_s,
    u"QGIS_SERVER_WMTS_SERVICE_URL"_s,
    u"QGIS_SERVER_WCS_SERVICE_URL"_s,
    u"SERVER_PROTOCOL"_s
  };

  QgsMessageLog::logMessage( u"Request URL: %2"_s.arg( url.url() ), u"Server"_s, Qgis::MessageLevel::Info );

  QgsMessageLog::logMessage( u"Environment:"_s, u"Server"_s, Qgis::MessageLevel::Info );
  QgsMessageLog::logMessage( u"------------------------------------------------"_s, u"Server"_s, Qgis::MessageLevel::Info );
  for ( const auto &envVar : envVars )
  {
    if ( getenv( envVar.toStdString().c_str() ) )
    {
      QgsMessageLog::logMessage( u"%1: %2"_s.arg( envVar ).arg( QString( getenv( envVar.toStdString().c_str() ) ) ), u"Server"_s, Qgis::MessageLevel::Info );
    }
  }

  qDebug() << "Headers:";
  qDebug() << "------------------------------------------------";
  const QMap<QString, QString> &hdrs = headers();
  for ( auto it = hdrs.constBegin(); it != hdrs.constEnd(); it++ )
  {
    qDebug() << it.key() << ": " << it.value();
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
    result = qgetenv( u"HTTP_%1"_s.arg( name.toUpper().replace( '-'_L1, '_'_L1 ) ).toStdString().c_str() );
  }
  return result;
}
