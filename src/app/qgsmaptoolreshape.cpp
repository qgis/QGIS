/***************************************************************************
    qgsmaptoolreshape.cpp
    ---------------------------
    begin                : Juli 2009
    copyright            : (C) 2009 by Marco Hugentobler
    email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptoolreshape.h"

#include "qgisapp.h"
#include "qgsavoidintersectionsoperation.h"
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsmapmouseevent.h"
#include "qgsmultipoint.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayereditutils.h"

#include "moc_qgsmaptoolreshape.cpp"

QgsMapToolReshape::QgsMapToolReshape( QgsMapCanvas *canvas )
  : QgsMapToolCapture( canvas, QgisApp::instance()->cadDockWidget(), QgsMapToolCapture::CaptureLine )
{
  mToolName = tr( "Reshape features" );
}

void QgsMapToolReshape::cadCanvasReleaseEvent( QgsMapMouseEvent *e )
{
  //check if we operate on a vector layer //todo: move this to a function in parent class to avoid duplication
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

  //add point to list and to rubber band
  if ( e->button() == Qt::LeftButton )
  {
    const int error = addVertex( e->mapPoint(), e->mapPointMatch() );
    if ( error == 2 )
    {
      //problem with coordinate transformation
      emit messageEmitted( tr( "Cannot transform the point to the layers coordinate system" ), Qgis::MessageLevel::Warning );
      return;
    }

    startCapturing();
  }
  else if ( e->button() == Qt::RightButton )
  {
    deleteTempRubberBand();

    if ( size() > 1 )
    {
      reshape( vlayer );
    }

    stopCapturing();
  }
}

bool QgsMapToolReshape::supportsTechnique( Qgis::CaptureTechnique technique ) const
{
  switch ( technique )
  {
    case Qgis::CaptureTechnique::StraightSegments:
    case Qgis::CaptureTechnique::CircularString:
    case Qgis::CaptureTechnique::Streaming:
    case Qgis::CaptureTechnique::NurbsCurve:
      return true;

    case Qgis::CaptureTechnique::Shape:
      return false;
  }
  return false;
}

bool QgsMapToolReshape::isBindingLine( QgsVectorLayer *vlayer, const QgsRectangle &bbox ) const
{
  if ( vlayer->geometryType() != Qgis::GeometryType::Line )
    return false;

  bool begin = false;
  bool end = false;
  const QgsPointXY beginPoint = pointsZM().first();
  const QgsPointXY endPoint = pointsZM().last();

  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( bbox ).setNoAttributes() );
  QgsFeature f;

  // check that extremities of the new line are contained by features
  while ( fit.nextFeature( f ) )
  {
    const QgsGeometry geom = f.geometry();
    if ( !geom.isNull() )
    {
      const QgsPolylineXY line = geom.asPolyline();

      if ( line.contains( beginPoint ) )
        begin = true;
      else if ( line.contains( endPoint ) )
        end = true;
    }
  }

  return end && begin;
}

void QgsMapToolReshape::reshape( QgsVectorLayer *vlayer )
{
  const QgsPointXY firstPoint = pointsZM().at( 0 );
  QgsRectangle bbox( firstPoint.x(), firstPoint.y(), firstPoint.x(), firstPoint.y() );
  for ( int i = 1; i < size(); ++i )
  {
    bbox.combineExtentWith( pointsZM().at( i ).x(), pointsZM().at( i ).y() );
  }

  const bool hasCurvedSegments = captureCurve()->hasCurvedSegments();
  QgsPointSequence pts;
  if ( !hasCurvedSegments )
  {
    captureCurve()->points( pts );
  }
  else
  {
    std::unique_ptr<QgsLineString> segmented( captureCurve()->curveToLine() );
    segmented->points( pts );
  }

  const QgsLineString reshapeLineString( pts );

  //query all the features that intersect bounding box of capture line
  QgsFeatureRequest req = QgsFeatureRequest().setFilterRect( bbox ).setNoAttributes();

  if ( vlayer->selectedFeatureCount() > 0 )
    req.setFilterFids( vlayer->selectedFeatureIds() );

  QgsFeatureIterator fit = vlayer->getFeatures( req );

  QgsFeature f;
  Qgis::GeometryOperationResult reshapeReturn = Qgis::GeometryOperationResult::Success;
  bool reshapeDone = false;
  const bool isBinding = isBindingLine( vlayer, bbox );
  QgsAvoidIntersectionsOperation avoidIntersections;
  connect( &avoidIntersections, &QgsAvoidIntersectionsOperation::messageEmitted, this, &QgsMapTool::messageEmitted );

  QHash<QgsFeatureId, QgsGeometry> reshapedGeometries;

  // we first gather the features that are actually going to be reshaped and the reshaped results
  while ( fit.nextFeature( f ) )
  {
    QgsGeometry geom = f.geometry();
    if ( !geom.isNull() )
    {
      // in case of a binding line, we just want to update the line from
      // the starting point and not both side
      if ( isBinding && !geom.asPolyline().contains( pts.constFirst() ) )
        continue;

      reshapeReturn = geom.reshapeGeometry( reshapeLineString );
      if ( reshapeReturn == Qgis::GeometryOperationResult::Success )
      {
        reshapedGeometries.insert( f.id(), geom );
      }
    }
  }
  // ignore features that are going to be reshaped
  // some intersected features may not be reshaped because of active selection or reshape line geometry
  const QHash<QgsVectorLayer *, QSet<QgsFeatureId>> ignoreFeatures { { vlayer, qgis::listToSet( reshapedGeometries.keys() ) } };

  // then we can apply intersection avoidance logic and eventually update the layer
  vlayer->beginEditCommand( tr( "Reshape" ) );
  for ( auto it = reshapedGeometries.begin(); it != reshapedGeometries.end(); ++it )
  {
    QgsFeatureId fid = it.key();
    QgsGeometry geom = it.value();

    //avoid intersections on polygon layers
    if ( vlayer->geometryType() == Qgis::GeometryType::Polygon )
    {
      const QgsAvoidIntersectionsOperation::Result res = avoidIntersections.apply( vlayer, fid, geom, ignoreFeatures );
      if ( res.operationResult == Qgis::GeometryOperationResult::InvalidInputGeometryType )
      {
        emit messageEmitted( tr( "An error was reported during intersection removal" ), Qgis::MessageLevel::Warning );
        vlayer->destroyEditCommand();
        stopCapturing();
        return;
      }

      if ( geom.isEmpty() ) //intersection removal might have removed the whole geometry
      {
        emit messageEmitted( tr( "The feature cannot be reshaped because the resulting geometry is empty" ), Qgis::MessageLevel::Critical );
        vlayer->destroyEditCommand();
        return;
      }
    }

    vlayer->changeGeometry( fid, geom );
    reshapeDone = true;
  }

  if ( reshapeDone )
  {
    // Add topological points
    if ( QgsProject::instance()->topologicalEditing() )
    {
      //check if we need to add topological points to other layers
      const QList<QgsMapLayer *> layers = canvas()->layers( true );
      QgsGeometry pointsAsGeom( new QgsMultiPoint( pts ) );
      QgsVectorLayerEditUtils::addTopologicalPointsToLayers( pointsAsGeom, vlayer, layers, mToolName );
    }

    vlayer->endEditCommand();
  }
  else
  {
    vlayer->destroyEditCommand();
  }
}
