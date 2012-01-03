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
  //create rubber band marker for moving points
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

QgsMapToolNodeTool::QgsMapToolNodeTool( QgsMapCanvas* canvas ): QgsMapToolVertexEdit( canvas )
{
  mSelectionFeature = NULL;
  mQRubberBand = NULL;
  mSelectAnother = false;
  mCtrl = false;
  mMoving = true;
  mClicked = false;
  mChangingGeometry = false;
  mIsPoint = false;
  //signal handling change of layer structure
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( layersChanged() ) );
  //signal when destination srs changed to repaint coordinates
  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( coordinatesChanged() ) );
  //signal changing of coordinate renderer changed to repaint markers
  connect( canvas->mapRenderer(), SIGNAL( hasCrsTransformEnabled( bool ) ), this, SLOT( coordinatesChanged() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( coordinatesChanged() ) );
  //signal changing of current layer
  connect( QgisApp::instance()->legend(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ),
           this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );
}

QgsMapToolNodeTool::~QgsMapToolNodeTool()
{
  removeRubberBands();
}

void QgsMapToolNodeTool::layersChanged()
{
  //deselection of feature when layer has changed
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( mSelectionFeature != NULL && mSelectionFeature->vlayer() != vlayer )
  {
    delete mSelectionFeature;
    mSelectionFeature = NULL;
  }
}

void QgsMapToolNodeTool::currentLayerChanged( QgsMapLayer* layer )
{
  //deselection of feature when current layer has changed
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( layer );
  if ( mSelectionFeature != NULL && mSelectionFeature->vlayer() != vlayer )
  {
    delete mSelectionFeature;
    mSelectionFeature = NULL;
  }
}


void QgsMapToolNodeTool::featureDeleted( QgsFeatureId featureId )
{
  //check if deleted feature is the one selected
  if ( mSelectionFeature != NULL && featureId == mSelectionFeature->featureId() )
  {
    //if it's delete selection and disconnect signals since tool is not used anymore
    delete mSelectionFeature;
    mSelectionFeature = NULL;
    disconnect( mCanvas->currentLayer(), SIGNAL( featureDeleted( QgsFeatureId ) ) );
    disconnect( mCanvas->currentLayer(), SIGNAL( layerModified( bool ) ) );
  }
}

void QgsMapToolNodeTool::layerModified( bool onlyGeometry )
{
  Q_UNUSED( onlyGeometry );
  //handling modification of
  QgsFeature feat;
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( mSelectionFeature && !mChangingGeometry && vlayer->featureAtId( mSelectionFeature->featureId(), feat, true, false ) )
  {
    if ( !feat.geometry()->isGeosEqual( *mSelectionFeature->feature()->geometry() ) )
    {
      mSelectionFeature->updateFromFeature();
      //throw error
    }
  }
}

