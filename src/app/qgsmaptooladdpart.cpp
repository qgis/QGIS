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
#include "qgsmapmouseevent.h"


QgsMapToolAddPart::QgsMapToolAddPart( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), CaptureNone )
{
  mToolName = tr( "Add part" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddPart::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddPart::stopCapturing );
}

QgsMapToolCapture::Capabilities QgsMapToolAddPart::capabilities() const
{
  return QgsMapToolCapture::SupportsCurves;
}

bool QgsMapToolAddPart::supportsTechnique( QgsMapToolCapture::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case QgsMapToolCapture::StraightSegments:
    case QgsMapToolCapture::Streaming:
      return true;

    case QgsMapToolCapture::CircularString:
      return false;
  }
  return false;
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

  if ( !checkSelection() )
  {
    stopCapturing();
    return;
  }

  QgsGeometry::OperationResult errorCode = QgsGeometry::Success;
  switch ( mode() )
  {
    case CapturePoint:
    {
      QgsPoint layerPoint;
      QgsPointXY mapPoint = e->mapPoint();

      if ( nextPoint( QgsPoint( mapPoint ), layerPoint ) != 0 )
      {
        QgsDebugMsg( QStringLiteral( "nextPoint failed" ) );
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
          QgsDebugMsg( QStringLiteral( "current layer is not a vector layer" ) );
          return;
        }
        else if ( error == 2 )
        {
          //problem with coordinate transformation
          emit messageEmitted( tr( "Coordinate transform error. Cannot transform the point to the layers coordinate system" ), Qgis::MessageLevel::Warning );
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

        QList<QgsVectorLayer *>  avoidIntersectionsLayers;
        switch ( QgsProject::instance()->avoidIntersectionsMode() )
        {
          case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsCurrentLayer:
            avoidIntersectionsLayers.append( vlayer );
            break;
          case QgsProject::AvoidIntersectionsMode::AvoidIntersectionsLayers:
            avoidIntersectionsLayers = QgsProject::instance()->avoidIntersectionsLayers();
            break;
          case QgsProject::AvoidIntersectionsMode::AllowIntersections:
            break;
        }
        if ( !avoidIntersectionsLayers.isEmpty() )
        {
          geom->avoidIntersections( avoidIntersectionsLayers );
        }

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
      errorCode = QgsGeometry::OperationResult::AddPartSelectedGeometryNotFound;
      break;
  }

  QString errorMessage;
  switch ( errorCode )
  {
    case QgsGeometry::OperationResult::Success:
    {
      // remove previous message
      emit messageDiscarded();

      //add points to other features to keep topology up-to-date
      bool topologicalEditing = QgsProject::instance()->topologicalEditing();
      if ( topologicalEditing )
      {
        addTopologicalPoints( pointsZM() );
      }

      vlayer->endEditCommand();

      vlayer->triggerRepaint();

      return;
    }

    case QgsGeometry::OperationResult::InvalidInputGeometryType:
      errorMessage = tr( "New part's geometry is empty or invalid." );
      break;

    case QgsGeometry::OperationResult::AddPartNotMultiGeometry:
      errorMessage = tr( "Selected feature is not multi part." );
      break;

    case QgsGeometry::OperationResult::SelectionIsEmpty:
      errorMessage = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table." );
      break;

    case QgsGeometry::OperationResult::SelectionIsGreaterThanOne:
      errorMessage = tr( "Several features are selected. Please select only one feature to which an island should be added." );
      break;

    case QgsGeometry::OperationResult::AddPartSelectedGeometryNotFound:
      errorMessage = tr( "Selected geometry could not be found." );
      break;

    case QgsGeometry::OperationResult::InvalidBaseGeometry:
      errorMessage = tr( "Base geometry is not valid." );
      break;

    case QgsGeometry::OperationResult::AddRingCrossesExistingRings:
    case QgsGeometry::OperationResult::AddRingNotClosed:
    case QgsGeometry::OperationResult::AddRingNotInExistingFeature:
    case QgsGeometry::OperationResult::AddRingNotValid:
    case QgsGeometry::OperationResult::GeometryEngineError:
    case QgsGeometry::OperationResult::LayerNotEditable:
    case QgsGeometry::OperationResult::NothingHappened:
    case QgsGeometry::OperationResult::SplitCannotSplitPoint:
      // Should not reach here
      // Other OperationResults should not be returned by addPart
      errorMessage = tr( "Unexpected OperationResult: %1" ).arg( errorCode );
  }

  emit messageEmitted( errorMessage, Qgis::MessageLevel::Warning );
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
    selectionErrorMsg = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table." );
  }
  else if ( nSelectedFeatures > 1 )
  {
    selectionErrorMsg = tr( "Several features are selected. Please select only one feature to which a part should be added." );
  }
  else
  {
    // Only one selected feature
    // For single-type layers only allow features without geometry
    QgsFeatureIterator selectedFeatures = vlayer->getSelectedFeatures();
    QgsFeature selectedFeature;
    selectedFeatures.nextFeature( selectedFeature );
    if ( QgsWkbTypes::isSingleType( vlayer->wkbType() ) &&
         selectedFeature.geometry().constGet() )
    {
      selectionErrorMsg = tr( "This layer does not support multipart geometries." );
    }
  }

  if ( !selectionErrorMsg.isEmpty() )
  {
    emit messageEmitted( tr( "Could not add part. %1" ).arg( selectionErrorMsg ), Qgis::MessageLevel::Warning );
  }

  return selectionErrorMsg.isEmpty();
}
