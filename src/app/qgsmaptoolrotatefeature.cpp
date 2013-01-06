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
#include <QMessageBox>
#include <QMouseEvent>
#include <QSettings>
#include <limits>
#include <math.h>
#include "qgsvertexmarker.h"

#define PI 3.14159265

QgsMapToolRotateFeature::QgsMapToolRotateFeature( QgsMapCanvas* canvas ): QgsMapToolEdit( canvas ), mRubberBand( 0 )
{
  mRotation = 0;
}

QgsMapToolRotateFeature::~QgsMapToolRotateFeature()
{
  delete mAnchorPoint;
  delete mRubberBand;
}

void QgsMapToolRotateFeature::canvasMoveEvent( QMouseEvent * e )
{
  if ( mCtrl == true )
  {
    mAnchorPoint->setCenter( toMapCoordinates( e->pos() ) );
    mStartPointMapCoords = toMapCoordinates( e->pos() );
    mStPoint = e->pos();
    return;


  }
  if ( mRubberBand )
  {
    double XDistance = mStPoint.x() - e->pos().x();
    double YDistance = mStPoint.y() - e->pos().y();
    mRotation = atan2( YDistance, XDistance ) * ( 180 / PI );

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
  if ( mCtrl == true )
  {
    return;
  }

  delete mRubberBand;
  mRubberBand = 0;

  mInitialPos = e->pos();

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
    mMovedFeatures = vlayer->selectedFeaturesIds();

    mRubberBand = createRubberBand();
    for ( int i = 0; i < vlayer->selectedFeatureCount(); i++ )
    {
      mRubberBand->addGeometry( vlayer->selectedFeatures()[i].geometry(), vlayer );
    }
  }

  mRubberBand->setColor( Qt::red );
  mRubberBand->setWidth( 2 );
  mRubberBand->show();

}

void QgsMapToolRotateFeature::canvasReleaseEvent( QMouseEvent * e )
{
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
  QgsPoint anchorPoint = mStartPointMapCoords;
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
  foreach ( QgsFeatureId id, mMovedFeatures )
  {
    QgsFeature feat;
    vlayer->featureAtId( id, feat );
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
    mStartPointMapCoords = bound.center();

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
    QMessageBox::information( 0, tr( "Layer not editable" ),
                              tr( "Cannot edit the vector layer. Use 'Toggle Editing' to make it editable." )
                            );
    return;
  }

  if ( vlayer->selectedFeatureCount() == 0 )
  {
    return;
  }
  else
  {

    QgsRectangle bound = vlayer->boundingBoxOfSelected();
    mStartPointMapCoords = bound.center();

    mAnchorPoint = new QgsVertexMarker( mCanvas );
    mAnchorPoint->setIconType( QgsVertexMarker::ICON_CROSS );
    mAnchorPoint->setCenter( mStartPointMapCoords );
    mAnchorPoint->acceptTouchEvents();

    mStPoint = toCanvasCoordinates( mStartPointMapCoords );
    mCtrl = false;

    QgsMapTool::activate();
  }
}

void QgsMapToolRotateFeature::deactivate()
{
  delete mRubberBand;
  delete mAnchorPoint;
  mRubberBand = 0;
  mAnchorPoint = 0;

  QgsMapTool::deactivate();
}
