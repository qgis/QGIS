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
#include "qgslayoututils.h"
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

void QgsLayoutItem::paint( QPainter *painter, const QStyleOptionGraphicsItem *itemStyle, QWidget * )
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
    QgsRenderContext context = QgsLayoutUtils::createRenderContextForLayout( mLayout, painter );
    draw( context, itemStyle );
  }

  painter->restore();
}

void QgsLayoutItem::setReferencePoint( const QgsLayoutItem::ReferencePoint &point )
{
  if ( point == mReferencePoint )
  {
    return;
  }

  mReferencePoint = point;

  //also need to adjust stored position
  updateStoredItemPosition();
  refreshItemPosition();
}

void QgsLayoutItem::attemptResize( const QgsLayoutSize &size )
{
  if ( !mLayout )
  {
    mItemSize = size;
    setRect( 0, 0, size.width(), size.height() );
    return;
  }

  QgsLayoutSize evaluatedSize = applyDataDefinedSize( size );
  QSizeF targetSizeLayoutUnits = mLayout->convertToLayoutUnits( evaluatedSize );
  QSizeF actualSizeLayoutUnits = applyMinimumSize( targetSizeLayoutUnits );
  actualSizeLayoutUnits = applyFixedSize( actualSizeLayoutUnits );

  if ( actualSizeLayoutUnits == rect().size() )
  {
    return;
  }

  QgsLayoutSize actualSizeTargetUnits = mLayout->convertFromLayoutUnits( actualSizeLayoutUnits, size.units() );
  mItemSize = actualSizeTargetUnits;

  setRect( 0, 0, actualSizeLayoutUnits.width(), actualSizeLayoutUnits.height() );
  refreshItemPosition();
}

void QgsLayoutItem::attemptMove( const QgsLayoutPoint &point )
{
  if ( !mLayout )
  {
    mItemPosition = point;
    setPos( point.toQPointF() );
    return;
  }

  QgsLayoutPoint evaluatedPoint = applyDataDefinedPosition( point );
  QPointF evaluatedPointLayoutUnits = mLayout->convertToLayoutUnits( evaluatedPoint );
  QPointF topLeftPointLayoutUnits = adjustPointForReferencePosition( evaluatedPointLayoutUnits, rect().size(), mReferencePoint );
  if ( topLeftPointLayoutUnits == scenePos() && point.units() == mItemPosition.units() )
  {
    //TODO - add test for second condition
    return;
  }

  QgsLayoutPoint referencePointTargetUnits = mLayout->convertFromLayoutUnits( evaluatedPointLayoutUnits, point.units() );
  mItemPosition = referencePointTargetUnits;

  setScenePos( topLeftPointLayoutUnits );
}

void QgsLayoutItem::setScenePos( const QPointF &destinationPos )
{
  //since setPos does not account for item rotation, use difference between
  //current scenePos (which DOES account for rotation) and destination pos
  //to calculate how much the item needs to move
  setPos( pos() + ( destinationPos - scenePos() ) );
}

double QgsLayoutItem::itemRotation() const
{
  return rotation();
}

QgsLayoutPoint QgsLayoutItem::applyDataDefinedPosition( const QgsLayoutPoint &position )
{
  if ( !mLayout )
  {
    return position;
  }

  QgsExpressionContext context = createExpressionContext();
  double evaluatedX = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionX, context, position.x() );
  double evaluatedY = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::PositionY, context, position.y() );
  return QgsLayoutPoint( evaluatedX, evaluatedY, position.units() );
}

QgsLayoutSize QgsLayoutItem::applyDataDefinedSize( const QgsLayoutSize &size )
{
  if ( !mLayout )
  {
    return size;
  }

  QgsExpressionContext context = createExpressionContext();
  double evaluatedWidth = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemWidth, context, size.width() );
  double evaluatedHeight = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemHeight, context, size.height() );
  return QgsLayoutSize( evaluatedWidth, evaluatedHeight, size.units() );
}

