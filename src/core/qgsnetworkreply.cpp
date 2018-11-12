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
  : mContent( reply->readAll() )
  , mError( reply->error() )
  , mErrorString( reply->errorString() )
  , mRawHeaderPairs( reply->rawHeaderPairs() )
{
  for ( int i = 0; i < QNetworkRequest::ResourceTypeAttribute; ++i )
  {
    if ( reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) ).isValid() )
      mAttributes[ static_cast< QNetworkRequest::Attribute>( i ) ] = reply->attribute( static_cast< QNetworkRequest::Attribute>( i ) );
  }
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