void QgsMapToolNodeTool::createMovingRubberBands()
{
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );

  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  QList<VertexEntry*> &vertexMap = mSelectionFeature->vertexMap();
  QgsGeometry* geometry = mSelectionFeature->feature()->geometry();
  int beforeVertex, afterVertex;
  int lastRubberBand = 0;
  int vertex;
  for ( int i = 0; i < vertexMap.size(); i++ )
  {
    //create rubber band
    if ( vertexMap[i]->isSelected() && !vertexMap[i]->isInRubberBand() )
    {
      geometry->adjacentVertices( i, beforeVertex, afterVertex );
      vertex = i;
      while ( beforeVertex !=  -1 )
      { //move forward NOTE: end if whole cycle is selected
        if ( vertexMap[beforeVertex]->isSelected() && beforeVertex != i ) //and take care of cycles
        {
          vertex = beforeVertex;
          geometry->adjacentVertices( vertex, beforeVertex, afterVertex );
        }
        else
        { //break if cycle is found
          break;
        }
      }
      //we have first vertex of moving part
      //create rubber band and set default paramaters
      QgsRubberBand* rb = new QgsRubberBand( mCanvas, false );
      rb->setWidth( 2 );
      rb->setColor( Qt::blue );
      int index = 0;
      if ( beforeVertex != -1 ) //adding first point which is not moving
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[beforeVertex]->point() ), false );
        vertexMap[beforeVertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
      }
      while ( vertex != -1 && vertexMap[vertex]->isSelected() && !vertexMap[vertex]->isInRubberBand() )
      {
        //topology rubber band creation if needed
        if ( topologicalEditing )
        {
          createTopologyRubbedBands( vlayer, vertexMap, vertex );
        }
        //adding point which will be moved
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex]->point() ), false );
        //setting values about added vertex
        vertexMap[vertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
        geometry->adjacentVertices( vertex, beforeVertex, vertex );
      }
      if ( vertex != -1 && !vertexMap[vertex]->isSelected() ) //add last point not moving if exists
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex]->point() ), true );
        vertexMap[vertex]->setRubberBandValues( true, lastRubberBand, index );
        index++;
      }
      mQgsRubberBands.append( rb );
      lastRubberBand++;
    }
  }
}

