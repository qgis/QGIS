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
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgstolerance.h"
#include "qgsgeometry.h"
#include "qgslogger.h"
#include "qgisapp.h"
#include "qgslegend.h"

#include <cmath>
#include <QMouseEvent>
#include <QMessageBox>
#include <QStatusBar>

VertexEntry::VertexEntry( QgsMapCanvas *canvas, QgsMapLayer *layer, QgsPoint p, int originalIndex, QString tooltip, QgsVertexMarker::IconType type, int penWidth )
    : mSelected( false )
    , mEquals( -1 )
    , mInRubberBand( false )
    , mRubberBandNr( 0 )
    , mOriginalIndex( originalIndex )
    , mPenWidth( penWidth )
    , mToolTip( tooltip )
    , mType( type )
    , mMarker( 0 )
    , mCanvas( canvas )
    , mLayer( layer )
{
  setCenter( p );
}

VertexEntry::~VertexEntry()
{
  if ( mMarker )
  {
    delete mMarker;
    mMarker = 0;
  }
}

void VertexEntry::setCenter( QgsPoint p )
{
  mPoint = p;
  p = mCanvas->mapRenderer()->layerToMapCoordinates( mLayer, p );

  if ( mCanvas->extent().contains( p ) )
  {
    if ( !mMarker )
    {
      mMarker = new QgsVertexMarker( mCanvas );
      mMarker->setIconType( mType );
      mMarker->setColor( mSelected ? Qt::blue : Qt::red );
      mMarker->setPenWidth( mPenWidth );

      if ( !mToolTip.isEmpty() )
        mMarker->setToolTip( mToolTip );
    }

    mMarker->setCenter( p );
  }
  else if ( mMarker )
  {
    delete mMarker;
    mMarker = 0;
  }
}

void VertexEntry::moveCenter( double x, double y )
{
  mPoint += QgsVector( x, y );
  setCenter( mPoint );
}

void VertexEntry::setSelected( bool selected )
{
  mSelected = selected;
  if ( mMarker )
  {
    mMarker->setColor( mSelected ? Qt::blue : Qt::red );
  }
}

void VertexEntry::setRubberBandValues( bool inRubberBand, int rubberBandNr, int indexInRubberBand )
{
  mIndex        = indexInRubberBand;
  mInRubberBand = inRubberBand;
  mRubberBandNr = rubberBandNr;
}

void VertexEntry::update()
{
  if ( mMarker )
    mMarker->update();
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
  QList<VertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
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

void QgsMapToolNodeTool::createTopologyRubberBands( QgsVectorLayer* vlayer, const QList<VertexEntry*> &vertexMap, int vertex )
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
        QList<VertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
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
      QList<VertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
      for ( int i = 0; i < vertexMap.size(); i++ )
      {

        if ( vertexMap[i]->isSelected() )
        {
          QgsPoint mapCoords = toMapCoordinates( vlayer, vertexMap[i]->point() );
          double x = mapCoords.x() + posMapCoord.x() - firstCoords.x();
          double y = mapCoords.y() + posMapCoord.y() - firstCoords.y();

          mRubberBands[vertexMap[i]->rubberBandNr()]->movePoint( vertexMap[i]->index(), QgsPoint( x, y ) );
          if ( vertexMap[i]->index() == 0 )
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

    mSelectedFeature = new SelectedFeature( snapResults[0].snappedAtGeometry, vlayer, mCanvas );
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
    // if (snapResults.size() < 1)
    if ( dist > tol )
    {
      // for points only selecting another feature
      // no vertexes found (selecting or inverting selection) if move
      // or select another feature if clicked there
      QgsSnapper::SnappingType snapType = QgsSnapper::SnapToSegment;
      if ( mIsPoint )
      {
        snapType = QgsSnapper::SnapToVertex;
      }
      mSnapper.snapToCurrentLayer( e->pos(), snapResults, snapType, tol );
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
      QList<VertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
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
  mSelectedFeature->beginGeometryChange();

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

  mSelectedFeature->endGeometryChange();
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


// selection object

SelectedFeature::SelectedFeature( QgsFeatureId featureId,
                                  QgsVectorLayer *vlayer,
                                  QgsMapCanvas *canvas )
    : mFeatureId( featureId )
    , mGeometry( 0 )
    , mChangingGeometry( false )
    , mRubberBand( 0 )
    , mValidator( 0 )
{
  QgsDebugMsg( "Entering." );
  setSelectedFeature( featureId, vlayer, canvas );
}

SelectedFeature::~SelectedFeature()
{
  QgsDebugMsg( "Entering." );

  deleteVertexMap();

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  if ( mValidator )
  {
    mValidator->stop();
    mValidator->wait();
    mValidator->deleteLater();
    mValidator = 0;
  }
}

void SelectedFeature::currentLayerChanged( QgsMapLayer *layer )
{
  QgsDebugMsg( "Entering. " );
  if ( layer == mVlayer )
    deleteLater();
}

void SelectedFeature::updateGeometry( QgsGeometry *geom )
{
  QgsDebugMsg( "Entering. " );

  delete mGeometry;

  if ( !geom )
  {
    QgsFeature f;
    mVlayer->featureAtId( mFeatureId, f );
    mGeometry = new QgsGeometry( *f.geometry() );
  }
  else
  {
    mGeometry = new QgsGeometry( *geom );
  }
}

void SelectedFeature::cleanRubberBandsData()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setRubberBandValues( false, 0, 0 );
  }
}

void SelectedFeature::setSelectedFeature( QgsFeatureId featureId, QgsVectorLayer* vlayer, QgsMapCanvas* canvas )
{
  mFeatureId = featureId;
  mVlayer = vlayer;
  mCanvas = canvas;

  delete mRubberBand;
  mRubberBand = 0;
  delete mGeometry;
  mGeometry = 0;

  // signal changing of current layer
  connect( QgisApp::instance()->legend(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );

  // feature was deleted
  connect( mVlayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );

  // geometry was changed
  connect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );

  // projection or extents changed
  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( updateVertexMarkersPosition() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( updateVertexMarkersPosition() ) );

  replaceVertexMap();
}

void SelectedFeature::beginGeometryChange()
{
  Q_ASSERT( !mChangingGeometry );
  mChangingGeometry = true;

  // geometry was changed
  disconnect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );
}

void SelectedFeature::endGeometryChange()
{
  Q_ASSERT( mChangingGeometry );
  mChangingGeometry = false;

  // geometry was changed
  connect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );
}

