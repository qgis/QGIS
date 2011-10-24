/***************************************************************************
                              qgsmaptoolmovevertex.cpp
                              ------------------------
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

#include "qgsmaptooladdvertex.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include <QMouseEvent>

QgsMapToolAddVertex::QgsMapToolAddVertex( QgsMapCanvas* canvas ): QgsMapToolVertexEdit( canvas ), mRubberBand( 0 )
{

}

QgsMapToolAddVertex::~QgsMapToolAddVertex()
{
  delete mRubberBand;
}

void QgsMapToolAddVertex::canvasMoveEvent( QMouseEvent * e )
{
  if ( mRubberBand )
  {
    QList<QgsSnappingResult> snapResults;
    if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
    {
      QgsPoint posMapCoord = snapPointFromResults( snapResults, e->pos() );
      mRubberBand->movePoint( 2, posMapCoord ); //consider that the first rubber band point is added twice
    }
  }
}

void QgsMapToolAddVertex::canvasPressEvent( QMouseEvent * e )
{
  delete mRubberBand;
  mRubberBand = 0;

  //snap to segments of the current layer
  if ( mSnapper.snapToCurrentLayer( e->pos(), mRecentSnappingResults, QgsSnapper::SnapToSegment ) != 0 )
  {
    //error
  }



  if ( mRecentSnappingResults.size() > 0 )
  {
    mRubberBand = createRubberBand();
    //take first snapping result and create the rubber band
    QgsSnappingResult firstResult = *( mRecentSnappingResults.begin() );
    mRubberBand->addPoint( firstResult.beforeVertex, false );
    mRubberBand->addPoint( firstResult.snappedVertex, false );
    mRubberBand->addPoint( firstResult.afterVertex, true );
  }
  else
  {
    displaySnapToleranceWarning();
  }
}

void QgsMapToolAddVertex::canvasReleaseEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  }

  if ( vlayer && mRecentSnappingResults.size() > 0 )
  {
    //snap point to background layers
    QgsPoint snappedPointMapCoord;
    QgsPoint snappedPointLayerCoord;
    QList<QgsSnappingResult> snapResults;

    if ( mSnapper.snapToBackgroundLayers( e->pos(), snapResults ) == 0 )
    {

      //add segment points in case of topological editing
      int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
      if ( topologicalEditing )
      {
        //ignore the snapping results that are on features / segments already considered in mRecentSnappingResults
        QList<QgsSnappingResult> filteredSnapResults = snapResults;
        QList<QgsSnappingResult>::iterator recentIt = mRecentSnappingResults.begin();
        for ( ; recentIt != mRecentSnappingResults.end(); ++recentIt )
        {
          QList<QgsSnappingResult>::iterator filterIt = filteredSnapResults.begin();
          for ( ; filterIt != filteredSnapResults.end(); ++filterIt )
          {
            if ( filterIt->snappedAtGeometry == recentIt->snappedAtGeometry
                 && filterIt->snappedVertexNr == recentIt->snappedVertexNr
                 && filterIt->beforeVertexNr == recentIt->beforeVertexNr )
            {
              filteredSnapResults.erase( filterIt );
              continue;
            }

          }
        }
        insertSegmentVerticesForSnap( filteredSnapResults, vlayer );
      }

      snappedPointMapCoord = snapPointFromResults( snapResults, e->pos() );
      snappedPointLayerCoord = toLayerCoordinates( vlayer, snappedPointMapCoord );

      //and change the feature points
      QList<QgsSnappingResult>::iterator sr_it = mRecentSnappingResults.begin();
      vlayer->beginEditCommand( tr( "Added vertex" ) );
      for ( ; sr_it != mRecentSnappingResults.end(); ++sr_it )
      {
        vlayer->insertVertex( snappedPointLayerCoord.x(), snappedPointLayerCoord.y(), sr_it->snappedAtGeometry, sr_it->afterVertexNr );
      }
      vlayer->endEditCommand();
    }
  }

  delete mRubberBand;
  mRubberBand = 0;

  mCanvas->refresh();
}

void QgsMapToolAddVertex::deactivate()
{
  delete mRubberBand;
  mRubberBand = 0;

  QgsMapTool::deactivate();
}
