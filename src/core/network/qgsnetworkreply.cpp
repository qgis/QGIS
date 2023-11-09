/***************************************************************************
    qgsnetworkreply.cpp
    -------------------
    begin                : November 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsnetworkreply.h"
#include <QNetworkReply>
#include <QRegularExpression>
#include <QRegularExpressionMatch>

QgsNetworkReplyContent::QgsNetworkReplyContent( QNetworkReply *reply )
  : mError( reply->error() )
  , mErrorString( reply->errorString() )
  , mRawHeaderPairs( reply->rawHeaderPairs() )
  , mRequest( reply->request() )
{
  const int maxAttribute = static_cast< int >( QNetworkRequest::Http2DirectAttribute );
  for ( int i = 0; i <= maxAttribute; ++i )
  {
    if ( reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) ).isValid() )
      mAttributes[ static_cast< QNetworkRequest::Attribute>( i ) ] = reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) );
  }

  bool ok = false;
  const int requestId = reply->property( "requestId" ).toInt( &ok );
  if ( ok )
    mRequestId = requestId;
}

void QgsNetworkReplyContent::clear()
{
  *this = QgsNetworkReplyContent();
}

QVariant QgsNetworkReplyContent::attribute( QNetworkRequest::Attribute code ) const
{
  return mAttributes.value( code );
}

bool QgsNetworkReplyContent::hasRawHeader( const QByteArray &headerName ) const
{
  for ( auto &header : mRawHeaderPairs )
  {
    if ( ! QString::fromLocal8Bit( header.first ).compare( QString::fromLocal8Bit( headerName ), Qt::CaseInsensitive ) )
      return true;
  }
  return false;
}

QList<QByteArray> QgsNetworkReplyContent::rawHeaderList() const
{
  QList< QByteArray > res;
  res.reserve( mRawHeaderPairs.length() );
  for ( auto &header : mRawHeaderPairs )
  {
    res << header.first;
  }
  return res;
}

QByteArray QgsNetworkReplyContent::rawHeader( const QByteArray &headerName ) const
{
  for ( auto &header : mRawHeaderPairs )
  {
    if ( ! QString::fromLocal8Bit( header.first ).compare( QString::fromLocal8Bit( headerName ), Qt::CaseInsensitive ) )
      return header.second;
  }
  return QByteArray();
}

QString QgsNetworkReplyContent::extractFilenameFromContentDispositionHeader( QNetworkReply *reply )
{
  if ( !reply )
    return QString();

  return extractFileNameFromContentDispositionHeader( reply->header( QNetworkRequest::ContentDispositionHeader ).toString() );
}

QString QgsNetworkReplyContent::extractFileNameFromContentDispositionHeader( const QString &header )
{
  const thread_local QRegularExpression rx( QStringLiteral( R"""(filename[^;\n]*=\s*(UTF-\d['"]*)?((['"]).*?[.]$\2|[^;\n]*)?)""" ), QRegularExpression::PatternOption::CaseInsensitiveOption );

  QRegularExpressionMatchIterator i = rx.globalMatch( header, 0 );
  QString res;
  // we want the last match here, as that will have the UTF filename when present
  while ( i.hasNext() )
  {
    const QRegularExpressionMatch match = i.next();
    res = match.captured( 2 );
  }

  if ( res.startsWith( '"' ) )
  {
    res = res.mid( 1 );
    if ( res.endsWith( '"' ) )
      res.chop( 1 );
  }
  if ( !res.isEmpty() )
  {
    res = QUrl::fromPercentEncoding( res.toUtf8() );
  }

  return res;
}
