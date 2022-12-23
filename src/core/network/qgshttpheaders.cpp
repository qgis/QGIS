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
#include "qgsdatasourceuri.h"
#include <QDir>
#include <QNetworkRequest>
#include <QUrlQuery>
#include <QDomElement>

//
// QgsHttpHeaders
//

const QString QgsHttpHeaders::PATH_PREFIX = "http-header/";
const QString QgsHttpHeaders::PARAM_PREFIX = "http-header:";
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

QgsHttpHeaders::QgsHttpHeaders( const QDomElement &element )
{
  setFromDomElement( element );
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

bool QgsHttpHeaders::updateUrlQuery( QUrlQuery &uri ) const
{
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    uri.addQueryItem( QgsHttpHeaders::PARAM_PREFIX + ite.key().toUtf8(), ite.value().toString().toUtf8() );
  }
  return true;
}

bool QgsHttpHeaders::updateSettings( QgsSettings &settings, const QString &key ) const
{
  QString keyFixed = sanitizeKey( key );
  if ( !keyFixed.isEmpty() )
    keyFixed = keyFixed + "/";

  QString keyHH = keyFixed + QgsHttpHeaders::PATH_PREFIX;
  settings.remove( keyHH ); // cleanup
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    settings.setValue( keyHH  + ite.key(), ite.value() );
  }

  if ( !mHeaders[QgsHttpHeaders::KEY_REFERER].toString().isEmpty() && settings.contains( keyFixed + QgsHttpHeaders::KEY_REFERER ) ) // backward comptibility
  {
    settings.setValue( keyFixed + QgsHttpHeaders::KEY_REFERER, mHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  }

// TODO REMOVE!
#if 0
  QgsLogger::debug( QString( "updateSettings key: %1" ).arg( keyFixed ) );
  for ( auto k : settings.allKeys() )
    if ( k.startsWith( keyFixed ) )
      QgsLogger::debug( QString( "updateSettings in settings: %1=%2" ).arg( k, settings.value( k ).toString() ) );
#endif
  return true;
}

bool QgsHttpHeaders::updateMap( QVariantMap &map ) const
{
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    map.insert( QgsHttpHeaders::PARAM_PREFIX + ite.key().toUtf8(), ite.value().toString() );
  }

  if ( mHeaders.contains( QgsHttpHeaders::KEY_REFERER ) )
  {
    map.insert( QgsHttpHeaders::KEY_REFERER, mHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  }

  return true;
}

bool QgsHttpHeaders::updateDomElement( QDomElement &el ) const
{
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    el.setAttribute( QgsHttpHeaders::PARAM_PREFIX + ite.key().toUtf8(), ite.value().toString() );
  }

  if ( mHeaders.contains( QgsHttpHeaders::KEY_REFERER ) )
  {
    el.setAttribute( QgsHttpHeaders::KEY_REFERER, mHeaders[QgsHttpHeaders::KEY_REFERER].toString() );
  }

  return true;
}




void QgsHttpHeaders::setFromSettings( const QgsSettings &settings, const QString &key )
{
  QString keyFixed = sanitizeKey( key );
  if ( !keyFixed.isEmpty() )
    keyFixed = keyFixed + "/";
  QString keyHH = keyFixed + QgsHttpHeaders::PATH_PREFIX;

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

void QgsHttpHeaders::setFromUrlQuery( const QUrlQuery &uri )
{
  const auto constQueryItems = uri.queryItems( QUrl::ComponentFormattingOption::FullyDecoded );
  for ( const QPair<QString, QString> &item : constQueryItems )
  {
    const QString &key = item.first;
    if ( key.startsWith( QgsHttpHeaders::PARAM_PREFIX ) )
    {
      QString name = key.right( key.size() - QgsHttpHeaders::PARAM_PREFIX.size() );
      mHeaders[sanitizeKey( name )] = item.second;
    }
  }
}

void QgsHttpHeaders::setFromMap( const QVariantMap &map )
{
  for ( auto ite = map.keyBegin(); ite != map.keyEnd(); ++ite )
  {
    QString key = *ite;
    if ( key.startsWith( QgsHttpHeaders::PARAM_PREFIX ) )
    {
      QString name = key.right( key.size() - QgsHttpHeaders::PARAM_PREFIX.size() );
      mHeaders[sanitizeKey( name )] = map [key].toString();
    }
  }

  if ( map.contains( QgsHttpHeaders::KEY_REFERER ) ) // backward comptibility
  {
    mHeaders[QgsHttpHeaders::KEY_REFERER] = map [QgsHttpHeaders::KEY_REFERER].toString();
  }
}


void QgsHttpHeaders::setFromDomElement( const QDomElement &el )
{
  QDomNamedNodeMap attribs = el.attributes();

  for ( int i = 0; i < attribs.length(); i++ )
  {
    QDomNode item = attribs.item( i );
    QString key = item.nodeName();
    if ( key.startsWith( QgsHttpHeaders::PARAM_PREFIX ) )
    {
      QString name = key.right( key.size() - QgsHttpHeaders::PARAM_PREFIX.size() );
      mHeaders[sanitizeKey( name )] = item.nodeValue();
    }
  }

  if ( attribs.contains( QgsHttpHeaders::KEY_REFERER ) ) // backward comptibility
  {
    mHeaders[QgsHttpHeaders::KEY_REFERER] = attribs.namedItem( QgsHttpHeaders::KEY_REFERER ).nodeValue();
  }

}

QString QgsHttpHeaders::toSpacedString() const
{
  QString out;
  for ( auto ite = mHeaders.constBegin(); ite != mHeaders.constEnd(); ++ite )
  {
    out += QStringLiteral( " %1%2='%3'" ).arg( QgsHttpHeaders::PARAM_PREFIX, ite.key(), ite.value().toString() );
  }

  if ( !mHeaders [ QgsHttpHeaders::KEY_REFERER ].toString().isEmpty() )
    out += QStringLiteral( " %1='%2'" ).arg( QgsHttpHeaders::KEY_REFERER, mHeaders [QgsHttpHeaders::KEY_REFERER].toString() );

  return out;
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
