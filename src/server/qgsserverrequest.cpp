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

QgsServerRequest::QgsServerRequest( const QString& url, Method method )
    : mUrl( url )
    , mMethod( method )
{

}

QgsServerRequest::QgsServerRequest( const QUrl& url, Method method )
    : mUrl( url )
    , mMethod( method )
{

}

//! destructor
QgsServerRequest::~QgsServerRequest()
{

}

QString QgsServerRequest::getHeader( const QString& name ) const
{
  Q_UNUSED( name );
  return QString();
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
  if ( mParams.isEmpty() && mUrl.hasQuery() )
  {
    typedef QPair<QString, QString> pair_t;

    QUrlQuery query( mUrl );
    QList<pair_t> items = query.queryItems();
    Q_FOREACH ( const pair_t& pair, items )
    {
      mParams.insert( pair.first.toUpper(), pair.second );
    }
  }
  return mParams;
}

QByteArray QgsServerRequest::data() const
{
  return QByteArray();
}


