/***************************************************************************
                             qgslayoutaligner.cpp
                             --------------------
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

#include "qgslayoutaligner.h"
#include "qgslayoutitem.h"
#include "qgslayout.h"

void QgsLayoutAligner::alignItems( QgsLayout *layout, const QList<QgsLayoutItem *> &items, QgsLayoutAligner::Alignment alignment )
{
  if ( !layout || items.size() < 2 )
  {
    return;
  }

  QRectF itemBBox = boundingRectOfItems( items );
  if ( !itemBBox.isValid() )
  {
    return;
  }

  double refCoord = 0;
  switch ( alignment )
  {
    case Left:
      refCoord = itemBBox.left();
      break;
    case HCenter:
      refCoord = itemBBox.center().x();
      break;
    case Right:
      refCoord = itemBBox.right();
      break;
    case Top:
      refCoord = itemBBox.top();
      break;
    case VCenter:
      refCoord = itemBBox.center().y();
      break;
    case Bottom:
      refCoord = itemBBox.bottom();
      break;
  }

  layout->undoStack()->beginMacro( QObject::tr( "Aligned items bottom" ) );
  for ( QgsLayoutItem *item : items )
  {
    layout->undoStack()->beginCommand( item, QString() );

    QPointF shifted = item->pos();
    switch ( alignment )
    {
      case Left:
        shifted.setX( refCoord );
        break;
      case HCenter:
        shifted.setX( refCoord - item->rect().width() / 2.0 );
        break;
      case Right:
        shifted.setX( refCoord - item->rect().width() );
        break;
      case Top:
        shifted.setY( refCoord );
        break;
      case VCenter:
        shifted.setY( refCoord - item->rect().height() / 2.0 );
        break;
      case Bottom:
        shifted.setY( refCoord - item->rect().height() );
        break;
    }

    // need to keep item units
    QgsLayoutPoint newPos = layout->convertFromLayoutUnits( shifted, item->positionWithUnits().units() );
    item->attemptMove( newPos );

    layout->undoStack()->endCommand();
  }
  layout->undoStack()->endMacro();
}

QRectF QgsLayoutAligner::boundingRectOfItems( const QList<QgsLayoutItem *> &items )
{
  if ( items.empty() )
  {
    return QRectF();
  }

  auto it = items.constBegin();
  //set the box to the first item
  QgsLayoutItem *currentItem = *it;
  it++;
  double minX = currentItem->pos().x();
  double minY = currentItem->pos().y();
  double maxX = minX + currentItem->rect().width();
  double maxY = minY + currentItem->rect().height();

  double currentMinX, currentMinY, currentMaxX, currentMaxY;

  for ( ; it != items.constEnd(); ++it )
  {
    currentItem = *it;
    currentMinX = currentItem->pos().x();
    currentMinY = currentItem->pos().y();
    currentMaxX = currentMinX + currentItem->rect().width();
    currentMaxY = currentMinY + currentItem->rect().height();

    if ( currentMinX < minX )
      minX = currentMinX;
    if ( currentMaxX > maxX )
      maxX = currentMaxX;
    if ( currentMinY < minY )
      minY = currentMinY;
    if ( currentMaxY > maxY )
      maxY = currentMaxY;
  }

  return QRectF( QPointF( minX, minY ), QPointF( maxX, maxY ) );
}
