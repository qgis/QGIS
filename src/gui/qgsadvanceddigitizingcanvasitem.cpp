/***************************************************************************
    qgsadvanceddigitizingcanvasitem.cpp  -  map canvas item for CAD tools
    ----------------------
    begin                : October 2014
    copyright            : (C) Denis Rouzaud
    email                : denis.rouzaud@gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QPainter>

#include "qgsadvanceddigitizingdockwidget.h"
#include "qgsadvanceddigitizingcanvasitem.h"
#include "qgsmapcanvas.h"


QgsAdvancedDigitizingCanvasItem::QgsAdvancedDigitizingCanvasItem( QgsMapCanvas *canvas, QgsAdvancedDigitizingDockWidget *cadDockWidget )
  : QgsMapCanvasItem( canvas )
  , mLockedPen( QPen( QColor( 0, 127, 0, 255 ), 1, Qt::DashLine ) )
  , mConstruction1Pen( QPen( QColor( 127, 127, 127, 150 ), 1, Qt::DashLine ) )
  , mConstruction2Pen( QPen( QColor( 127, 127, 127, 255 ), 1, Qt::DashLine ) )
  , mSnapPen( QPen( QColor( 127, 0, 0, 150 ), 1 ) )
  , mSnapLinePen( QPen( QColor( 127, 0, 0, 150 ), 1, Qt::DashLine ) )
  , mCursorPen( QPen( QColor( 127, 127, 127, 255 ), 1 ) )
  , mConstructionGuidesPen( QPen( QColor( 20, 210, 150 ), 1, Qt::DashLine ) )
  , mAdvancedDigitizingDockWidget( cadDockWidget )
{
}

void QgsAdvancedDigitizingCanvasItem::paint( QPainter *painter )
{
  if ( !mAdvancedDigitizingDockWidget->cadEnabled() )
    return;

  painter->setRenderHint( QPainter::Antialiasing );
  painter->setCompositionMode( QPainter::CompositionMode_Difference );

  // Draw construction guides
  if ( mAdvancedDigitizingDockWidget->showConstructionGuides() )
  {
    if ( QgsVectorLayer *constructionGuidesLayer = mAdvancedDigitizingDockWidget->constructionGuidesLayer() )
    {
      QgsFeatureIterator it = constructionGuidesLayer->getFeatures( QgsFeatureRequest().setNoAttributes().setFilterRect( mMapCanvas->mapSettings().visibleExtent() ) );
      QgsFeature feature;
      painter->setPen( mConstructionGuidesPen );
      while ( it.nextFeature( feature ) )
      {
        QgsGeometry geom = feature.geometry();
        geom.mapToPixel( *mMapCanvas->getCoordinateTransform() );
        const QPolygonF polygon = geom.asQPolygonF();
        painter->drawPolyline( polygon );
      }
    }
  }

  // Draw current tool
  if ( QgsAdvancedDigitizingTool *tool = mAdvancedDigitizingDockWidget->tool() )
  {
    // if a tool is active in the dock, then delegate to that tool to handle decorating the canvas instead of using the default decorations
    tool->paint( painter );
    return;
  }

  // Use visible polygon rather than extent to properly handle rotated maps
  QPolygonF mapPoly = mMapCanvas->mapSettings().visiblePolygon();
  const double canvasWidth = QLineF( mapPoly[0], mapPoly[1] ).length();
  const double canvasHeight = QLineF( mapPoly[0], mapPoly[3] ).length();

  const int nPoints = mAdvancedDigitizingDockWidget->pointsCount();
  if ( !nPoints )
    return;

  bool previousPointExist, penulPointExist;
  const QgsPointXY curPoint = mAdvancedDigitizingDockWidget->currentPointV2();
  const QgsPointXY prevPoint = mAdvancedDigitizingDockWidget->previousPointV2( &previousPointExist );
  const QgsPointXY penulPoint = mAdvancedDigitizingDockWidget->penultimatePointV2( &penulPointExist );
  const bool snappedToVertex = mAdvancedDigitizingDockWidget->snappedToVertex();
  const QList<QgsPointXY> snappedSegment = mAdvancedDigitizingDockWidget->snappedSegment();
  const bool hasSnappedSegment = snappedSegment.count() == 2;

  const bool curPointExist = mapPoly.containsPoint( curPoint.toQPointF(), Qt::OddEvenFill );

  const double mupp = mMapCanvas->getCoordinateTransform()->mapUnitsPerPixel();
  if ( mupp == 0 )
    return;

  const double canvasRotationRad = mMapCanvas->rotation() * M_PI / 180;
  const double canvasDiagonalDimension = ( canvasWidth + canvasHeight ) / mupp;

  QPointF curPointPix, prevPointPix, penulPointPix, snapSegmentPix1, snapSegmentPix2;

  if ( curPointExist )
  {
    curPointPix = toCanvasCoordinates( curPoint );
  }
  if ( previousPointExist )
  {
    prevPointPix = toCanvasCoordinates( prevPoint );
  }
  if ( penulPointExist )
  {
    penulPointPix = toCanvasCoordinates( penulPoint );
  }
  if ( hasSnappedSegment )
  {
    snapSegmentPix1 = toCanvasCoordinates( snappedSegment[0] );
    snapSegmentPix2 = toCanvasCoordinates( snappedSegment[1] );
  }

  // Draw point snap
  if ( curPointExist && snappedToVertex )
  {
    painter->setPen( mSnapPen );
    painter->drawEllipse( curPointPix, 10, 10 );
  }

  // Draw segment snap
  if ( hasSnappedSegment && !snappedToVertex )
  {
    painter->setPen( mSnapPen );
    painter->drawLine( snapSegmentPix1, snapSegmentPix2 );

    if ( curPointExist )
    {
      painter->setPen( mSnapLinePen );
      painter->drawLine( snapSegmentPix1, curPointPix );
    }
  }

  // Draw segment par/per input
  if ( mAdvancedDigitizingDockWidget->betweenLineConstraint() != Qgis::BetweenLineConstraint::NoConstraint && hasSnappedSegment )
  {
    painter->setPen( mConstruction2Pen );
    painter->drawLine( snapSegmentPix1, snapSegmentPix2 );
  }

  // Draw angle
  if ( nPoints > 1 )
  {
    double a0, a;
    if ( mAdvancedDigitizingDockWidget->constraintAngle()->relative() && nPoints > 2 )
    {
      a0 = std::atan2( -( prevPoint.y() - penulPoint.y() ), prevPoint.x() - penulPoint.x() );
    }
    else
    {
      a0 = 0;
    }
    if ( mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() )
    {
      a = a0 - mAdvancedDigitizingDockWidget->constraintAngle()->value() * M_PI / 180;
    }
    else
    {
      a = std::atan2( -( curPoint.y() - prevPoint.y() ), curPoint.x() - prevPoint.x() );
    }

    a0 += canvasRotationRad;
    a += canvasRotationRad;

    painter->setPen( mConstruction2Pen );
    painter->drawArc( QRectF( prevPointPix.x() - 20, prevPointPix.y() - 20, 40, 40 ), static_cast<int>( 16 * -a0 * 180 / M_PI ), static_cast<int>( 16 * ( a0 - a ) * 180 / M_PI ) );
    painter->drawLine( prevPointPix, prevPointPix + 60 * QPointF( std::cos( a0 ), std::sin( a0 ) ) );


    if ( mAdvancedDigitizingDockWidget->constraintAngle()->isLocked() )
    {
      painter->setPen( mLockedPen );
      const double canvasPadding = QLineF( prevPointPix, curPointPix ).length();
      painter->drawLine( prevPointPix + ( canvasPadding - canvasDiagonalDimension ) * QPointF( std::cos( a ), std::sin( a ) ), prevPointPix + ( canvasPadding + canvasDiagonalDimension ) * QPointF( std::cos( a ), std::sin( a ) ) );
    }
  }

  // Draw distance
  if ( nPoints > 1 && mAdvancedDigitizingDockWidget->constraintDistance()->isLocked() )
  {
    painter->setPen( mLockedPen );
    const double r = mAdvancedDigitizingDockWidget->constraintDistance()->value() / mupp;
    QPainterPath ellipsePath;
    ellipsePath.addEllipse( prevPointPix, r, r );
    const double a = std::atan2( -( curPoint.y() - prevPoint.y() ), curPoint.x() - prevPoint.x() ) + canvasRotationRad;
    const QTransform t = QTransform().translate( prevPointPix.x(), prevPointPix.y() ).rotateRadians( a ).translate( -prevPointPix.x(), -prevPointPix.y() );
    const QPolygonF ellipsePoly = ellipsePath.toFillPolygon( t );
    painter->drawPolygon( ellipsePoly );
  }

  // Draw x
  if ( mAdvancedDigitizingDockWidget->constraintX()->isLocked() )
  {
    double x = 0.0;
    bool draw = true;
    painter->setPen( mLockedPen );
    if ( mAdvancedDigitizingDockWidget->constraintX()->relative() )
    {
      if ( nPoints > 1 )
      {
        x = mAdvancedDigitizingDockWidget->constraintX()->value() + prevPoint.x();
      }
      else
      {
        draw = false;
      }
    }
    else
    {
      x = mAdvancedDigitizingDockWidget->constraintX()->value();
    }
    if ( draw )
    {
      painter->drawLine( toCanvasCoordinates( QgsPointXY( x, mapPoly[0].y() ) ) - canvasDiagonalDimension * QPointF( std::sin( -canvasRotationRad ), std::cos( -canvasRotationRad ) ), toCanvasCoordinates( QgsPointXY( x, mapPoly[0].y() ) ) + canvasDiagonalDimension * QPointF( std::sin( -canvasRotationRad ), std::cos( -canvasRotationRad ) ) );
    }
  }

  // Draw y
  if ( mAdvancedDigitizingDockWidget->constraintY()->isLocked() )
  {
    double y = 0.0;
    bool draw = true;
    painter->setPen( mLockedPen );
    if ( mAdvancedDigitizingDockWidget->constraintY()->relative() )
    {
      if ( nPoints > 1 )
      {
        y = mAdvancedDigitizingDockWidget->constraintY()->value() + prevPoint.y();
      }
      else
      {
        draw = false;
      }
    }
    else
    {
      y = mAdvancedDigitizingDockWidget->constraintY()->value();
    }
    if ( draw )
    {
      painter->drawLine( toCanvasCoordinates( QgsPointXY( mapPoly[0].x(), y ) ) - canvasDiagonalDimension * QPointF( std::cos( -canvasRotationRad ), -std::sin( -canvasRotationRad ) ), toCanvasCoordinates( QgsPointXY( mapPoly[0].x(), y ) ) + canvasDiagonalDimension * QPointF( std::cos( -canvasRotationRad ), -std::sin( -canvasRotationRad ) ) );
    }
  }

  // Draw constraints
  if ( mAdvancedDigitizingDockWidget->betweenLineConstraint() == Qgis::BetweenLineConstraint::NoConstraint )
  {
    if ( curPointExist && previousPointExist )
    {
      painter->setPen( mConstruction2Pen );
      painter->drawLine( prevPointPix, curPointPix );
    }

    if ( previousPointExist && penulPointExist )
    {
      painter->setPen( mConstruction1Pen );
      painter->drawLine( penulPointPix, prevPointPix );
    }
  }

  if ( curPointExist )
  {
    painter->setPen( mCursorPen );
    painter->drawLine( curPointPix + QPointF( -5, -5 ), curPointPix + QPointF( +5, +5 ) );
    painter->drawLine( curPointPix + QPointF( -5, +5 ), curPointPix + QPointF( +5, -5 ) );
  }

  auto lineExtensionSide = mAdvancedDigitizingDockWidget->lineExtensionSide();
  if ( mAdvancedDigitizingDockWidget->constraintLineExtension()->isLocked() && lineExtensionSide != Qgis::LineExtensionSide::NoVertex && !mAdvancedDigitizingDockWidget->lockedSnapVertices().isEmpty() )
  {
    painter->setPen( mLockedPen );

    const QgsPointLocator::Match snap = mAdvancedDigitizingDockWidget->lockedSnapVertices().constLast();
    const QPointF snappedPoint = toCanvasCoordinates( snap.point() );

    QgsFeatureRequest req;
    req.setFilterFid( snap.featureId() );
    req.setNoAttributes();
    req.setDestinationCrs( mMapCanvas->mapSettings().destinationCrs(), mMapCanvas->mapSettings().transformContext() );
    QgsFeatureIterator featureIt = snap.layer()->getFeatures( req );

    QgsFeature feature;
    featureIt.nextFeature( feature );

    const QgsGeometry geometry = feature.geometry();
    const QgsAbstractGeometry *geom = geometry.constGet();

    QgsPoint vertex;
    QgsVertexId vertexId;
    geometry.vertexIdFromVertexNr( snap.vertexIndex(), vertexId );
    if ( vertexId.isValid() )
    {
      QgsVertexId previousVertexId;
      QgsVertexId nextVertexId;
      geom->adjacentVertices( vertexId, previousVertexId, nextVertexId );

      if ( lineExtensionSide == Qgis::LineExtensionSide::BeforeVertex )
      {
        vertex = geom->vertexAt( previousVertexId );
      }
      else
      {
        vertex = geom->vertexAt( nextVertexId );
      }
    }

    if ( !vertex.isEmpty() )
    {
      const QPointF point = toCanvasCoordinates( vertex );
      const double angle = std::atan2( snappedPoint.y() - point.y(), snappedPoint.x() - point.x() );

      const double canvasPadding = QLineF( snappedPoint, curPointPix ).length();
      painter->drawLine( snappedPoint + ( canvasPadding - canvasDiagonalDimension ) * QPointF( std::cos( angle ), std::sin( angle ) ), snappedPoint + ( canvasPadding + canvasDiagonalDimension ) * QPointF( std::cos( angle ), std::sin( angle ) ) );
    }
  }

  if ( mAdvancedDigitizingDockWidget->constraintXyVertex()->isLocked() )
  {
    painter->setPen( mLockedPen );

    double coordinateExtension = mAdvancedDigitizingDockWidget->softLockX();
    if ( coordinateExtension != std::numeric_limits<double>::quiet_NaN() )
    {
      const QgsPointXY point( coordinateExtension, mapPoly[0].y() );
      const QPointF rotation( std::sin( -canvasRotationRad ), std::cos( -canvasRotationRad ) );
      painter->drawLine( toCanvasCoordinates( point ) - canvasDiagonalDimension * rotation, toCanvasCoordinates( point ) + canvasDiagonalDimension * rotation );
    }

    coordinateExtension = mAdvancedDigitizingDockWidget->softLockY();
    if ( coordinateExtension != std::numeric_limits<double>::quiet_NaN() )
    {
      const QgsPointXY point( mapPoly[0].x(), coordinateExtension );
      const QPointF rotation( std::cos( -canvasRotationRad ), -std::sin( -canvasRotationRad ) );
      painter->drawLine( toCanvasCoordinates( point ) - canvasDiagonalDimension * rotation, toCanvasCoordinates( point ) + canvasDiagonalDimension * rotation );
    }
  }

  painter->setPen( mCursorPen );

  const QList<QgsPointLocator::Match> lockedSnapVertices = mAdvancedDigitizingDockWidget->lockedSnapVertices();
  for ( const QgsPointLocator::Match &snapMatch : lockedSnapVertices )
  {
    const QgsPointXY point = snapMatch.point();
    const QPointF canvasPoint = toCanvasCoordinates( point );

    painter->drawLine( canvasPoint + QPointF( 5, 5 ), canvasPoint - QPointF( 5, 5 ) );
    painter->drawLine( canvasPoint + QPointF( -5, 5 ), canvasPoint - QPointF( -5, 5 ) );
  }

  if ( !lockedSnapVertices.isEmpty() )
  {
    const QgsPointXY point = lockedSnapVertices.last().point();
    const QPointF canvasPoint = toCanvasCoordinates( point );

    painter->drawLine( canvasPoint + QPointF( 0, 5 ), canvasPoint - QPointF( 0, 5 ) );
    painter->drawLine( canvasPoint + QPointF( 5, 0 ), canvasPoint - QPointF( 5, 0 ) );
  }
}

void QgsAdvancedDigitizingCanvasItem::updatePosition()
{
  // Use visible polygon rather than extent to properly handle rotated maps
  QPolygonF mapPoly = mMapCanvas->mapSettings().visiblePolygon();
  const double canvasWidth = QLineF( mapPoly[0], mapPoly[1] ).length();
  const double canvasHeight = QLineF( mapPoly[0], mapPoly[3] ).length();
  const QgsRectangle mapRect = QgsRectangle( mapPoly[0], QgsPointXY( mapPoly[0].x() + canvasWidth, mapPoly[0].y() - canvasHeight ) );
  if ( rect() != mapRect )
    setRect( mapRect );
}