double QgsLayoutItem::applyDataDefinedRotation( const double rotation )
{
  if ( !mLayout )
  {
    return rotation;
  }

  QgsExpressionContext context = createExpressionContext();
  double evaluatedRotation = mDataDefinedProperties.valueAsDouble( QgsLayoutObject::ItemRotation, context, rotation );
  return evaluatedRotation;
}

void QgsLayoutItem::refreshDataDefinedProperty( const QgsLayoutObject::DataDefinedProperty property )
{
  //update data defined properties and update item to match

  //evaluate width and height first, since they may affect position if non-top-left reference point set
  if ( property == QgsLayoutObject::ItemWidth || property == QgsLayoutObject::ItemHeight ||
       property == QgsLayoutObject::AllProperties )
  {
    refreshItemSize();
  }
  if ( property == QgsLayoutObject::PositionX || property == QgsLayoutObject::PositionY ||
       property == QgsLayoutObject::AllProperties )
  {
    refreshItemPosition();
  }
  if ( property == QgsLayoutObject::ItemRotation || property == QgsLayoutObject::AllProperties )
  {
    refreshItemRotation();
  }
}

void QgsLayoutItem::setItemRotation( const double angle )
{
  QPointF itemCenter = positionAtReferencePoint( QgsLayoutItem::Middle );
  double rotationRequired = angle - itemRotation();
  rotateItem( rotationRequired, itemCenter );
}

void QgsLayoutItem::updateStoredItemPosition()
{
  QPointF layoutPosReferencePoint = positionAtReferencePoint( mReferencePoint );
  mItemPosition = mLayout->convertFromLayoutUnits( layoutPosReferencePoint, mItemPosition.units() );
}

void QgsLayoutItem::rotateItem( const double angle, const QPointF &transformOrigin )
{
  double evaluatedAngle = angle + rotation();
  evaluatedAngle = QgsLayoutUtils::normalizedAngle( evaluatedAngle, true );
  evaluatedAngle = applyDataDefinedRotation( evaluatedAngle );

  QPointF itemTransformOrigin = mapFromScene( transformOrigin );
  setTransformOriginPoint( itemTransformOrigin );
  setRotation( evaluatedAngle );

  //adjust stored position of item to match scene pos of reference point
  updateStoredItemPosition();

  //TODO
  //  emit itemRotationChanged( rotation );

  //TODO
  //update bounds of scene, since rotation may affect this
  //mLayout->updateBounds();
}


void QgsLayoutItem::refresh()
{
  QgsLayoutObject::refresh();
  refreshItemSize();

  refreshDataDefinedProperty();
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

QPointF QgsLayoutItem::itemPositionAtReferencePoint( const ReferencePoint reference, const QSizeF &size ) const
{
  switch ( reference )
  {
    case UpperMiddle:
      return QPointF( size.width() / 2.0, 0 );
    case UpperRight:
      return QPointF( size.width(), 0 );
    case MiddleLeft:
      return QPointF( 0, size.height() / 2.0 );
    case Middle:
      return QPointF( size.width() / 2.0, size.height() / 2.0 );
    case MiddleRight:
      return QPointF( size.width(), size.height() / 2.0 );
    case LowerLeft:
      return QPointF( 0, size.height() );
    case LowerMiddle:
      return QPointF( size.width() / 2.0, size.height() );
    case LowerRight:
      return QPointF( size.width(), size.height() );
    case UpperLeft:
      return QPointF( 0, 0 );
  }
  // no warnings
  return QPointF( 0, 0 );
}

QPointF QgsLayoutItem::adjustPointForReferencePosition( const QPointF &position, const QSizeF &size, const ReferencePoint &reference ) const
{
  QPointF itemPosition = mapFromScene( position ); //need to map from scene to handle item rotation
  QPointF adjustedPointInsideItem = itemPosition - itemPositionAtReferencePoint( reference, size );
  return mapToScene( adjustedPointInsideItem );
}

QPointF QgsLayoutItem::positionAtReferencePoint( const QgsLayoutItem::ReferencePoint &reference ) const
{
  QPointF pointWithinItem = itemPositionAtReferencePoint( reference, rect().size() );
  return mapToScene( pointWithinItem );
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

void QgsLayoutItem::refreshItemRotation()
{
  setItemRotation( itemRotation() );
}
