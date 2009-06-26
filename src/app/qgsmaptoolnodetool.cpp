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
#include <math.h>
#include <QMouseEvent>
#include <QMessageBox>

QgsRubberBand* QgsMapToolNodeTool::createRubberBandMarker( QgsPoint center, QgsVectorLayer* vlayer )
{
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
  connect( canvas, SIGNAL( layersChanged() ), this, SLOT( layersChanged() ) );
  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( coordinatesChanged() ) );
}

QgsMapToolNodeTool::~QgsMapToolNodeTool()
{
  removeRubberBands();
}

void QgsMapToolNodeTool::layersChanged()
{
  QgsVectorLayer* vlayer = vlayer = dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
  if ( mSelectionFeature != NULL && mSelectionFeature->vlayer() != vlayer )
  {
    delete mSelectionFeature;
    mSelectionFeature = NULL;
  }
}

void QgsMapToolNodeTool::currentLayerChanged( QgsMapLayer* layer )
{
  QgsVectorLayer* vlayer = vlayer = dynamic_cast<QgsVectorLayer*>( layer );
  if ( mSelectionFeature != NULL && mSelectionFeature->vlayer() != vlayer )
  {
    delete mSelectionFeature;
    mSelectionFeature = NULL;
  }
}


void QgsMapToolNodeTool::featureDeleted( int featureId )
{
  if ( mSelectionFeature != NULL && featureId == mSelectionFeature->featureId() )
  {
    delete mSelectionFeature;
    mSelectionFeature = NULL;
    disconnect( mCanvas->currentLayer(), SIGNAL( featureDeleted( int ) ) );
    disconnect( mCanvas->currentLayer(), SIGNAL( layerModified( bool ) ) );
  }
}

void QgsMapToolNodeTool::layerModified( bool onlyGeometry )
{
  QgsFeature feat;
  QgsVectorLayer* vlayer = vlayer = dynamic_cast<QgsVectorLayer*>( mCanvas->currentLayer() );
  if ( mSelectionFeature != NULL && !mChangingGeometry && ( vlayer->featureAtId( mSelectionFeature->featureId(), feat, true, false ) ) )
  {
    if ( !GEOSEquals( feat.geometry()->asGeos(), mSelectionFeature->feature()->geometry()->asGeos() ) )
    {
      qDebug( "update from feature" );
      mSelectionFeature->updateFromFeature();
      //throw error
    }
  }
}

void QgsMapToolNodeTool::createMovingRubberBands()
{
  QList<VertexEntry> vertexMap = mSelectionFeature->vertexMap();
  QgsGeometry* geometry = mSelectionFeature->feature()->geometry();
  int beforeVertex, afterVertex;
  int lastRubberBand = 0;
  int vertex;
  for ( int i = 0; i < vertexMap.size(); i++ )
  {
    //create rubber band
    if ( vertexMap[i].selected && !( vertexMap[i].inRubberBand ) )
    {
      geometry->adjacentVertices( i, beforeVertex, afterVertex );
      vertex = i;
      while ( beforeVertex !=  -1 )
      { //move forward NOTE: end if whole cycle is selected
        if ( vertexMap[beforeVertex].selected && beforeVertex != i ) //and take care of cycles
        {
          vertex = beforeVertex;
          geometry->adjacentVertices( vertex, beforeVertex, afterVertex );
        }
        else
        { //break
          break;
        }
      }
      //we have first vertex of moving part
      QgsRubberBand* rb = new QgsRubberBand( mCanvas, false );
      rb->setWidth( 2 );
      rb->setColor( Qt::blue );
      int index = 0;
      if ( beforeVertex != -1 ) //adding first point which is not moving
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[beforeVertex].point ), false );
        mSelectionFeature->setRubberBandValues( beforeVertex, true, lastRubberBand, index );
        vertexMap[beforeVertex].inRubberBand = true;
        index ++;
      }
      while ( vertex != -1 && vertexMap[vertex].selected && !vertexMap[vertex].inRubberBand )
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex].point ), false );
        mSelectionFeature->setRubberBandValues( vertex, true, lastRubberBand, index );
        vertexMap[vertex].inRubberBand = true;
        index ++;
        geometry->adjacentVertices( vertex, beforeVertex, vertex );
      }
      if ( vertex != -1 && !vertexMap[vertex].selected ) //add last point if exists
      {
        rb->addPoint( toMapCoordinates( mCanvas->currentLayer(), vertexMap[vertex].point ), true );
        mSelectionFeature->setRubberBandValues( vertex, true, lastRubberBand, index );
        vertexMap[vertex].inRubberBand = true;
        index ++;
      }
      mQgsRubberBands.append( rb );
      lastRubberBand ++;
    }
  }
}


