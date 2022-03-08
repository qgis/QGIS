/***************************************************************************
    qgsmaptoolsplitfeatures.cpp
    ---------------------------
    begin                : August 2007
    copyright            : (C) 2007 by Marco Hugentobler
    email                : marco.hugentobler@karto.baug.ethz.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgisapp.h"
#include "qgsmessagebar.h"
#include "qgsmapcanvas.h"
#include "qgsmaptoolsplitfeatures.h"
#include "qgsproject.h"
#include "qgssnappingutils.h"
#include "qgsvectorlayer.h"
#include "qgsmapmouseevent.h"


QgsMapToolSplitFeatures::QgsMapToolSplitFeatures( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine )
{
  mToolName = tr( "Split features" );
  setSnapToLayerGridEnabled( false );
}

bool QgsMapToolSplitFeatures::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
      return true;

    case Qgis::CaptureTechnique::Shape:
      return false;
  }
  return false;
}

void QgsMapToolSplitFeatures::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
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

  bool split = false;


  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    //If we snap the first point on a vertex of a line layer, we directly split the feature at this point
    if ( vlayer->geometryType() == QgsWkbTypes::LineGeometry && pointsZM().isEmpty() )
    {
      const QgsPointLocator::Match m = mCanvas->snappingUtils()->snapToCurrentLayer( e->pos(), QgsPointLocator::Vertex );
      if ( m.isValid() )
      {
        split = true;
      }
    }

    const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
    if ( error == 2 )
    {
      //problem with coordinate transformation
      QgisApp::instance()->messageBar()->pushMessage(
        tr( "Coordinate transform error" ),
        tr( "Cannot transform the point to the layers coordinate system" ),
        Qgis::MessageLevel::Info );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    split = true;
  }

  if ( split )
  {
    deleteTempRubberBand();

    //bring up dialog if a split was not possible (polygon) or only done once (line)
    const bool topologicalEditing = QgsProject::instance()->topologicalEditing();
    QgsPointSequence topologyTestPoints;
    vlayer->beginEditCommand( tr( "Features split" ) );
    const Qgis::GeometryOperationResult returnCode = vlayer->splitFeatures( captureCurve(), topologyTestPoints, true, topologicalEditing );
    if ( returnCode == Qgis::GeometryOperationResult::Success )
    {
      vlayer->endEditCommand();
    }
    else
    {
      vlayer->destroyEditCommand();
    }

    switch ( returnCode )
    {
      case Qgis::GeometryOperationResult::Success:
        if ( topologicalEditing == true &&
             ! topologyTestPoints.isEmpty() )
        {
          //check if we need to add topological points to other layers
          const auto layers = canvas()->layers( true );
          for ( QgsMapLayer *layer : layers )
          {
            QgsVectorLayer *vectorLayer = qobject_cast<QgsVectorLayer *>( layer );
            if ( vectorLayer &&
                 vectorLayer->isEditable() &&
                 vectorLayer->isSpatial() &&
                 vectorLayer != vlayer &&
                 ( vectorLayer->geometryType() == QgsWkbTypes::LineGeometry ||
                   vectorLayer->geometryType() == QgsWkbTypes::PolygonGeometry ) )
            {
              vectorLayer->beginEditCommand( tr( "Topological points from Features split" ) );
              const int returnValue = vectorLayer->addTopologicalPoints( topologyTestPoints );
              if ( returnValue == 0 )
              {
                vectorLayer->endEditCommand();
              }
              else
              {
                // the layer was not modified, leave the undo buffer intact
                vectorLayer->destroyEditCommand();
              }
            }
          }
        }
        break;
      case Qgis::GeometryOperationResult::NothingHappened:
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "No features were split" ),
          tr( "If there are selected features, the split tool only applies to those. If you would like to split all features under the split line, clear the selection." ),
          Qgis::MessageLevel::Warning );
        break;
      case Qgis::GeometryOperationResult::GeometryEngineError:
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "No feature split done" ),
          tr( "Cut edges detected. Make sure the line splits features into multiple parts." ),
          Qgis::MessageLevel::Warning );
        break;
      case Qgis::GeometryOperationResult::InvalidBaseGeometry:
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "No feature split done" ),
          tr( "The geometry is invalid. Please repair before trying to split it." ),
          Qgis::MessageLevel::Warning );
        break;
      default:
        //several intersections but only one split (most likely line)
        QgisApp::instance()->messageBar()->pushMessage(
          tr( "No feature split done" ),
          tr( "An error occurred during splitting." ),
          Qgis::MessageLevel::Warning );
        break;
    }
    stopCapturing();
  }
}
