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

#include "qgsrequesthandler.h"

#include "qgis.h"
#include "qgsmessagelog.h"
#include "qgsserverrequest.h"
#include "qgsserverresponse.h"

#include <QByteArray>
#include <QDomDocument>
#include <QUrl>
#include <QUrlQuery>

QgsRequestHandler::QgsRequestHandler( QgsServerRequest &request, QgsServerResponse &response )
  : mRequest( request )
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
  const QString infoFormat = parameters.value( u"INFO_FORMAT"_s );
  if ( !infoFormat.isEmpty() )
  {
    mFormat = infoFormat;
  }
  else //capabilities format or GetMap format
  {
    mFormatString = parameters.value( u"FORMAT"_s );
    QString formatString = mFormatString;
    if ( !formatString.isEmpty() )
    {
      //remove the image/ in front of the format
      if ( formatString.contains( "image/png"_L1, Qt::CaseInsensitive ) || formatString.compare( "png"_L1, Qt::CaseInsensitive ) == 0 )
      {
        formatString = u"PNG"_s;
      }
      else if ( formatString.contains( "image/jpeg"_L1, Qt::CaseInsensitive ) || formatString.contains( "image/jpg"_L1, Qt::CaseInsensitive )
                || formatString.compare( "jpg"_L1, Qt::CaseInsensitive ) == 0 )
      {
        formatString = u"JPG"_s;
      }
      else if ( formatString.compare( "svg"_L1, Qt::CaseInsensitive ) == 0 )
      {
        formatString = u"SVG"_s;
      }
      else if ( formatString.contains( "pdf"_L1, Qt::CaseInsensitive ) )
      {
        formatString = u"PDF"_s;
      }

      mFormat = formatString;
    }
  }
}

void QgsRequestHandler::parseInput()
{
  if ( mRequest.method() == QgsServerRequest::PostMethod || mRequest.method() == QgsServerRequest::PutMethod || mRequest.method() == QgsServerRequest::PatchMethod )
  {
    if ( mRequest.header( u"Content-Type"_s ).contains( u"json"_s ) )
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
        // Output Warning about POST without XML content
        QgsMessageLog::logMessage( u"Error parsing post data as XML: at line %1, column %2: %3. Assuming urlencoded query string sent in the post body."_s.arg( line ).arg( column ).arg( errorMsg ), u"Server"_s, Qgis::MessageLevel::Warning );

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
        mRequest.setParameter( u"REQUEST"_s, docElem.tagName() );
        // loop through the attributes which are the parameters
        // excepting the attributes started by xmlns or xsi
        const QDomNamedNodeMap map = docElem.attributes();
        for ( int i = 0; i < map.length(); ++i )
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
        mRequest.setParameter( u"REQUEST_BODY"_s, inputString.replace( '+', "%2B"_L1 ) );
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
    if ( key.compare( "MAP"_L1, Qt::CaseInsensitive ) == 0 )
    {
      QgsMessageLog::logMessage( u"Changing the 'MAP' parameter will have no effect on config path: use QgsSerververInterface::setConfigFilePath instead"_s, u"Server"_s, Qgis::MessageLevel::Warning );
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