void QgsMapToolNodeTool::coordinatesChanged()
{
  if ( mSelectionFeature != NULL )
  {
    mSelectionFeature->updateVertexMarkersPosition( mCanvas );
  }
}



void QgsMapToolNodeTool::canvasMoveEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
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

      //create rubberband
      if ( mQgsRubberBands.empty() )
      {
        if ( mIsPoint )
        {
          QList<VertexEntry> vertexMap = mSelectionFeature->vertexMap();
          for ( int i = 0; i < vertexMap.size(); i++ )
          {
            if ( vertexMap[i].selected )
            {
              QgsRubberBand* rb = createRubberBandMarker( vertexMap[i].point, vlayer );
              mQgsRubberBands.append( rb );
            }
          }
        }
        createMovingRubberBands();
      }
      else
      {
        QList<QgsSnappingResult> snapResults;
        QgsPoint firstCoords = mCanvas->getCoordinateTransform()->toMapPoint( mLastCoordinates->x(), mLastCoordinates->y() );
        QList<QgsPoint> excludePoints;
        excludePoints.append( mClosestVertex );
        mSnapper.snapToBackgroundLayers( e->pos(), snapResults, excludePoints );
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
        QList<VertexEntry> vertexMap = mSelectionFeature->vertexMap();
        for ( int i = 0; i < vertexMap.size(); i++ )
        {

          if ( vertexMap[i].selected )
          {
            QgsPoint mapCoords = toMapCoordinates( vlayer, vertexMap[i].point );
            double x = mapCoords.x() + posMapCoord.x() - firstCoords.x();
            double y = mapCoords.y() + posMapCoord.y() - firstCoords.y();
            //QgsPoint pom = toLayerCoordinates(
            mQgsRubberBands[vertexMap[i].rubberBandNr]->movePoint( vertexMap[i].index + 1, QgsPoint( x, y ) );
            if ( vertexMap[i].index == 0 )
            {
              mQgsRubberBands[vertexMap[i].rubberBandNr]->movePoint( 0, QgsPoint( x, y ) );
            }
          }
        }
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
  connect( vlayer, SIGNAL( featureDeleted( int ) ), this, SLOT( featureDeleted( int ) ) );
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
                          tr( "Feature was deleted on background.\n" ),
                          QMessageBox::Ok ,
                          QMessageBox::Ok );
    delete mSelectionFeature;
    mSelectionFeature = NULL;
    return false;
  }
  if ( !GEOSEquals( feat.geometry()->asGeos(), mSelectionFeature->feature()->geometry()->asGeos() ) )
  {
    //update markers
    mSelectionFeature->updateFromFeature();
  }
  return true;
}




