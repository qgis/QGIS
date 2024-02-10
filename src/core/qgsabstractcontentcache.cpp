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
#include "qgssetrequestinitiator_p.h"

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