void QgsMapToolNodeTool::createTopologyRubbedBands( QgsVectorLayer* vlayer, const QList<VertexEntry*> &vertexMap, int vertex )
{
  QMultiMap<double, QgsSnappingResult> currentResultList;
  QgsGeometry* geometry = mSelectionFeature->feature()->geometry();

  //snapp from current vertex
  currentResultList.clear();
  vlayer->snapWithContext( vertexMap[vertex]->point(), ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToVertex );
  QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

  for ( ; resultIt != currentResultList.end(); ++resultIt )
  {
    //move all other
    if ( mSelectionFeature->featureId() != resultIt.value().snappedAtGeometry )
    {
      if ( mTopologyMovingVertexes.contains( resultIt.value().snappedAtGeometry ) )
      {
        if ( mTopologyMovingVertexes[resultIt.value().snappedAtGeometry]->contains( resultIt.value().snappedVertexNr ) )
        {
          //skip vertex already exists in some rubberband
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
      int tVertexFirst = tVertex;//vertex number to check for cycling
      QgsFeature topolFeature;

      vlayer->featureAtId( resultIt.value().snappedAtGeometry, topolFeature, true, false );
      QgsGeometry* topolGeometry = topolFeature.geometry();

      while ( tVertex != -1 ) //looking for first vertex to rubber band
      {
        tVertexBackup = tVertex;
        topolGeometry->adjacentVertices( tVertex, tVertex, tVertexAfter );
        if ( tVertex == -1 || tVertex == tVertexFirst )
          break;//chceck if this is not first vertex of the feature or cycling error
        //if closest vertex is not from selected feature or is not selected end
        double dist;
        QgsPoint point = topolGeometry->vertexAt( tVertex );
        int at, before, after;
        geometry->closestVertex( point, at, before, after, dist );
        if ( dist > ZERO_TOLERANCE || !vertexMap[at]->isSelected() ) //problem with double precision
        {
          break; //found first vertex
        }
      }

      int movingPointIndex = 0;
      Vertexes* movingPoints = new Vertexes();
      Vertexes* addedPoints = new Vertexes();
      if ( mTopologyMovingVertexes.contains( resultIt.value().snappedAtGeometry ) )
      {
        addedPoints = mTopologyMovingVertexes[ resultIt.value().snappedAtGeometry ];
      }
      if ( tVertex == -1 ) //adding first point if needed
      {
        tVertex = tVertexBackup;
      }
      else
      {
        trb->addPoint( topolGeometry->vertexAt( tVertex ) );
        if ( tVertex == tVertexFirst ) //cycle first vertex need to be added also
        {
          movingPoints->insert( movingPointIndex );
        }
        movingPointIndex = 1;
        topolGeometry->adjacentVertices( tVertex, tVertexAfter, tVertex );
      }

      while ( tVertex != -1 )
      {
        //if closest vertex is not from selected feature or is not selected end
        double dist;
        QgsPoint point = topolGeometry->vertexAt( tVertex );
        int at, before, after;
        geometry->closestVertex( point, at, before, after, dist );
        // find first no matching vertex
        if ( dist > ZERO_TOLERANCE || !vertexMap[at]->isSelected() ) //problem with double precision
        {
          trb->addPoint( topolGeometry->vertexAt( tVertex ) );
          break; //found first vertex
        }
        else //add moving point to rubber band
        {
          if ( addedPoints->contains( tVertex ) )
            break; //just preventing to circle
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

void QgsMapToolNodeTool::coordinatesChanged()
{
  //need to update vertex markers since coordinate systems have changed
  if ( mSelectionFeature != NULL )
  {
    mSelectionFeature->updateVertexMarkersPosition();
  }
}

void QgsMapToolNodeTool::canvasMoveEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  }

  if ( mSelectionFeature == NULL )
  {
    return; // check that we are able to move something
  }
  if ( mClicked )
  {
    mSelectAnother = false;
    if ( mMoving )
    {
      //create rubberband if none exists
      if ( mQgsRubberBands.empty() )
      {
        if ( mIsPoint )
        {
          QList<VertexEntry*> &vertexMap = mSelectionFeature->vertexMap();
          for ( int i = 0; i < vertexMap.size(); i++ )
          {
            if ( vertexMap[i]->isSelected() )
            {
              QgsRubberBand* rb = createRubberBandMarker( vertexMap[i]->point(), vlayer );
              mQgsRubberBands.append( rb );
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
        //move rubber band
        QList<QgsSnappingResult> snapResults;
        QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
        QList<QgsPoint> excludePoints;
        excludePoints.append( mClosestVertex );
        mSnapper.snapToBackgroundLayers( e->pos(), snapResults, excludePoints );
        //get correct coordinates to move
        QgsPoint posMapCoord = snapPointFromResults( snapResults, e->pos() );
        if ( snapResults.size() > 0 )
        {
          firstCoords = toMapCoordinates( vlayer, mClosestVertex );
        }

        //special handling of points
        if ( mIsPoint )
        {
          double offsetX = posMapCoord.x() - firstCoords.x();
          double offsetY = posMapCoord.y() - firstCoords.y();
          for ( int i = 0; i < mQgsRubberBands.size(); i++ )
          {
            mQgsRubberBands[i]->setTranslationOffset( offsetX, offsetY );
          }
          return;
        }

        //move points
        QList<VertexEntry*> &vertexMap = mSelectionFeature->vertexMap();
        for ( int i = 0; i < vertexMap.size(); i++ )
        {

          if ( vertexMap[i]->isSelected() )
          {
            QgsPoint mapCoords = toMapCoordinates( vlayer, vertexMap[i]->point() );
            double x = mapCoords.x() + posMapCoord.x() - firstCoords.x();
            double y = mapCoords.y() + posMapCoord.y() - firstCoords.y();

            mQgsRubberBands[vertexMap[i]->rubberBandNr()]->movePoint( vertexMap[i]->index(), QgsPoint( x, y ) );
            if ( vertexMap[i]->index() == 0 )
            {
              mQgsRubberBands[vertexMap[i]->rubberBandNr()]->movePoint( 0, QgsPoint( x, y ) );
            }
          }
        }

        //topological editing
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
        mQRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
        mRect = new QRect();
        mRect->setTopLeft( QPoint( mLastCoordinates->x(), mLastCoordinates->y() ) );
      }
      mRect->setBottomRight( e->pos() );
      QRect normalizedRect = mRect->normalized();
      mQRubberBand->setGeometry( normalizedRect );
      mQRubberBand->show();
    }
  }

}

void QgsMapToolNodeTool::connectSignals( QgsVectorLayer* vlayer )
{
  connect( vlayer, SIGNAL( featureDeleted( QgsFeatureId ) ), this, SLOT( featureDeleted( QgsFeatureId ) ) );
  connect( vlayer, SIGNAL( layerModified( bool ) ), this, SLOT( layerModified( bool ) ) );
}

bool QgsMapToolNodeTool::checkCorrectnessOfFeature( QgsVectorLayer *vlayer )
{
  QgsFeature feat;
  if ( !vlayer->featureAtId( mSelectionFeature->featureId(), feat, true, false ) )
  {
    qDebug( "feature doesn't exist any more" );
    QMessageBox::warning( NULL,
                          tr( "Node tool" ),
                          tr( "Feature was deleted on background." ),
                          QMessageBox::Ok ,
                          QMessageBox::Ok );
    delete mSelectionFeature;
    mSelectionFeature = NULL;
    return false;
  }
  try
  {
    if ( !feat.geometry()->isGeosEqual( *mSelectionFeature->feature()->geometry() ) )
    {
      //update markers when geometries are not valid
      mSelectionFeature->updateFromFeature();
    }
  }
  catch ( ... )
  {
    // for cases when geos throws an exception for example for invalid polygon
    return true;
  }
  return true;
}




void QgsMapToolNodeTool::canvasPressEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  }
  mClicked = true;
  mLastCoordinates = new QgsPoint( e->pos().x(), e->pos().y() );
  QList<QgsSnappingResult> snapResults;
  if ( mSelectionFeature == NULL )
  {
    mSelectAnother = false;
    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertexAndSegment, -1 );

    if ( snapResults.size() < 1 )
    {
      displaySnapToleranceWarning();
      return;
    }
    mSelectionFeature = new SelectionFeature();
    mSelectionFeature->setSelectedFeature( snapResults[0].snappedAtGeometry,  vlayer,  NULL, mCanvas );
    mIsPoint = vlayer->dataProvider()->geometryType() == QGis::WKBPoint
               || vlayer->dataProvider()->geometryType() == QGis::WKBMultiPoint;
    connectSignals( vlayer );
  }
  else
  {
    //check if feature is not modified from backgound and still exists
    //Note this code should never be reached signals should handle this error
    if ( !checkCorrectnessOfFeature( vlayer ) )
      return;

    //some feature already selected
    QgsPoint mapCoordPoint = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
    double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapRenderer() );
    //get geometry and find if snapping is near it
    QgsFeature f;
    vlayer->featureAtId( mSelectionFeature->featureId(), f, true, false );
    QgsGeometry* g = f.geometry();
    int atVertex, beforeVertex, afterVertex;
    double dist;
    g->closestVertex( toLayerCoordinates( vlayer, mapCoordPoint ), atVertex, beforeVertex, afterVertex, dist );
    dist = sqrt( dist );

    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertex, tol );
    //if (snapResults.size() < 1)
    if ( dist > tol )
    {
      //for points only selecting another feature
      //no vertexes found (selecting or inverting selection) if move
      //or select another feature if clicked there
      QgsSnapper::SnappingType snapType = QgsSnapper::SnapToSegment;
      if ( mIsPoint )
      {
        snapType = QgsSnapper::SnapToVertex;
      }
      mSnapper.snapToCurrentLayer( e->pos(), snapResults, snapType, tol );
      if ( snapResults.size() > 0 )
      {
        //need to check all if there is a point in my selected feature
        mAnother = snapResults.first().snappedAtGeometry;
        mSelectAnother = true;
        QList<QgsSnappingResult>::iterator it = snapResults.begin();
        QgsSnappingResult snapResult;
        for ( ; it != snapResults.end(); ++it )
        {
          if ( it->snappedAtGeometry == mSelectionFeature->featureId() )
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
          mClosestVertex = getClosestVertex( toLayerCoordinates( vlayer, point ) );
          if ( !mSelectionFeature->isSelected( snapResult.beforeVertexNr ) ||
               !mSelectionFeature->isSelected( snapResult.afterVertexNr ) )
          {
            mSelectionFeature->deselectAllVertexes();
            mSelectionFeature->selectVertex( snapResult.afterVertexNr );
            mSelectionFeature->selectVertex( snapResult.beforeVertexNr );
          }
        }
      }
      else
      {
        if ( !mCtrl )
        {
          mSelectionFeature->deselectAllVertexes();
        }
      }
    }
    else
    {
      //some vertex selected
      mMoving = true;
      QgsPoint point = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
      mClosestVertex = getClosestVertex( toLayerCoordinates( vlayer, point ) );
      if ( mMoving )
      {
        if ( mCtrl )
        {
          mSelectionFeature->invertVertexSelection( atVertex );
        }
        else
        {
          if ( !( mSelectionFeature->isSelected( atVertex ) ) )
          {
            mSelectionFeature->deselectAllVertexes();
            mSelectionFeature->selectVertex( atVertex );
          }
        }
      }
      else
      {
        //select another feature
        mAnother = snapResults.first().snappedAtGeometry;
        mSelectAnother = true;
      }
    }
  }

}

void QgsMapToolNodeTool::canvasReleaseEvent( QMouseEvent * e )
{
  if ( mSelectionFeature == NULL )
  {
    // no feature is selected
    return;
  }
  removeRubberBands();
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  }

  mClicked = false;
  mSelectionRectangle = false;
  QgsPoint coords = toMapCoordinates( e->pos() );
  //QgsPoint coords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(),  e->pos().y() );
  //QgsPoint firstCoords = toMapCoordinates( *mLastCoordinates );
  QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
  if ( mQRubberBand != NULL )
  {
    mQRubberBand->close();
    delete mQRubberBand;
    mQRubberBand = NULL;
  }
  if ( mLastCoordinates->x() == e->pos().x() && mLastCoordinates->y() == e->pos().y() )
  {
    if ( mSelectAnother )
    {
      //select another feature (this should deselect current one ;-) )
      mSelectionFeature->setSelectedFeature( mAnother, vlayer, NULL, mCanvas );
      mIsPoint = vlayer->dataProvider()->geometryType() == QGis::WKBPoint ||
                 vlayer->dataProvider()->geometryType() == QGis::WKBMultiPoint;
      mSelectAnother = false;
    }
  }
  else
  {
    // coordinates has to be coordinates from layer not canvas
    QgsPoint layerCoords = toLayerCoordinates( vlayer, coords );
    QgsPoint layerFirstCoords = toLayerCoordinates( vlayer, firstCoords );

    //got correct coordinates
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
      //coords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(), e->pos().y() );
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
      mChangingGeometry = true;
      mSelectionFeature->moveSelectedVertexes( changeX, changeY );
      mCanvas->refresh();
      mChangingGeometry = false;
      //movingVertexes
    }
    else //selecting vertexes by rubber band
    {
      QList<VertexEntry*> &vertexMap = mSelectionFeature->vertexMap();
      if ( !mCtrl )
      {
        mSelectionFeature->deselectAllVertexes();
      }
      for ( int i = 0; i < vertexMap.size(); i++ )
      {
        if ( vertexMap[i]->point().x() < bottomX && vertexMap[i]->point().y() > leftY
             && vertexMap[i]->point().x() > topX    && vertexMap[i]->point().y() < rightY )
        {
          //inverting selection is enough because all were deselected if ctrl is not pressed
          mSelectionFeature->invertVertexSelection( i, false );
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
  delete mSelectionFeature;
  mSelectionFeature = NULL;

  disconnect( mCanvas->currentLayer(), SIGNAL( featureDeleted( QgsFeatureId ) ) );
  disconnect( mCanvas->currentLayer(), SIGNAL( layerModified( bool ) ) );

  mQRubberBand = NULL;
  mSelectAnother = false;
  mCtrl = false;
  mMoving = true;
  mClicked = false;
  QgsMapTool::deactivate();
}

void QgsMapToolNodeTool::removeRubberBands()
{
  //cleanup rubber bands and list
  QList<QgsRubberBand*>::iterator rb_it = mQgsRubberBands.begin();
  for ( ; rb_it != mQgsRubberBands.end(); ++rb_it )
  {
    delete *rb_it;
  }
  mQgsRubberBands.clear();

  rb_it = mTopologyRubberBand.begin();
  for ( ; rb_it != mTopologyRubberBand.end(); ++rb_it )
  {
    delete *rb_it;
  }
  mTopologyRubberBand.clear();

  mTopologyMovingVertexes.clear();
  mTopologyRubberBandVertexes.clear();

  //remove all data from selected feature (no change to rubber bands itself)
  if ( mSelectionFeature != NULL )
    mSelectionFeature->cleanRubberBandsData();
}


void QgsMapToolNodeTool::canvasDoubleClickEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;
  if ( currentLayer )
  {
    vlayer = qobject_cast<QgsVectorLayer *>( currentLayer );
  }

  QList<QgsSnappingResult> snapResults;
  mMoving = false;
  double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapRenderer() );
  mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToSegment, tol );
  if ( snapResults.size() < 1 )
  {
    //nowhere to pul vertex
  }
  else
  {
    //some segment selected
    if ( snapResults.first().snappedAtGeometry == mSelectionFeature->featureId() )
    {
      if ( snapResults.first().snappedVertexNr == -1 )
      {
        QgsPoint coords = snapResults.first().snappedVertex;
        QgsPoint layerCoords = toLayerCoordinates( vlayer, coords );
        if ( topologicalEditing )
        {
          //snapp from adding position to this vertex when topological editing is enabled
          currentResultList.clear();
          vlayer->snapWithContext( layerCoords, ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToSegment );
        }

        vlayer->beginEditCommand( tr( "Inserted vertex" ) );
        mChangingGeometry = true;

        //add vertex
        vlayer->insertVertex( layerCoords.x(), layerCoords.y(), mSelectionFeature->featureId(), snapResults.first().afterVertexNr );

        if ( topologicalEditing )
        {
          QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

          for ( ; resultIt != currentResultList.end(); ++resultIt )
          {
            //create vertexes on same position when topological editing is enabled
            if ( mSelectionFeature->featureId() !=  resultIt.value().snappedAtGeometry )
              vlayer->insertVertex( layerCoords.x(), layerCoords.y(), resultIt.value().snappedAtGeometry, resultIt.value().afterVertexNr );
          }
        }

        vlayer->endEditCommand();

        // make sure that new node gets its vertex marker
        mCanvas->refresh();

        mSelectionFeature->updateFromFeature();
        mChangingGeometry = false;
      }
    }
  }
}

QgsPoint QgsMapToolNodeTool::getClosestVertex( QgsPoint point )
{
  int at;
  int before;
  int after;
  double dist;
  return mSelectionFeature->feature()->geometry()->closestVertex( point, at, before, after, dist );
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
  if ( mSelectionFeature && e->key() == Qt::Key_Delete )
  {
    mChangingGeometry = true;
    mSelectionFeature->deleteSelectedVertexes();
    mCanvas->refresh();
    mChangingGeometry = false;
  }
}


//selection object


SelectionFeature::SelectionFeature()
{
  mFeature = new QgsFeature();
  mFeatureId = 0;
  mFeatureSelected = false;
  mValidator = 0;
}

SelectionFeature::~SelectionFeature()
{
  deleteVertexMap();

  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }
}

void SelectionFeature::updateFeature()
{
  if ( mFeatureSelected )
  {
    delete mFeature;
    mFeature = new QgsFeature();
    mVlayer->featureAtId( mFeatureId, *mFeature );
  }
}

void SelectionFeature::cleanRubberBandsData()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setRubberBandValues( false, 0, 0 );
  }
}

