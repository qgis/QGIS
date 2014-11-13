/***************************************************************************
    qgsmaptoolrotatefeature.cpp  -  map tool for rotating features by mouse drag
    ---------------------
    begin                : January 2012
    copyright            : (C) 2012 by Vinayan Parameswaran
    email                : vinayan123 at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolrotatefeature.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgstolerance.h"
#include <QMouseEvent>
#include <QSettings>
#include <limits>
#include <math.h>
#include "qgsvertexmarker.h"

#define PI 3.14159265

QgsMapToolRotateFeature::QgsMapToolRotateFeature( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 )
{
  mRotation = 0;
  mAnchorPoint = 0;
  mCtrl = false;
}

QgsMapToolRotateFeature::~QgsMapToolRotateFeature()
{
  delete mAnchorPoint;
  delete mRubberBand;
}

void QgsMapToolRotateFeature::canvasMoveEvent( QMouseEvent * e )
{
  if ( mCtrl )
  {
    if ( !mAnchorPoint )
    {
      return;
    }
    mAnchorPoint->setCenter( toMapCoordinates( e->pos() ) );
    mStartPointMapCoords = toMapCoordinates( e->pos() );
    mStPoint = e->pos();
    return;


  }
  if ( mRubberBand )
  {
    double XDistance = e->pos().x() - mStPoint.x();
    double YDistance = e->pos().y() - mStPoint.y();
    mRotation = atan2( YDistance, XDistance ) * ( 180 / PI );
    mRotation = mRotation - mRotationOffset;

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
    double offsetX = mStPoint.x() - mRubberBand->x();
    double offsetY = mStPoint.y() - mRubberBand->y();

    mRubberBand->setTransform( QTransform().translate( offsetX, offsetY ).rotate( mRotation ).translate( -1 * offsetX, -1 * offsetY ) );
    mRubberBand->update();
  }
}

void QgsMapToolRotateFeature::canvasPressEvent( QMouseEvent * e )
{
  mRotation = 0;
  if ( mCtrl )
  {
    return;
  }

  delete mRubberBand;
  mRubberBand = 0;

  mInitialPos = e->pos();

  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return;
  }

  if ( !vlayer->isEditable() )
  {
    notifyNotEditableLayer();
    return;
  }

  QgsPoint layerCoords = toLayerCoordinates( vlayer, e->pos() );
  double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
  QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                           layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );

  if ( vlayer->selectedFeatureCount() == 0 )
  {
    QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ).setSubsetOfAttributes( QgsAttributeList() ) );

    //find the closest feature
    QgsGeometry* pointGeometry = QgsGeometry::fromPoint( layerCoords );
    if ( !pointGeometry )
    {
      return;
    }

    double minDistance = std::numeric_limits<double>::max();

    QgsFeature cf;
    QgsFeature f;
    while ( fit.nextFeature( f ) )
    {
      if ( f.geometry() )
      {
        double currentDistance = pointGeometry->distance( *f.geometry() );
        if ( currentDistance < minDistance )
        {
          minDistance = currentDistance;
          cf = f;
        }
      }

    }

    delete pointGeometry;

    if ( minDistance == std::numeric_limits<double>::max() )
    {
      return;
    }

    QgsRectangle bound = cf.geometry()->boundingBox();
    mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

    if ( !mAnchorPoint )
    {
      mAnchorPoint = new QgsVertexMarker( mCanvas );
    }
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mStartPointMapCoords );

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );

    mRotatedFeatures.clear();
    mRotatedFeatures << cf.id(); //todo: take the closest feature, not the first one...

    mRubberBand = createRubberBand( vlayer->geometryType() );
    mRubberBand->setToGeometry( cf.geometry(), vlayer );
  }
  else
  {
    mRotatedFeatures = vlayer->selectedFeaturesIds();

    mRubberBand = createRubberBand( vlayer->geometryType() );

    QgsFeature feat;
    QgsFeatureIterator it = vlayer->selectedFeaturesIterator();
    while ( it.nextFeature( feat ) )
    {
      mRubberBand->addGeometry( feat.geometry(), vlayer );
    }
  }

  mRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
  mRubberBand->setWidth( 2 );
  mRubberBand->show();

  double XDistance = mInitialPos.x() - mAnchorPoint->x();
  double YDistance = mInitialPos.y() - mAnchorPoint->y() ;
  mRotationOffset = atan2( YDistance, XDistance ) * ( 180 / PI );
}

void QgsMapToolRotateFeature::canvasReleaseEvent( QMouseEvent * e )
{
  Q_UNUSED( e );
  if ( !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    return;
  }

  //calculations for affine transformation
  double angle = -1 * mRotation * ( PI / 180 );
  QgsPoint anchorPoint = toLayerCoordinates( vlayer, mStartPointMapCoords );
  double a = cos( angle );
  double b = -1 * sin( angle );
  double c = anchorPoint.x() - cos( angle ) * anchorPoint.x() + sin( angle ) * anchorPoint.y();
  double d = sin( angle );
  double ee = cos( angle );
  double f = anchorPoint.y() - sin( angle ) * anchorPoint.x() - cos( angle ) * anchorPoint.y();

  vlayer->beginEditCommand( tr( "Features Rotated" ) );

  int start;
  if ( vlayer->geometryType() == 2 )
  {
    start = 1;
  }
  else
  {
    start = 0;
  }

  int i = 0;
  foreach ( QgsFeatureId id, mRotatedFeatures )
  {
    QgsFeature feat;
    vlayer->getFeatures( QgsFeatureRequest().setFilterFid( id ) ).nextFeature( feat );
    QgsGeometry* geom = feat.geometry();
    i = start;

    QgsPoint vertex = geom->vertexAt( i );
    while ( vertex != QgsPoint( 0, 0 ) )
    {
      double newX = a * vertex.x() + b * vertex.y() + c;
      double newY = d * vertex.x() + ee * vertex.y() + f;

      vlayer->moveVertex( newX, newY, id, i );
      i = i + 1;
      vertex = geom->vertexAt( i );
    }

  }

  double anchorX = a * anchorPoint.x() + b * anchorPoint.y() + c;
  double anchorY = d * anchorPoint.x() + ee * anchorPoint.y() + f;

  mAnchorPoint->setCenter( QgsPoint( anchorX, anchorY ) );

  delete mRubberBand;
  mRubberBand = 0;

  mCanvas->refresh();
  vlayer->endEditCommand();

}

void QgsMapToolRotateFeature::resetAnchor()
{
  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    return;
  }

  if ( vlayer->selectedFeatureCount() == 0 )
  {
    return;
  }
  else
  {

    QgsRectangle bound = vlayer->boundingBoxOfSelected();
    mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

    mAnchorPoint->setCenter( mStartPointMapCoords );

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
  }

}


void QgsMapToolRotateFeature::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Control )
  {
    mCtrl = true;
    mCanvas->viewport()->setMouseTracking( true );
    return;
  }

  if ( e->key() == Qt::Key_Escape )
  {
    this->resetAnchor();
  }
}

void QgsMapToolRotateFeature::keyReleaseEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Control )
  {
    mCtrl = false;
    mCanvas->viewport()->setMouseTracking( false );

    return;
  }

}

void QgsMapToolRotateFeature::activate()
{

  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    return;
  }

  if ( !vlayer->isEditable() )
  {
    return;
  }

  if ( vlayer->selectedFeatureCount() == 0 )
  {
    return;
  }
  else
  {

    QgsRectangle bound = vlayer->boundingBoxOfSelected();
    mStartPointMapCoords = toMapCoordinates( vlayer, bound.center() );

    mAnchorPoint = new QgsVertexMarker( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mStartPointMapCoords );

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );

    QgsMapTool::activate();
  }
}

void QgsMapToolRotateFeature::deactivate()
{
  delete mRubberBand;
  delete mAnchorPoint;
  mRubberBand = 0;
  mAnchorPoint = 0;
  mRotationOffset = 0;

  QgsMapTool::deactivate();
}
