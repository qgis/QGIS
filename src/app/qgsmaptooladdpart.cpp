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
#include "qgscurvepolygonv2.h"
#include "qgsgeometry.h"
#include "qgslinestringv2.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QMouseEvent>

QgsMapToolAddPart::QgsMapToolAddPart( QgsMapCanvas* canvas )
    : QgsMapToolCapture( canvas )
{
  mToolName = tr( "Add part" );
}

QgsMapToolAddPart::~QgsMapToolAddPart()
{
}

void QgsMapToolAddPart::canvasMapReleaseEvent( QgsMapMouseEvent * e )
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
    emit messageEmitted( tr( "Could not add part. %1" ).arg( selectionErrorMsg ), QgsMessageBar::WARNING );
    stopCapturing();
    return;
  }

  int errorCode = 0;
  switch ( mode() )
  {
    case CapturePoint:
    {
      QgsPoint layerPoint;
      QgsPoint mapPoint = e->mapPoint();

      if ( nextPoint( mapPoint, layerPoint ) != 0 )
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
        int error = addVertex( e->mapPoint() );
        if ( error == 1 )
        {
          QgsDebugMsg( "current layer is not a vector layer" );
          return;
        }
        else if ( error == 2 )
        {
          //problem with coordinate transformation
          emit messageEmitted( tr( "Coordinate transform error. Cannot transform the point to the layers coordinate system" ), QgsMessageBar::WARNING );
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

      if ( !isCapturing() )
        return;

      if ( mode() == CapturePolygon )
      {
        closePolygon();
      }

      //does compoundcurve contain circular strings?
      //does provider support circular strings?
      bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
      bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::CircularGeometries;

      QgsCurveV2* curveToAdd = 0;
      if ( hasCurvedSegments && providerSupportsCurvedSegments )
      {
        curveToAdd = dynamic_cast<QgsCurveV2*>( captureCurve()->clone() );
      }
      else
      {
        curveToAdd = captureCurve()->curveToLine();
      }

      vlayer->beginEditCommand( tr( "Part added" ) );
      if ( mode() == CapturePolygon )
      {
        //avoid intersections
        QgsCurvePolygonV2* cp = new QgsCurvePolygonV2();
        cp->setExteriorRing( curveToAdd );
        QgsGeometry* geom = new QgsGeometry( cp );
        geom->avoidIntersections();

        const QgsCurvePolygonV2* cpGeom = dynamic_cast<const QgsCurvePolygonV2*>( geom->geometry() );
        if ( !cpGeom )
        {
          stopCapturing();
          delete geom;
          vlayer->destroyEditCommand();
          return;
        }

        errorCode = vlayer->addPart( dynamic_cast<QgsCurveV2*>( cpGeom->exteriorRing()->clone() ) );
        delete geom;
      }
      else
      {
        errorCode = vlayer->addPart( curveToAdd );
      }
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
      // remove previous message
      emit messageDiscarded();

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

  emit messageEmitted( errorMessage, QgsMessageBar::WARNING );
  vlayer->destroyEditCommand();
}
