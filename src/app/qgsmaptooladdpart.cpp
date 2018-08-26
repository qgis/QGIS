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
#include "qgsadvanceddigitizingdockwidget.h"
#include "qgscurvepolygon.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectordataprovider.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"
#include "qgisapp.h"

#include <QMouseEvent>

QgsMapToolAddPart::QgsMapToolAddPart( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), CaptureNone )
{
  mToolName = tr( "Add part" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddPart::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddPart::stopCapturing );
}

void QgsMapToolAddPart::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( checkSelection() )
  {
    QgsMapToolAdvancedDigitizing::canvasReleaseEvent( e );
  }
  else
  {
    cadDockWidget()->clear();
  }
}

void QgsMapToolAddPart::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = currentVectorLayer();
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

  bool isGeometryEmpty = false;
  if ( vlayer->selectedFeatureCount() > 0 )
  {
    // be efficient here - only grab the first selected feature if there's a selection, don't
    // fetch all the other features which we don't require.
    QgsFeatureIterator selectedFeatures = vlayer->getSelectedFeatures();
    QgsFeature firstSelectedFeature;
    if ( selectedFeatures.nextFeature( firstSelectedFeature ) )
      if ( !firstSelectedFeature.geometry().isNull() )
        isGeometryEmpty = true;
  }

  if ( !checkSelection() )
  {
    stopCapturing();
    return;
  }

  int errorCode = 0;
  switch ( mode() )
  {
    case CapturePoint:
    {
      QgsPoint layerPoint;
      QgsPointXY mapPoint = e->mapPoint();

      if ( nextPoint( QgsPoint( mapPoint ), layerPoint ) != 0 )
      {
        QgsDebugMsg( "nextPoint failed" );
        return;
      }

      vlayer->beginEditCommand( tr( "Part added" ) );
      errorCode = vlayer->addPart( QgsPointSequence() << layerPoint );
    }
    break;

    case CaptureLine:
    case CapturePolygon:
    {
      //add point to list and to rubber band
      if ( e->button() == Qt::LeftButton )
      {
        int error = addVertex( e->mapPoint(), e->mapPointMatch() );
        if ( error == 1 )
        {
          QgsDebugMsg( "current layer is not a vector layer" );
          return;
        }
        else if ( error == 2 )
        {
          //problem with coordinate transformation
          emit messageEmitted( tr( "Coordinate transform error. Cannot transform the point to the layers coordinate system" ), Qgis::Warning );
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

      QgsCurve *curveToAdd = nullptr;
      if ( hasCurvedSegments && providerSupportsCurvedSegments )
      {
        curveToAdd = captureCurve()->clone();
      }
      else
      {
        curveToAdd = captureCurve()->curveToLine();
      }

      vlayer->beginEditCommand( tr( "Part added" ) );
      if ( mode() == CapturePolygon )
      {
        //avoid intersections
        QgsCurvePolygon *cp = new QgsCurvePolygon();
        cp->setExteriorRing( curveToAdd );
        QgsGeometry *geom = new QgsGeometry( cp );
        geom->avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers() );

        const QgsCurvePolygon *cpGeom = qgsgeometry_cast<const QgsCurvePolygon *>( geom->constGet() );
        if ( !cpGeom )
        {
          stopCapturing();
          delete geom;
          vlayer->destroyEditCommand();
          return;
        }

        errorCode = vlayer->addPart( cpGeom->exteriorRing()->clone() );
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
      bool topologicalEditing = QgsProject::instance()->topologicalEditing();
      if ( topologicalEditing )
      {
        addTopologicalPoints( points() );
      }

      vlayer->endEditCommand();

      vlayer->triggerRepaint();

      if ( ( !isGeometryEmpty ) && QgsWkbTypes::isSingleType( vlayer->wkbType() ) )
      {
        emit messageEmitted( tr( "Add part: Feature geom is single part and you've added more than one" ), Qgis::Warning );
      }

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

  emit messageEmitted( errorMessage, Qgis::Warning );
  vlayer->destroyEditCommand();
}

void QgsMapToolAddPart::activate()
{
  checkSelection();
  QgsMapToolCapture::activate();
}

bool QgsMapToolAddPart::checkSelection()
{
  //check if we operate on a vector layer
  QgsVectorLayer *vlayer = currentVectorLayer();
  if ( !vlayer )
  {
    notifyNotVectorLayer();
    return false;
  }

  //inform user at the begin of the digitizing action that the island tool only works if exactly one feature is selected
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
    emit messageEmitted( tr( "Could not add part. %1" ).arg( selectionErrorMsg ), Qgis::Warning );
  }

  return selectionErrorMsg.isEmpty();
}
