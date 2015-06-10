/***************************************************************************
    qgsmaptoolreshape.cpp
    ---------------------------
    begin                : Juli 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolreshape.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolReshape::QgsMapToolReshape( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas, QgsMapToolCapture::CaptureLine )
{
}

QgsMapToolReshape::~QgsMapToolReshape()
{
}

void QgsMapToolReshape::canvasMapReleaseEvent( QgsMapMouseEvent * e )
{
  //check if we operate on a vector layer //todo: move this to a function in parent class to avoid duplication
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

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

  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    int error = addVertex( e->mapPoint() );
    if ( error == 1 )
    {
      //current layer is not a vector layer
      return;
    }
    else if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), QgsMessageBar::WARNING );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    deleteTempRubberBand();

    //find out bounding box of mCaptureList
    if ( size() < 1 )
    {
      stopCapturing();
      return;
    }
    QgsPoint firstPoint = points().at( 0 );
    QgsRectangle bbox( firstPoint.x(), firstPoint.y(), firstPoint.x(), firstPoint.y() );
    for ( int i = 1; i < size(); ++i )
    {
      bbox.combineExtentWith( points().at( i ).x(), points().at( i ).y() );
    }

    //query all the features that intersect bounding box of capture line
    QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( bbox ).setSubsetOfAttributes( QgsAttributeList() ) );
    QgsFeature f;
    int reshapeReturn;
    bool reshapeDone = false;

    vlayer->beginEditCommand( tr( "Reshape" ) );
    while ( fit.nextFeature( f ) )
    {
      //query geometry
      //call geometry->reshape(mCaptureList)
      //register changed geometry in vector layer
      QgsGeometry* geom = f.geometry();
      if ( geom )
      {
        reshapeReturn = geom->reshapeGeometry( points() );
        if ( reshapeReturn == 0 )
        {
          vlayer->changeGeometry( f.id(), geom );
          reshapeDone = true;
        }
      }
    }

    if ( reshapeDone )
    {
      vlayer->endEditCommand();
    }
    else
    {
      vlayer->destroyEditCommand();
    }

    stopCapturing();
  }
}