void SelectedFeature::canvasLayersChanged()
{
  currentLayerChanged( mCanvas->currentLayer() );
}

void SelectedFeature::featureDeleted( QgsFeatureId fid )
{
  if ( fid == mFeatureId )
    deleteLater();
}

void SelectedFeature::geometryChanged( QgsFeatureId fid, QgsGeometry &geom )
{
  QgsDebugMsg( "Entering." );
  if ( !mVlayer || fid != mFeatureId )
    return;

  updateGeometry( &geom );
  replaceVertexMap();
}

void SelectedFeature::validateGeometry( QgsGeometry *g )
{
  QSettings settings;
  if ( settings.value( "/qgis/digitizing/validate_geometries", 1 ).toInt() == 0 )
    return;

  if ( !g )
    g = mGeometry;

  mTip.clear();

  if ( mValidator )
  {
    mValidator->stop();
    mValidator->wait();
    mValidator->deleteLater();
    mValidator = 0;
  }

  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    QgsVertexMarker *vm = mGeomErrorMarkers.takeFirst();
    QgsDebugMsg( "deleting " + vm->toolTip() );
    delete vm;
  }

  mValidator = new QgsGeometryValidator( g );
  connect( mValidator, SIGNAL( errorFound( QgsGeometry::Error ) ), this, SLOT( addError( QgsGeometry::Error ) ) );
  connect( mValidator, SIGNAL( finished() ), this, SLOT( validationFinished() ) );
  mValidator->start();

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation started." ) );
}

void SelectedFeature::addError( QgsGeometry::Error e )
{
  mGeomErrors << e;
  if ( !mTip.isEmpty() )
    mTip += "\n";
  mTip += e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
    marker->setCenter( mCanvas->mapRenderer()->layerToMapCoordinates( mVlayer, e.where() ) );
    marker->setIconType( QgsVertexMarker::ICON_X );
    marker->setColor( Qt::green );
    marker->setZValue( marker->zValue() + 1 );
    marker->setPenWidth( 2 );
    marker->setToolTip( e.what() );
    mGeomErrorMarkers << marker;
  }

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( e.what() );
  sb->setToolTip( mTip );
}

void SelectedFeature::validationFinished()
{
  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation finished (%n error(s) gefunden).", "number of geometry errors", mGeomErrorMarkers.size() ) );
}

