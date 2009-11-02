/***************************************************************************
                              qgsmaptooldeletevertex.cpp
                              --------------------------
  begin                : June 30, 2007
  copyright            : (C) 2007 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaptooldeletevertex.h"
#include "qgsmapcanvas.h"
#include "qgsvertexmarker.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolDeleteVertex::QgsMapToolDeleteVertex( QgsMapCanvas* canvas ): QgsMapToolVertexEdit( canvas ), mCross( 0 )
{

}

QgsMapToolDeleteVertex::~QgsMapToolDeleteVertex()
{
  delete mCross;
}

void QgsMapToolDeleteVertex::canvasMoveEvent( QMouseEvent * e )
{
  //nothing to do
}

void QgsMapToolDeleteVertex::canvasPressEvent( QMouseEvent * e )
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

void QgsMapToolDeleteVertex::canvasReleaseEvent( QMouseEvent * e )
{
  delete mCross;
  mCross = 0;

  //delete the vertices in mRecentSnappingResults
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
  }

  bool success = true;
  if ( vlayer && mRecentSnappingResults.size() > 0 )
  {
    QList<QgsSnappingResult>::iterator sr_it = mRecentSnappingResults.begin();
    vlayer->beginEditCommand( tr( "Vertex deleted" ) );
    for ( ; sr_it != mRecentSnappingResults.end(); ++sr_it )
    {
      if ( !vlayer->deleteVertex( sr_it->snappedAtGeometry, sr_it->snappedVertexNr ) )
      {
        success = false;
        vlayer->destroyEditCommand();
        break;
      }
    }
    if ( success )
    {
      vlayer->endEditCommand();
    }
  }

  mCanvas->refresh();
}

//! called when map tool is being deactivated
void QgsMapToolDeleteVertex::deactivate()
{
  delete mCross;
  mCross = 0;

  QgsMapTool::deactivate();
}
