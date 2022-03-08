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


#include "qgsmaptooladdring.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgscurvepolygon.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"


QgsMapToolAddRing::QgsMapToolAddRing( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CapturePolygon )
{
  mToolName = tr( "Add ring" );
  connect( QgisApp::instance(), &QgisApp::newProject, this, &QgsMapToolAddRing::stopCapturing );
  connect( QgisApp::instance(), &QgisApp::projectRead, this, &QgsMapToolAddRing::stopCapturing );
}

QgsMapToolCapture::Capabilities QgsMapToolAddRing::capabilities() const
{
  return QgsMapToolCapture::SupportsCurves | QgsMapToolCapture::ValidateGeometries;
}

bool QgsMapToolAddRing::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Shape:
      return true;
  }
  return false;
}

void QgsMapToolAddRing::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  emit messageDiscarded();

  getCheckLayer();

  QgsMapToolCapture::cadCanvasReleaseEvent( e );
}


void QgsMapToolAddRing::polygonCaptured( const QgsCurvePolygon *polygon )
{
  QgsVectorLayer *vlayer = getCheckLayer();
  if ( !vlayer )
    return;

  vlayer->beginEditCommand( tr( "Ring added" ) );
  const Qgis::GeometryOperationResult addRingReturnCode = vlayer->addRing( polygon->exteriorRing()->clone() );
  QString errorMessage;
  switch ( addRingReturnCode )
  {
    case Qgis::GeometryOperationResult::Success:
      break;
    case Qgis::GeometryOperationResult::InvalidInputGeometryType:
      errorMessage = tr( "a problem with geometry type occurred" );
      break;
    case Qgis::GeometryOperationResult::AddRingNotClosed:
      errorMessage = tr( "the inserted ring is not closed" );
      break;
    case Qgis::GeometryOperationResult::AddRingNotValid:
      errorMessage = tr( "the inserted ring is not a valid geometry" );
      break;
    case Qgis::GeometryOperationResult::AddRingCrossesExistingRings:
      errorMessage = tr( "the inserted ring crosses existing rings" );
      break;
    case Qgis::GeometryOperationResult::AddRingNotInExistingFeature:
      errorMessage = tr( "the inserted ring is not contained in a feature" );
      break;
    case Qgis::GeometryOperationResult::SplitCannotSplitPoint:
    case Qgis::GeometryOperationResult::InvalidBaseGeometry:
    case Qgis::GeometryOperationResult::NothingHappened:
    case Qgis::GeometryOperationResult::SelectionIsEmpty:
    case Qgis::GeometryOperationResult::SelectionIsGreaterThanOne:
    case Qgis::GeometryOperationResult::GeometryEngineError:
    case Qgis::GeometryOperationResult::LayerNotEditable:
    case Qgis::GeometryOperationResult::AddPartSelectedGeometryNotFound:
    case Qgis::GeometryOperationResult::AddPartNotMultiGeometry:
      errorMessage = tr( "an unknown error occurred (%1)" ).arg( qgsEnumValueToKey( addRingReturnCode ) );
      break;
  }

  if ( addRingReturnCode != Qgis::GeometryOperationResult::Success )
  {
    emit messageEmitted( tr( "Could not add ring: %1." ).arg( errorMessage ), Qgis::MessageLevel::Critical );
    vlayer->destroyEditCommand();
  }
  else
  {
    vlayer->endEditCommand();
  }
}

QgsVectorLayer *QgsMapToolAddRing::getCheckLayer()
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

  return layer;
}
