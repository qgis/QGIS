/***************************************************************************
    qgsmaptooladdisland.cpp  - map tool to add new polygons to multipolygon features
    -----------------------
    begin                : Mai 2007
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
/* $Id$ */

#include "qgsmaptooladdisland.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMessageBox>
#include <QMouseEvent>

QgsMapToolAddIsland::QgsMapToolAddIsland( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas, QgsMapToolCapture::CapturePolygon )
{

}

QgsMapToolAddIsland::~QgsMapToolAddIsland()
{

}

void QgsMapToolAddIsland::canvasReleaseEvent( QMouseEvent * e )
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    QMessageBox::information( 0,
                              tr( "Not a vector layer" ),
                              tr( "The current layer is not a vector layer" ) );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    QMessageBox::information( 0,
                              tr( "Layer not editable" ),
                              tr( "Cannot edit the vector layer. To make it editable, go to the file item "
                                  "of the layer, right click and check 'Allow Editing'." ) );
    return;
  }

  //inform user at the begin of the digitising action that the island tool only works if exactly one feature is selected
  int nSelectedFeatures = vlayer->selectedFeatureCount();
  QString selectionErrorMsg;
  if ( nSelectedFeatures < 1 )
  {
    selectionErrorMsg = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table" );
  }
  else if ( nSelectedFeatures > 1 )
  {
    selectionErrorMsg = tr( "Several features are selected. Please select only one feature to which an island should be added." );
  }

  if ( !selectionErrorMsg.isEmpty() )
  {
    QMessageBox::critical( 0, tr( "Error, could not add island" ), selectionErrorMsg );
    mCaptureList.clear();
    delete mRubberBand;
    mRubberBand = 0;
    return;
  }

  //add point to list and to rubber band
  int error = addVertex( e->pos() );
  if ( error == 1 )
  {
    //current layer is not a vector layer
    return;
  }
  else if ( error == 2 )
  {
    //problem with coordinate transformation
    QMessageBox::information( 0,
                              tr( "Coordinate transform error" ),
                              tr( "Cannot transform the point to the layers coordinate system" ) );
    return;
  }

  if ( e->button() == Qt::LeftButton )
  {
    mCapturing = TRUE;
  }
  else if ( e->button() == Qt::RightButton )
  {
    mCapturing = FALSE;
    delete mRubberBand;
    mRubberBand = 0;

    //close polygon
    mCaptureList.push_back( *mCaptureList.begin() );
    vlayer->beginEditCommand( tr( "Island added" ) );
    int errorCode = vlayer->addIsland( mCaptureList );
    vlayer->endEditCommand();
    QString errorMessage;

    if ( errorCode != 0 )
    {
      if ( errorCode == 1 )
      {
        errorMessage = tr( "Selected feature is not a multipolygon" );
      }
      else if ( errorCode == 2 )
      {
        errorMessage = tr( "New ring is not a valid geometry" );
      }
      else if ( errorCode == 3 )
      {
        errorMessage = tr( "New polygon ring not disjoint with existing polygons" );
      }
      else if ( errorCode == 4 )
      {
        errorMessage = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table" );
      }
      else if ( errorCode == 5 )
      {
        errorMessage = tr( "Several features are selected. Please select only one feature to which an island should be added." );
      }
      else if ( errorCode == 6 )
      {
        errorMessage = tr( "Selected geometry could not be found" );
      }
      QMessageBox::critical( 0, tr( "Error, could not add island" ), errorMessage );
    }
    else
    {
      //add points to other features to keep topology up-to-date
      int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
      if ( topologicalEditing )
      {
        addTopologicalPoints( mCaptureList );
      }
    }

    mCaptureList.clear();
    mCanvas->refresh();
  }
}
