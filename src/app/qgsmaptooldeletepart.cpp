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

#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgssnappingutils.h"
#include "qgstolerance.h"

#include <QMouseEvent>

QgsMapToolDeletePart::QgsMapToolDeletePart( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , vlayer( NULL )
    , mRubberBand( 0 )
    , mPressedFid( 0 )
    , mPressedPartNum( 0 )
{
  mToolName = tr( "Delete part" );
}

QgsMapToolDeletePart::~QgsMapToolDeletePart()
{
  delete mRubberBand;
}

void QgsMapToolDeletePart::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeletePart::canvasPressEvent( QMouseEvent *e )
{
  mPressedFid = -1;
  mPressedPartNum = -1;
  delete mRubberBand;
  mRubberBand = 0;

  QgsMapLayer* currentLayer = mCanvas->currentLayer();
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

  QgsGeometry* geomPart = partUnderPoint( e->pos(), mPressedFid, mPressedPartNum );

  if ( mPressedFid != -1 )
  {
    mRubberBand = createRubberBand( vlayer->geometryType() );

    mRubberBand->setToGeometry( geomPart, vlayer );
    mRubberBand->show();
  }

  delete geomPart;
}

void QgsMapToolDeletePart::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );

  delete mRubberBand;
  mRubberBand = 0;

  if ( !vlayer || !vlayer->isEditable() )
  {
    return;
  }

  if ( mPressedFid == -1 )
    return;

  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( mPressedFid ) ).nextFeature( f );
  QgsGeometry* g = f.geometry();

  if ( g->deletePart( mPressedPartNum ) )
  {
    vlayer->beginEditCommand( tr( "Part of multipart feature deleted" ) );
    vlayer->changeGeometry( f.id(), g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }
  else
  {
    emit messageEmitted( tr( "Couldn't remove the selected part." ) );
  }
  return;
}

QgsGeometry* QgsMapToolDeletePart::partUnderPoint( QPoint point, QgsFeatureId& fid, int& partNum )
{
  QgsFeature f;
  QgsGeometry* geomPart = new QgsGeometry();

  switch ( vlayer->geometryType() )
  {
    case QGis::Point:
    case QGis::Line:
    {
      QgsPointLocator::Match match = mCanvas->snappingUtils()->snapToCurrentLayer( point, QgsPointLocator::Vertex | QgsPointLocator::Edge );
      if ( !match.isValid() )
        return geomPart;

      int snapVertex = match.vertexIndex();
      vlayer->getFeatures( QgsFeatureRequest().setFilterFid( match.featureId() ) ).nextFeature( f );
      const QgsGeometry* g = f.constGeometry();
      if ( !g->isMultipart() )
        return geomPart;
      if ( g->wkbType() == QGis::WKBMultiPoint || g->wkbType() == QGis::WKBMultiPoint25D )
      {
        fid = match.featureId();
        partNum = snapVertex;
        delete geomPart;
        return QgsGeometry::fromPoint( match.point() );
      }
      if ( g->wkbType() == QGis::WKBMultiLineString || g->wkbType() == QGis::WKBMultiLineString25D )
      {
        QgsMultiPolyline mline = g->asMultiPolyline();
        for ( int part = 0; part < mline.count(); part++ )
        {
          if ( snapVertex < mline[part].count() )
          {
            fid = match.featureId();
            partNum = part;
            delete geomPart;
            return QgsGeometry::fromPolyline( mline[part] );
          }
          snapVertex -= mline[part].count();
        }
      }
      break;
    }
    case QGis::Polygon:
    {
      QgsPoint layerCoords = toLayerCoordinates( vlayer, point );
      double searchRadius = QgsTolerance::vertexSearchRadius( mCanvas->currentLayer(), mCanvas->mapSettings() );
      QgsRectangle selectRect( layerCoords.x() - searchRadius, layerCoords.y() - searchRadius,
                               layerCoords.x() + searchRadius, layerCoords.y() + searchRadius );
      QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( selectRect ) );
      fit.nextFeature( f );
      const QgsGeometry* g = f.constGeometry();
      if ( !g )
        return geomPart;
      if ( !g->isMultipart() )
        return geomPart;
      QgsMultiPolygon mpolygon = g->asMultiPolygon();
      for ( int part = 0; part < mpolygon.count(); part++ ) // go through the polygons
      {
        const QgsPolygon& polygon = mpolygon[part];
        QgsGeometry* partGeo = QgsGeometry::fromPolygon( polygon );
        if ( partGeo->contains( &layerCoords ) )
        {
          fid = f.id();
          partNum = part;
          delete geomPart;
          return partGeo;
        }
        delete partGeo;
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

