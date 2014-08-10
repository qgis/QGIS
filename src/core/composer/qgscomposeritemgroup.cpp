/***************************************************************************
                         qgscomposeritemgroup.cpp
                         ------------------------
    begin                : 2nd June 2008
    copyright            : (C) 2008 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscomposeritemgroup.h"
#include "qgscomposition.h"
#include "qgscomposerutils.h"
#include "qgslogger.h"
#include "qgscomposermodel.h"

#include <QPen>
#include <QPainter>

QgsComposerItemGroup::QgsComposerItemGroup( QgsComposition* c )
    : QgsComposerItem( c )
{
  setZValue( 90 );
  show();
}

QgsComposerItemGroup::~QgsComposerItemGroup()
{
  //loop through group members and remove them from the scene
  QSet<QgsComposerItem*>::iterator itemIt = mItems.begin();
  for ( ; itemIt != mItems.end(); ++itemIt )
  {
    if ( *itemIt )
    {
      //inform model that we are about to remove an item from the scene
      mComposition->itemsModel()->setItemRemoved( *itemIt );
      mComposition->removeItem( *itemIt );
      ( *itemIt )->setIsGroupMember( false );
    }
  }
}

void QgsComposerItemGroup::addItem( QgsComposerItem* item )
{
  if ( !item )
  {
    return;
  }

  if ( mItems.contains( item ) )
  {
    return;
  }

  connect( item, SIGNAL( destroyed() ), this, SLOT( itemDestroyed() ) );

  mItems.insert( item );
  item->setSelected( false );
  item->setIsGroupMember( true );

  //update extent
  if ( mBoundingRectangle.isEmpty() ) //we add the first item
  {
    mBoundingRectangle = QRectF( 0, 0, item->rect().width(), item->rect().height() );
    //call method of superclass to avoid repositioning of items
    QgsComposerItem::setSceneRect( QRectF( item->pos().x(), item->pos().y(), item->rect().width(), item->rect().height() ) );

    if ( item->itemRotation() != 0 )
    {
      setItemRotation( item->itemRotation() );
    }
  }
  else
  {
    if ( item->itemRotation() != itemRotation() )
    {
      //items have mixed rotation, so reset rotation of group
      mBoundingRectangle = mapRectToScene( mBoundingRectangle );
      setItemRotation( 0 );
      mBoundingRectangle = mBoundingRectangle.united( item->mapRectToScene( item->rect() ) );
      //call method of superclass to avoid repositioning of items
      QgsComposerItem::setSceneRect( mBoundingRectangle );
    }
    else
    {
      //items have same rotation, so keep rotation of group
      mBoundingRectangle = mBoundingRectangle.united( mapRectFromItem( item, item->rect() ) );
      QPointF newPos = mapToScene( mBoundingRectangle.topLeft().x(), mBoundingRectangle.topLeft().y() );
      mBoundingRectangle = QRectF( 0, 0, mBoundingRectangle.width(), mBoundingRectangle.height() );
      QgsComposerItem::setSceneRect( QRectF( newPos.x(), newPos.y(), mBoundingRectangle.width(), mBoundingRectangle.height() ) );
    }
  }

}

void QgsComposerItemGroup::removeItems()
{
  QSet<QgsComposerItem*>::iterator item_it = mItems.begin();
  for ( ; item_it != mItems.end(); ++item_it )
  {
    ( *item_it )->setIsGroupMember( false );
    ( *item_it )->setSelected( true );
  }
  mItems.clear();
}

void QgsComposerItemGroup::itemDestroyed()
{
  mItems.remove( static_cast<QgsComposerItem*>( sender() ) );
}

void QgsComposerItemGroup::paint( QPainter * painter, const QStyleOptionGraphicsItem * option, QWidget * widget )
{
  Q_UNUSED( option );
  Q_UNUSED( widget );
  drawFrame( painter );
  if ( isSelected() )
  {
    drawSelectionBoxes( painter );
  }
}

void QgsComposerItemGroup::setSceneRect( const QRectF& rectangle )
{
  //resize all items in this group
  //first calculate new group rectangle in current group coordsys
  QPointF newOrigin = mapFromScene( rectangle.topLeft() );
  QRectF newRect = QRectF( newOrigin.x(), newOrigin.y(), rectangle.width(), rectangle.height() );

  QSet<QgsComposerItem*>::iterator item_it = mItems.begin();
  for ( ; item_it != mItems.end(); ++item_it )
  {
    //each item needs to be scaled relatively to the final size of the group
    QRectF itemRect = mapRectFromItem(( *item_it ), ( *item_it )->rect() );
    QgsComposerUtils::relativeResizeRect( itemRect, rect(), newRect );

    QPointF newPos = mapToScene( itemRect.topLeft() );
    ( *item_it )->setSceneRect( QRectF( newPos.x(), newPos.y(), itemRect.width(), itemRect.height() ) );
  }
  //lastly, set new rect for group
  QgsComposerItem::setSceneRect( rectangle );
}

void QgsComposerItemGroup::setVisibility( const bool visible )
{
  //also set visibility for all items within the group
  QSet<QgsComposerItem*>::iterator item_it = mItems.begin();
  for ( ; item_it != mItems.end(); ++item_it )
  {
    ( *item_it )->setVisibility( visible );
  }
  //lastly set visibility for group item itself
  QgsComposerItem::setVisibility( visible );
}

void QgsComposerItemGroup::drawFrame( QPainter* p )
{
  if ( !mComposition )
  {
    return;
  }

  if ( mFrame && mComposition->plotStyle() == QgsComposition::Preview )
  {
    QPen newPen( pen() );
    newPen.setStyle( Qt::DashLine );
    newPen.setColor( QColor( 128, 128, 128, 128 ) );
    p->setPen( newPen );
    p->setRenderHint( QPainter::Antialiasing, true );
    p->drawRect( QRectF( 0, 0, rect().width(), rect().height() ) );
  }
}

bool QgsComposerItemGroup::writeXML( QDomElement& elem, QDomDocument & doc ) const
{
  QDomElement group = doc.createElement( "ComposerItemGroup" );

  QSet<QgsComposerItem*>::const_iterator itemIt = mItems.begin();
  for ( ; itemIt != mItems.end(); ++itemIt )
  {
    QDomElement item = doc.createElement( "ComposerItemGroupElement" );
    item.setAttribute( "uuid", ( *itemIt )->uuid() );
    group.appendChild( item );
  }

  elem.appendChild( group );

  return _writeXML( group, doc );
}

bool QgsComposerItemGroup::readXML( const QDomElement& itemElem, const QDomDocument& doc )
{
  //restore general composer item properties
  QDomNodeList composerItemList = itemElem.elementsByTagName( "ComposerItem" );
  if ( composerItemList.size() > 0 )
  {
    QDomElement composerItemElem = composerItemList.at( 0 ).toElement();
    _readXML( composerItemElem, doc );
  }

  QList<QGraphicsItem *> items = mComposition->items();

  QDomNodeList elementNodes = itemElem.elementsByTagName( "ComposerItemGroupElement" );
  for ( int i = 0; i < elementNodes.count(); ++i )
  {
    QDomNode elementNode = elementNodes.at( i );
    if ( !elementNode.isElement() )
      continue;

    QString uuid = elementNode.toElement().attribute( "uuid" );

    for ( QList<QGraphicsItem *>::iterator it = items.begin(); it != items.end(); ++it )
    {
      QgsComposerItem *item = dynamic_cast<QgsComposerItem *>( *it );
      if ( item && ( item->mUuid == uuid || item->mTemplateUuid == uuid ) )
      {
        addItem( item );
        break;
      }
    }
  }

  return true;
}