void SelectionFeature::setSelectedFeature( QgsFeatureId featureId,
    QgsVectorLayer* vlayer,
    QgsRubberBand* rubberBand,
    QgsMapCanvas* canvas,
    QgsFeature* feature )
{
  if ( mFeatureSelected )
  {
    deleteVertexMap();
  }
  mFeatureSelected = true;
  mFeatureId = featureId;
  mVlayer = vlayer;
  mCanvas = canvas;
  mRubberBand = rubberBand;
  if ( feature == NULL )
  {
    vlayer->featureAtId( featureId, *mFeature );
  }
  else
  {
    mFeature = feature;
  }

  //createvertexmap
  createVertexMap();

  validateGeometry();
}

void SelectionFeature::validateGeometry( QgsGeometry *g )
{
  QSettings settings;
  if ( settings.value( "/qgis/digitizing/validate_geometries", 1 ).toInt() == 0 )
    return;

  if ( !g )
    g = mFeature->geometry();

  mTip.clear();

  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    delete mGeomErrorMarkers.takeFirst();
  }

  if ( mValidator )
  {
    mValidator->deleteLater();
    mValidator = 0;
  }

  mValidator = new QgsGeometryValidator( g );
  connect( mValidator, SIGNAL( errorFound( QgsGeometry::Error ) ), this, SLOT( addError( QgsGeometry::Error ) ) );
  connect( mValidator, SIGNAL( finished() ), this, SLOT( validationFinished() ) );
  mValidator->start();

  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation started." ) );
}

