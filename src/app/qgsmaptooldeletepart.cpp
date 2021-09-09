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

#include "qgsfeatureiterator.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsrubberband.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"
#include "qgisapp.h"
#include "qgsmapmouseevent.h"

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

  if ( mPressedFid != -1 )
  {
    mRubberBand = createRubberBand( vlayer->geometryType() );

    mRubberBand->setToGeometry( geomPart, vlayer );
    mRubberBand->show();
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

  if ( mPressedFid == -1 )
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
}

QgsGeometry QgsMapToolDeletePart::partUnderPoint( QPoint point, QgsFeatureId &fid, int &partNum )
{
  QgsFeature f;
  const QgsGeometry geomPart;

  switch ( vlayer->geometryType() )
  {
    case QgsWkbTypes::PointGeometry:
    case QgsWkbTypes::LineGeometry:
    {
      const QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Types( QgsPointLocator::Vertex | QgsPointLocator::Edge ) );
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
      else if ( QgsWkbTypes::geometryType( g.wkbType() ) == QgsWkbTypes::PointGeometry )
      {
        fid = match.featureId();
        partNum = snapVertex;
        return QgsGeometry::fromPointXY( match.point() );
      }
      else if ( QgsWkbTypes::geometryType( g.wkbType() ) == QgsWkbTypes::LineGeometry )
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
    case QgsWkbTypes::PolygonGeometry:
    {
      const QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Area );
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

