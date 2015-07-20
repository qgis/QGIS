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
#include "nodetool/qgsnodeeditor.h"

#include "qgisapp.h"
#include "qgslayertreeview.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsproject.h"
#include "qgsgeometryrubberband.h"
#include "qgsvectorlayer.h"

#include <QMouseEvent>
#include <QRubberBand>

QgsMapToolNodeTool::QgsMapToolNodeTool( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mSelectedFeature( 0 )
    , mNodeEditor( 0 )
    , mRect( 0 )
    , mIsPoint( false )
{
  mSnapper.setMapCanvas( canvas );
  mCadAllowed = true;
  mSnapOnPress = true;
}

QgsMapToolNodeTool::~QgsMapToolNodeTool()
{
  cleanTool();
}

void QgsMapToolNodeTool::canvasMapPressEvent( QgsMapMouseEvent* e )
{
  QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
  if ( !vlayer )
  {
    return;
  }

  bool ctrlModifier = e->modifiers() & Qt::ControlModifier;
  QList<QgsSnappingResult> snapResults;

  QgsFeatureId bkFeatureId;
  if ( mSelectedFeature )
  {
    bkFeatureId = mSelectedFeature->featureId();
  }

  bool hasVertexSelection = mSelectedFeature && mSelectedFeature->hasSelection();

  if ( !mSelectedFeature || !hasVertexSelection )
  {
    //try to select feature
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
    if ( !vlayer )
    {
      return;
    }

    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertexAndSegment, -1 );

    if ( snapResults.size() < 1 )
    {
      emit messageEmitted( tr( "could not snap to a segment on the current layer." ) );
      return;
    }

    // remove previous warning
    emit messageDiscarded();

    delete mSelectedFeature;
    mSelectedFeature = new QgsSelectedFeature( snapResults[0].snappedAtGeometry, vlayer, mCanvas );
    connect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );
    connect( mSelectedFeature, SIGNAL( destroyed() ), this, SLOT( selectedFeatureDestroyed() ) );
    connect( mSelectedFeature, SIGNAL( lastVertexChanged( const QgsPointV2& ) ), this, SLOT( changeLastVertex( const QgsPointV2& ) ) );
    connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
    mIsPoint = vlayer->geometryType() == QGis::Point;
    mNodeEditor = new QgsNodeEditor( vlayer, mSelectedFeature, mCanvas );
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mNodeEditor );
  }

  //select or move vertices if selected feature has not been changed
  if ( mSelectedFeature->featureId() == bkFeatureId )
  {
    if ( mSelectedFeature->hasSelection() && !ctrlModifier ) //move vertices
    {
      QgsPoint targetCoords = e->mapPoint();
      mSelectedFeature->moveSelectedVertexes( targetCoords - mClosestMapVertex );
      mCanvas->refresh();
      mSelectedFeature->deselectAllVertexes();
    }
    else //add vertex selection
    {
      int atVertex, beforeVertex, afterVertex;
      double dist;

      QgsPoint closestLayerVertex = mSelectedFeature->geometry()->closestVertex( e->mapPoint(), atVertex, beforeVertex, afterVertex, dist );
      mSelectedFeature->selectVertex( atVertex );
      //mClosestMapVertex = toMapCoordinates( vlayer, closestLayerVertex );
    }
  }
}

void QgsMapToolNodeTool::selectedFeatureDestroyed()
{
  QgsDebugCall;
  cleanTool( false );
}

void QgsMapToolNodeTool::currentLayerChanged( QgsMapLayer *layer )
{
  if ( mSelectedFeature && layer != mSelectedFeature->vlayer() )
  {
    cleanTool();
  }
}

void QgsMapToolNodeTool::editingToggled()
{
  cleanTool();
}

void QgsMapToolNodeTool::deactivate()
{
  cleanTool();
  QgsMapTool::deactivate();
}