void SelectionFeature::addError( QgsGeometry::Error e )
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

void SelectionFeature::validationFinished()
{
  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation finished." ) );
}

void SelectionFeature::deleteSelectedVertexes()
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
        //to avoid try to delete some vertex twice
        mVertexMap[ mVertexMap[i]->equals()]->setSelected( false );
      }

      if ( topologicalEditing )
      {
        //snapp from current vertex
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
          //move all other
          if ( mFeatureId !=  resultIt.value().snappedAtGeometry )
            mVlayer->deleteVertex( resultIt.value().snappedAtGeometry, resultIt.value().snappedVertexNr );
        }
      }
    }
  }
  QgsFeature f;
  mVlayer->featureAtId( mFeatureId, f, true, false );

  bool wasValid = false; // mGeomErrors.isEmpty();
  bool isValid = f.geometry()->isGeosValid();
  if ( wasValid && !isValid )
  {
    QMessageBox::warning( NULL,
                          tr( "Node tool" ),
                          tr( "Result geometry is invalid. Reverting last changes." ),
                          QMessageBox::Ok ,
                          QMessageBox::Ok );
  }

  if ( count != 0 && ( !wasValid || isValid ) )
  {
    mVlayer->endEditCommand();
    validateGeometry( f.geometry() );
  }
  else
  {
    mVlayer->destroyEditCommand(); // no changes... or invalid changes
  }

  updateFromFeature();
}

