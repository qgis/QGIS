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

#include "qgslayout.h"
#include "qgslayoutitemmap.h"

#include <QUuid>

#include "moc_qgslayoutitemmapitem.cpp"

QgsLayoutItemMapItem::QgsLayoutItemMapItem( const QString &name, QgsLayoutItemMap *map )
  : QgsLayoutObject( map->layout() )
  , mName( name )
  , mMap( map )
  , mUuid( QUuid::createUuid().toString() )
{

}

bool QgsLayoutItemMapItem::writeXml( QDomElement &element, QDomDocument &document, const QgsReadWriteContext &context ) const
{
  Q_UNUSED( document )

  QgsLayoutObject::writeObjectPropertiesToElement( element, document, context );

  element.setAttribute( u"uuid"_s, mUuid );
  element.setAttribute( u"name"_s, mName );
  element.setAttribute( u"show"_s, mEnabled );
  element.setAttribute( u"position"_s, static_cast< int >( mStackingPosition ) );

  if ( mStackingLayer )
  {
    element.setAttribute( u"stackingLayer"_s, mStackingLayer.layerId );
    element.setAttribute( u"stackingLayerName"_s, mStackingLayer.name );
    element.setAttribute( u"stackingLayerSource"_s, mStackingLayer.source );
    element.setAttribute( u"stackingLayerProvider"_s, mStackingLayer.provider );
  }

  return true;
}

bool QgsLayoutItemMapItem::readXml( const QDomElement &itemElem, const QDomDocument &doc, const QgsReadWriteContext &context )
{
  QgsLayoutObject::readObjectPropertiesFromElement( itemElem, doc, context );

  mUuid = itemElem.attribute( u"uuid"_s );
  mName = itemElem.attribute( u"name"_s );
  mEnabled = ( itemElem.attribute( u"show"_s, u"0"_s ) != "0"_L1 );
  mStackingPosition = static_cast< StackingPosition >( itemElem.attribute( u"position"_s, QString::number( QgsLayoutItemMapItem::StackBelowMapLabels ) ).toInt() );

  const QString layerId = itemElem.attribute( u"stackingLayer"_s );
  const QString layerName = itemElem.attribute( u"stackingLayerName"_s );
  const QString layerSource = itemElem.attribute( u"stackingLayerSource"_s );
  const QString layerProvider = itemElem.attribute( u"stackingLayerProvider"_s );
  mStackingLayer = QgsMapLayerRef( layerId, layerName, layerSource, layerProvider );
  if ( mMap && mMap->layout() )
    mStackingLayer.resolveWeakly( mMap->layout()->project() );

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

QgsMapLayer *QgsLayoutItemMapItem::stackingLayer() const
{
  return mStackingLayer.get();
}

void QgsLayoutItemMapItem::setStackingLayer( QgsMapLayer *layer )
{
  mStackingLayer.setLayer( layer );
}

QgsExpressionContext QgsLayoutItemMapItem::createExpressionContext() const
{
  if ( mMap )
    return mMap->createExpressionContext();

  return QgsLayoutObject::createExpressionContext();
}

bool QgsLayoutItemMapItem::accept( QgsStyleEntityVisitorInterface * ) const
{
  return true;
}

QgsMapLayer *QgsLayoutItemMapItem::mapLayer()
{
  return nullptr;
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
  mItems.swapItemsAt( index, index + 1 );
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
  mItems.swapItemsAt( index, index - 1 );
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
  for ( QgsLayoutItemMapItem *item : std::as_const( mItems ) )
  {
    item->finalizeRestoreFromXml();
  }
}

void QgsLayoutItemMapItemStack::drawItems( QPainter *painter, bool ignoreStacking )
{
  if ( !painter )
  {
    return;
  }

  for ( QgsLayoutItemMapItem *item : std::as_const( mItems ) )
  {
    switch ( item->stackingPosition() )
    {
      case QgsLayoutItemMapItem::StackBelowMap:
      case QgsLayoutItemMapItem::StackAboveMapLayer:
      case QgsLayoutItemMapItem::StackBelowMapLayer:
      case QgsLayoutItemMapItem::StackBelowMapLabels:
        if ( !ignoreStacking )
          break;

        [[fallthrough]];
      case QgsLayoutItemMapItem::StackAboveMapLabels:
        item->draw( painter );
        break;

    }
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

bool QgsLayoutItemMapItemStack::hasEnabledItems() const
{
  for ( QgsLayoutItemMapItem *item : mItems )
  {
    if ( item->enabled() )
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



