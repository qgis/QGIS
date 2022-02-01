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
#include "qgsvectordataprovider.h"
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

bool QgsMapToolAddRing::supportsTechnique( QgsMapToolCapture::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case QgsMapToolCapture::StraightSegments:
    case QgsMapToolCapture::Streaming:
    case QgsMapToolCapture::CircularString:
      return true;
  }
  return false;
}

void QgsMapToolAddRing::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{

  emit messageDiscarded();

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

  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
    if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system." ), Qgis::MessageLevel::Warning );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    if ( !isCapturing() )
      return;

    deleteTempRubberBand();

    closePolygon();

    vlayer->beginEditCommand( tr( "Ring added" ) );

    //does compoundcurve contain circular strings?
    //does provider support circular strings?
    const bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
    const bool providerSupportsCurvedSegments = vlayer->dataProvider()->capabilities() & QgsVectorDataProvider::CircularGeometries;

    QgsCurve *curveToAdd = nullptr;
    if ( hasCurvedSegments && providerSupportsCurvedSegments )
    {
      curveToAdd = captureCurve()->clone();
    }
    else
    {
      curveToAdd = captureCurve()->curveToLine();
    }

    const Qgis::GeometryOperationResult addRingReturnCode = vlayer->addRing( curveToAdd );
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

    stopCapturing();
  }
}
