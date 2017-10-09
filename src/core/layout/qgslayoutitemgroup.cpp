/***************************************************************************
                              qgslayoutitemgroup.cpp
                             -----------------------
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

#include "qgslayoutitemgroup.h"
#include "qgslayoutitemregistry.h"
#include "qgslayout.h"

QgsLayoutItemGroup::QgsLayoutItemGroup( QgsLayout *layout )
  : QgsLayoutItem( layout )
{}

QgsLayoutItemGroup::~QgsLayoutItemGroup()
{
  if ( mLayout )
    mLayout->undoStack()->beginMacro( tr( "Removed group" ) );
  //loop through group members and remove them from the scene
  for ( QgsLayoutItem *item : qgsAsConst( mItems ) )
  {
    if ( !item )
      continue;

    //inform model that we are about to remove an item from the scene
    if ( mLayout )
      mLayout->removeLayoutItem( item );
    else
      item->deleteLater();
  }
  if ( mLayout )
    mLayout->undoStack()->endMacro();
}

int QgsLayoutItemGroup::type() const
{
  return QgsLayoutItemRegistry::LayoutGroup;
}

QString QgsLayoutItemGroup::stringType() const
{
  return QStringLiteral( "ItemGroup" );
}

QString QgsLayoutItemGroup::displayName() const
{
  //return id, if it's not empty
  if ( !id().isEmpty() )
  {
    return id();
  }
  return tr( "<Group>" );
}

void QgsLayoutItemGroup::addItem( QgsLayoutItem *item )
{
  if ( !item )
  {
    return;
  }

  if ( mItems.contains( item ) )
  {
    return;
  }

  mItems << QPointer< QgsLayoutItem >( item );
  item->setParentGroup( this );

  updateBoundingRect();
}

void QgsLayoutItemGroup::removeItems()
{
  for ( QgsLayoutItem *item : qgsAsConst( mItems ) )
  {
    if ( !item )
      continue;

    item->setParentGroup( nullptr );
  }
  mItems.clear();
}

QList<QgsLayoutItem *> QgsLayoutItemGroup::items() const
{
  QList<QgsLayoutItem *> val;
  for ( QgsLayoutItem *item : qgsAsConst( mItems ) )
  {
    if ( !item )
      continue;
    val << item;
  }
  return val;
}

void QgsLayoutItemGroup::setVisibility( const bool visible )
{
  if ( mLayout )
    mLayout->undoStack()->beginMacro( tr( "Set group visibility" ) );
  //also set visibility for all items within the group
  for ( QgsLayoutItem *item : qgsAsConst( mItems ) )
  {
    if ( !item )
      continue;
    item->setVisibility( visible );
  }
  //lastly set visibility for group item itself
  QgsLayoutItem::setVisibility( visible );
  if ( mLayout )
    mLayout->undoStack()->endMacro();
}

void QgsLayoutItemGroup::draw( QgsRenderContext &, const QStyleOptionGraphicsItem * )
{
  // nothing to draw here!
}

void QgsLayoutItemGroup::updateBoundingRect()
{
#if 0
  mBoundingRectangle = QRectF();

  for ( QgsLayoutItem *item : qgsAsConst( mItems ) )
  {
    if ( !item )
      continue;

    //update extent
    if ( mBoundingRectangle.isEmpty() ) //we add the first item
    {
      mBoundingRectangle = QRectF( item->pos().x(), item->pos().y(), item->rect().width(), item->rect().height() );

      if ( !qgsDoubleNear( item->itemRotation(), 0.0 ) )
      {
        setItemRotation( item->itemRotation() );
      }
    }
    else
    {
      if ( !qgsDoubleNear( item->itemRotation(), itemRotation() ) )
      {
        //items have mixed rotation, so reset rotation of group
        mBoundingRectangle = mapRectToScene( mBoundingRectangle );
        setItemRotation( 0 );
        mBoundingRectangle = mBoundingRectangle.united( item->mapRectToScene( item->rect() ) );
      }
      else
      {
        //items have same rotation, so keep rotation of group
        mBoundingRectangle = mBoundingRectangle.united( mapRectFromItem( item, item->rect() ) );
        mBoundingRectangle = QRectF( 0, 0, mBoundingRectangle.width(), mBoundingRectangle.height() );
      }
    }
  }

  //call method of superclass to avoid repositioning of items
  QgsLayoutItem::setSceneRect( mBoundingRectangle );
#endif
}
