/***************************************************************************
                              qgshttprequesthandler.cpp
                              -------------------------
  begin                : June 29, 2007
  copyright            : (C) 2007 by Marco Hugentobler
                         (C) 2014 by Alessandro Pasotti
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
                         a dot pasotti at itopen dot it
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
#include "qgsrequesthandler.h"
#if QT_VERSION < 0x050000
#include "qgsftptransaction.h"
#include "qgshttptransaction.h"
#endif
#include "qgsmessagelog.h"
#include "qgsserverexception.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include <QBuffer>
#include <QByteArray>
#include <QDomDocument>
#include <QFile>
#include <QImage>
#include <QTextStream>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

QgsRequestHandler::QgsRequestHandler( QgsServerRequest& request, QgsServerResponse& response )
    : mExceptionRaised( false )
    , mRequest( request )
    , mResponse( response )
{
}

QgsRequestHandler::~QgsRequestHandler()
{
}

QMap<QString, QString> QgsRequestHandler::parameterMap() const
{
  return mRequest.parameters();
}

void QgsRequestHandler::setHttpResponse( const QByteArray& ba, const QString &format )
{
  QgsMessageLog::logMessage( QStringLiteral( "Checking byte array is ok to set..." ) );

  if ( ba.size() < 1 )
  {
    return;
  }

  QgsMessageLog::logMessage( QStringLiteral( "Byte array looks good, setting response..." ) );
  appendBody( ba );
  setInfoFormat( format );
}

bool QgsRequestHandler::exceptionRaised() const
{
  return mExceptionRaised;
}

void QgsRequestHandler::setHeader( const QString &name, const QString &value )
{
  mResponse.setHeader( name, value );
}

void QgsRequestHandler::clear()
{
  mResponse.clear();
}

void QgsRequestHandler::removeHeader( const QString &name )
{
  mResponse.clearHeader( name );
}

QString QgsRequestHandler::getHeader( const QString& name ) const
{
  return mResponse.getHeader( name );
}

QList<QString> QgsRequestHandler::headerKeys() const
{
  return mResponse.headerKeys();
}

bool QgsRequestHandler::headersSent() const
{
  return mResponse.headersSent();
}

void QgsRequestHandler::appendBody( const QByteArray &body )
{
  mResponse.write( body );
}

void QgsRequestHandler::setInfoFormat( const QString &format )
{
  mInfoFormat = format;

  // Update header
  QString fmt = mInfoFormat;
  if ( mInfoFormat.startsWith( QLatin1String( "text/" ) ) || mInfoFormat.startsWith( QLatin1String( "application/vnd.ogc.gml" ) ) )
  {
    fmt.append( "; charset=utf-8" );
  }
  setHeader( QStringLiteral( "Content-Type" ), fmt );

}

void QgsRequestHandler::sendResponse()
{
  // Send data to output
  mResponse.flush();
}



QString QgsRequestHandler::formatToMimeType( const QString& format ) const
{
  if ( format.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 )
  {
    return QStringLiteral( "image/png" );
  }
  else if ( format.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0 )
  {
    return QStringLiteral( "image/jpeg" );
  }
  else if ( format.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
  {
    return QStringLiteral( "image/svg+xml" );
  }
  else if ( format.compare( QLatin1String( "pdf" ), Qt::CaseInsensitive ) == 0 )
  {
    return QStringLiteral( "application/pdf" );
  }
  return format;
}

void QgsRequestHandler::setGetCapabilitiesResponse( const QDomDocument& doc )
{
  QByteArray ba = doc.toByteArray();
  setHttpResponse( ba, QStringLiteral( "text/xml" ) );
}

void QgsRequestHandler::setServiceException( const QgsServerException& ex )
{
  // Safety measure to avoid potential leaks if called repeatedly
  mExceptionRaised = true;
  mResponse.write( ex );
}

void QgsRequestHandler::setGetCoverageResponse( QByteArray* ba )
{
  if ( ba )
  {
    setHttpResponse( *ba, QStringLiteral( "image/tiff" ) );
  }
}

void QgsRequestHandler::setupParameters()
{
  const QgsServerRequest::Parameters parameters = mRequest.parameters();

  // SLD

  QString value = parameters.value( QStringLiteral( "SLD" ) );
  if ( !value.isEmpty() )
  {
    // XXX Why keeping this ????
#if QT_VERSION < 0x050000
    QByteArray fileContents;
    if ( value.startsWith( "http", Qt::CaseInsensitive ) )
    {
      QgsHttpTransaction http( value );
      if ( !http.getSynchronously( fileContents ) )
      {
        fileContents.clear();
      }
    }
    else if ( value.startsWith( "ftp", Qt::CaseInsensitive ) )
    {
      Q_NOWARN_DEPRECATED_PUSH;
      QgsFtpTransaction ftp;
      if ( !ftp.get( value, fileContents ) )
      {
        fileContents.clear();
      }
      value = QUrl::fromPercentEncoding( fileContents );
      Q_NOWARN_DEPRECATED_POP;
    }

    if fileContents.size() > 0 )
    {
      mRequest.setParameter( QStringLiteral( "SLD" ),  QUrl::fromPercentEncoding( fileContents ) );
    }
#else
    QgsMessageLog::logMessage( QStringLiteral( "http and ftp methods not supported with Qt5." ) );
#endif

  }

  // SLD_BODY
  value = parameters.value( QStringLiteral( "SLD_BODY" ) );
  if ( ! value.isEmpty() )
  {
    mRequest.setParameter( QStringLiteral( "SLD" ), value );
  }

  //feature info format?
  QString infoFormat = parameters.value( QStringLiteral( "INFO_FORMAT" ) );
  if ( !infoFormat.isEmpty() )
  {
    mFormat = infoFormat;
  }
  else //capabilities format or GetMap format
  {
    mFormatString = parameters.value( QStringLiteral( "FORMAT" ) );
    QString formatString = mFormatString;
    if ( !formatString.isEmpty() )
    {
      QgsMessageLog::logMessage( QStringLiteral( "formatString is: %1" ).arg( formatString ) );

      //remove the image/ in front of the format
      if ( formatString.contains( QLatin1String( "image/png" ), Qt::CaseInsensitive ) || formatString.compare( QLatin1String( "png" ), Qt::CaseInsensitive ) == 0 )
      {
        formatString = QStringLiteral( "PNG" );
      }
      else if ( formatString.contains( QLatin1String( "image/jpeg" ), Qt::CaseInsensitive ) || formatString.contains( QLatin1String( "image/jpg" ), Qt::CaseInsensitive )
                || formatString.compare( QLatin1String( "jpg" ), Qt::CaseInsensitive ) == 0 )
      {
        formatString = QStringLiteral( "JPG" );
      }
      else if ( formatString.compare( QLatin1String( "svg" ), Qt::CaseInsensitive ) == 0 )
      {
        formatString = QStringLiteral( "SVG" );
      }
      else if ( formatString.contains( QLatin1String( "pdf" ), Qt::CaseInsensitive ) )
      {
        formatString = QStringLiteral( "PDF" );
      }

      mFormat = formatString;
    }
  }

}

void QgsRequestHandler::parseInput()
{
  if ( mRequest.method() == QgsServerRequest::PostMethod )
  {
    QString inputString( mRequest.data() );

    QDomDocument doc;
    QString errorMsg;
    int line = -1;
    int column = -1;
    if ( !doc.setContent( inputString, true, &errorMsg, &line, &column ) )
    {
      // XXX Output error but continue processing request ?
      QgsMessageLog::logMessage( QStringLiteral( "Error parsing post data: at line %1, column %2: %3." )
                                 .arg( line ).arg( column ).arg( errorMsg ) );

      // Process input string as a simple query text

      typedef QPair<QString, QString> pair_t;
      QUrlQuery query( inputString );
      QList<pair_t> items = query.queryItems();
      Q_FOREACH ( const pair_t& pair, items )
      {
        mRequest.setParameter( pair.first.toUpper(), pair.second );
      }
      setupParameters();
    }
    else
    {
      // we have an XML document

      setupParameters();

      QDomElement docElem = doc.documentElement();
      if ( docElem.hasAttribute( QStringLiteral( "version" ) ) )
      {
        mRequest.setParameter( QStringLiteral( "VERSION" ), docElem.attribute( QStringLiteral( "version" ) ) );
      }
      if ( docElem.hasAttribute( QStringLiteral( "service" ) ) )
      {
        mRequest.setParameter( QStringLiteral( "SERVICE" ), docElem.attribute( QStringLiteral( "service" ) ) );
      }
      mRequest.setParameter( QStringLiteral( "REQUEST" ), docElem.tagName() );
      mRequest.setParameter( QStringLiteral( "REQUEST_BODY" ), inputString );
    }
  }
  else
  {
    setupParameters();
  }

}

void QgsRequestHandler::setParameter( const QString &key, const QString &value )
{
  if ( !( key.isEmpty() || value.isEmpty() ) )
  {
    mRequest.setParameter( key, value );
  }
}


QString QgsRequestHandler::parameter( const QString &key ) const
{
  return mRequest.getParameter( key );
}

void QgsRequestHandler::removeParameter( const QString &key )
{
  mRequest.removeParameter( key );
}


