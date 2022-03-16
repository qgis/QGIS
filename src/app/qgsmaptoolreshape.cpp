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
#include "qgsfeatureiterator.h"
#include "qgsgeometry.h"
#include "qgslinestring.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"

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

    //find out bounding box of mCaptureList
    if ( size() < 1 )
    {
      stopCapturing();
      return;
    }

    reshape( vlayer );

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
      return true;

    case Qgis::CaptureTechnique::Shape:
      return false;
  }
  return false;
}

bool QgsMapToolReshape::isBindingLine( QgsVectorLayer *vlayer, const QgsRectangle &bbox ) const
{
  if ( vlayer->geometryType() != QgsWkbTypes::LineGeometry )
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
    std::unique_ptr< QgsLineString > segmented( captureCurve()->curveToLine() );
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

  vlayer->beginEditCommand( tr( "Reshape" ) );
  while ( fit.nextFeature( f ) )
  {
    //query geometry
    //call geometry->reshape(mCaptureList)
    //register changed geometry in vector layer
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
        //avoid intersections on polygon layers
        if ( vlayer->geometryType() == QgsWkbTypes::PolygonGeometry )
        {
          //ignore all current layer features as they should be reshaped too
          QHash<QgsVectorLayer *, QSet<QgsFeatureId> > ignoreFeatures;
          ignoreFeatures.insert( vlayer, vlayer->allFeatureIds() );

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
          int res = -1;
          if ( avoidIntersectionsLayers.size() > 0 )
          {
            res = geom.avoidIntersections( QgsProject::instance()->avoidIntersectionsLayers(), ignoreFeatures );
            if ( res == 1 )
            {
              emit messageEmitted( tr( "An error was reported during intersection removal" ), Qgis::MessageLevel::Critical );
              vlayer->destroyEditCommand();
              stopCapturing();
              return;
            }
          }

          if ( geom.isEmpty() ) //intersection removal might have removed the whole geometry
          {
            emit messageEmitted( tr( "The feature cannot be reshaped because the resulting geometry is empty" ), Qgis::MessageLevel::Critical );
            vlayer->destroyEditCommand();
            return;
          }
          if ( res == 3 )
          {
            emit messageEmitted( tr( "At least one geometry intersected is invalid. These geometries must be manually repaired." ), Qgis::MessageLevel::Warning );
          }
        }

        vlayer->changeGeometry( f.id(), geom );
        reshapeDone = true;
      }
    }
  }

  if ( reshapeDone )
  {
    // Add topological points
    if ( QgsProject::instance()->topologicalEditing() )
    {
      const QList<QgsPointLocator::Match> sm = snappingMatches();
      Q_ASSERT( pts.size() == sm.size() );
      for ( int i = 0; i < sm.size() ; ++i )
      {
        if ( sm.at( i ).layer() )
        {
          sm.at( i ).layer()->addTopologicalPoints( pts.at( i ) );
        }
      }
    }
    vlayer->endEditCommand();
  }
  else
  {
    vlayer->destroyEditCommand();
  }
}
