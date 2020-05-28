/***************************************************************************
                             qgslayoutsnapper.cpp
                             --------------------
    begin                : July 2017
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

#include "qgslayoutsnapper.h"
#include "qgslayout.h"
#include "qgsreadwritecontext.h"
#include "qgsproject.h"
#include "qgslayoutpagecollection.h"
#include "qgssettings.h"

QgsLayoutSnapper::QgsLayoutSnapper( QgsLayout *layout )
  : mLayout( layout )
{
  QgsSettings s;
  mTolerance = s.value( QStringLiteral( "LayoutDesigner/defaultSnapTolerancePixels" ), 5, QgsSettings::Gui ).toInt();
}

QgsLayout *QgsLayoutSnapper::layout()
{
  return mLayout;
}

void QgsLayoutSnapper::setSnapTolerance( const int snapTolerance )
{
  mTolerance = snapTolerance;
}

void QgsLayoutSnapper::setSnapToGrid( bool enabled )
{
  mSnapToGrid = enabled;
}

void QgsLayoutSnapper::setSnapToGuides( bool enabled )
{
  mSnapToGuides = enabled;
}

void QgsLayoutSnapper::setSnapToItems( bool enabled )
{
  mSnapToItems = enabled;
}

QPointF QgsLayoutSnapper::snapPoint( QPointF point, double scaleFactor, bool &snapped, QGraphicsLineItem *horizontalSnapLine, QGraphicsLineItem *verticalSnapLine,
                                     const QList< QgsLayoutItem * > *ignoreItems ) const
{
  snapped = false;

  // highest priority - guides
  bool snappedXToGuides = false;
  double newX = snapPointToGuides( point.x(), Qt::Vertical, scaleFactor, snappedXToGuides );
  if ( snappedXToGuides )
  {
    snapped = true;
    point.setX( newX );
    if ( verticalSnapLine )
      verticalSnapLine->setVisible( false );
  }
  bool snappedYToGuides = false;
  double newY = snapPointToGuides( point.y(), Qt::Horizontal, scaleFactor, snappedYToGuides );
  if ( snappedYToGuides )
  {
    snapped = true;
    point.setY( newY );
    if ( horizontalSnapLine )
      horizontalSnapLine->setVisible( false );
  }

  bool snappedXToItems = false;
  bool snappedYToItems = false;
  if ( !snappedXToGuides )
  {
    newX = snapPointToItems( point.x(), Qt::Horizontal, scaleFactor, ignoreItems ? *ignoreItems : QList< QgsLayoutItem * >(), snappedXToItems, verticalSnapLine );
    if ( snappedXToItems )
    {
      snapped = true;
      point.setX( newX );
    }
  }
  if ( !snappedYToGuides )
  {
    newY = snapPointToItems( point.y(), Qt::Vertical, scaleFactor, ignoreItems ? *ignoreItems : QList< QgsLayoutItem * >(), snappedYToItems, horizontalSnapLine );
    if ( snappedYToItems )
    {
      snapped = true;
      point.setY( newY );
    }
  }

  bool snappedXToGrid = false;
  bool snappedYToGrid = false;
  QPointF res = snapPointToGrid( point, scaleFactor, snappedXToGrid, snappedYToGrid );
  if ( snappedXToGrid && !snappedXToGuides && !snappedXToItems )
  {
    snapped = true;
    point.setX( res.x() );
  }
  if ( snappedYToGrid && !snappedYToGuides && !snappedYToItems )
  {
    snapped = true;
    point.setY( res.y() );
  }

  return point;
}

QRectF QgsLayoutSnapper::snapRect( const QRectF &rect, double scaleFactor, bool &snapped, QGraphicsLineItem *horizontalSnapLine, QGraphicsLineItem *verticalSnapLine, const QList<QgsLayoutItem *> *ignoreItems ) const
{
  snapped = false;
  QRectF snappedRect = rect;

  QList< double > xCoords;
  xCoords << rect.left() << rect.center().x() << rect.right();
  QList< double > yCoords;
  yCoords << rect.top() << rect.center().y() << rect.bottom();

  // highest priority - guides
  bool snappedXToGuides = false;
  double deltaX = snapPointsToGuides( xCoords, Qt::Vertical, scaleFactor, snappedXToGuides );
  if ( snappedXToGuides )
  {
    snapped = true;
    snappedRect.translate( deltaX, 0 );
    if ( verticalSnapLine )
      verticalSnapLine->setVisible( false );
  }
  bool snappedYToGuides = false;
  double deltaY = snapPointsToGuides( yCoords, Qt::Horizontal, scaleFactor, snappedYToGuides );
  if ( snappedYToGuides )
  {
    snapped = true;
    snappedRect.translate( 0, deltaY );
    if ( horizontalSnapLine )
      horizontalSnapLine->setVisible( false );
  }

  bool snappedXToItems = false;
  bool snappedYToItems = false;
  if ( !snappedXToGuides )
  {
    deltaX = snapPointsToItems( xCoords, Qt::Horizontal, scaleFactor, ignoreItems ? *ignoreItems : QList< QgsLayoutItem * >(), snappedXToItems, verticalSnapLine );
    if ( snappedXToItems )
    {
      snapped = true;
      snappedRect.translate( deltaX, 0 );
    }
  }
  if ( !snappedYToGuides )
  {
    deltaY = snapPointsToItems( yCoords, Qt::Vertical, scaleFactor, ignoreItems ? *ignoreItems : QList< QgsLayoutItem * >(), snappedYToItems, horizontalSnapLine );
    if ( snappedYToItems )
    {
      snapped = true;
      snappedRect.translate( 0, deltaY );
    }
  }

  bool snappedXToGrid = false;
  bool snappedYToGrid = false;
  QList< QPointF > points;
  points << rect.topLeft() << rect.topRight() << rect.bottomLeft() << rect.bottomRight();
  QPointF res = snapPointsToGrid( points, scaleFactor, snappedXToGrid, snappedYToGrid );
  if ( snappedXToGrid && !snappedXToGuides && !snappedXToItems )
  {
    snapped = true;
    snappedRect.translate( res.x(), 0 );
  }
  if ( snappedYToGrid && !snappedYToGuides && !snappedYToItems )
  {
    snapped = true;
    snappedRect.translate( 0, res.y() );
  }

  return snappedRect;
}

QPointF QgsLayoutSnapper::snapPointToGrid( QPointF point, double scaleFactor, bool &snappedX, bool &snappedY ) const
{
  QPointF delta = snapPointsToGrid( QList< QPointF >() << point, scaleFactor, snappedX, snappedY );
  return point + delta;
}

QPointF QgsLayoutSnapper::snapPointsToGrid( const QList<QPointF> &points, double scaleFactor, bool &snappedX, bool &snappedY ) const
{
  snappedX = false;
  snappedY = false;
  if ( !mLayout || !mSnapToGrid )
  {
    return QPointF( 0, 0 );
  }
  const QgsLayoutGridSettings &grid = mLayout->gridSettings();
  if ( grid.resolution().length() <= 0 )
    return QPointF( 0, 0 );

  double deltaX = 0;
  double deltaY = 0;
  double smallestDiffX = std::numeric_limits<double>::max();
  double smallestDiffY = std::numeric_limits<double>::max();
  for ( QPointF point : points )
  {
    //calculate y offset to current page
    QPointF pagePoint = mLayout->pageCollection()->positionOnPage( point );

    double yPage = pagePoint.y(); //y-coordinate relative to current page
    double yAtTopOfPage = mLayout->pageCollection()->page( mLayout->pageCollection()->pageNumberForPoint( point ) )->pos().y();

    //snap x coordinate
    double gridRes = mLayout->convertToLayoutUnits( grid.resolution() );
    QPointF gridOffset = mLayout->convertToLayoutUnits( grid.offset() );
    int xRatio = static_cast< int >( ( point.x() - gridOffset.x() ) / gridRes + 0.5 ); //NOLINT
    int yRatio = static_cast< int >( ( yPage - gridOffset.y() ) / gridRes + 0.5 ); //NOLINT

    double xSnapped = xRatio * gridRes + gridOffset.x();
    double ySnapped = yRatio * gridRes + gridOffset.y() + yAtTopOfPage;

    double currentDiffX = std::fabs( xSnapped - point.x() );
    if ( currentDiffX < smallestDiffX )
    {
      smallestDiffX = currentDiffX;
      deltaX = xSnapped - point.x();
    }

    double currentDiffY = std::fabs( ySnapped - point.y() );
    if ( currentDiffY < smallestDiffY )
    {
      smallestDiffY = currentDiffY;
      deltaY = ySnapped - point.y();
    }
  }

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  QPointF delta( 0, 0 );
  if ( smallestDiffX <= alignThreshold )
  {
    //snap distance is inside of tolerance
    snappedX = true;
    delta.setX( deltaX );
  }
  if ( smallestDiffY <= alignThreshold )
  {
    //snap distance is inside of tolerance
    snappedY = true;
    delta.setY( deltaY );
  }

  return delta;
}

double QgsLayoutSnapper::snapPointToGuides( double original, Qt::Orientation orientation, double scaleFactor, bool &snapped ) const
{
  double delta = snapPointsToGuides( QList< double >() << original, orientation, scaleFactor, snapped );
  return original + delta;
}

double QgsLayoutSnapper::snapPointsToGuides( const QList<double> &points, Qt::Orientation orientation, double scaleFactor, bool &snapped ) const
{
  snapped = false;
  if ( !mLayout || !mSnapToGuides )
  {
    return 0;
  }

  //convert snap tolerance from pixels to layout units
  double alignThreshold = mTolerance / scaleFactor;

  double bestDelta = 0;
  double smallestDiff = std::numeric_limits<double>::max();

  for ( double p : points )
  {
    const auto constGuides = mLayout->guides().guides( orientation );
    for ( QgsLayoutGuide *guide : constGuides )
    {
      double guidePos = guide->layoutPosition();
      double diff = std::fabs( p - guidePos );
      if ( diff < smallestDiff )
      {
        smallestDiff = diff;
        bestDelta = guidePos - p;
      }
    }
  }

  if ( smallestDiff <= alignThreshold )
  {
    snapped = true;
    return bestDelta;
  }
  else
  {
    return 0;
  }
}

double QgsLayoutSnapper::snapPointToItems( double original, Qt::Orientation orientation, double scaleFactor, const QList<QgsLayoutItem *> &ignoreItems, bool &snapped,
    QGraphicsLineItem *snapLine ) const
{
  double delta = snapPointsToItems( QList< double >() << original, orientation, scaleFactor, ignoreItems, snapped, snapLine );
  return original + delta;
}

double QgsLayoutSnapper::snapPointsToItems( const QList<double> &points, Qt::Orientation orientation, double scaleFactor, const QList<QgsLayoutItem *> &ignoreItems, bool &snapped, QGraphicsLineItem *snapLine ) const
{
  snapped = false;
  if ( !mLayout || !mSnapToItems )
  {
    if ( snapLine )
      snapLine->setVisible( false );
    return 0;
  }

  double alignThreshold = mTolerance / scaleFactor;

  double bestDelta = 0;
  double smallestDiff = std::numeric_limits<double>::max();
  double closest = 0;
  const QList<QGraphicsItem *> itemList = mLayout->items();
  QList< double > currentCoords;
  for ( QGraphicsItem *item : itemList )
  {
    QgsLayoutItem *currentItem = dynamic_cast< QgsLayoutItem *>( item );
    if ( !currentItem || ignoreItems.contains( currentItem ) )
      continue;
    if ( currentItem->type() == QgsLayoutItemRegistry::LayoutGroup )
      continue; // don't snap to group bounds, instead we snap to group item bounds
    if ( !currentItem->isVisible() )
      continue; // don't snap to invisible items

    QRectF itemRect;
    if ( dynamic_cast<const QgsLayoutItemPage *>( currentItem ) )
    {
      //if snapping to paper use the paper item's rect rather then the bounding rect,
      //since we want to snap to the page edge and not any outlines drawn around the page
      itemRect = currentItem->mapRectToScene( currentItem->rect() );
    }
    else
    {
      itemRect = currentItem->mapRectToScene( currentItem->rectWithFrame() );
    }

    currentCoords.clear();
    switch ( orientation )
    {
      case Qt::Horizontal:
      {
        currentCoords << itemRect.left();
        currentCoords << itemRect.right();
        currentCoords << itemRect.center().x();
        break;
      }

      case Qt::Vertical:
      {
        currentCoords << itemRect.top();
        currentCoords << itemRect.center().y();
        currentCoords << itemRect.bottom();
        break;
      }
    }

    for ( double val : qgis::as_const( currentCoords ) )
    {
      for ( double p : points )
      {
        double dist = std::fabs( p - val );
        if ( dist <= alignThreshold && dist < smallestDiff )
        {
          snapped = true;
          smallestDiff = dist;
          bestDelta = val - p;
          closest = val;
        }
      }
    }
  }

  if ( snapLine )
  {
    if ( snapped )
    {
      snapLine->setVisible( true );
      switch ( orientation )
      {
        case Qt::Vertical:
        {
          snapLine->setLine( QLineF( -100000, closest, 100000, closest ) );
          break;
        }

        case Qt::Horizontal:
        {
          snapLine->setLine( QLineF( closest, -100000, closest, 100000 ) );
          break;
        }
      }
    }
    else
    {
      snapLine->setVisible( false );
    }
  }

  return bestDelta;
}


bool QgsLayoutSnapper::writeXml( QDomElement &parentElement, QDomDocument &document, const QgsReadWriteContext & ) const
{
  QDomElement element = document.createElement( QStringLiteral( "Snapper" ) );

  element.setAttribute( QStringLiteral( "tolerance" ), mTolerance );
  element.setAttribute( QStringLiteral( "snapToGrid" ), mSnapToGrid );
  element.setAttribute( QStringLiteral( "snapToGuides" ), mSnapToGuides );
  element.setAttribute( QStringLiteral( "snapToItems" ), mSnapToItems );

  parentElement.appendChild( element );
  return true;
}

bool QgsLayoutSnapper::readXml( const QDomElement &e, const QDomDocument &, const QgsReadWriteContext & )
{
  QDomElement element = e;
  if ( element.nodeName() != QStringLiteral( "Snapper" ) )
  {
    element = element.firstChildElement( QStringLiteral( "Snapper" ) );
  }

  if ( element.nodeName() != QStringLiteral( "Snapper" ) )
  {
    return false;
  }

  mTolerance = element.attribute( QStringLiteral( "tolerance" ), QStringLiteral( "5" ) ).toInt();
  mSnapToGrid = element.attribute( QStringLiteral( "snapToGrid" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  mSnapToGuides = element.attribute( QStringLiteral( "snapToGuides" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  mSnapToItems = element.attribute( QStringLiteral( "snapToItems" ), QStringLiteral( "0" ) ) != QLatin1String( "0" );
  return true;
}
