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
  : QgsMapToolCaptureLayerGeometry( canvas, QgisApp::instance()->cadDockWidget(), CaptureNone )
{
  mToolName = tr( "Add part" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddPart::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddPart::stopCapturing );
}

QgsMapToolCapture::Capabilities QgsMapToolAddPart::capabilities() const
{
  return QgsMapToolCapture::SupportsCurves | QgsMapToolCapture::ValidateGeometries;
}

bool QgsMapToolAddPart::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::Streaming:
      return true;

    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Shape:
      return mode() != QgsMapToolCapture::CapturePoint;
  }
  return false;
}

void QgsMapToolAddPart::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  if ( getLayerAndCheckSelection() )
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
  QgsVectorLayer *layer = getLayerAndCheckSelection();
  if ( !layer )
    return;

  QgsMapToolCapture::cadCanvasReleaseEvent( e );
}

void QgsMapToolAddPart::layerPointCaptured( const QgsPoint &point )
{
  QgsVectorLayer *layer = getLayerAndCheckSelection();
  if ( !layer )
    return;
  layer->beginEditCommand( tr( "Part added" ) );
  Qgis::GeometryOperationResult errorCode = layer->addPart( QgsPointSequence() << point );
  finalizeEditCommand( layer, errorCode );
}

void QgsMapToolAddPart::layerLineCaptured( const QgsCurve *line )
{
  QgsVectorLayer *layer = getLayerAndCheckSelection();
  if ( !layer )
    return;
  layer->beginEditCommand( tr( "Part added" ) );
  Qgis::GeometryOperationResult errorCode = layer->addPart( line->clone() );
  finalizeEditCommand( layer, errorCode );
}

void QgsMapToolAddPart::layerPolygonCaptured( const QgsCurvePolygon *polygon )
{
  QgsVectorLayer *layer = getLayerAndCheckSelection();
  if ( !layer )
    return;
  layer->beginEditCommand( tr( "Part added" ) );
  Qgis::GeometryOperationResult errorCode = layer->addPart( polygon->exteriorRing()->clone() );
  finalizeEditCommand( layer, errorCode );
}

void QgsMapToolAddPart::finalizeEditCommand( QgsVectorLayer *layer, Qgis::GeometryOperationResult errorCode )
{
  QString errorMessage;
  switch ( errorCode )
  {
    case Qgis::GeometryOperationResult::Success:
    {
      // remove previous message
      emit messageDiscarded();

      //add points to other features to keep topology up-to-date
      const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
      if ( topologicalEditing )
      {
        addTopologicalPoints( pointsZM() );
      }

      layer->endEditCommand();

      layer->triggerRepaint();

      return;
    }

    case Qgis::GeometryOperationResult::InvalidInputGeometryType:
      errorMessage = tr( "New part's geometry is empty or invalid." );
      break;

    case Qgis::GeometryOperationResult::AddPartNotMultiGeometry:
      errorMessage = tr( "Selected feature is not multi part." );
      break;

    case Qgis::GeometryOperationResult::SelectionIsEmpty:
      errorMessage = tr( "No feature selected. Please select a feature with the selection tool or in the attribute table." );
      break;

    case Qgis::GeometryOperationResult::SelectionIsGreaterThanOne:
      errorMessage = tr( "Several features are selected. Please select only one feature to which an island should be added." );
      break;

    case Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound:
      errorMessage = tr( "Selected geometry could not be found." );
      break;

    case Qgis::GeometryOperationResult::InvalidBaseGeometry:
      errorMessage = tr( "Base geometry is not valid." );
      break;

    case Qgis::GeometryOperationResult::AddRingCrossesExistingRings:
    case Qgis::GeometryOperationResult::AddRingNotClosed:
    case Qgis::GeometryOperationResult::AddRingNotInExistingFeature:
    case Qgis::GeometryOperationResult::AddRingNotValid:
    case Qgis::GeometryOperationResult::GeometryEngineError:
    case Qgis::GeometryOperationResult::LayerNotEditable:
    case Qgis::GeometryOperationResult::NothingHappened:
    case Qgis::GeometryOperationResult::SplitCannotSplitPoint:
      // Should not reach here
      // Other OperationResults should not be returned by addPart
      errorMessage = tr( "Unexpected OperationResult: %1" ).arg( qgsEnumValueToKey( errorCode ) );
  }

  emit messageEmitted( errorMessage, Qgis::MessageLevel::Warning );
  layer->destroyEditCommand();
}

void QgsMapToolAddPart::activate()
{
  getLayerAndCheckSelection();
  QgsMapToolCapture::activate();
}

QgsVectorLayer *QgsMapToolAddPart::getLayerAndCheckSelection()
{
  //check if we operate on a vector layer
  QgsVectorLayer *layer = currentVectorLayer();
  if ( !layer )
  {
    notifyNotVectorLayer();
    return nullptr;
  }

  if ( !layer->isEditable() )
  {
    notifyNotEditableLayer();
    return nullptr;
  }

  //inform user at the begin of the digitizing action that the island tool only works if exactly one feature is selected
  const int nSelectedFeatures = layer->selectedFeatureCount();
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
    QgsFeatureIterator selectedFeatures = layer->getSelectedFeatures();
    QgsFeature selectedFeature;
    selectedFeatures.nextFeature( selectedFeature );
    if ( QgsWkbTypes::isSingleType( layer->wkbType() ) &&
         selectedFeature.geometry().constGet() )
    {
      selectionErrorMsg = tr( "This layer does not support multipart geometries." );
    }
  }

  if ( !selectionErrorMsg.isEmpty() )
  {
    emit messageEmitted( tr( "Could not add part. %1" ).arg( selectionErrorMsg ), Qgis::MessageLevel::Warning );
  }

  if ( selectionErrorMsg.isEmpty() )
    return layer;
  else
    return nullptr;
}