void QgsMapToolNodeTool::canvasPressEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
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
      return;
    }
    mSelectionFeature = new SelectionFeature();
    mSelectionFeature->setSelectedFeature( snapResults[0].snappedAtGeometry,  vlayer,  NULL, mCanvas );
    mIsPoint = (( vlayer->dataProvider()->geometryType() == QGis::WKBPoint ) || ( vlayer->dataProvider()->geometryType() == QGis::WKBMultiPoint ) );
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
        for ( ; it != snapResults.end() ; ++it )
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
          if ( !mSelectionFeature->isSelected( snapResult.beforeVertexNr ) || !mSelectionFeature->isSelected( snapResult.afterVertexNr ) )
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
          //mSelectionFeature->invertVertexSelection( snapResult.snappedVertexNr);
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
    vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
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
      mIsPoint = (( vlayer->dataProvider()->geometryType() == QGis::WKBPoint ) || ( vlayer->dataProvider()->geometryType() == QGis::WKBMultiPoint ) );
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
      if ( snapResults.size() > 0 )
      {
        firstCoords = toMapCoordinates( vlayer, mClosestVertex );
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
      QList<VertexEntry> vertexMap = mSelectionFeature->vertexMap();
      if ( !mCtrl )
      {
        mSelectionFeature->deselectAllVertexes();
      }
      for ( int i = 0; i < vertexMap.size(); i++ )
      {
        if ( vertexMap[i].point.x() < bottomX && vertexMap[i].point.y() > leftY && vertexMap[i].point.x() > topX && vertexMap[i].point.y() < rightY )
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
  disconnect( mCanvas->currentLayer(), SIGNAL( featureDeleted( int ) ) );
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
  for ( ;rb_it != mQgsRubberBands.end(); ++rb_it )
  {
    delete *rb_it;
  }
  mQgsRubberBands.clear();

  //remove all data from selected feature (no change to rubber bands itself
  if ( mSelectionFeature != NULL )
    mSelectionFeature->cleanRubberBandsData();
}


void QgsMapToolNodeTool::canvasDoubleClickEvent( QMouseEvent * e )
{
  QgsMapLayer* currentLayer = mCanvas->currentLayer();
  QgsVectorLayer* vlayer = 0;
  if ( currentLayer )
  {
    vlayer = dynamic_cast<QgsVectorLayer*>( currentLayer );
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
        //QgsPoint coords = mCanvas->getCoordinateTransform()->toMapPoint( e->pos().x(),  e->pos().y() );
        //add vertex
        vlayer->beginEditCommand( tr( "Inserted vertex" ) );
        mChangingGeometry = true;
        vlayer->insertVertex( layerCoords.x(), layerCoords.y(), mSelectionFeature->featureId(), snapResults.first().afterVertexNr );
        vlayer->endEditCommand();

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
  if ( e->key() == Qt::Key_Delete )
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
}

SelectionFeature::~SelectionFeature()
{
  deleteVertexMap();
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
    mVertexMap[i].rubberBandNr = 0;
    mVertexMap[i].index = 0;
    mVertexMap[i].inRubberBand = false;
  }
}

void SelectionFeature::setSelectedFeature( int featureId,  QgsVectorLayer* vlayer,  QgsRubberBand* rubberBand, QgsMapCanvas* canvas, QgsFeature* feature )
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
}

void SelectionFeature::deleteSelectedVertexes()
{
  mVlayer->beginEditCommand( QObject::tr( "Deleted vertices" ) );
  int count = 0;
  for ( int i = mVertexMap.size() - 1; i > -1 ; i-- )
  {
    if ( mVertexMap[i].selected )
    {
      mVlayer->deleteVertex( mFeatureId, i );
      count++;

      if ( mVertexMap[i].equals != -1 && !mVertexMap[mVertexMap[i].equals].selected )
      {
        //for polygon delete both
      }
    }
  }
  if ( count )
    mVlayer->endEditCommand();
  else
    mVlayer->destroyEditCommand(); // no changes...

  updateFromFeature();
}


void SelectionFeature::moveSelectedVertexes( double changeX, double changeY )
{
  mVlayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  for ( int i = mVertexMap.size() - 1; i > -1 ; i-- )
  {
    if ( mVertexMap[i].selected )
    {
      mVlayer->moveVertex( mVertexMap[i].point.x() + changeX, mVertexMap[i].point.y() + changeY, mFeatureId, i );
      mVertexMap[i].point = QgsPoint( mVertexMap[i].point.x() + changeX, mVertexMap[i].point.y() + changeY );
      QgsPoint point = mVertexMap[i].point;
      point = mCanvas->mapRenderer()->layerToMapCoordinates( mVlayer, mVertexMap[i].point );

      mVertexMap[i].vertexMarker->setCenter( point );
      mVertexMap[i].vertexMarker->update();
      if ( mVertexMap[i].equals != -1 && !mVertexMap[mVertexMap[i].equals].selected )
      {
        int index = mVertexMap[i].equals;
        mVertexMap[index].point = QgsPoint( mVertexMap[index].point.x() + changeX, mVertexMap[index].point.y() + changeY );
        mVertexMap[i].vertexMarker->setCenter( point );
        mVertexMap[i].vertexMarker->update();
        //for polygon delete both
      }
    }
  }
  mVlayer->endEditCommand();
  updateFeature();
}

