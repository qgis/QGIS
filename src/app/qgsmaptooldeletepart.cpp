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

#include <QMouseEvent>
#include <QMessageBox>

QgsMapToolDeletePart::QgsMapToolDeletePart( QgsMapCanvas* canvas )
    : QgsMapToolVertexEdit( canvas ), mCross( 0 )
{
}

QgsMapToolDeletePart::~QgsMapToolDeletePart()
{
  delete mCross;
}

void QgsMapToolDeletePart::canvasMoveEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  //nothing to do
}

void QgsMapToolDeletePart::canvasPressEvent( QMouseEvent *e )
{
  delete mCross;
  mCross = 0;

  mRecentSnappingResults.clear();
  //do snap -> new recent snapping results
  if ( mSnapper.snapToCurrentLayer( e->pos(), mRecentSnappingResults, QgsSnapper::SnapToVertex ) != 0 )
  {
    //error
  }

  if ( mRecentSnappingResults.size() > 0 )
  {
    QgsPoint markerPoint = mRecentSnappingResults.begin()->snappedVertex;

    //show vertex marker
    mCross = new QgsVertexMarker( mCanvas );
    mCross->setIconType( QgsVertexMarker::ICON_X );
    mCross->setCenter( markerPoint );
  }
  else
  {
    displaySnapToleranceWarning();
  }
}

void QgsMapToolDeletePart::canvasReleaseEvent( QMouseEvent *e )
{
  Q_UNUSED( e );
  delete mCross;
  mCross = 0;

  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  if ( !currentLayer )
    return;

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  if ( !vlayer )
    return;

  if ( mRecentSnappingResults.size() > 0 )
  {
    QList<QgsSnappingResult>::iterator sr_it = mRecentSnappingResults.begin();
    for ( ; sr_it != mRecentSnappingResults.end(); ++sr_it )
    {
      deletePart( sr_it->snappedAtGeometry, sr_it->snappedVertexNr, vlayer );
    }
  }

}


void QgsMapToolDeletePart::deletePart( QgsFeatureId fId, int beforeVertexNr, QgsVectorLayer* vlayer )
{
  QgsFeature f;
  vlayer->getFeatures( QgsFeatureRequest().setFilterFid( fId ) ).nextFeature( f );

  // find out the part number
  QgsGeometry* g = f.geometry();
  if ( !g->isMultipart() )
  {
    QMessageBox::information( mCanvas, tr( "Delete part" ), tr( "This isn't a multipart geometry." ) );
    return;
  }

  int partNum = partNumberOfVertex( g, beforeVertexNr );

  if ( g->deletePart( partNum ) )
  {
    vlayer->beginEditCommand( tr( "Part of multipart feature deleted" ) );
    vlayer->changeGeometry( fId, g );
    vlayer->endEditCommand();
    mCanvas->refresh();
  }
  else
  {
    QMessageBox::information( mCanvas, tr( "Delete part" ), tr( "Couldn't remove the selected part." ) );
  }

}

int QgsMapToolDeletePart::partNumberOfVertex( QgsGeometry* g, int beforeVertexNr )
{
  int part;

  switch ( g->wkbType() )
  {
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
  delete mCross;
  mCross = 0;

  QgsMapTool::deactivate();
}

