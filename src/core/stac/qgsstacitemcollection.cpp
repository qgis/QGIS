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
    if ( link.relation() == "self"_L1 ||
         link.relation() == "root"_L1 ||
         link.relation() == "parent"_L1 ||
         link.relation() == "collection"_L1 ||
         link.relation() == "next"_L1 )
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
  return QUrl( mUrls.value( u"self"_s, QString() ) );
}

QUrl QgsStacItemCollection::rootUrl() const
{
  return QUrl( mUrls.value( u"root"_s, QString() ) );
}

QUrl QgsStacItemCollection::parentUrl() const
{
  return QUrl( mUrls.value( u"parent"_s, QString() ) );
}

QUrl QgsStacItemCollection::collectionUrl() const
{
  return QUrl( mUrls.value( u"collection"_s, QString() ) );
}

QUrl QgsStacItemCollection::nextUrl() const
{
  return QUrl( mUrls.value( u"next"_s, QString() ) );
}

int QgsStacItemCollection::numberReturned() const
{
  return mItems.size();
}

int QgsStacItemCollection::numberMatched() const
{
  return mNumberMatched;
}