void SelectedFeature::deleteSelectedVertexes()
{
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;
  mVlayer->beginEditCommand( QObject::tr( "Deleted vertices" ) );
  int count = 0;
  for ( int i = mVertexMap.size() - 1; i > -1; i-- )
  {
    if ( mVertexMap[i]->isSelected() )
    {
      if ( mVertexMap[i]->equals() != -1 )
      {
        // to avoid try to delete some vertex twice
        mVertexMap[ mVertexMap[i]->equals()]->setSelected( false );
      }

      if ( topologicalEditing )
      {
        // snap from current vertex
        currentResultList.clear();
        mVlayer->snapWithContext( mVertexMap[i]->point(), ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToVertex );
      }
      if ( !mVlayer->deleteVertex( mFeatureId, i ) )
      {
        count = 0;
        qDebug( "Setting count 0 and calling break;" );
        break;
      };
      count++;

      if ( topologicalEditing )
      {
        QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

        for ( ; resultIt != currentResultList.end(); ++resultIt )
        {
          // move all other
          if ( mFeatureId !=  resultIt.value().snappedAtGeometry )
            mVlayer->deleteVertex( resultIt.value().snappedAtGeometry, resultIt.value().snappedVertexNr );
        }
      }
    }
  }

  if ( count > 0 )
  {
    mVlayer->endEditCommand();
  }
  else
  {
    mVlayer->destroyEditCommand();
  }
}

void SelectedFeature::moveSelectedVertexes( double changeX, double changeY )
{
  mVlayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;
  for ( int i = mVertexMap.size() - 1; i > -1; i-- )
  {
    if ( mVertexMap[i]->isSelected() )
    {
      if ( topologicalEditing )
      {
        // snap from current vertex
        currentResultList.clear();
        mVlayer->snapWithContext( mVertexMap[i]->point(), ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToVertex );
      }

      mVlayer->moveVertex( mVertexMap[i]->point().x() + changeX, mVertexMap[i]->point().y() + changeY, mFeatureId, i );

      mVertexMap[i]->moveCenter( changeX, changeY );

      if ( topologicalEditing )
      {
        QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

        for ( ; resultIt != currentResultList.end(); ++resultIt )
        {
          // move all other
          if ( mFeatureId !=  resultIt.value().snappedAtGeometry )
            mVlayer->moveVertex( mVertexMap[i]->point().x(), mVertexMap[i]->point().y(),
                                 resultIt.value().snappedAtGeometry, resultIt.value().snappedVertexNr );
        }

      }

      VertexEntry *entry = mVertexMap[i];
      entry->setCenter( entry->point() );
      entry->update();

      if ( entry->equals() != -1 && !mVertexMap[ entry->equals()]->isSelected() )
      {
        mVertexMap[ entry->equals()]->moveCenter( changeX, changeY );
        mVertexMap[ entry->equals()]->update();
        // for polygon delete both
      }
    }
  }

  mVlayer->endEditCommand();
}

void SelectedFeature::replaceVertexMap()
{
  // delete old map
  deleteVertexMap();

  delete mGeometry;
  mGeometry = 0;

  // create new map
  createVertexMap();

  // validate the geometry
  validateGeometry();
}

void SelectedFeature::deleteVertexMap()
{
  foreach( VertexEntry *entry, mVertexMap )
  {
    delete entry;
  }

  mVertexMap.clear();
}

bool SelectedFeature::isSelected( int vertexNr )
{
  return mVertexMap[vertexNr]->isSelected();
}

QgsGeometry *SelectedFeature::geometry()
{
  Q_ASSERT( mGeometry );
  return mGeometry;
}

void SelectedFeature::createVertexMapPolygon()
{
  int y = 0;
  QgsPolygon polygon = mGeometry->asPolygon();
  if ( !polygon.empty() )
  {
    // polygon
    for ( int i2 = 0; i2 < polygon.size(); i2++ )
    {
      const QgsPolyline& poly = polygon[i2];
      int i;
      for ( i = 0; i < poly.size(); i++ )
      {
        mVertexMap.insert( y + i, new VertexEntry( mCanvas, mVlayer, poly[i], i, tr( "ring %1, vertex %2" ).arg( i2 ).arg( i ) ) );
      }
      mVertexMap[y + i - 1 ]->setEqual( y );
      mVertexMap[y]->setEqual( y + i - 1 );
      y += poly.size();
    }
  }
  else // multipolygon
  {
    QgsMultiPolygon multiPolygon = mGeometry->asMultiPolygon();
    for ( int i2 = 0; i2 < multiPolygon.size(); i2++ )
    {
      // iterating through polygons
      const QgsPolygon& poly2 = multiPolygon[i2];
      for ( int i3 = 0; i3 < poly2.size(); i3++ )
      {
        // iterating through polygon rings
        const QgsPolyline& poly = poly2[i3];
        int i;
        for ( i = 0; i < poly.size(); i++ )
        {
          mVertexMap.insert( y + i, new VertexEntry( mCanvas, mVlayer, poly[i], y + i - 1, tr( "polygon %1, ring %2, vertex %3" ).arg( i2 ).arg( i3 ).arg( i ) ) );
        }
        mVertexMap[y + i - 1]->setEqual( y );
        mVertexMap[y]->setEqual( y + i - 1 );
        y += poly.size();
      }
    }
  }
}

