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
#include <QDir>

//
// QgsHttpHeaders
//

const QString QgsHttpHeaders::KEY_PREFIX = "http-header/";
const QString QgsHttpHeaders::KEY_REFERER = "referer";

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

QgsHttpHeaders::QgsHttpHeaders( const QString &key )
{
  setFromSettings( QgsSettings(), key );
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

bool QgsHttpHeaders::updateDataSourceUri( QgsDataSourceUri &uri ) const
{
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    uri.setParam( ite.key().toUtf8(), ite.value().toString().toUtf8() );
  }
  return true;
}

void QgsHttpHeaders::updateSettings( QgsSettings &settings, const QString &key ) const
{
  QString keyFixed = sanitizeKey( key );
  if ( !keyFixed.isEmpty() )
    keyFixed = keyFixed + "/";

  QString keyHH = keyFixed + QgsHttpHeaders::KEY_PREFIX;
  settings.remove( keyHH ); // cleanup
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    settings.setValue( keyHH  + ite.key(), ite.value() );
  }

  if ( !mHeaders[QgsHttpHeaders::KEY_REFERER].toString().isEmpty() && settings.contains( keyFixed + QgsHttpHeaders::KEY_REFERER ) ) // backward comptibility
  {
    settings.setValue( keyFixed + QgsHttpHeaders::KEY_REFERER, mHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  }

#if 0
  QgsLogger::debug( QString( "updateSettings key: %1" ).arg( keyFixed ) );
  for ( auto k : settings.allKeys() )
    if ( k.startsWith( keyFixed ) )
      QgsLogger::debug( QString( "updateSettings in settings: %1=%2" ).arg( k, settings.value( k ).toString() ) );
#endif
}

void QgsHttpHeaders::setFromSettings( const QgsSettings &settings, const QString &key )
{
  QString keyFixed = sanitizeKey( key );
  if ( !keyFixed.isEmpty() )
    keyFixed = keyFixed + "/";
  QString keyHH = keyFixed + QgsHttpHeaders::KEY_PREFIX;

#if 0
  QgsLogger::debug( QString( "setFromSettings key: %1" ).arg( keyFixed ) );
  for ( auto k : settings.allKeys() )
    if ( k.startsWith( keyFixed ) )
      QgsLogger::debug( QString( "setFromSettings called: %1=%2" ).arg( k, settings.value( k ).toString() ) );
  QgsLogger::debug( QString( "setFromSettings keyHH: %1" ).arg( keyHH ) );
#endif

  QStringList keys = settings.allKeys();
  for ( auto ite = keys.cbegin(); ite != keys.cend(); ++ite )
  {
    if ( ite->startsWith( keyHH ) )
    {
      QString name = ite->right( ite->size() - keyHH.size() );
      mHeaders.insert( name, settings.value( *ite ).toString() );
    }
  }
  if ( settings.contains( keyFixed + QgsHttpHeaders::KEY_REFERER ) ) // backward comptibility
  {
    mHeaders[QgsHttpHeaders::KEY_REFERER] = settings.value( keyFixed + QgsHttpHeaders::KEY_REFERER ).toString(); // retrieve value from old location
  }

#if 0
  for ( auto k : mHeaders.keys() )
    QgsLogger::debug( QString( "setFromSettings mHeaders[%1]=%2" ).arg( k, mHeaders[k].toString() ) );
#endif
}

// To clean the path
QString QgsHttpHeaders::sanitizeKey( const QString &key ) const
{
  QString out = QDir::cleanPath( key );
  return out;
}


QVariant &QgsHttpHeaders::operator[]( const QString &key )
{
  return mHeaders[sanitizeKey( key )];
}

const QVariant QgsHttpHeaders::operator[]( const QString &key ) const
{
  return mHeaders[sanitizeKey( key )];
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
