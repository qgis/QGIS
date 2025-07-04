/***************************************************************************
    qgsstaccollectionlist.cpp
    ---------------------
    begin                : October 2024
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

#include "qgsstaccollectionlist.h"

QgsStacCollectionList::QgsStacCollectionList( const QVector< QgsStacCollection * > collections, const QVector< QgsStacLink > links, int numberMatched )
  : mCollections( collections )
  , mNumberMatched( numberMatched )
{
  for ( const QgsStacLink &link : links )
  {
    if ( link.relation() == QLatin1String( "self" ) ||
         link.relation() == QLatin1String( "root" ) ||
         link.relation() == QLatin1String( "next" ) ||
         link.relation() == QLatin1String( "prev" ) )
      mUrls.insert( link.relation(), link.href() );
  }
}


QgsStacCollectionList::~QgsStacCollectionList()
{
  qDeleteAll( mCollections );
}

QVector<QgsStacCollection *> QgsStacCollectionList::collections() const
{
  return mCollections;
}

QVector<QgsStacCollection *> QgsStacCollectionList::takeCollections()
{
  QVector< QgsStacCollection * > cols = mCollections;
  mCollections.clear(); // detach
  return cols;
}

int QgsStacCollectionList::numberReturned() const
{
  return mCollections.size();
}

int QgsStacCollectionList::numberMatched() const
{
  return mNumberMatched;
}

QUrl QgsStacCollectionList::url() const
{
  return QUrl( mUrls.value( QStringLiteral( "self" ), QString() ) );
}

QUrl QgsStacCollectionList::rootUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "root" ), QString() ) );
}

QUrl QgsStacCollectionList::nextUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "next" ), QString() ) );
}

QUrl QgsStacCollectionList::prevUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "prev" ), QString() ) );
}
