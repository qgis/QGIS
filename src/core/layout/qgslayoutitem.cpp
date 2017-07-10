/***************************************************************************
                              qgslayoutitem.cpp
                             -------------------
    begin                : June 2017
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

#include "qgslayoutitem.h"
#include "qgslayout.h"
#include <QPainter>

QgsLayoutItem::QgsLayoutItem( QgsLayout *layout )
  : QgsLayoutObject( layout )
  , QGraphicsRectItem( 0 )
{
  setCacheMode( QGraphicsItem::DeviceCoordinateCache );

  //record initial position
  QgsUnitTypes::LayoutUnit initialUnits = layout ? layout->units() : QgsUnitTypes::LayoutMillimeters;
  mItemPosition = QgsLayoutPoint( scenePos().x(), scenePos().y(), initialUnits );
  mItemSize = QgsLayoutSize( rect().width(), rect().height(), initialUnits );

  initConnectionsToLayout();
}

void QgsLayoutItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget *pWidget )
{
  if ( !painter || !painter->device() )
  {
    return;
  }

  //TODO - remember to disable saving/restoring on graphics view!!
  painter->save();
  preparePainter( painter );

  if ( shouldDrawDebugRect() )
  {
    drawDebugRect( painter );
  }
  else
  {
    draw( painter, itemStyle, pWidget );
  }

  painter->restore();
}

void QgsLayoutItem::setReferencePoint( const QgsLayoutItem::ReferencePoint &point )
{
  mReferencePoint = point;
}

void QgsLayoutItem::attemptResize( const QgsLayoutSize &size )
{
  if ( !mLayout )
  {
    mItemSize = size;
    setRect( 0, 0, size.width(), size.height() );
    return;
  }

  QSizeF targetSizeLayoutUnits = mLayout->convertToLayoutUnits( size );
  QSizeF actualSizeLayoutUnits = applyMinimumSize( targetSizeLayoutUnits );
  actualSizeLayoutUnits = applyFixedSize( actualSizeLayoutUnits );

  if ( actualSizeLayoutUnits == rect().size() )
  {
    return;
  }

  QgsLayoutSize actualSizeTargetUnits = mLayout->convertFromLayoutUnits( actualSizeLayoutUnits, size.units() );
  mItemSize = actualSizeTargetUnits;

  setRect( 0, 0, actualSizeLayoutUnits.width(), actualSizeLayoutUnits.height() );
}

void QgsLayoutItem::attemptMove( const QgsLayoutPoint &point )
{
  if ( !mLayout )
  {
    mItemPosition = point;
    setPos( point.toQPointF() );
    return;
  }

  QPointF targetPointLayoutUnits = mLayout->convertToLayoutUnits( point );
  //TODO - apply data defined position here
  targetPointLayoutUnits = adjustPointForReferencePosition( targetPointLayoutUnits, rect().size() );
  QPointF actualPointLayoutUnits = targetPointLayoutUnits;

  QgsLayoutPoint actualPointTargetUnits = mLayout->convertFromLayoutUnits( actualPointLayoutUnits, point.units() );
  mItemPosition = actualPointTargetUnits;

  setPos( targetPointLayoutUnits );
}

void QgsLayoutItem::refresh()
{
  QgsLayoutObject::refresh();
  refreshItemSize();
  refreshItemPosition();
}

void QgsLayoutItem::drawDebugRect( QPainter *painter )
{
  if ( !painter )
  {
    return;
  }

  painter->save();
  painter->setRenderHint( QPainter::Antialiasing, false );
  painter->setPen( Qt::NoPen );
  painter->setBrush( QColor( 100, 255, 100, 200 ) );
  painter->drawRect( rect() );
  painter->restore();
}

void QgsLayoutItem::setFixedSize( const QgsLayoutSize &size )
{
  mFixedSize = size;
  refreshItemSize();
}

void QgsLayoutItem::setMinimumSize( const QgsLayoutSize &size )
{
  mMinimumSize = size;
  refreshItemSize();
}

void QgsLayoutItem::refreshItemSize()
{
  attemptResize( mItemSize );
}

void QgsLayoutItem::refreshItemPosition()
{
  attemptMove( mItemPosition );
}

QPointF QgsLayoutItem::adjustPointForReferencePosition( const QPointF &position, const QSizeF &size ) const
{
  switch ( mReferencePoint )
  {
    case UpperMiddle:
      return QPointF( position.x() - size.width() / 2.0, position.y() );
    case UpperRight:
      return QPointF( position.x() - size.width(), position.y() );
    case MiddleLeft:
      return QPointF( position.x(), position.y() - size.height() / 2.0 );
    case Middle:
      return QPointF( position.x() - size.width() / 2.0, position.y() - size.height() / 2.0 );
    case MiddleRight:
      return QPointF( position.x() - size.width(), position.y() - size.height() / 2.0 );
    case LowerLeft:
      return QPointF( position.x(), position.y() - size.height() );
    case LowerMiddle:
      return QPointF( position.x() - size.width() / 2.0, position.y() - size.height() );
    case LowerRight:
      return QPointF( position.x() - size.width(), position.y() - size.height() );
    case UpperLeft:
      return position;
  }
  // no warnings
  return position;
}

void QgsLayoutItem::initConnectionsToLayout()
{
  if ( !mLayout )
    return;

}

void QgsLayoutItem::preparePainter( QPainter *painter )
{
  if ( !painter || !painter->device() )
  {
    return;
  }

  painter->setRenderHint( QPainter::Antialiasing, shouldDrawAntialiased() );
}

bool QgsLayoutItem::shouldDrawAntialiased() const
{
  if ( !mLayout )
  {
    return true;
  }
  return mLayout->context().testFlag( QgsLayoutContext::FlagAntialiasing ) && !mLayout->context().testFlag( QgsLayoutContext::FlagDebug );
}

bool QgsLayoutItem::shouldDrawDebugRect() const
{
  return mLayout && mLayout->context().testFlag( QgsLayoutContext::FlagDebug );
}

QSizeF QgsLayoutItem::applyMinimumSize( const QSizeF &targetSize )
{
  if ( !mLayout || minimumSize().isEmpty() )
  {
    return targetSize;
  }
  QSizeF minimumSizeLayoutUnits = mLayout->convertToLayoutUnits( minimumSize() );
  return targetSize.expandedTo( minimumSizeLayoutUnits );
}

QSizeF QgsLayoutItem::applyFixedSize( const QSizeF &targetSize )
{
  if ( !mLayout || fixedSize().isEmpty() )
  {
    return targetSize;
  }
  QSizeF fixedSizeLayoutUnits = mLayout->convertToLayoutUnits( fixedSize() );
  return targetSize.expandedTo( fixedSizeLayoutUnits );
}
