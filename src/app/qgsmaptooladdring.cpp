/***************************************************************************
    qgsmaptooladdring.cpp  - map tool to cut rings in polygon and multipolygon features
    ---------------------
    begin                : April 2007
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

#include "qgsmaptooladdring.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMessageBox>
#include <QMouseEvent>

QgsMapToolAddRing::QgsMapToolAddRing( QgsMapCanvas* canvas ): QgsMapToolCapture( canvas, QgsMapToolCapture::CapturePolygon )
{

}

QgsMapToolAddRing::~QgsMapToolAddRing()
{

}

void QgsMapToolAddRing::canvasReleaseEvent( QMouseEvent * e )
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
  {
    QMessageBox::information( 0, tr( "Not a vector layer" ),
                              tr( "The current layer is not a vector layer" ) );
    return;
  }

  if ( !vlayer->isEditable() )
  {
    QMessageBox::information( 0, tr( "Layer not editable" ),
                              tr( "Cannot edit the vector layer. To make it editable, go to the file item "
                                  "of the layer, right click and check 'Allow Editing'." ) );
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
    QMessageBox::information( 0, tr( "Coordinate transform error" ),
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

    vlayer->beginEditCommand( tr( "Ring added" ) );
    int addRingReturnCode = vlayer->addRing( mCaptureList );
    vlayer->endEditCommand();
    if ( addRingReturnCode != 0 )
    {
      QString errorMessage;
      //todo: open message box to communicate errors
      if ( addRingReturnCode == 1 )
      {
        errorMessage = tr( "A problem with geometry type occured" );
      }
      else if ( addRingReturnCode == 2 )
      {
        errorMessage = tr( "The inserted Ring is not closed" );
      }
      else if ( addRingReturnCode == 3 )
      {
        errorMessage = tr( "The inserted Ring is not a valid geometry" );
      }
      else if ( addRingReturnCode == 4 )
      {
        errorMessage = tr( "The inserted Ring crosses existing rings" );
      }
      else if ( addRingReturnCode == 5 )
      {
        errorMessage = tr( "The inserted Ring is not contained in a feature" );
      }
      else
      {
        errorMessage = tr( "An unknown error occured" );
      }
      QMessageBox::critical( 0, tr( "Error, could not add ring" ), errorMessage );
    }
    mCaptureList.clear();
    mCanvas->refresh();
  }
}
