/***************************************************************************
    qgsmaptooladdpart.cpp  - map tool to add new parts to multipart features
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

#include "qgsmaptooladdpart.h"
#include "qgsgeometry.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QMessageBox>
#include <QMouseEvent>

QgsMapToolAddPart::QgsMapToolAddPart( QgsMapCanvas* canvas )
    : QgsMapToolCapture( canvas )
{
}

QgsMapToolAddPart::~QgsMapToolAddPart()
{
}

void QgsMapToolAddPart::canvasReleaseEvent( QMouseEvent * e )
{
  //check if we operate on a vector layer
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

  //inform user at the begin of the digitising action that the island tool only works if exactly one feature is selected
  int nSelectedFeatures = vlayer->selectedFeatureCount();
  QString selectionErrorMsg;
  if ( nSelectedFeatures < 1 )
  {
    selectionErrorMsg = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table" );
  }
  else if ( nSelectedFeatures > 1 )
  {
    selectionErrorMsg = tr( "Several features are selected. Please select only one feature to which an part should be added." );
  }

  if ( !selectionErrorMsg.isEmpty() )
  {
    QMessageBox::critical( 0, tr( "Error. Could not add part." ), selectionErrorMsg );
    stopCapturing();
    return;
  }

  int errorCode;
  switch ( mode() )
  {
    case CapturePoint:
    {
      QgsPoint layerPoint;
      QgsPoint mapPoint;

      if ( nextPoint( e->pos(), layerPoint, mapPoint ) != 0 )
      {
        QgsDebugMsg( "nextPoint failed" );
        return;
      }

      vlayer->beginEditCommand( tr( "Part added" ) );
      errorCode = vlayer->addPart( QList<QgsPoint>() << layerPoint );
    }
    break;

    case CaptureLine:
    case CapturePolygon:
    {
      //add point to list and to rubber band
      if ( e->button() == Qt::LeftButton )
      {
       int error = addVertex( e->pos() );
        if ( error == 1 )
        {
          QgsDebugMsg( "current layer is not a vector layer" );
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

        startCapturing();
        return;
      }
      else if ( e->button() != Qt::RightButton )
      {
        deleteTempRubberBand();

        return;
      }

      if ( mode() == CapturePolygon )
      {
        //close polygon
        closePolygon();
        //avoid intersections
        QgsGeometry* geom = QgsGeometry::fromPolygon( QgsPolygon() << points().toVector() );
        if ( geom )
        {
          geom->avoidIntersections();
          QgsPolygon poly = geom->asPolygon();
          if ( poly.size() < 1 )
          {
            stopCapturing();
            delete geom;
            vlayer->destroyEditCommand();
            return;
          }
          setPoints( geom->asPolygon()[0].toList() );
          delete geom;
        }
      }

      vlayer->beginEditCommand( tr( "Part added" ) );
      errorCode = vlayer->addPart( points() );

      stopCapturing();
    }
    break;
    default:
      Q_ASSERT( !"invalid capture mode" );
      errorCode = 6;
      break;
  }

  QString errorMessage;
  switch ( errorCode )
  {
    case 0:
    {
      //add points to other features to keep topology up-to-date
      int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
      if ( topologicalEditing )
      {
        addTopologicalPoints( points() );
      }

      vlayer->endEditCommand();

      mCanvas->refresh();
      return;
    }

    case 1:
      errorMessage = tr( "Selected feature is not multi part." );
      break;

    case 2:
      errorMessage = tr( "New part's geometry is not valid." );
      break;

    case 3:
      errorMessage = tr( "New polygon ring not disjoint with existing polygons." );
      break;

    case 4:
      errorMessage = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table" );
      break;

    case 5:
      errorMessage = tr( "Several features are selected. Please select only one feature to which an island should be added." );
      break;

    case 6:
      errorMessage = tr( "Selected geometry could not be found" );
      break;
  }

  QMessageBox::critical( 0, tr( "Error, could not add part" ), errorMessage );
  vlayer->destroyEditCommand();
}
