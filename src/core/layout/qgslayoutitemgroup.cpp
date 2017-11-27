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
#include "qgslayoututils.h"

QgsLayoutItemGroup::QgsLayoutItemGroup( QgsLayout *layout )
  : QgsLayoutItem( layout )
{}

QgsLayoutItemGroup::~QgsLayoutItemGroup()
{
  //loop through group members and remove them from the scene
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    if ( !item )
      continue;

    //inform model that we are about to remove an item from the scene
    if ( mLayout )
      mLayout->removeLayoutItem( item );
    else
      delete item;
  }
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

QgsLayoutItemGroup *QgsLayoutItemGroup::create( QgsLayout *layout )
{
  return new QgsLayoutItemGroup( layout );
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

  updateBoundingRect( item );
}

void QgsLayoutItemGroup::removeItems()
{
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
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
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    if ( !item )
      continue;
    val << item;
  }
  return val;
}

void QgsLayoutItemGroup::setVisibility( const bool visible )
{
  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->beginMacro( tr( "Set Group Visibility" ) );
  //also set visibility for all items within the group
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    if ( !item )
      continue;
    bool prev = item->mBlockUndoCommands;
    item->mBlockUndoCommands = mBlockUndoCommands;
    item->setVisibility( visible );
    item->mBlockUndoCommands = prev;
  }
  //lastly set visibility for group item itself
  QgsLayoutItem::setVisibility( visible );

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->endMacro();
}

void QgsLayoutItemGroup::attemptMove( const QgsLayoutPoint &point, bool useReferencePoint, bool includesFrame )
{
  Q_UNUSED( useReferencePoint ); //groups should always have reference point in top left
  if ( !mLayout )
    return;

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->beginMacro( tr( "Move group" ) );

  QPointF scenePoint = mLayout->convertToLayoutUnits( point );
  double deltaX = scenePoint.x() - pos().x();
  double deltaY = scenePoint.y() - pos().y();

  //also move all items within the group
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    if ( !item )
      continue;

    std::unique_ptr< QgsAbstractLayoutUndoCommand > command;
    if ( !shouldBlockUndoCommands() )
    {
      command.reset( createCommand( QString(), 0 ) );
      command->saveBeforeState();
    }

    // need to convert delta from layout units -> item units
    QgsLayoutPoint itemPos = item->positionWithUnits();
    QgsLayoutPoint deltaPos = mLayout->convertFromLayoutUnits( QPointF( deltaX, deltaY ), itemPos.units() );
    itemPos.setX( itemPos.x() + deltaPos.x() );
    itemPos.setY( itemPos.y() + deltaPos.y() );
    item->attemptMove( itemPos, true, includesFrame );

    if ( command )
    {
      command->saveAfterState();
      mLayout->undoStack()->stack()->push( command.release() );
    }
  }
  //lastly move group item itself
  QgsLayoutItem::attemptMove( point, includesFrame );
  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->endMacro();
  resetBoundingRect();
}

void QgsLayoutItemGroup::attemptResize( const QgsLayoutSize &size, bool includesFrame )
{
  if ( !mLayout )
    return;

  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->beginMacro( tr( "Resize Group" ) );

  QRectF oldRect = rect();
  QSizeF newSizeLayoutUnits = mLayout->convertToLayoutUnits( size );
  QRectF newRect;
  newRect.setSize( newSizeLayoutUnits );

  //also resize all items within the group
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    if ( !item )
      continue;

    std::unique_ptr< QgsAbstractLayoutUndoCommand > command;
    if ( !shouldBlockUndoCommands() )
    {
      command.reset( createCommand( QString(), 0 ) );
      command->saveBeforeState();
    }

    QRectF itemRect = mapRectFromItem( item, item->rect() );
    QgsLayoutUtils::relativeResizeRect( itemRect, oldRect, newRect );

    itemRect = itemRect.normalized();
    QPointF newPos = mapToScene( itemRect.topLeft() );

    QgsLayoutSize itemSize = mLayout->convertFromLayoutUnits( itemRect.size(), item->sizeWithUnits().units() );
    item->attemptResize( itemSize, includesFrame );

    // translate new position to current item units
    QgsLayoutPoint itemPos = mLayout->convertFromLayoutUnits( newPos, item->positionWithUnits().units() );
    item->attemptMove( itemPos, false );

    if ( command )
    {
      command->saveAfterState();
      mLayout->undoStack()->stack()->push( command.release() );
    }
  }
  QgsLayoutItem::attemptResize( size );
  if ( !shouldBlockUndoCommands() )
    mLayout->undoStack()->endMacro();

  resetBoundingRect();
}

