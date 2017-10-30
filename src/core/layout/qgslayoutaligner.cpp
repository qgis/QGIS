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
    case AlignLeft:
      refCoord = itemBBox.left();
      break;
    case AlignHCenter:
      refCoord = itemBBox.center().x();
      break;
    case AlignRight:
      refCoord = itemBBox.right();
      break;
    case AlignTop:
      refCoord = itemBBox.top();
      break;
    case AlignVCenter:
      refCoord = itemBBox.center().y();
      break;
    case AlignBottom:
      refCoord = itemBBox.bottom();
      break;
  }

  layout->undoStack()->beginMacro( undoText( alignment ) );
  for ( QgsLayoutItem *item : items )
  {
    layout->undoStack()->beginCommand( item, QString() );

    QPointF shifted = item->pos();
    switch ( alignment )
    {
      case AlignLeft:
        shifted.setX( refCoord );
        break;
      case AlignHCenter:
        shifted.setX( refCoord - item->rect().width() / 2.0 );
        break;
      case AlignRight:
        shifted.setX( refCoord - item->rect().width() );
        break;
      case AlignTop:
        shifted.setY( refCoord );
        break;
      case AlignVCenter:
        shifted.setY( refCoord - item->rect().height() / 2.0 );
        break;
      case AlignBottom:
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

void QgsLayoutAligner::distributeItems( QgsLayout *layout, const QList<QgsLayoutItem *> &items, QgsLayoutAligner::Distribution distribution )
{
  if ( items.size() < 2 )
    return;

  auto collectReferenceCoord = [distribution]( QgsLayoutItem * item )->double
  {
    QRectF itemBBox = item->sceneBoundingRect();
    switch ( distribution )
    {
      case AlignLeft:
        return itemBBox.left();
      case AlignHCenter:
        return itemBBox.center().x();
      case AlignRight:
        return itemBBox.right();
      case AlignTop:
        return itemBBox.top();
      case AlignVCenter:
        return itemBBox.center().y();
      case AlignBottom:
        return itemBBox.bottom();
    }
    // no warnings
    return itemBBox.left();
  };


  double minCoord = DBL_MAX;
  double maxCoord = -DBL_MAX;
  QMap< double, QgsLayoutItem * > itemCoords;
  for ( QgsLayoutItem *item : items )
  {
    double refCoord = collectReferenceCoord( item );
    minCoord = std::min( minCoord, refCoord );
    maxCoord = std::max( maxCoord, refCoord );
    itemCoords.insert( refCoord, item );
  }

  double step = ( maxCoord - minCoord ) / ( items.size() - 1 );

  auto distributeItemToCoord = [layout, distribution]( QgsLayoutItem * item, double refCoord )
  {
    QPointF shifted = item->pos();
    switch ( distribution )
    {
      case DistributeLeft:
        shifted.setX( refCoord );
        break;
      case DistributeHCenter:
        shifted.setX( refCoord - item->rect().width() / 2.0 );
        break;
      case DistributeRight:
        shifted.setX( refCoord - item->rect().width() );
        break;
      case DistributeTop:
        shifted.setY( refCoord );
        break;
      case DistributeVCenter:
        shifted.setY( refCoord - item->rect().height() / 2.0 );
        break;
      case DistributeBottom:
        shifted.setY( refCoord - item->rect().height() );
        break;
    }

    // need to keep item units
    QgsLayoutPoint newPos = layout->convertFromLayoutUnits( shifted, item->positionWithUnits().units() );
    item->attemptMove( newPos );
  };


  layout->undoStack()->beginMacro( undoText( distribution ) );
  double currentVal = minCoord;
  for ( auto itemIt = itemCoords.constBegin(); itemIt != itemCoords.constEnd(); ++itemIt )
  {
    layout->undoStack()->beginCommand( itemIt.value(), QString() );
    distributeItemToCoord( itemIt.value(), currentVal );
    layout->undoStack()->endCommand();

    currentVal += step;
  }
  layout->undoStack()->endMacro();
}

void QgsLayoutAligner::resizeItems( QgsLayout *layout, const QList<QgsLayoutItem *> &items, QgsLayoutAligner::Resize resize )
{
  if ( !( items.size() >= 2 || ( items.size() == 1 && resize == ResizeToSquare ) ) )
    return;

  auto collectSize = [resize]( QgsLayoutItem * item )->double
  {
    QRectF itemBBox = item->sceneBoundingRect();
    switch ( resize )
    {
      case ResizeNarrowest:
      case ResizeWidest:
      case ResizeToSquare:
        return itemBBox.width();
      case ResizeShortest:
      case ResizeTallest:
        return itemBBox.height();
    }
    // no warnings
    return itemBBox.width();
  };

  double newSize = collectSize( items.at( 0 ) );
  for ( QgsLayoutItem *item : items )
  {
    double size = collectSize( item );
    switch ( resize )
    {
      case ResizeNarrowest:
      case ResizeShortest:
        newSize = std::min( size, newSize );
        break;
      case ResizeTallest:
      case ResizeWidest:
        newSize = std::max( size, newSize );
        break;
      case ResizeToSquare:
        break;
    }
  }

  auto resizeItemToSize = [layout, resize]( QgsLayoutItem * item, double size )
  {
    QSizeF newSize = item->rect().size();
    switch ( resize )
    {
      case ResizeNarrowest:
      case ResizeWidest:
        newSize.setWidth( size );
        break;
      case ResizeTallest:
      case ResizeShortest:
        newSize.setHeight( size );
        break;
      case ResizeToSquare:
      {
        if ( newSize.width() > newSize.height() )
          newSize.setHeight( newSize.width() );
        else
          newSize.setWidth( newSize.height() );
        break;
      }
    }

    // need to keep item units
    QgsLayoutSize newSizeWithUnits = layout->convertFromLayoutUnits( newSize, item->sizeWithUnits().units() );
    item->attemptResize( newSizeWithUnits );
  };

  layout->undoStack()->beginMacro( undoText( resize ) );
  for ( QgsLayoutItem *item : items )
  {
    layout->undoStack()->beginCommand( item, QString() );
    resizeItemToSize( item, newSize );
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

QString QgsLayoutAligner::undoText( Distribution distribution )
{
  switch ( distribution )
  {
    case DistributeLeft:
      return QObject::tr( "Distribute Items by Left" );
    case DistributeHCenter:
      return QObject::tr( "Distribute Items by Center" );
    case DistributeRight:
      return QObject::tr( "Distribute Items by Right" );
    case DistributeTop:
      return QObject::tr( "Distribute Items by Top" );
    case DistributeVCenter:
      return QObject::tr( "Distribute Items by Vertical Center" );
    case DistributeBottom:
      return QObject::tr( "Distribute Items by Bottom" );
  }
  return QString(); //no warnings
}

QString QgsLayoutAligner::undoText( QgsLayoutAligner::Resize resize )
{
  switch ( resize )
  {
    case ResizeNarrowest:
      return QObject::tr( "Resize Items to Narrowest" );
    case ResizeWidest:
      return QObject::tr( "Resize Items to Widest" );
    case ResizeShortest:
      return QObject::tr( "Resize Items to Shortest" );
    case ResizeTallest:
      return QObject::tr( "Resize Items to Tallest" );
    case ResizeToSquare:
      return QObject::tr( "Resize Items to Square" );
  }
  return QString(); //no warnings
}

QString QgsLayoutAligner::undoText( Alignment alignment )
{
  switch ( alignment )
  {
    case AlignLeft:
      return QObject::tr( "Align Items to Left" );
    case AlignHCenter:
      return QObject::tr( "Align Items to Center" );
    case AlignRight:
      return QObject::tr( "Align Items to Right" );
    case AlignTop:
      return QObject::tr( "Align Items to Top" );
    case AlignVCenter:
      return QObject::tr( "Align Items to Vertical Center" );
    case AlignBottom:
      return QObject::tr( "Align Items to Bottom" );
  }
  return QString(); //no warnings
}