void QgsMapToolNodeTool::cleanTool( bool deleteSelectedFeature )
{
  if ( mSelectedFeature )
  {
    QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
    Q_ASSERT( vlayer );

    disconnect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );
    disconnect( mSelectedFeature, SIGNAL( destroyed() ), this, SLOT( selectedFeatureDestroyed() ) );
    disconnect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );

    if ( deleteSelectedFeature ) delete mSelectedFeature;
    mSelectedFeature = 0;
  }
  if ( mNodeEditor )
  {
    delete mNodeEditor;
    mNodeEditor = 0;
  }
}

void QgsMapToolNodeTool::canvasDoubleClickEvent( QMouseEvent * e )
{
  if ( !mSelectedFeature )
    return;

  QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
  Q_ASSERT( vlayer );

  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;

  QList<QgsSnappingResult> snapResults;
  double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapSettings() );
  mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToSegment, tol );
  if ( snapResults.size() < 1 ||
       snapResults.first().snappedAtGeometry != mSelectedFeature->featureId() ||
       snapResults.first().snappedVertexNr != -1 )
    return;

  // some segment selected
  QgsPoint layerCoords = toLayerCoordinates( vlayer, snapResults.first().snappedVertex );
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

void QgsMapToolNodeTool::keyPressEvent( QKeyEvent* e )
{
  if ( mSelectedFeature && ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete ) )
  {
    int firstSelectedIndex = firstSelectedVertex();
    if ( firstSelectedIndex == -1 )
      return;

    mSelectedFeature->deleteSelectedVertexes();
    safeSelectVertex( firstSelectedIndex );
    mCanvas->refresh();

    // Override default shortcut management in MapCanvas
    e->ignore();
  }
  else if ( mSelectedFeature && ( e->key() == Qt::Key_Less || e->key() == Qt::Key_Comma ) )
  {
    int firstSelectedIndex = firstSelectedVertex();
    if ( firstSelectedIndex == -1 )
      return;

    mSelectedFeature->deselectAllVertexes();
    safeSelectVertex( firstSelectedIndex - 1 );
    e->ignore();
  }
  else if ( mSelectedFeature && ( e->key() == Qt::Key_Greater || e->key() == Qt::Key_Period ) )
  {
    int firstSelectedIndex = firstSelectedVertex();
    if ( firstSelectedIndex == -1 )
      return;

    mSelectedFeature->deselectAllVertexes();
    safeSelectVertex( firstSelectedIndex + 1 );
    e->ignore();
  }
}

int QgsMapToolNodeTool::firstSelectedVertex( )
{
  if ( mSelectedFeature )
  {
    QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
    for ( int i = 0, n = vertexMap.size(); i < n; ++i )
    {
      if ( vertexMap[i]->isSelected() )
      {
        return i;
      }
    }
  }
  return -1;
}

void QgsMapToolNodeTool::safeSelectVertex( int vertexNr )
{
  if ( mSelectedFeature )
  {
    int n = mSelectedFeature->vertexMap().size();
    mSelectedFeature->selectVertex(( vertexNr + n ) % n );
  }
}

QgsPoint QgsMapToolNodeTool::snapPointFromResults( const QList<QgsSnappingResult>& snapResults, const QPoint& screenCoords )
{
  if ( snapResults.size() < 1 )
  {
    return toMapCoordinates( screenCoords );
  }
  else
  {
    return snapResults.constBegin()->snappedVertex;
  }
}

int QgsMapToolNodeTool::insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults, QgsVectorLayer* editedLayer )
{
  QgsPoint layerPoint;

  if ( !editedLayer || !editedLayer->isEditable() )
  {
    return 1;
  }

  //transform snaping coordinates to layer crs first
  QList<QgsSnappingResult> transformedSnapResults = snapResults;
  QList<QgsSnappingResult>::iterator it = transformedSnapResults.begin();
  for ( ; it != transformedSnapResults.constEnd(); ++it )
  {
    QgsPoint layerPoint = toLayerCoordinates( editedLayer, it->snappedVertex );
    it->snappedVertex = layerPoint;
  }

  return editedLayer->insertSegmentVerticesForSnap( transformedSnapResults );
}

void QgsMapToolNodeTool::changeLastVertex( const QgsPointV2& pt )
{
  mClosestMapVertex = toMapCoordinates( currentVectorLayer(), QgsPoint( pt.x(), pt.y() ) );
}