void SelectionFeature::moveSelectedVertexes( double changeX, double changeY )
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
        //snapp from current vertex
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
          //move all other
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
        //for polygon delete both
      }
    }
  }
  QgsFeature f;
  mVlayer->featureAtId( mFeatureId, f, true, false );
  bool wasValid = false; // mGeomErrors.isEmpty();
  bool isValid = f.geometry()->isGeosValid();
  if ( wasValid && !isValid )
  {
    QMessageBox::warning( NULL,
                          tr( "Node tool" ),
                          tr( "Result geometry is invalid. Reverting last changes." ),
                          QMessageBox::Ok ,
                          QMessageBox::Ok );
    //redo last operations
    mVlayer->destroyEditCommand();
    updateFromFeature();
  }
  else
  {
    mVlayer->endEditCommand();
    validateGeometry( f.geometry() );
  }
  updateFeature();
}

void SelectionFeature::updateFromFeature()
{
  //delete old map
  deleteVertexMap();

  //create new map
  createVertexMap();
}

void SelectionFeature::deleteVertexMap()
{
  foreach( VertexEntry *entry, mVertexMap )
  {
    delete entry;
  }

  mVertexMap.clear();
}

bool SelectionFeature::isSelected( int vertexNr )
{
  return mVertexMap[vertexNr]->isSelected();
}