QgsVertexMarker* SelectionFeature::createVertexMarker( QgsPoint center )
{
  QgsVertexMarker* marker = new QgsVertexMarker( mCanvas );
  QgsPoint newCenter = mCanvas->mapRenderer()->layerToMapCoordinates( mVlayer, center );
  marker->setCenter( newCenter );

  marker->setIconType( QgsVertexMarker::ICON_BOX );

  marker->setColor( Qt::red );

  marker->setPenWidth( 2 );

  return marker;
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
  while ( !mVertexMap.empty() )
  {
    VertexEntry entry = mVertexMap.takeLast();
    delete entry.vertexMarker;
  }
}

bool SelectionFeature::isSelected( int vertexNr )
{
  return mVertexMap[vertexNr].selected;
}

QgsFeature* SelectionFeature::feature()
{
  return mFeature;
}

void SelectionFeature::createVertexMapPolygon()
{
  int y = 0;
  if ( !mFeature->geometry()->asPolygon().empty() )
  { //polygon
    for ( int i2 = 0; i2 < mFeature->geometry()->asPolygon().size(); i2++ )
    {
      QgsPolyline poly = mFeature->geometry()->asPolygon()[i2];
      int i;
      for ( i = 0; i < poly.size(); i++ )
      {
        VertexEntry entry;
        entry.selected = false;
        entry.point = poly[i];
        entry.equals = -1;
        entry.rubberBandNr = 0;
        entry.originalIndex = i;
        entry.inRubberBand = false;
        QgsVertexMarker* marker = createVertexMarker( poly[i] );
        entry.vertexMarker = marker;
        mVertexMap.insert( y + i, entry );
      }
      mVertexMap[y + i - 1 ].equals = y;
      mVertexMap[y].equals = y + i - 1;
      y = y + poly.size();
    }
  }
  else //multipolygon
  {
    //print("Number of polygons: " + str(len(feature.geometry().asMultiPolygon())))
    for ( int i2 = 0 ; i2 < mFeature->geometry()->asMultiPolygon().size(); i2++ )
    {
      QgsPolygon poly2 = mFeature->geometry()->asMultiPolygon()[i2];
      for ( int i3 = 0; i3 < poly2.size(); i3++ )
      {
        QgsPolyline poly = poly2[i3];
        int i;
        //print("Number of vertexes:" + str(len(poly)))
        for ( i = 0; i < poly.size(); i++ )
        {
          VertexEntry entry;
          entry.selected = false;
          entry.point = poly[i];
          entry.equals = -1;
          entry.rubberBandNr = 0;
          entry.originalIndex = y + i - 1;
          entry.inRubberBand = false;
          QgsVertexMarker* marker = createVertexMarker( poly[i] );

          entry.vertexMarker = marker;
          mVertexMap.insert( y + i, entry );
        }
        mVertexMap[y + i - 1].equals = y;
        mVertexMap[y].equals = y + i - 1;
        y = y + poly.size();
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
      QgsPolyline poly = mLine[i2];
      int i;
      for ( i = 0; i < poly.size(); i++ )
      {
        VertexEntry entry;
        entry.selected = false;
        entry.point = poly[i];
        entry.equals = -1;
        entry.rubberBandNr = 0;
        entry.originalIndex = i;
        entry.inRubberBand = false;
        QgsVertexMarker* marker = createVertexMarker( poly[i] );
        entry.vertexMarker = marker;
        mVertexMap.insert( y + i, entry );
      }
      y = y + poly.size();
    }
  }
  else
  {
    QgsPolyline poly = mFeature->geometry()->asPolyline();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      VertexEntry entry;
      entry.selected = false;
      entry.point = poly[i];
      entry.equals = -1;
      entry.rubberBandNr = 0;
      entry.originalIndex = i;
      entry.inRubberBand = false;
      QgsVertexMarker* marker = createVertexMarker( poly[i] );
      entry.vertexMarker = marker;
      mVertexMap.insert( i, entry );
    }
  }
}

