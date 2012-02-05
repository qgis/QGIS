/***************************************************************************
    qgsmaptoolnodetool.cpp  - add/move/delete vertex integrated in one tool
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

#include "qgsmaptoolnodetool.h"

#include "nodetool/qgsselectedfeature.h"
#include "nodetool/qgsvertexentry.h"

#include "qgsproject.h"
#include "qgsmapcanvas.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgslogger.h"

#include <QMouseEvent>
#include <QRubberBand>

QgsMapToolNodeTool::QgsMapToolNodeTool( QgsMapCanvas* canvas )
    : QgsMapToolVertexEdit( canvas )
    , mSelectedFeature( 0 )
    , mMoving( true )
    , mClicked( false )
    , mCtrl( false )
    , mSelectAnother( false )
    , mRubberBand( 0 )
    , mIsPoint( false )
{
}

QgsMapToolNodeTool::~QgsMapToolNodeTool()
{
  removeRubberBands();
}

void QgsMapToolNodeTool::createMovingRubberBands()
{
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
  QgsGeometry* geometry = mSelectedFeature->geometry();
  int beforeVertex, afterVertex;
  int lastRubberBand = 0;
  int vertex;
  for ( int i = 0; i < vertexMap.size(); i++ )
  {
    // create rubberband
    if ( vertexMap[i]->isSelected() && !vertexMap[i]->isInRubberBand() )
    {
      geometry->adjacentVertices( i, beforeVertex, afterVertex );
      vertex = i;
      while ( beforeVertex !=  -1 )
      {
        // move forward NOTE: end if whole cycle is selected
        if ( vertexMap[beforeVertex]->isSelected() && beforeVertex != i ) // and take care of cycles
        {
          vertex = beforeVertex;
          geometry->adjacentVertices( vertex, beforeVertex, afterVertex );
        }
        else
        {
          // break if cycle is found
          break;
        }
      }
      // we have first vertex of moving part
      // create rubberband and set default paramaters
      QgsRubberBand* rb = new QgsRubberBand( mCanvas, false );
      rb->setWidth( 2 );
      rb->setColor( Qt::blue );
      int index = 0;
      if ( beforeVertex != -1 ) // adding first point which is not moving
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[beforeVertex]->point() ), false );
        vertexMap[beforeVertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
      }
      while ( vertex != -1 && vertexMap[vertex]->isSelected() && !vertexMap[vertex]->isInRubberBand() )
      {
        // topology rubberband creation if needed
        if ( topologicalEditing )
        {
          createTopologyRubberBands( vlayer, vertexMap, vertex );
        }
        // adding point which will be moved
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex]->point() ), false );
        // setting values about added vertex
        vertexMap[vertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
        geometry->adjacentVertices( vertex, beforeVertex, vertex );
      }
      if ( vertex != -1 && !vertexMap[vertex]->isSelected() ) // add last point not moving if exists
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex]->point() ), true );
        vertexMap[vertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
      }
      mRubberBands.append( rb );
      lastRubberBand++;
    }
  }
}

void QgsMapToolNodeTool::createTopologyRubberBands( QgsVectorLayer* vlayer, const QList<QgsVertexEntry*> &vertexMap, int vertex )
{
  QMultiMap<double, QgsSnappingResult> currentResultList;
  QgsGeometry *geometry = mSelectedFeature->geometry();

  // snap from current vertex
  currentResultList.clear();
  vlayer->snapWithContext( vertexMap[vertex]->point(), ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToVertex );
  QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

  for ( ; resultIt != currentResultList.end(); ++resultIt )
  {
    // move all other
    if ( mSelectedFeature->featureId() != resultIt.value().snappedAtGeometry )
    {
      if ( mTopologyMovingVertexes.contains( resultIt.value().snappedAtGeometry ) )
      {
        if ( mTopologyMovingVertexes[resultIt.value().snappedAtGeometry]->contains( resultIt.value().snappedVertexNr ) )
        {
          // skip vertex already exists in some rubberband
          continue;
        }
      }
      QgsRubberBand* trb = new QgsRubberBand( mCanvas, false );
      mTopologyRubberBand.append( trb );
      int rbId = mTopologyRubberBand.size() - 1;
      trb->setWidth( 1 );
      trb->setColor( Qt::red );

      int tVertex = resultIt.value().snappedVertexNr;
      int tVertexBackup = -1, tVertexAfter = -1;
      int tVertexFirst = tVertex; // vertex number to check for cycling
      QgsFeature topolFeature;

      vlayer->featureAtId( resultIt.value().snappedAtGeometry, topolFeature, true, false );
      QgsGeometry* topolGeometry = topolFeature.geometry();

      while ( tVertex != -1 ) // looking for first vertex to rubberband
      {
        tVertexBackup = tVertex;
        topolGeometry->adjacentVertices( tVertex, tVertex, tVertexAfter );
        if ( tVertex == -1 || tVertex == tVertexFirst )
          break; // check if this is not first vertex of the feature or cycling error
        // if closest vertex is not from selected feature or is not selected end
        double dist;
        QgsPoint point = topolGeometry->vertexAt( tVertex );
        int at, before, after;
        geometry->closestVertex( point, at, before, after, dist );
        if ( dist > ZERO_TOLERANCE || !vertexMap[at]->isSelected() ) // problem with double precision
        {
          break; // found first vertex
        }
      }

      int movingPointIndex = 0;
      Vertexes* movingPoints = new Vertexes();
      Vertexes* addedPoints = new Vertexes();
      if ( mTopologyMovingVertexes.contains( resultIt.value().snappedAtGeometry ) )
      {
        addedPoints = mTopologyMovingVertexes[ resultIt.value().snappedAtGeometry ];
      }
      if ( tVertex == -1 ) // adding first point if needed
      {
        tVertex = tVertexBackup;
      }
      else
      {
        trb->addPoint( topolGeometry->vertexAt( tVertex ) );
        if ( tVertex == tVertexFirst ) // cycle first vertex need to be added also
        {
          movingPoints->insert( movingPointIndex );
        }
        movingPointIndex = 1;
        topolGeometry->adjacentVertices( tVertex, tVertexAfter, tVertex );
      }

      while ( tVertex != -1 )
      {
        // if closest vertex is not from selected feature or is not selected end
        double dist;
        QgsPoint point = topolGeometry->vertexAt( tVertex );
        int at, before, after;
        geometry->closestVertex( point, at, before, after, dist );
        // find first no matching vertex
        if ( dist > ZERO_TOLERANCE || !vertexMap[at]->isSelected() ) // problem with double precision
        {
          trb->addPoint( topolGeometry->vertexAt( tVertex ) );
          break; // found first vertex
        }
        else // add moving point to rubberband
        {
          if ( addedPoints->contains( tVertex ) )
            break; // just preventing to circle
          trb->addPoint( topolGeometry->vertexAt( tVertex ) );
          movingPoints->insert( movingPointIndex );
          movingPointIndex++;
          addedPoints->insert( tVertex );
        }
        topolGeometry->adjacentVertices( tVertex, tVertexAfter, tVertex );
      }
      mTopologyMovingVertexes.insert( resultIt.value().snappedAtGeometry, addedPoints );
      mTopologyRubberBandVertexes.insert( rbId, movingPoints );
    }
  }
}

void QgsMapToolNodeTool::canvasMoveEvent( QMouseEvent * e )
{
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !mSelectedFeature || !vlayer || !mClicked )
    return;

  if ( !vlayer )
    return;

  mSelectAnother = false;
  if ( mMoving )
  {
    // create rubberband if none exists
    if ( mRubberBands.empty() )
    {
      if ( mIsPoint )
      {
        QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
        for ( int i = 0; i < vertexMap.size(); i++ )
        {
          if ( vertexMap[i]->isSelected() )
          {
            QgsRubberBand* rb = createRubberBandMarker( vertexMap[i]->point(), vlayer );
            mRubberBands.append( rb );
          }
        }
      }
      createMovingRubberBands();
      QList<QgsSnappingResult> snapResults;
      QgsPoint posMapCoord = snapPointFromResults( snapResults, e->pos() );
      mPosMapCoordBackup = posMapCoord;
    }
    else
    {
      // move rubberband
      QList<QgsSnappingResult> snapResults;
      QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
      QList<QgsPoint> excludePoints;
      excludePoints.append( mClosestVertex );
      mSnapper.snapToBackgroundLayers( e->pos(), snapResults, excludePoints );
      // get correct coordinates to move
      QgsPoint posMapCoord = snapPointFromResults( snapResults, e->pos() );
      if ( snapResults.size() > 0 )
      {
        firstCoords = toMapCoordinates( vlayer, mClosestVertex );
      }

      // special handling of points
      if ( mIsPoint )
      {
        double offsetX = posMapCoord.x() - firstCoords.x();
        double offsetY = posMapCoord.y() - firstCoords.y();
        for ( int i = 0; i < mRubberBands.size(); i++ )
        {
          mRubberBands[i]->setTranslationOffset( offsetX, offsetY );
        }
        return;
      }

      // move points
      QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
      for ( int i = 0; i < vertexMap.size(); i++ )
      {

        if ( vertexMap[i]->isSelected() )
        {
          QgsPoint mapCoords = toMapCoordinates( vlayer, vertexMap[i]->point() );
          double x = mapCoords.x() + posMapCoord.x() - firstCoords.x();
          double y = mapCoords.y() + posMapCoord.y() - firstCoords.y();

          mRubberBands[vertexMap[i]->rubberBandNr()]->movePoint( vertexMap[i]->rubberBandIndex(), QgsPoint( x, y ) );
          if ( vertexMap[i]->rubberBandIndex() == 0 )
          {
            mRubberBands[vertexMap[i]->rubberBandNr()]->movePoint( 0, QgsPoint( x, y ) );
          }
        }
      }

      // topological editing
      double offsetX = posMapCoord.x() - mPosMapCoordBackup.x();
      double offsetY = posMapCoord.y() - mPosMapCoordBackup.y();
      for ( int i = 0; i < mTopologyRubberBand.size(); i++ )
      {
        for ( int pointIndex = 0; pointIndex < mTopologyRubberBand[i]->numberOfVertices() - 1; pointIndex++ )
        {
          if ( mTopologyRubberBandVertexes[i]->contains( pointIndex ) )
          {
            const QgsPoint* point = mTopologyRubberBand[i]->getPoint( 0, pointIndex );
            if ( point == 0 )
            {
              break;
            }
            mTopologyRubberBand[i]->movePoint( pointIndex, QgsPoint( point->x() + offsetX, point->y() + offsetY ) );
            if ( pointIndex == 0 )
            {
              mTopologyRubberBand[i]->movePoint( pointIndex , QgsPoint( point->x() + offsetX, point->y() + offsetY ) );
            }
          }
        }
      }
      mPosMapCoordBackup = posMapCoord;
    }
  }
  else
  {
    if ( !mSelectionRectangle )
    {
      mSelectionRectangle = true;
      mRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
      mRect = new QRect();
      mRect->setTopLeft( QPoint( mLastCoordinates->x(), mLastCoordinates->y() ) );
    }
    mRect->setBottomRight( e->pos() );
    QRect normalizedRect = mRect->normalized();
    mRubberBand->setGeometry( normalizedRect );
    mRubberBand->show();
  }
}

void QgsMapToolNodeTool::canvasPressEvent( QMouseEvent * e )
{
  QgsDebugMsg( "Entering." );
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  mClicked = true;
  mLastCoordinates = new QgsPoint( e->pos().x(), e->pos().y() );
  QList<QgsSnappingResult> snapResults;
  if ( !mSelectedFeature )
  {
    mSelectAnother = false;
    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertexAndSegment, -1 );

    if ( snapResults.size() < 1 )
    {
      displaySnapToleranceWarning();
      return;
    }

    mSelectedFeature = new QgsSelectedFeature( snapResults[0].snappedAtGeometry, vlayer, mCanvas );
    connect( mSelectedFeature, SIGNAL( destroyed() ), this, SLOT( selectedFeatureDestroyed() ) );
    mIsPoint = vlayer->geometryType() == QGis::Point;
  }
  else
  {
    // some feature already selected
    QgsPoint mapCoordPoint = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
    double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapRenderer() );

    // get geometry and find if snapping is near it
    int atVertex, beforeVertex, afterVertex;
    double dist;
    mSelectedFeature->geometry()->closestVertex( toLayerCoordinates( vlayer, mapCoordPoint ), atVertex, beforeVertex, afterVertex, dist );
    dist = sqrt( dist );

    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertex, tol );
    if ( dist > tol )
    {
      // for points only selecting another feature
      // no vertexes found (selecting or inverting selection) if move
      // or select another feature if clicked there
      mSnapper.snapToCurrentLayer( e->pos(), snapResults, mIsPoint ? QgsSnapper::SnapToVertex : QgsSnapper::SnapToSegment, tol );
      if ( snapResults.size() > 0 )
      {
        // need to check all if there is a point in my selected feature
        mAnother = snapResults.first().snappedAtGeometry;
        mSelectAnother = true;
        QList<QgsSnappingResult>::iterator it = snapResults.begin();
        QgsSnappingResult snapResult;
        for ( ; it != snapResults.end(); ++it )
        {
          if ( it->snappedAtGeometry == mSelectedFeature->featureId() )
          {
            snapResult = *it;
            mAnother = 0;
            mSelectAnother = false;
            break;
          }
        }
        if ( !mSelectAnother )
        {
          mMoving = true;
          QgsPoint point = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
          mClosestVertex = closestVertex( toLayerCoordinates( vlayer, point ) );
          if ( !mSelectedFeature->isSelected( snapResult.beforeVertexNr ) ||
               !mSelectedFeature->isSelected( snapResult.afterVertexNr ) )
          {
            mSelectedFeature->deselectAllVertexes();
            mSelectedFeature->selectVertex( snapResult.afterVertexNr );
            mSelectedFeature->selectVertex( snapResult.beforeVertexNr );
          }
        }
      }
      else if ( !mCtrl )
      {
        mSelectedFeature->deselectAllVertexes();
      }
    }
    else
    {
      // some vertex selected
      mMoving = true;
      QgsPoint point = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
      mClosestVertex = closestVertex( toLayerCoordinates( vlayer, point ) );
      if ( mMoving )
      {
        if ( mCtrl )
        {
          mSelectedFeature->invertVertexSelection( atVertex );
        }
        else if ( !mSelectedFeature->isSelected( atVertex ) )
        {
          mSelectedFeature->deselectAllVertexes();
          mSelectedFeature->selectVertex( atVertex );
        }
      }
      else
      {
        // select another feature
        mAnother = snapResults.first().snappedAtGeometry;
        mSelectAnother = true;
      }
    }
  }
  QgsDebugMsg( "Leaving." );
}

void QgsMapToolNodeTool::selectedFeatureDestroyed()
{
  QgsDebugMsg( "Entered." );
  mSelectedFeature = 0;
}

void QgsMapToolNodeTool::canvasReleaseEvent( QMouseEvent * e )
{
  if ( !mSelectedFeature )
  {
    // no feature is selected
    return;
  }

  removeRubberBands();

  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  mClicked = false;
  mSelectionRectangle = false;
  QgsPoint coords = toMapCoordinates( e->pos() );
  // QgsPoint coords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
  // QgsPoint firstCoords = toMapCoordinates( *mLastCoordinates );
  QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
  if ( mRubberBand )
  {
    mRubberBand->close();
    delete mRubberBand;
    mRubberBand = 0;
  }
  if ( mLastCoordinates->x() == e->pos().x() && mLastCoordinates->y() == e->pos().y() )
  {
    if ( mSelectAnother )
    {
      // select another feature (this should deselect current one ;-) )
      mSelectedFeature->setSelectedFeature( mAnother, vlayer, mCanvas );
      mIsPoint = vlayer->geometryType() == QGis::Point;
      mSelectAnother = false;
    }
  }
  else
  {
    // coordinates has to be coordinates from layer not canvas
    QgsPoint layerCoords = toLayerCoordinates( vlayer, coords );
    QgsPoint layerFirstCoords = toLayerCoordinates( vlayer, firstCoords );

    // got correct coordinates
    double topX;
    double bottomX;
    if ( layerCoords.x() > layerFirstCoords.x() )
    {
      topX = layerFirstCoords.x();
      bottomX = layerCoords.x();
    }
    else
    {
      topX = layerCoords.x();
      bottomX = layerFirstCoords.x();
    }
    double leftY;
    double rightY;
    if ( layerCoords.y() > layerFirstCoords.y() )
    {
      leftY = layerFirstCoords.y();
      rightY = layerCoords.y();
    }
    else
    {
      leftY = layerCoords.y();
      rightY = layerFirstCoords.y();
    }
    if ( mMoving )
    {
      mMoving = false;
      QList<QgsSnappingResult> snapResults;
      QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
      QList<QgsPoint> excludePoints;
      excludePoints.append( mClosestVertex );
      mSnapper.snapToBackgroundLayers( e->pos(), snapResults, excludePoints );
      coords = snapPointFromResults( snapResults, e->pos() );
      // coords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
      int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
      if ( snapResults.size() > 0 )
      {
        firstCoords = toMapCoordinates( vlayer, mClosestVertex );
        if ( topologicalEditing )
        {
          insertSegmentVerticesForSnap( snapResults, vlayer );
        }
      }
      QgsPoint layerCoords = toLayerCoordinates( vlayer, coords );
      QgsPoint layerFirstCoords = toLayerCoordinates( vlayer, firstCoords );
      double changeX = layerCoords.x() - layerFirstCoords.x();
      double changeY = layerCoords.y() - layerFirstCoords.y();
      mSelectedFeature->beginGeometryChange();
      mSelectedFeature->moveSelectedVertexes( changeX, changeY );
      mCanvas->refresh();
      mSelectedFeature->endGeometryChange();
      // movingVertexes
    }
    else // selecting vertexes by rubberband
    {
      QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
      if ( !mCtrl )
      {
        mSelectedFeature->deselectAllVertexes();
      }
      for ( int i = 0; i < vertexMap.size(); i++ )
      {
        if ( vertexMap[i]->point().x() < bottomX && vertexMap[i]->point().y() > leftY
             && vertexMap[i]->point().x() > topX    && vertexMap[i]->point().y() < rightY )
        {
          // inverting selection is enough because all were deselected if ctrl is not pressed
          mSelectedFeature->invertVertexSelection( i, false );
        }
      }
    }
  }
  mMoving = false;

  removeRubberBands();

  mRecentSnappingResults.clear();
  mExcludePoint.clear();
}

void QgsMapToolNodeTool::deactivate()
{
  removeRubberBands();

  delete mSelectedFeature;
  mSelectedFeature = 0;

  mRubberBand = 0;
  mSelectAnother = false;
  mCtrl = false;
  mMoving = true;
  mClicked = false;

  QgsMapTool::deactivate();
}

void QgsMapToolNodeTool::removeRubberBands()
{
  // cleanup rubberbands and list
  foreach( QgsRubberBand *rb, mRubberBands )
  {
    delete rb;
  }
  mRubberBands.clear();

  foreach( QgsRubberBand *rb, mTopologyRubberBand )
  {
    delete rb;
  }
  mTopologyRubberBand.clear();

  mTopologyMovingVertexes.clear();
  mTopologyRubberBandVertexes.clear();

  // remove all data from selected feature (no change to rubberbands itself)
  if ( mSelectedFeature )
    mSelectedFeature->cleanRubberBandsData();
}


void QgsMapToolNodeTool::canvasDoubleClickEvent( QMouseEvent * e )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
    return;

  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;

  QList<QgsSnappingResult> snapResults;
  mMoving = false;
  double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapRenderer() );
  mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToSegment, tol );
  if ( snapResults.size() < 1 ||
       snapResults.first().snappedAtGeometry != mSelectedFeature->featureId() ||
       snapResults.first().snappedVertexNr != -1 )
    return;

  // some segment selected
  QgsPoint coords = snapResults.first().snappedVertex;
  QgsPoint layerCoords = toLayerCoordinates( vlayer, coords );
  if ( topologicalEditing )
  {
    // snap from adding position to this vertex when topological editing is enabled
    currentResultList.clear();
    vlayer->snapWithContext( layerCoords, ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToSegment );
  }

  vlayer->beginEditCommand( tr( "Inserted vertex" ) );

  // add vertex
  vlayer->insertVertex( layerCoords.x(), layerCoords.y(), mSelectedFeature->featureId(), snapResults.first().afterVertexNr );

  if ( topologicalEditing )
  {
    QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

    for ( ; resultIt != currentResultList.end(); ++resultIt )
    {
      // create vertexes on same position when topological editing is enabled
      if ( mSelectedFeature->featureId() !=  resultIt.value().snappedAtGeometry )
        vlayer->insertVertex( layerCoords.x(), layerCoords.y(), resultIt.value().snappedAtGeometry, resultIt.value().afterVertexNr );
    }
  }

  vlayer->endEditCommand();

  // make sure that new node gets its vertex marker
  mCanvas->refresh();
}

QgsPoint QgsMapToolNodeTool::closestVertex( QgsPoint point )
{
  int at;
  int before;
  int after;
  double dist;
  return mSelectedFeature->geometry()->closestVertex( point, at, before, after, dist );
}

void QgsMapToolNodeTool::keyPressEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Control )
  {
    mCtrl = true;
  }
}

void QgsMapToolNodeTool::keyReleaseEvent( QKeyEvent* e )
{
  if ( e->key() == Qt::Key_Control )
  {
    mCtrl = false;
    return;
  }

  if ( mSelectedFeature && e->key() == Qt::Key_Delete )
  {
    mSelectedFeature->beginGeometryChange();
    mSelectedFeature->deleteSelectedVertexes();
    mCanvas->refresh();
    mSelectedFeature->endGeometryChange();
  }
}

QgsRubberBand* QgsMapToolNodeTool::createRubberBandMarker( QgsPoint center, QgsVectorLayer* vlayer )
{
  // create rubberband marker for moving points
  QgsRubberBand* marker = new QgsRubberBand( mCanvas, true );
  marker->setColor( Qt::red );
  marker->setWidth( 2 );
  double movement = 4;
  double s = QgsTolerance::toleranceInMapUnits( movement, vlayer, mCanvas->mapRenderer(), QgsTolerance::Pixels );
  QgsPoint pom = toMapCoordinates( vlayer, center );
  pom.setX( pom.x() - s );
  pom.setY( pom.y() - s );
  marker->addPoint( pom );
  pom.setX( pom.x() + 2*s );
  marker->addPoint( pom );
  pom.setY( pom.y() + 2*s );
  marker->addPoint( pom );
  pom.setX( pom.x() - 2*s );
  marker->addPoint( pom );
  pom.setY( pom.y() - 2*s );
  marker->addPoint( pom );
  return marker;
}
