/***************************************************************************
                          qgsserverrequest.cpp

  Define ruquest class for getting request contents
  -------------------
  begin                : 2016-12-05
  copyright            : (C) 2016 by David Marteau
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

#include "qgsserverrequest.h"
#include <QUrlQuery>

QgsServerRequest::QgsServerRequest( const QString &url, Method method, const Headers &headers )
  : mUrl( url )
  , mMethod( method )
  , mHeaders( headers )
{

}

QgsServerRequest::QgsServerRequest( const QUrl &url, Method method, const Headers &headers )
  : mUrl( url )
  , mMethod( method )
  , mHeaders( headers )
{

}

QString QgsServerRequest::header( const QString &name ) const
{
  return mHeaders.value( name );
}


void QgsServerRequest::setHeader( const QString &name, const QString &value )
{
  mHeaders.insert( name, value );
}

QMap<QString, QString> QgsServerRequest::headers() const
{
  return mHeaders;
}


void QgsServerRequest::removeHeader( const QString &name )
{
  mHeaders.remove( name );
}

QUrl QgsServerRequest::url() const
{
  return mUrl;
}

QgsServerRequest::Method QgsServerRequest::method() const
{
  return mMethod;
}

QMap<QString, QString> QgsServerRequest::parameters() const
{
  // Lazy build of the parameter map
  if ( !mDecoded && mUrl.hasQuery() )
  {
    typedef QPair<QString, QString> pair_t;

    QUrlQuery query( mUrl );
    QList<pair_t> items = query.queryItems( QUrl::FullyDecoded );
    Q_FOREACH ( const pair_t &pair, items )
    {
      // prepare the value
      QString value = pair.second;
      value.replace( "+", " " );

      mParams.insert( pair.first.toUpper(), value );
    }
    mDecoded = true;
  }
  return mParams;
}

QByteArray QgsServerRequest::data() const
{
  return QByteArray();
}

void QgsServerRequest::setParameter( const QString &key, const QString &value )
{
  parameters();
  mParams.insert( key, value );
}

QString QgsServerRequest::parameter( const QString &key ) const
{
  parameters();
  return mParams.value( key );
}

void QgsServerRequest::removeParameter( const QString &key )
{
  parameters();
  mParams.remove( key );
}

void QgsServerRequest::setUrl( const QUrl &url )
{
  mUrl = url;
  mDecoded = false;
  mParams.clear();
}

void QgsServerRequest::setMethod( Method method )
{
  mMethod = method;
}