void SelectionFeature::createVertexMapPoint()
{
  if ( mFeature->geometry()->isMultipart() )
  {//multipoint
    QgsMultiPoint poly = mFeature->geometry()->asMultiPoint();
    int i;
    for ( i = 0; i < poly.size(); i++ )
    {
      VertexEntry entry;
      entry.selected = false;
      entry.point = poly[i];
      entry.equals = -1;
      entry.rubberBandNr = 0;
      entry.originalIndex = 1;
      entry.inRubberBand = false;
      QgsVertexMarker* marker = createVertexMarker( poly[i] );
      entry.vertexMarker = marker;
      mVertexMap.insert( i, entry );
    }
  }
  else
  {//single point
    QgsPoint poly = mFeature->geometry()->asPoint();
    VertexEntry entry;
    entry.selected = false;
    entry.point = poly;
    entry.equals = -1;
    entry.rubberBandNr = 0;
    entry.originalIndex = 1;
    entry.inRubberBand = false;
    QgsVertexMarker* marker = createVertexMarker( poly );
    entry.vertexMarker = marker;
    mVertexMap.insert( 1, entry );
  }
}

void SelectionFeature::createVertexMap()
{
  mVlayer->featureAtId( mFeatureId, *mFeature );
  //createvertexmap
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

void SelectionFeature::setRubberBandValues( int index, bool inRubberBand, int rubberBandNr, int indexInRubberBand )
{
  mVertexMap[index].index = indexInRubberBand;
  mVertexMap[index].inRubberBand = inRubberBand;
  mVertexMap[index].rubberBandNr = rubberBandNr;
}

void SelectionFeature::selectVertex( int vertexNr )
{
  mVertexMap[vertexNr].selected = true;
  mVertexMap[vertexNr].vertexMarker->setColor( Qt::blue );
  mVertexMap[vertexNr].vertexMarker->update();
  if ( mVertexMap[vertexNr].equals != -1 )
  {
    mVertexMap[mVertexMap[vertexNr].equals].selected = true;
    mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->setColor( Qt::blue );
    mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->update();

  }
}


void SelectionFeature::deselectVertex( int vertexNr )
{
  mVertexMap[vertexNr].selected = false;
  mVertexMap[vertexNr].vertexMarker->setColor( Qt::red );
  mVertexMap[vertexNr].vertexMarker->update();
}


void SelectionFeature::deselectAllVertexes()
{
  for ( int i = 0; i < mVertexMap.size() ;i++ )
  {
    mVertexMap[i].selected = false;
    mVertexMap[i].vertexMarker->setColor( Qt::red );
    mVertexMap[i].vertexMarker->update();
  }
}


void SelectionFeature::invertVertexSelection( int vertexNr, bool invert )
{
  if ( mVertexMap[vertexNr].selected == false )
  {
    mVertexMap[vertexNr].selected = true;
    mVertexMap[vertexNr].vertexMarker->setColor( Qt::blue );
    mVertexMap[vertexNr].vertexMarker->update();
    if ( mVertexMap[vertexNr].equals != -1 && invert )
    {
      mVertexMap[mVertexMap[vertexNr].equals].selected = true;
      mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->setColor( Qt::blue );
      mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->update();
    }
  }
  else
  {
    mVertexMap[vertexNr].selected = false;
    mVertexMap[vertexNr].vertexMarker->setColor( Qt::red );
    mVertexMap[vertexNr].vertexMarker->update();
    if ( mVertexMap[vertexNr].equals != -1 && invert )
    {
      mVertexMap[mVertexMap[vertexNr].equals].selected = false;
      mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->setColor( Qt::red );
      mVertexMap[mVertexMap[vertexNr].equals].vertexMarker->update();
    }
  }
}


void SelectionFeature::updateVertexMarkersPosition( QgsMapCanvas* canvas )
{
  for ( int i = 0; i < mVertexMap.size() ;i++ )
  {
    mVertexMap[i].vertexMarker->setCenter( mCanvas->mapRenderer()->layerToMapCoordinates( mVlayer, mVertexMap[i].point ) );
    mVertexMap[i].vertexMarker->update();
  }
}


int SelectionFeature::featureId()
{
  return mFeatureId;
}


QList<VertexEntry> SelectionFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer* SelectionFeature::vlayer()
{
  return mVlayer;
}



