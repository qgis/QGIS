/***************************************************************************
                         QgsAbstractContentCache.cpp
                         -----------------
    begin                : December 2018
    copyright            : (C) 2018 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   *
 *  
 *        *
 *                                     *
 *                                                                         *
 ***************************************************************************/

#include "qgsabstractcontentcache.h"

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