QgsFeature *SelectionFeature::feature()
{
  return mFeature;
}

void SelectionFeature::createVertexMapPolygon()
{
  int y = 0;
  QgsPolygon polygon = mFeature->geometry()->asPolygon();
  if ( !polygon.empty() )
  { //polygon
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
  else //multipolygon
  {
    QgsMultiPolygon multiPolygon = mFeature->geometry()->asMultiPolygon();
    for ( int i2 = 0; i2 < multiPolygon.size(); i2++ )
    { //iterating through polygons
      const QgsPolygon& poly2 = multiPolygon[i2];
      for ( int i3 = 0; i3 < poly2.size(); i3++ )
      { //iterating through polygon rings
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

void SelectionFeature::createVertexMapLine()
{
  if ( mFeature->geometry()->isMultipart() )
  {
    int y = 0;
    QgsMultiPolyline mLine = mFeature->geometry()->asMultiPolyline();
    for ( int i2 = 0; i2 < mLine.size(); i2++ )
    {
      //iterating through polylines
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
    QgsPolyline poly = mFeature->geometry()->asPolyline();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      mVertexMap.insert( i, new VertexEntry( mCanvas, mVlayer, poly[i], i, tr( "vertex %1" ).arg( i ) ) );
    }
  }
}

void SelectionFeature::createVertexMapPoint()
{
  if ( mFeature->geometry()->isMultipart() )
  {
    //multipoint
    QgsMultiPoint poly = mFeature->geometry()->asMultiPoint();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      mVertexMap[i]  = new VertexEntry( mCanvas, mVlayer, poly[i], 1, tr( "point %1" ).arg( i ) );
    }
  }
  else
  {
    //single point
    mVertexMap[ 1] = new VertexEntry( mCanvas, mVlayer, mFeature->geometry()->asPoint(), 1, tr( "single point" ) );
  }
}

void SelectionFeature::createVertexMap()
{
  mVlayer->featureAtId( mFeatureId, *mFeature );
  //createvertexmap for correct geometry type
  if ( mFeature->geometry()->type() == QGis::Polygon )
  {
    createVertexMapPolygon();
  }
  else if ( mFeature->geometry()->type() == QGis::Line )
  {
    createVertexMapLine();
  }
  else if ( mFeature->geometry()->type() == QGis::Point )
  {
    createVertexMapPoint();
  }
}

void SelectionFeature::selectVertex( int vertexNr )
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

void SelectionFeature::deselectVertex( int vertexNr )
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

void SelectionFeature::deselectAllVertexes()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setSelected( false );
    mVertexMap[i]->update();
  }
}

void SelectionFeature::invertVertexSelection( int vertexNr, bool invert )
{
  VertexEntry *entry = mVertexMap[vertexNr];

  bool selected = entry->isSelected();

  entry->setSelected( selected );
  entry->update();

  if ( entry->equals() != -1 && invert )
  {
    entry = mVertexMap[ mVertexMap[vertexNr]->equals()];
    entry->setSelected( selected );
    entry->update();
  }
}

void SelectionFeature::updateVertexMarkersPosition()
{
  //function for on-line updating vertex markers without refresh of canvas
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    VertexEntry *entry = mVertexMap[i];
    entry->setCenter( entry->point() );
    entry->update();
  }
}

QgsFeatureId SelectionFeature::featureId()
{
  return mFeatureId;
}

QList<VertexEntry*> &SelectionFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer* SelectionFeature::vlayer()
{
  return mVlayer;
}