bool QgsLayoutItemGroup::writePropertiesToElement( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "LayoutItem" ) );
  element.setAttribute( QStringLiteral( "type" ), stringType() );

  for ( QgsLayoutItem *item : mItems )
  {
    if ( !item )
      continue;

    QDomElement childItem = document.createElement( QStringLiteral( "ComposerItemGroupElement" ) );
    childItem.setAttribute( QStringLiteral( "uuid" ), item->uuid() );
    element.appendChild( childItem );
  }

  parentElement.appendChild( element );

  return true;
}

bool QgsLayoutItemGroup::readPropertiesFromElement( const QDomElement &itemElement, const QDomDocument &, const QgsReadWriteContext & )
{
  if ( itemElement.nodeName() != QStringLiteral( "LayoutItem" ) || itemElement.attribute( QStringLiteral( "type" ) ) != stringType() )
  {
    return false;
  }

  QList<QgsLayoutItem *> items;
  mLayout->layoutItems( items );

  QDomNodeList elementNodes = itemElement.elementsByTagName( QStringLiteral( "ComposerItemGroupElement" ) );
  for ( int i = 0; i < elementNodes.count(); ++i )
  {
    QDomNode elementNode = elementNodes.at( i );
    if ( !elementNode.isElement() )
      continue;

    QString uuid = elementNode.toElement().attribute( QStringLiteral( "uuid" ) );

    for ( QgsLayoutItem *item : qgis::as_const( items ) )
    {
      if ( item && ( item->mUuid == uuid /* TODO || item->mTemplateUuid == uuid */ ) )
      {
        addItem( item );
        break;
      }
    }
  }

  resetBoundingRect();

  return true;
}

void QgsLayoutItemGroup::paint( QPainter *, const QStyleOptionGraphicsItem *, QWidget * )
{
}

void QgsLayoutItemGroup::draw( QgsRenderContext &, const QStyleOptionGraphicsItem * )
{
  // nothing to draw here!
}

void QgsLayoutItemGroup::resetBoundingRect()
{
  mBoundingRectangle = QRectF();
  for ( QgsLayoutItem *item : qgis::as_const( mItems ) )
  {
    updateBoundingRect( item );
  }
}

void QgsLayoutItemGroup::updateBoundingRect( QgsLayoutItem *item )
{
  //update extent
  if ( mBoundingRectangle.isEmpty() ) //we add the first item
  {
    mBoundingRectangle = QRectF( 0, 0, item->rect().width(), item->rect().height() );
    setSceneRect( QRectF( item->pos().x(), item->pos().y(), item->rect().width(), item->rect().height() ) );

    if ( !qgsDoubleNear( item->rotation(), 0.0 ) )
    {
      setItemRotation( item->rotation() );
    }
  }
  else
  {
    if ( !qgsDoubleNear( item->rotation(), rotation() ) )
    {
      //items have mixed rotation, so reset rotation of group
      mBoundingRectangle = mapRectToScene( mBoundingRectangle );
      setItemRotation( 0 );
      mBoundingRectangle = mBoundingRectangle.united( item->mapRectToScene( item->rect() ) );
      setSceneRect( mBoundingRectangle );
    }
    else
    {
      //items have same rotation, so keep rotation of group
      mBoundingRectangle = mBoundingRectangle.united( mapRectFromItem( item, item->rect() ) );
      QPointF newPos = mapToScene( mBoundingRectangle.topLeft().x(), mBoundingRectangle.topLeft().y() );
      mBoundingRectangle = QRectF( 0, 0, mBoundingRectangle.width(), mBoundingRectangle.height() );
      setSceneRect( QRectF( newPos.x(), newPos.y(), mBoundingRectangle.width(), mBoundingRectangle.height() ) );
    }
  }
}

void QgsLayoutItemGroup::setSceneRect( const QRectF &rectangle )
{
  mItemPosition = mLayout->convertFromLayoutUnits( rectangle.topLeft(), positionWithUnits().units() );
  mItemSize = mLayout->convertFromLayoutUnits( rectangle.size(), sizeWithUnits().units() );
  setScenePos( rectangle.topLeft() );
  setRect( 0, 0, rectangle.width(), rectangle.height() );
}
