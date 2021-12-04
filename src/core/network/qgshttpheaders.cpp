/***************************************************************************
                       qgshttpheaders.cpp
  This class implements simple http header management.

                              -------------------
          begin                : 2021-09-09
          copyright            : (C) 2021 B. De Mezzo
          email                : benoit dot de dot mezzo at oslandia dot com

***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshttpheaders.h"

//
// QgsHttpHeaders
//

const QString QgsHttpHeaders::KEY_PREFIX = "http-header/";

QgsHttpHeaders::QgsHttpHeaders() = default;

QgsHttpHeaders::QgsHttpHeaders( const QMap<QString, QVariant> &headers )
  : mHeaders( headers )
{
  mHeaders.detach(); // clone like
}

QgsHttpHeaders::QgsHttpHeaders( const QgsSettings &settings, const QString &key )
{
  setFromSettings( settings, key );
}

QgsHttpHeaders::~QgsHttpHeaders() = default;

bool QgsHttpHeaders::updateNetworkRequest( QNetworkRequest &request ) const
{
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    request.setRawHeader( ite.key().toUtf8(), ite.value().toString().toUtf8() );
  }
  return true;
}

void QgsHttpHeaders::updateSettings( QgsSettings &settings, const QString &key ) const
{
  QString keyFixed = key;
  if ( !keyFixed.isEmpty() && !keyFixed.endsWith( "/" ) )
    keyFixed = keyFixed + "/";
  QString keyHH = keyFixed + QgsHttpHeaders::KEY_PREFIX;
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    settings.setValue( keyHH  + ite.key(), ite.value() );
  }

  if ( !mHeaders["referer"].toString().isEmpty() && settings.contains( keyFixed + "referer" ) ) // backward comptibility
  {
    settings.setValue( keyFixed + "referer", mHeaders["referer"].toString() );
  }
}

void QgsHttpHeaders::setFromSettings( const QgsSettings &settings, const QString &key )
{
  QString keyFixed = key;
  if ( !keyFixed.isEmpty() && !keyFixed.endsWith( "/" ) )
    keyFixed = keyFixed + "/";
  QString keyHH = keyFixed + QgsHttpHeaders::KEY_PREFIX;
  QStringList keys = settings.allKeys();
  for ( auto ite = keys.cbegin(); ite != keys.cend(); ++ite )
  {
    if ( ite->startsWith( keyHH ) )
    {
      QString name = ite->right( ite->size() - keyHH.size() );
      mHeaders.insert( name, settings.value( *ite ).toString() );
    }
  }
  if ( mHeaders["referer"].toString().isEmpty() ) // backward comptibility
  {
    mHeaders["referer"] = settings.value( keyFixed + "referer" ).toString(); // retrieve value from old location
  }
}


QVariant &QgsHttpHeaders::operator[]( const QString &key )
{
  return mHeaders[key];
}

const QVariant QgsHttpHeaders::operator[]( const QString &key ) const
{
  return mHeaders[key];
}

QgsHttpHeaders &QgsHttpHeaders::operator = ( const QMap<QString, QVariant> &headers )
{
  mHeaders = headers;
  return *this;
}

QList<QString> QgsHttpHeaders::keys() const
{
  return mHeaders.keys();
}
