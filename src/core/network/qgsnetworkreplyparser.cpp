/***************************************************************************
      qgsnetworkreplyparser.cpp - Multipart QNetworkReply parser
                             -------------------
    begin                : 4 January, 2013
    copyright            : (C) 2013 by Radim Blazek
    email                : radim dot blazek at gmail.com

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsnetworkreplyparser.h"

#include <QNetworkReply>
#include <QObject>
#include <QRegularExpression>
#include <QString>
#include <QStringList>

QgsNetworkReplyParser::QgsNetworkReplyParser( QNetworkReply *reply )
  : mReply( reply )
  , mValid( false )
{
  if ( !mReply ) return;

  // Content type examples:
  //   multipart/mixed; boundary=wcs
  //   multipart/mixed; boundary="wcs"\n
  if ( !isMultipart( mReply ) )
  {
    // reply is not multipart, copy body and headers
    QMap<QByteArray, QByteArray> headers;
    const auto constRawHeaderList = mReply->rawHeaderList();
    for ( const QByteArray &h : constRawHeaderList )
    {
      headers.insert( h, mReply->rawHeader( h ) );
    }
    mHeaders.append( headers );
    mBodies.append( mReply->readAll() );
  }
  else // multipart
  {
    const QString contentType = mReply->header( QNetworkRequest::ContentTypeHeader ).toString();
    QgsDebugMsg( "contentType: " + contentType );

    const QRegularExpression re( ".*boundary=\"?([^\"]+)\"?\\s?", QRegularExpression::CaseInsensitiveOption );
    const QRegularExpressionMatch match = re.match( contentType );
    if ( !( match.capturedStart( 0 ) == 0 ) )
    {
      mError = tr( "Cannot find boundary in multipart content type" );
      return;
    }

    QString boundary = match.captured( 1 );
    QgsDebugMsg( QStringLiteral( "boundary = %1 size = %2" ).arg( boundary ).arg( boundary.size() ) );
    boundary = "--" + boundary;

    // Lines should be terminated by CRLF ("\r\n") but any new line combination may appear
    const QByteArray data = mReply->readAll();
    int from, to;
    from = data.indexOf( boundary.toLatin1(), 0 ) + boundary.length() + 1;
    //QVector<QByteArray> partHeaders;
    //QVector<QByteArray> partBodies;
    while ( true )
    {
      // 'to' is not really 'to', but index of the next byte after the end of part
      to = data.indexOf( boundary.toLatin1(), from );
      if ( to < 0 )
      {
        QgsDebugMsg( QStringLiteral( "No more boundaries, rest size = %1" ).arg( data.size() - from - 1 ) );
        // It may be end, last boundary is followed by '--'
        if ( data.size() - from - 1 == 2 && QString( data.mid( from, 2 ) ) == QLatin1String( "--" ) ) // end
        {
          break;
        }

        // It may happen that boundary is missing at the end (GeoServer)
        // in that case, take everything to the end
        if ( data.size() - from > 1 )
        {
          to = data.size(); // out of range OK
        }
        else
        {
          break;
        }
      }
      QgsDebugMsg( QStringLiteral( "part %1 - %2" ).arg( from ).arg( to ) );
      QByteArray part = data.mid( from, to - from );
      // Remove possible new line from beginning
      while ( !part.isEmpty() && ( part.at( 0 ) == '\r' || part.at( 0 ) == '\n' ) )
      {
        part.remove( 0, 1 );
      }
      // Split header and data (find empty new line)
      // New lines should be CRLF, but we support also CRLFCRLF, LFLF to find empty line
      int pos = 0; // body start
      while ( pos < part.size() - 1 )
      {
        if ( part.at( pos ) == '\n' && ( part.at( pos + 1 ) == '\n' || part.at( pos + 1 ) == '\r' ) )
        {
          if ( part.at( pos + 1 ) == '\r' ) pos++;
          pos += 2;
          break;
        }
        pos++;
      }
      // parse headers
      RawHeaderMap headersMap;
      const QByteArray headers = part.left( pos );
      QgsDebugMsg( "headers:\n" + headers );

      const QStringList headerRows = QString( headers ).split( QRegularExpression( "[\n\r]+" ) );
      const auto constHeaderRows = headerRows;
      for ( const QString &row : constHeaderRows )
      {
        QgsDebugMsg( "row = " + row );
        const QStringList kv = row.split( QStringLiteral( ": " ) );
        headersMap.insert( kv.value( 0 ).toLatin1(), kv.value( 1 ).toLatin1() );
      }
      mHeaders.append( headersMap );

      mBodies.append( part.mid( pos ) );

      from = to + boundary.length();
    }
  }
  mValid = true;
}

bool QgsNetworkReplyParser::isMultipart( QNetworkReply *reply )
{
  if ( !reply ) return false;

  const QString contentType = reply->header( QNetworkRequest::ContentTypeHeader ).toString();
  QgsDebugMsg( "contentType: " + contentType );

  // Multipart content type examples:
  //   multipart/mixed; boundary=wcs
  //   multipart/mixed; boundary="wcs"\n
  return contentType.startsWith( QLatin1String( "multipart/" ), Qt::CaseInsensitive );
}
