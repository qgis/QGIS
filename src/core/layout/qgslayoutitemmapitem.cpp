/***************************************************************************
                             qgslayoutitemmapitem.cpp
                             -------------------
    begin                : October 2017
    copyright            : (C) 2017 by Nyall Dawson
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

#include "qgslayoutitemmapitem.h"
#include "qgslayoutitemmap.h"
#include <QUuid>

QgsLayoutItemMapItem::QgsLayoutItemMapItem( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutObject( map->layout() )
  , mName( name )
  , mMap( map )
  , mUuid( QUuid::createUuid().toString() )
  , mEnabled( true )
{

}

bool QgsLayoutItemMapItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext & ) const
{
  Q_UNUSED( document );
  element.setAttribute( QStringLiteral( "uuid" ), mUuid );
  element.setAttribute( QStringLiteral( "name" ), mName );
  element.setAttribute( QStringLiteral( "show" ), mEnabled );
  return true;
}

bool QgsLayoutItemMapItem::readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext & )
{
  Q_UNUSED( doc );
  mUuid = itemElem.attribute( QStringLiteral( "uuid" ) );
  mName = itemElem.attribute( QStringLiteral( "name" ) );
  mEnabled = ( itemElem.attribute( QStringLiteral( "show" ), QStringLiteral( "0" ) ) != QLatin1String( "0" ) );
  return true;
}

void QgsLayoutItemMapItem::finalizeRestoreFromXml()
{
}

void QgsLayoutItemMapItem::setMap( QgsLayoutItemMap *map )
{
  mMap = map;
}

const QgsLayoutItemMap *QgsLayoutItemMapItem::map() const
{
  return mMap;
}

void QgsLayoutItemMapItem::setName( const QString &name )
{
  mName = name;
}

QString QgsLayoutItemMapItem::name() const
{
  return mName;
}

void QgsLayoutItemMapItem::setEnabled( const bool enabled )
{
  mEnabled = enabled;
}

bool QgsLayoutItemMapItem::enabled() const
{
  return mEnabled;
}

bool QgsLayoutItemMapItem::usesAdvancedEffects() const
{
  return false;
}

//
// QgsLayoutItemMapItemStack
//

QgsLayoutItemMapItemStack::QgsLayoutItemMapItemStack( QgsLayoutItemMap *map )
  : mMap( map )
{

}

QgsLayoutItemMapItemStack::~QgsLayoutItemMapItemStack()
{
  removeItems();
}

void QgsLayoutItemMapItemStack::addItem( QgsLayoutItemMapItem *item )
{
  mItems.append( item );
}

void QgsLayoutItemMapItemStack::removeItem( const QString &itemId )
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

void QgsLayoutItemMapItemStack::moveItemUp( const QString &itemId )
{
  QgsLayoutItemMapItem *targetItem = item( itemId );
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

void QgsLayoutItemMapItemStack::moveItemDown( const QString &itemId )
{
  QgsLayoutItemMapItem *targetItem = item( itemId );
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

QgsLayoutItemMapItem *QgsLayoutItemMapItemStack::item( const QString &itemId ) const
{
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( item->id() == itemId )
    {
      return item;
    }
  }

  return nullptr;
}

QgsLayoutItemMapItem *QgsLayoutItemMapItemStack::item( const int index ) const
{
  if ( index < mItems.length() )
  {
    return mItems.at( index );
  }

  return nullptr;
}

QgsLayoutItemMapItem &QgsLayoutItemMapItemStack::operator[]( int idx )
{
  return *mItems[idx];
}

QList<QgsLayoutItemMapItem *> QgsLayoutItemMapItemStack::asList() const
{
  QList< QgsLayoutItemMapItem * > list;
  list.reserve( mItems.size() );
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    list.append( item );
  }
  return list;
}

bool QgsLayoutItemMapItemStack::writeXml( QDomElement &elem, QDomDocument &doc, const QgsReadWriteContext &context ) const
{
  //write item stack
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    item->writeXml( elem, doc, context );
  }

  return true;
}

void QgsLayoutItemMapItemStack::finalizeRestoreFromXml()
{
  for ( QgsLayoutItemMapItem *item : qgis::as_const( mItems ) )
  {
    item->finalizeRestoreFromXml();
  }
}

void QgsLayoutItemMapItemStack::drawItems( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }

  for ( QgsLayoutItemMapItem *item : qgis::as_const( mItems ) )
  {
    item->draw( painter );
  }
}

bool QgsLayoutItemMapItemStack::containsAdvancedEffects() const
{
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( item->enabled() && item->usesAdvancedEffects() )
    {
      return true;
    }
  }
  return false;
}

void QgsLayoutItemMapItemStack::removeItems()
{
  qDeleteAll( mItems );
  mItems.clear();
}



