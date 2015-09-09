/***************************************************************************
    qgsmaptoolmovefeature.cpp  -  map tool for translating features by mouse drag
    ---------------------
    begin                : Juli 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolmovefeature.h"
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
QgsMapToolMoveFeature::QgsMapToolMoveFeature( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mRubberBand( 0 )
{
  mToolName = tr( "Move feature" );
}

QgsMapToolMoveFeature::~QgsMapToolMoveFeature()
{
  delete mRubberBand;
}

void QgsMapToolMoveFeature::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRubberBand )
  {
    QgsPoint pointCanvasCoords = toMapCoordinates( e->pos() );
    double offsetX = pointCanvasCoords.x() - mStartPointMapCoords.x();
    double offsetY = pointCanvasCoords.y() - mStartPointMapCoords.y();
    mRubberBand->setTranslationOffset( offsetX, offsetY );
    mRubberBand->updatePosition();
    mRubberBand->update();
  }
}

void QgsMapToolMoveFeature::canvasPressEvent( QMouseEvent * e )
{
  delete mRubberBand;
  mRubberBand = 0;

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

  //find first geometry under mouse cursor and store iterator to it
  QgsPoint layerCoords = toLayerCoordinates( vlayer, e->pos() );
  QSettings settings;
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
      if ( f.constGeometry() )
      {
        double currentDistance = pointGeometry->distance( *f.constGeometry() );
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

    mMovedFeatures.clear();
    mMovedFeatures << cf.id(); //todo: take the closest feature, not the first one...

    mRubberBand = createRubberBand( vlayer->geometryType() );
    mRubberBand->setToGeometry( cf.constGeometry(), vlayer );
  }
  else
  {
    mMovedFeatures = vlayer->selectedFeaturesIds();

    mRubberBand = createRubberBand( vlayer->geometryType() );
    QgsFeature feat;
    QgsFeatureIterator it = vlayer->selectedFeaturesIterator( QgsFeatureRequest().setSubsetOfAttributes( QgsAttributeList() ) );

    while ( it.nextFeature( feat ) )
    {
      mRubberBand->addGeometry( feat.constGeometry(), vlayer );
    }
  }

  mStartPointMapCoords = toMapCoordinates( e->pos() );
  mRubberBand->setColor( QColor( 255, 0, 0, 65 ) );
  mRubberBand->setWidth( 2 );
  mRubberBand->show();

}

void QgsMapToolMoveFeature::canvasReleaseEvent( QMouseEvent * e )
{
  //QgsDebugMsg("entering.");
  if ( !mRubberBand )
  {
    return;
  }

  QgsVectorLayer* vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    return;
  }

  QgsPoint startPointLayerCoords = toLayerCoordinates(( QgsMapLayer* )vlayer, mStartPointMapCoords );
  QgsPoint stopPointLayerCoords = toLayerCoordinates(( QgsMapLayer* )vlayer, e->pos() );

  double dx = stopPointLayerCoords.x() - startPointLayerCoords.x();
  double dy = stopPointLayerCoords.y() - startPointLayerCoords.y();
  vlayer->beginEditCommand( tr( "Feature moved" ) );
  Q_FOREACH ( QgsFeatureId id, mMovedFeatures )
  {
    vlayer->translateFeature( id, dx, dy );
  }
  delete mRubberBand;
  mRubberBand = 0;
  mCanvas->refresh();
  vlayer->endEditCommand();
}

//! called when map tool is being deactivated
void QgsMapToolMoveFeature::deactivate()
{
  //delete rubber band
  delete mRubberBand;
  mRubberBand = 0;

  QgsMapTool::deactivate();
}
