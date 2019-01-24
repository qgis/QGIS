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

QgsNetworkReplyContent::QgsNetworkReplyContent( QNetworkReply *reply )
  : mError( reply->error() )
  , mErrorString( reply->errorString() )
  , mRawHeaderPairs( reply->rawHeaderPairs() )
  , mRequest( reply->request() )
{
  int maxAttribute = static_cast< int >( QNetworkRequest::RedirectPolicyAttribute );
#if QT_VERSION >= QT_VERSION_CHECK( 5, 11, 0 )
  maxAttribute = static_cast< int >( QNetworkRequest::Http2DirectAttribute );
#endif
  for ( int i = 0; i <= maxAttribute; ++i )
  {
    if ( reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) ).isValid() )
      mAttributes[ static_cast< QNetworkRequest::Attribute>( i ) ] = reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) );
  }

  bool ok = false;
  int requestId = reply->property( "requestId" ).toInt( &ok );
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
    if ( header.first == headerName )
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
    if ( header.first == headerName )
      return header.second;
  }
  return QByteArray();
}
