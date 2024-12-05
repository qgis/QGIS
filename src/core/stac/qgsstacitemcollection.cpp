/***************************************************************************
    qgsstacitemcollection.cpp
    ---------------------
    begin                : August 2024
    copyright            : (C) 2024 by Stefanos Natsis
    email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsstacitemcollection.h"

QgsStacItemCollection::QgsStacItemCollection( const QVector< QgsStacItem * > &items, const QVector< QgsStacLink > &links, int numberMatched )
  : mItems( items )
  , mLinks( links )
  , mNumberMatched( numberMatched )
{
  for ( const QgsStacLink &link : mLinks )
  {
    if ( link.relation() == QLatin1String( "self" ) ||
         link.relation() == QLatin1String( "root" ) ||
         link.relation() == QLatin1String( "parent" ) ||
         link.relation() == QLatin1String( "collection" ) ||
         link.relation() == QLatin1String( "next" ) )
      mUrls.insert( link.relation(), link.href() );
  }
}

QgsStacItemCollection::~QgsStacItemCollection()
{
  qDeleteAll( mItems );
}

QVector< QgsStacItem * > QgsStacItemCollection::items() const
{
  return mItems;
}

QVector< QgsStacItem * > QgsStacItemCollection::takeItems()
{
  QVector< QgsStacItem * > items = mItems;
  mItems.clear(); // detach
  return items;
}

QUrl QgsStacItemCollection::url() const
{
  return QUrl( mUrls.value( QStringLiteral( "self" ), QString() ) );
}

QUrl QgsStacItemCollection::rootUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "root" ), QString() ) );
}

QUrl QgsStacItemCollection::parentUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "parent" ), QString() ) );
}

QUrl QgsStacItemCollection::collectionUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "collection" ), QString() ) );
}

QUrl QgsStacItemCollection::nextUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "next" ), QString() ) );
}

int QgsStacItemCollection::numberReturned() const
{
  return mItems.size();
}

int QgsStacItemCollection::numberMatched() const
{
  return mNumberMatched;
}
