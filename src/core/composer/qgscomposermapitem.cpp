/***************************************************************************
                         qgscomposermapitem.cpp
                             -------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposermapitem.h"
#include "qgscomposermap.h"
#include <QUuid>

QgsComposerMapItem::QgsComposerMapItem( const QString &name, QgsComposerMap *map )
    : QgsComposerObject( map->composition() )
    , mName( name )
    , mComposerMap( map )
    , mUuid( QUuid::createUuid().toString() )
    , mEnabled( true )
{

}

QgsComposerMapItem::~QgsComposerMapItem()
{

}

bool QgsComposerMapItem::writeXML( QDomElement &elem, QDomDocument &doc ) const
{
  Q_UNUSED( doc );
  elem.setAttribute( "uuid", mUuid );
  elem.setAttribute( "name", mName );
  elem.setAttribute( "show", mEnabled );
  return true;
}

bool QgsComposerMapItem::readXML( const QDomElement &itemElem, const QDomDocument &doc )
{
  Q_UNUSED( doc );
  mUuid = itemElem.attribute( "uuid" );
  mName = itemElem.attribute( "name" );
  mEnabled = ( itemElem.attribute( "show", "0" ) != "0" );
  return true;
}

void QgsComposerMapItem::setComposerMap( QgsComposerMap *map )
{
  mComposerMap = map;
}

//
// QgsComposerMapItemStack
//

QgsComposerMapItemStack::QgsComposerMapItemStack( QgsComposerMap *map )
    : mComposerMap( map )
{

}

QgsComposerMapItemStack::~QgsComposerMapItemStack()
{
  removeItems();
}

void QgsComposerMapItemStack::addItem( QgsComposerMapItem *item )
{
  mItems.append( item );
}

void QgsComposerMapItemStack::removeItem( const QString &itemId )
{
  for ( int i = mItems.size() - 1; i >= 0; --i )
  {
    if ( mItems.at( i )->id() == itemId )
    {
      delete mItems.takeAt( i );
      return;
    }
  }
}

void QgsComposerMapItemStack::moveItemUp( const QString &itemId )
{
  QgsComposerMapItem* targetItem = item( itemId );
  if ( !targetItem )
  {
    return;
  }

  int index = mItems.indexOf( targetItem );
  if ( index >= mItems.size() - 1 )
  {
    return;
  }
  mItems.swap( index, index + 1 );
}

void QgsComposerMapItemStack::moveItemDown( const QString &itemId )
{
  QgsComposerMapItem* targetItem = item( itemId );
  if ( !targetItem )
  {
    return;
  }

  int index = mItems.indexOf( targetItem );
  if ( index < 1 )
  {
    return;
  }
  mItems.swap( index, index - 1 );
}

const QgsComposerMapItem *QgsComposerMapItemStack::constItem( const QString &itemId ) const
{
  QList< QgsComposerMapItem* >::const_iterator it = mItems.constBegin();
  for ( ; it != mItems.constEnd(); ++it )
  {
    if (( *it )->id() == itemId )
    {
      return ( *it );
    }
  }

  return nullptr;
}

QgsComposerMapItem *QgsComposerMapItemStack::item( const QString &itemId ) const
{
  QList< QgsComposerMapItem* >::const_iterator it = mItems.begin();
  for ( ; it != mItems.end(); ++it )
  {
    if (( *it )->id() == itemId )
    {
      return ( *it );
    }
  }

  return nullptr;
}

QgsComposerMapItem *QgsComposerMapItemStack::item( const int index ) const
{
  if ( index < mItems.length() )
  {
    return mItems.at( index );
  }

  return nullptr;
}

QgsComposerMapItem &QgsComposerMapItemStack::operator[]( int idx )
{
  return *mItems[idx];
}

QList<QgsComposerMapItem *> QgsComposerMapItemStack::asList() const
{
  QList< QgsComposerMapItem* > list;
  QList< QgsComposerMapItem* >::const_iterator it = mItems.begin();
  for ( ; it != mItems.end(); ++it )
  {
    list.append( *it );
  }
  return list;
}

bool QgsComposerMapItemStack::writeXML( QDomElement &elem, QDomDocument &doc ) const
{
  //write item stack
  QList< QgsComposerMapItem* >::const_iterator itemIt = mItems.constBegin();
  for ( ; itemIt != mItems.constEnd(); ++itemIt )
  {
    ( *itemIt )->writeXML( elem, doc );
  }

  return true;
}

void QgsComposerMapItemStack::drawItems( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }

  QList< QgsComposerMapItem* >::const_iterator itemIt = mItems.constBegin();
  for ( ; itemIt != mItems.constEnd(); ++itemIt )
  {
    ( *itemIt )->draw( painter );
  }
}

bool QgsComposerMapItemStack::containsAdvancedEffects() const
{
  QList< QgsComposerMapItem* >::const_iterator it = mItems.constBegin();
  for ( ; it != mItems.constEnd(); ++it )
  {
    if (( *it )->enabled() && ( *it )->usesAdvancedEffects() )
    {
      return true;
    }
  }
  return false;
}

void QgsComposerMapItemStack::removeItems()
{
  qDeleteAll( mItems );
  mItems.clear();
}