void SelectedFeature::createVertexMapLine()
{
  Q_ASSERT( mGeometry );

  if ( mGeometry->isMultipart() )
  {
    int y = 0;
    QgsMultiPolyline mLine = mGeometry->asMultiPolyline();
    for ( int i2 = 0; i2 < mLine.size(); i2++ )
    {
      // iterating through polylines
      QgsPolyline poly = mLine[i2];
      int i;
      for ( i = 0; i < poly.size(); i++ )
      {
        mVertexMap.insert( y + i, new VertexEntry( mCanvas, mVlayer, poly[i], i, tr( "polyline %1, vertex %2" ).arg( i2 ).arg( i ) ) );
      }
      y += poly.size();
    }
  }
  else
  {
    QgsPolyline poly = mGeometry->asPolyline();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      mVertexMap.insert( i, new VertexEntry( mCanvas, mVlayer, poly[i], i, tr( "vertex %1" ).arg( i ) ) );
    }
  }
}

void SelectedFeature::createVertexMapPoint()
{
  Q_ASSERT( mGeometry );

  if ( mGeometry->isMultipart() )
  {
    // multipoint
    QgsMultiPoint poly = mGeometry->asMultiPoint();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      mVertexMap.insert( i,  new VertexEntry( mCanvas, mVlayer, poly[i], 1, tr( "point %1" ).arg( i ) ) );
    }
  }
  else
  {
    // single point
    mVertexMap.insert( 1, new VertexEntry( mCanvas, mVlayer, mGeometry->asPoint(), 1, tr( "single point" ) ) );
  }
}

void SelectedFeature::createVertexMap()
{
  QgsDebugMsg( "Entering." );

  if ( !mGeometry )
  {
    QgsDebugMsg( "Loading feature" );
    updateGeometry( 0 );
  }

  Q_ASSERT( mGeometry );

  // createvertexmap for correct geometry type
  switch ( mGeometry->type() )
  {
    case QGis::Polygon:
      createVertexMapPolygon();
      break;

    case QGis::Line:
      createVertexMapLine();
      break;

    case QGis::Point:
      createVertexMapPoint();
      break;

    case QGis::UnknownGeometry:
    case QGis::NoGeometry:
      break;
  }
}

void SelectedFeature::selectVertex( int vertexNr )
{
  VertexEntry *entry = mVertexMap[vertexNr];
  entry->setSelected();
  entry->update();

  if ( entry->equals() != -1 )
  {
    // select both vertexes if this is first/last vertex
    entry = mVertexMap[ entry->equals()];
    entry->setSelected();
    entry->update();
  }
}

void SelectedFeature::deselectVertex( int vertexNr )
{
  VertexEntry *entry = mVertexMap[vertexNr];
  entry->setSelected( false );
  entry->update();

  if ( entry->equals() != -1 )
  {
    // deselect both vertexes if this is first/last vertex
    entry = mVertexMap[ entry->equals()];
    entry->setSelected( false );
    entry->update();
  }
}

void SelectedFeature::deselectAllVertexes()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setSelected( false );
    mVertexMap[i]->update();
  }
}

void SelectedFeature::invertVertexSelection( int vertexNr, bool invert )
{
  VertexEntry *entry = mVertexMap[vertexNr];

  bool selected = !entry->isSelected();

  entry->setSelected( selected );
  entry->update();

  if ( entry->equals() != -1 && invert )
  {
    entry = mVertexMap[ entry->equals()];
    entry->setSelected( selected );
    entry->update();
  }
}

void SelectedFeature::updateVertexMarkersPosition()
{
  // function for on-line updating vertex markers without refresh of canvas
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    VertexEntry *entry = mVertexMap[i];
    entry->setCenter( entry->point() );
    entry->update();
  }
}

QgsFeatureId SelectedFeature::featureId()
{
  return mFeatureId;
}

QList<VertexEntry*> &SelectedFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer* SelectedFeature::vlayer()
{
  return mVlayer;
}
