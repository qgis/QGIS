/***************************************************************************
                              qgsservervexception.h
                              -------------------
  begin                : January 11, 2017
  copyright            : (C) 2017 David Marteau
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

#include "qgsserverexception.h"

#include <QDomDocument>

// QgsServerException
QgsServerException::QgsServerException( const QString &message, int responseCode )
  : QgsException( message )
  , mResponseCode( responseCode )
{

}

QByteArray QgsServerException::formatResponse( QString &responseFormat ) const
{
  QDomDocument doc;
  const QDomNode header = doc.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
  doc.appendChild( header );

  QDomElement root = doc.createElement( QStringLiteral( "ServerException" ) );
  doc.appendChild( root );
  root.appendChild( doc.createTextNode( what() ) );

  responseFormat = QStringLiteral( "text/xml; charset=utf-8" );
  return doc.toByteArray();
}


// QgsOgcServiceException
QgsOgcServiceException:: QgsOgcServiceException( const QString &code, const QString &message, const QString &locator,
    int responseCode, const QString &version )
  : QgsServerException( message, responseCode )
  , mCode( code )
  , mMessage( message )
  , mLocator( locator )
  , mVersion( version )
{
}

QByteArray QgsOgcServiceException::formatResponse( QString &responseFormat ) const
{
  QDomDocument doc;
  const QDomNode header = doc.createProcessingInstruction( QStringLiteral( "xml" ), QStringLiteral( "version=\"1.0\" encoding=\"UTF-8\"" ) );
  doc.appendChild( header );

  QDomElement root = doc.createElement( QStringLiteral( "ServiceExceptionReport" ) );
  root.setAttribute( QStringLiteral( "version" ), mVersion );
  root.setAttribute( QStringLiteral( "xmlns" ), QStringLiteral( "http://www.opengis.net/ogc" ) );
  doc.appendChild( root );

  QDomElement elem = doc.createElement( QStringLiteral( "ServiceException" ) );
  elem.setAttribute( QStringLiteral( "code" ), mCode );
  elem.appendChild( doc.createTextNode( mMessage ) );
  root.appendChild( elem );

  if ( ! mLocator.isEmpty() )
  {
    elem.setAttribute( QStringLiteral( "locator" ), mLocator );
  }

  responseFormat = QStringLiteral( "text/xml; charset=utf-8" );
  return doc.toByteArray();
}



