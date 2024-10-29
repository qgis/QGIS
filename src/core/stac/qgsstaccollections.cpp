/***************************************************************************
    qgsstaccollections.cpp
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

#include "qgsstaccollections.h"

QgsStacCollections::QgsStacCollections( const QVector< QgsStacCollection * > collections, const QVector< QgsStacLink > links, int numberMatched )
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


QgsStacCollections::~QgsStacCollections()
{
  qDeleteAll( mCollections );
}

QVector<QgsStacCollection *> QgsStacCollections::collections() const
{
  return mCollections;
}

QVector<QgsStacCollection *> QgsStacCollections::takeCollections()
{
  QVector< QgsStacCollection * > cols = mCollections;
  mCollections.clear(); // detach
  return cols;
}

int QgsStacCollections::numberReturned() const
{
  return mCollections.size();
}

int QgsStacCollections::numberMatched() const
{
  return mNumberMatched;
}

QUrl QgsStacCollections::url() const
{
  return QUrl( mUrls.value( QStringLiteral( "self" ), QString() ) );
}

QUrl QgsStacCollections::rootUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "root" ), QString() ) );
}

QUrl QgsStacCollections::nextUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "next" ), QString() ) );
}

QUrl QgsStacCollections::prevUrl() const
{
  return QUrl( mUrls.value( QStringLiteral( "prev" ), QString() ) );
}
