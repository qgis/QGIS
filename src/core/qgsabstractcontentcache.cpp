/***************************************************************************
                         QgsAbstractContentCache.cpp
                         -----------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractcontentcache.h"
#include "moc_qgsabstractcontentcache.cpp"
#include "qgssetrequestinitiator_p.h"
#include <QRegularExpression>

//
// QgsAbstractContentCacheEntry
//

QgsAbstractContentCacheEntry::QgsAbstractContentCacheEntry( const QString &path )
  : path( path )
{
}

//
// QgsAbstractContentCacheBase
//

QgsAbstractContentCacheBase::QgsAbstractContentCacheBase( QObject *parent )
  : QObject( parent )
{}


void QgsAbstractContentCacheBase::onRemoteContentFetched( const QString &, bool )
{

}

bool QgsAbstractContentCacheBase::parseBase64DataUrl( const QString &path, QString *mimeType, QString *data )
{
  const thread_local QRegularExpression sRx( QStringLiteral( "^data:(.*/.*?);?(?:base64)?,(.*)$" ) );
  const QRegularExpressionMatch base64Match = sRx.match( path );
  if ( !base64Match.hasMatch() )
    return false;

  if ( mimeType )
    *mimeType = base64Match.captured( 1 );
  if ( data )
    *data = base64Match.captured( 2 );
  return true;
}

bool QgsAbstractContentCacheBase::isBase64Data( const QString &path )
{
  return path.startsWith( QLatin1String( "base64:" ) )
         || parseBase64DataUrl( path );
}
