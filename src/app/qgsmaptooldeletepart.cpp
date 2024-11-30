/***************************************************************************
    qgsmaptooldeletepart.cpp  - delete a part from multipart geometry
    ---------------------
    begin                : April 2009
    copyright            : (C) 2009 by Richard Kostecky
    email                : csf dot kostej at mail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldeletepart.h"
#include "moc_qgsmaptooldeletepart.cpp"

#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgsmapmouseevent.h"


/**
 * A filter to limit the matches to selected features, if a selection is present.
 * If there is no selection, any feature can be matched.
 */
class SelectedOnlyFilter : public QgsPointLocator::MatchFilter
{
    bool acceptMatch( const QgsPointLocator::Match &match ) override
    {
      // If there is a selection, we limit matches to selected features
      if ( match.layer() && match.layer()->selectedFeatureCount() > 0 && !match.layer()->selectedFeatureIds().contains( match.featureId() ) )
      {
        return false;
      }
      return true;
    }
};


QgsMapToolDeletePart::QgsMapToolDeletePart( QgsMapCanvas *canvas )
  : QgsMapToolEdit( canvas )
  , mPressedFid( 0 )
  , mPressedPartNum( 0 )
{
  mToolName = tr( "Delete part" );
}

QgsMapToolDeletePart::~QgsMapToolDeletePart()
{
  delete mRubberBand;
}

void QgsMapToolDeletePart::canvasMoveEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )
  //nothing to do
}

void QgsMapToolDeletePart::canvasPressEvent( QgsMapMouseEvent *e )
{
  mPressedFid = -1;
  mPressedPartNum = -1;
  delete mRubberBand;
  mRubberBand = nullptr;

  QgsMapLayer *currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
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

  const QgsGeometry geomPart = partUnderPoint( e->pos(), mPressedFid, mPressedPartNum );

  if ( mPressedPartNum != -1 )
  {
    mRubberBand = createRubberBand( vlayer->geometryType() );

    mRubberBand->setToGeometry( geomPart, vlayer );
    mRubberBand->show();
  }
  else if ( vlayer->selectedFeatureCount() > 0 )
  {
    emit messageEmitted(
      tr( "If there are selected features, the delete parts tool only applies to those. Clear the selection and try again." ),
      Qgis::MessageLevel::Warning
    );
  }
}

void QgsMapToolDeletePart::canvasReleaseEvent( QgsMapMouseEvent *e )
{
  Q_UNUSED( e )

  delete mRubberBand;
  mRubberBand = nullptr;

  if ( !vlayer || !vlayer->isEditable() )
  {
    return;
  }

  if ( mPressedPartNum == -1 )
    return;

  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );
  QgsGeometry g = f.geometry();

  if ( g.deletePart( mPressedPartNum ) )
  {
    vlayer->beginEditCommand( tr( "Part of multipart feature deleted" ) );
    vlayer->changeGeometry( f.id(), g );
    vlayer->endEditCommand();
    vlayer->triggerRepaint();
  }
  else
  {
    emit messageEmitted( tr( "Couldn't remove the selected part." ) );
  }

  if ( g.isEmpty() )
  {
    emit messageEmitted( tr( "All geometry parts deleted from feature %1. Feature has no geometry now!" ).arg( mPressedFid ) );
  }
}

QgsGeometry QgsMapToolDeletePart::partUnderPoint( QPoint point, QgsFeatureId &fid, int &partNum )
{
  QgsFeature f;
  const QgsGeometry geomPart;

  switch ( vlayer->geometryType() )
  {
    case Qgis::GeometryType::Point:
    case Qgis::GeometryType::Line:
    {
      SelectedOnlyFilter filter;
      const QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Types( QgsPointLocator::Vertex | QgsPointLocator::Edge ), &filter );
      if ( !match.isValid() )
        return geomPart;

      int snapVertex = match.vertexIndex();
      vlayer->getFeatures( QgsFeatureRequest().setFilterFid( match.featureId() ) ).nextFeature( f );
      const QgsGeometry g = f.geometry();
      if ( !g.isMultipart() )
      {
        fid = match.featureId();
        return QgsGeometry::fromPointXY( match.point() );
      }
      else if ( QgsWkbTypes::geometryType( g.wkbType() ) == Qgis::GeometryType::Point )
      {
        fid = match.featureId();
        partNum = snapVertex;
        return QgsGeometry::fromPointXY( match.point() );
      }
      else if ( QgsWkbTypes::geometryType( g.wkbType() ) == Qgis::GeometryType::Line )
      {
        QgsMultiPolylineXY mline = g.asMultiPolyline();
        for ( int part = 0; part < mline.count(); part++ )
        {
          if ( snapVertex < mline[part].count() )
          {
            fid = match.featureId();
            partNum = part;
            return QgsGeometry::fromPolylineXY( mline[part] );
          }
          snapVertex -= mline[part].count();
        }
      }
      break;
    }
    case Qgis::GeometryType::Polygon:
    {
      SelectedOnlyFilter filter;
      const QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Area, &filter );
      if ( !match.isValid() )
        return geomPart;

      vlayer->getFeatures( QgsFeatureRequest().setFilterFid( match.featureId() ) ).nextFeature( f );
      const QgsGeometry g = f.geometry();
      if ( g.isNull() )
        return geomPart;

      const QgsPointXY layerCoords = toLayerCoordinates( vlayer, point );

      if ( !g.isMultipart() )
      {
        fid = f.id();
        partNum = 0;
        return geomPart;
      }
      QgsMultiPolygonXY mpolygon = g.asMultiPolygon();
      for ( int part = 0; part < mpolygon.count(); part++ ) // go through the polygons
      {
        const QgsPolygonXY &polygon = mpolygon[part];
        const QgsGeometry partGeo = QgsGeometry::fromPolygonXY( polygon );
        if ( partGeo.contains( &layerCoords ) )
        {
          fid = f.id();
          partNum = part;
          return partGeo;
        }
      }
      break;
    }
    default:
    {
      break;
    }
  }
  return geomPart;
}

void QgsMapToolDeletePart::deactivate()
{
  QgsMapTool::deactivate();
}
