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

#include "qgisapp.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include "qgsmessagebar.h"
#include "qgsgeometry.h"

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolDeletePart::QgsMapToolDeletePart( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
{
}

QgsMapToolDeletePart::~QgsMapToolDeletePart()
{
}

void QgsMapToolDeletePart::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeletePart::canvasPressEvent( QMouseEvent *e )
{

  mRecentSnappingResults.clear();
  //do snap -> new recent snapping results
  if ( mSnapper.snapToCurrentLayer( e->pos(), mRecentSnappingResults, QgsSnapper::SnapToVertexAndSegment ) != 0 )
  {
    //error
  }
}

void QgsMapToolDeletePart::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );

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
  QgsGeometry* g;
  QgsFeature f;
  int partNum;
  switch( vlayer->geometryType() )
  {
    case QGis::Point:
    case QGis::Line:
    {
      if ( mRecentSnappingResults.size() == 0 )
      {
        return;
      }
      QgsSnappingResult sr = mRecentSnappingResults.first();
      vlayer->getFeatures( QgsFeatureRequest().setFilterFid( sr.snappedAtGeometry ) ).nextFeature( f );
      g = f.geometry();
      if ( !g )
        return;
      if ( g->wkbType() == QGis::WKBPoint25D || g->wkbType() == QGis::WKBPoint ||
           g->wkbType() == QGis::WKBLineString25D || g->wkbType() == QGis::WKBLineString)
      {
        emit messageEmitted( tr( "The Delete part tool cannot be used on single part features." ) );
        return;
      }
      int vertex = sr.snappedVertexNr;
      if ( vertex == -1 )
      {
        vertex = sr.beforeVertexNr;
      }
      partNum = partNumberOfVertex( g, vertex );
      break;
    }
    case QGis::Polygon:
    {
      QgsPoint p = mCanvas->getCoordinateTransform()->toMapCoordinates( e->x(),e->y());
      p = toLayerCoordinates(vlayer, p);
      f = featureUnderPoint(p);
      g = f.geometry();
      if ( !g )
        return;

      if ( g->wkbType() == QGis::WKBPolygon25D || g->wkbType() == QGis::WKBPolygon)
      {
        emit messageEmitted( tr( "The Delete part tool cannot be used on single part features." ) );
        return;
      }
      partNum = partNumberOfPoint( g, p );
      if ( partNum < 0 )
        return;
      break;
    }
    default:
    {
      QgsDebugMsg("Unknown geometry type");
      return;
    }
  }
  if ( g->deletePart( partNum ) )
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

QgsFeature QgsMapToolDeletePart::featureUnderPoint(QgsPoint p)
{
  QgsRectangle r;
  double searchRadius = mCanvas->extent().width()/100;
  r.setXMinimum( p.x() - searchRadius );
  r.setXMaximum( p.x() + searchRadius );
  r.setYMinimum( p.y() - searchRadius );
  r.setYMaximum( p.y() + searchRadius );
  QgsFeatureIterator fit = vlayer->getFeatures( QgsFeatureRequest().setFilterRect( r ) );
  QgsFeature f;
  fit.nextFeature( f );
  return f;
}


int QgsMapToolDeletePart::partNumberOfPoint(QgsGeometry *g, QgsPoint point)
{
  int part;
  switch ( g->wkbType() )
  {
    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = g->asMultiPolygon();
      for ( part = 0; part < mpolygon.count(); part++ ) // go through the polygons
      {
        const QgsPolygon& polygon = mpolygon[part];
        QgsGeometry* partGeo = QgsGeometry::fromPolygon(polygon);
        if ( partGeo->contains( &point ) )
          return part;
      }
      return -1; // not found
    }
    default:
      return -1;
  }
}

int QgsMapToolDeletePart::partNumberOfVertex( QgsGeometry* g, int beforeVertexNr )
{
  int part;

  switch ( g->wkbType() )
  {
    case QGis::WKBLineString25D:
    case QGis::WKBLineString:
    case QGis::WKBPoint25D:
    case QGis::WKBPoint:
    case QGis::WKBPolygon25D:
    case QGis::WKBPolygon:
      return 1;

    case QGis::WKBMultiPoint25D:
    case QGis::WKBMultiPoint:
      if ( beforeVertexNr < g->asMultiPoint().count() )
        return beforeVertexNr;
      else
        return -1;

    case QGis::WKBMultiLineString25D:
    case QGis::WKBMultiLineString:
    {
      QgsMultiPolyline mline = g->asMultiPolyline();
      for ( part = 0; part < mline.count(); part++ )
      {
        if ( beforeVertexNr < mline[part].count() )
          return part;

        beforeVertexNr -= mline[part].count();
      }
      return -1; // not found
    }

    case QGis::WKBMultiPolygon25D:
    case QGis::WKBMultiPolygon:
    {
      QgsMultiPolygon mpolygon = g->asMultiPolygon();
      for ( part = 0; part < mpolygon.count(); part++ ) // go through the polygons
      {
        const QgsPolygon& polygon = mpolygon[part];
        for ( int ring = 0; ring < polygon.count(); ring++ ) // go through the rings
        {
          if ( beforeVertexNr < polygon[ring].count() )
            return part;

          beforeVertexNr -= polygon[ring].count();
        }
      }
      return -1; // not found
    }

    default:
      return -1;
  }
}

void QgsMapToolDeletePart::deactivate()
{
  QgsMapTool::deactivate();
}

void QgsMapToolDeletePart::notifySinglePart()
{
  QgisApp::instance()->messageBar()->pushMessage(
    tr( "Cannot use delete part" ),
    tr( "The Delete part tool cannot be used on single part features." ),
    QgsMessageBar::INFO,
    QgisApp::instance()->messageTimeout() );
  return;
}
