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
#include "qgsmessagelog.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"
#include <QByteArray>
#include <QDomDocument>
#include <QUrl>
#include <QUrlQuery>

QgsRequestHandler::QgsRequestHandler( QgsServerRequest &request, QgsServerResponse &response )
  : mExceptionRaised( false )
  , mRequest( request )
  , mResponse( response )
{
}

QMap<QString, QString> QgsRequestHandler::parameterMap() const
{
  return mRequest.parameters();
}

bool QgsRequestHandler::exceptionRaised() const
{
  return mExceptionRaised;
}

void QgsRequestHandler::setResponseHeader( const QString &name, const QString &value )
{
  mResponse.setHeader( name, value );
}

void QgsRequestHandler::clear()
{
  mResponse.clear();
}

void QgsRequestHandler::removeResponseHeader( const QString &name )
{
  mResponse.removeHeader( name );
}

QString QgsRequestHandler::responseHeader( const QString &name ) const
{
  return mResponse.header( name );
}

QMap<QString, QString> QgsRequestHandler::responseHeaders() const
{
  return mResponse.headers();
}

void QgsRequestHandler::setRequestHeader( const QString &name, const QString &value )
{
  mRequest.setHeader( name, value );
}

void QgsRequestHandler::removeRequestHeader( const QString &name )
{
  mRequest.removeHeader( name );
}

QString QgsRequestHandler::requestHeader( const QString &name ) const
{
  return mRequest.header( name );
}


QMap<QString, QString> QgsRequestHandler::requestHeaders() const
{
  return mRequest.headers();
}


bool QgsRequestHandler::headersSent() const
{
  return mResponse.headersSent();
}

void QgsRequestHandler::appendBody( const QByteArray &body )
{
  mResponse.write( body );
}

void QgsRequestHandler::clearBody()
{
  mResponse.truncate();
}

QByteArray QgsRequestHandler::body() const
{
  return mResponse.data();
}

QByteArray QgsRequestHandler::data() const
{
  return mRequest.data();
}

QString QgsRequestHandler::url() const
{
  return mRequest.url().toString();
}

QString QgsRequestHandler::path() const
{
  return mRequest.url().path();
}

void QgsRequestHandler::setStatusCode( int code )
{
  mResponse.setStatusCode( code );
}

int QgsRequestHandler::statusCode() const
{
  return mResponse.statusCode();
}

void QgsRequestHandler::sendResponse()
{
  // Send data to output
  mResponse.flush();
}

void QgsRequestHandler::setServiceException( const QgsServerException &ex )
{
  // Safety measure to avoid potential leaks if called repeatedly
  mExceptionRaised = true;
  mResponse.write( ex );
}

void QgsRequestHandler::setupParameters()
{
  const QgsServerRequest::Parameters parameters = mRequest.parameters();

  //feature info format?
  const QString infoFormat = parameters.value( QStringLiteral( "INFO_FORMAT" ) );
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
  if ( mRequest.method() == QgsServerRequest::PostMethod ||
       mRequest.method() == QgsServerRequest::PutMethod ||
       mRequest.method() == QgsServerRequest::PatchMethod )
  {
    if ( mRequest.header( QStringLiteral( "Content-Type" ) ).contains( QStringLiteral( "json" ) ) )
    {
      setupParameters();
    }
    else
    {
      QString inputString( mRequest.data() );
      QDomDocument doc;
      QString errorMsg;
      int line = -1;
      int column = -1;
      if ( !doc.setContent( inputString, true, &errorMsg, &line, &column ) )
      {
        // XXX Output error but continue processing request ?
        QgsMessageLog::logMessage( QStringLiteral( "Warning: error parsing post data as XML: at line %1, column %2: %3. Assuming urlencoded query string sent in the post body." )
                                   .arg( line ).arg( column ).arg( errorMsg ) );

        // Process input string as a simple query text

        typedef QPair<QString, QString> pair_t;
        const QUrlQuery query( inputString );
        const QList<pair_t> items = query.queryItems();
        for ( const pair_t &pair : items )
        {
          mRequest.setParameter( pair.first, pair.second );
        }
        setupParameters();
      }
      else
      {
        // we have an XML document

        setupParameters();

        const QDomElement docElem = doc.documentElement();
        // the document element tag name is the request
        mRequest.setParameter( QStringLiteral( "REQUEST" ), docElem.tagName() );
        // loop through the attributes which are the parameters
        // excepting the attributes started by xmlns or xsi
        const QDomNamedNodeMap map = docElem.attributes();
        for ( int i = 0 ; i < map.length() ; ++i )
        {
          if ( map.item( i ).isNull() )
            continue;

          const QDomNode attrNode = map.item( i );
          const QDomAttr attr = attrNode.toAttr();
          if ( attr.isNull() )
            continue;

          const QString attrName = attr.name();
          if ( attrName.startsWith( "xmlns" ) || attrName.startsWith( "xsi:" ) )
            continue;

          mRequest.setParameter( attrName.toUpper(), attr.value() );
        }
        mRequest.setParameter( QStringLiteral( "REQUEST_BODY" ), inputString.replace( '+', QLatin1String( "%2B" ) ) );
      }
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
    // Warn for potential breaking change if plugin set the MAP parameter
    // expecting changing the config file path, see PR #9773
    if ( key.compare( QLatin1String( "MAP" ), Qt::CaseInsensitive ) == 0 )
    {
      QgsMessageLog::logMessage( QStringLiteral( "Changing the 'MAP' parameter will have no effect on config path: use QgsSerververInterface::setConfigFilePath instead" ),
                                 "Server", Qgis::MessageLevel::Warning );
    }
    mRequest.setParameter( key, value );
  }
}


QString QgsRequestHandler::parameter( const QString &key ) const
{
  return mRequest.parameter( key );
}

void QgsRequestHandler::removeParameter( const QString &key )
{
  mRequest.removeParameter( key );
}
