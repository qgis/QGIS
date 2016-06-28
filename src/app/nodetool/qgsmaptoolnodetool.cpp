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
#include "qgsvectordataprovider.h"

#include <QMouseEvent>
#include <QRubberBand>
#include <QSettings>

QgsMapToolNodeTool::QgsMapToolNodeTool( QgsMapCanvas* canvas )
    : QgsMapToolEdit( canvas )
    , mSelectRubberBand( nullptr )
    , mSelectedFeature( nullptr )
    , mNodeEditor( nullptr )
    , mMoving( true )
    , mSelectAnother( false )
    , mAnother( 0 )
    , mSelectionRubberBand( nullptr )
    , mRect( nullptr )
    , mIsPoint( false )
    , mDeselectOnRelease( -1 )
{
  mSnapper.setMapCanvas( canvas );
}

QgsMapToolNodeTool::~QgsMapToolNodeTool()
{
  cleanTool();
}

void QgsMapToolNodeTool::createTopologyRubberBands()
{
  QgsVectorLayer* vlayer = mSelectedFeature->vlayer();

  Q_FOREACH ( const QgsVertexEntry* vertexEntry, mSelectedFeature->vertexMap() )
  {
    if ( !vertexEntry->isSelected() )
    {
      continue;
    }

    // Snap vertex
    QMultiMap<double, QgsSnappingResult> snapResults;
    vlayer->snapWithContext( vertexEntry->pointV1(), ZERO_TOLERANCE, snapResults, QgsSnapper::SnapToVertex );
    Q_FOREACH ( const QgsSnappingResult& snapResult, snapResults )
    {
      // Get geometry of snapped feature
      QgsFeatureId snapFeatureId = snapResult.snappedAtGeometry;
      QgsFeature feature;
      if ( !vlayer->getFeatures( QgsFeatureRequest( snapFeatureId ).setSubsetOfAttributes( QgsAttributeList() ) ).nextFeature( feature ) )
      {
        continue;
      }
      // Get VertexId of snapped vertex
      QgsVertexId vid;
      if ( !feature.constGeometry()->vertexIdFromVertexNr( snapResult.snappedVertexNr, vid ) )
      {
        continue;
      }
      // Add rubberband if not already added
      if ( !mMoveRubberBands.contains( snapFeatureId ) )
      {
        QgsGeometryRubberBand* rb = new QgsGeometryRubberBand( mCanvas, feature.constGeometry()->type() );
        QSettings settings;
        QColor color(
          settings.value( "/qgis/digitizing/line_color_red", 255 ).toInt(),
          settings.value( "/qgis/digitizing/line_color_green", 0 ).toInt(),
          settings.value( "/qgis/digitizing/line_color_blue", 0 ).toInt() );
        double myAlpha = settings.value( "/qgis/digitizing/line_color_alpha", 30 ).toInt() / 255.0 ;
        color.setAlphaF( myAlpha );
        rb->setOutlineColor( color );
        rb->setBrushStyle( Qt::NoBrush );
        rb->setOutlineWidth( settings.value( "/qgis/digitizing/line_width", 1 ).toInt() );
        QgsAbstractGeometryV2* rbGeom = feature.constGeometry()->geometry()->clone();
        if ( mCanvas->mapSettings().layerTransform( vlayer ) )
          rbGeom->transform( *mCanvas->mapSettings().layerTransform( vlayer ) );
        rb->setGeometry( rbGeom );
        mMoveRubberBands.insert( snapFeatureId, rb );
      }
      // Add to list of vertices to be moved
      mMoveVertices[snapFeatureId].append( qMakePair( vid, toMapCoordinates( vlayer, feature.constGeometry()->geometry()->vertexAt( vid ) ) ) );
    }
  }
}

void QgsMapToolNodeTool::canvasMoveEvent( QgsMapMouseEvent* e )
{
  if ( !mSelectedFeature || e->buttons() == Qt::NoButton )
    return;

  QgsVectorLayer* vlayer = mSelectedFeature->vlayer();
  Q_ASSERT( vlayer );

  mSelectAnother = false;

  if ( mMoving )
  {
    if ( mMoveRubberBands.empty() )
    {
      QSettings settings;
      bool ghostLine = settings.value( "/qgis/digitizing/line_ghost", false ).toBool();
      if ( !ghostLine )
      {
        delete mSelectRubberBand;
        mSelectRubberBand = nullptr;
      }
      QgsGeometryRubberBand* rb = new QgsGeometryRubberBand( mCanvas, mSelectedFeature->geometry()->type() );
      QColor color(
        settings.value( "/qgis/digitizing/line_color_red", 255 ).toInt(),
        settings.value( "/qgis/digitizing/line_color_green", 0 ).toInt(),
        settings.value( "/qgis/digitizing/line_color_blue", 0 ).toInt() );
      double myAlpha = settings.value( "/qgis/digitizing/line_color_alpha", 30 ).toInt() / 255.0 ;
      color.setAlphaF( myAlpha );
      rb->setOutlineColor( color );
      rb->setBrushStyle( Qt::NoBrush );
      rb->setOutlineWidth( settings.value( "/qgis/digitizing/line_width", 1 ).toInt() );
      QgsAbstractGeometryV2* rbGeom = mSelectedFeature->geometry()->geometry()->clone();
      if ( mCanvas->mapSettings().layerTransform( vlayer ) )
        rbGeom->transform( *mCanvas->mapSettings().layerTransform( vlayer ) );
      rb->setGeometry( rbGeom );
      mMoveRubberBands.insert( mSelectedFeature->featureId(), rb );
      Q_FOREACH ( const QgsVertexEntry* vertexEntry, mSelectedFeature->vertexMap() )
      {
        if ( vertexEntry->isSelected() )
          mMoveVertices[mSelectedFeature->featureId()].append( qMakePair( vertexEntry->vertexId(), toMapCoordinates( vlayer, vertexEntry->point() ) ) );
      }
      if ( QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 ) )
      {
        createTopologyRubberBands();
      }
    }
    else
    {
      // move rubberband
      QList<QgsSnappingResult> snapResults;
      mSnapper.snapToBackgroundLayers( e->pos(), snapResults, QList<QgsPoint>() << mClosestMapVertex );

      QgsPoint curPos = snapPointFromResults( snapResults, e->pos() );
      QgsPoint pressPos = !snapResults.isEmpty() ? mClosestMapVertex : toMapCoordinates( mPressCoordinates );
      double deltaX = curPos.x() - pressPos.x();
      double deltaY = curPos.y() - pressPos.y();

      Q_FOREACH ( QgsFeatureId fid, mMoveRubberBands.keys() )
      {
        typedef QPair<QgsVertexId, QgsPointV2> MoveVertex;
        Q_FOREACH ( const MoveVertex& pair, mMoveVertices[fid] )
        {
          QgsPointV2 newPos( pair.second.x() + deltaX, pair.second.y() + deltaY );
          mMoveRubberBands.value( fid )->moveVertex( pair.first, newPos );
        }
      }
    }
  }
  else
  {
    if ( !mRect )
    {
      mSelectionRubberBand = new QRubberBand( QRubberBand::Rectangle, mCanvas );
      mRect = new QRect();
      mRect->setTopLeft( mPressCoordinates );
    }
    mRect->setBottomRight( e->pos() );
    QRect normalizedRect = mRect->normalized();
    mSelectionRubberBand->setGeometry( normalizedRect );
    mSelectionRubberBand->show();
  }
}

QgsFeature QgsMapToolNodeTool::getFeatureAtPoint( QgsMapMouseEvent* e )
{
  QgsFeature feature;
  QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );

  if ( !vlayer )
    return feature;

  QgsFeatureRequest request;
  request.setFilterRect( QgsRectangle( e->mapPoint().x(), e->mapPoint().y(), e->mapPoint().x(), e->mapPoint().y() ) );
  QgsFeatureIterator features = vlayer->getFeatures( request );
  features.nextFeature( feature );

  return feature;
}

void QgsMapToolNodeTool::canvasPressEvent( QgsMapMouseEvent* e )
{
  QgsDebugCall;

  mPressCoordinates = e->pos();
  bool ctrlModifier = e->modifiers() & Qt::ControlModifier;
  QList<QgsSnappingResult> snapResults;
  if ( !mSelectedFeature )
  {
    QgsVectorLayer *vlayer = qobject_cast<QgsVectorLayer *>( mCanvas->currentLayer() );
    if ( !vlayer )
      return;

    mSelectAnother = false;
    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertexAndSegment, -1 );

    if ( snapResults.size() < 1 )
    {
      QgsFeature feature = getFeatureAtPoint( e );
      if ( !feature.constGeometry() )
      {
        emit messageEmitted( tr( "Could not snap to a feature in the current layer." ) );
        return;
      }
      else
      {
        // remove previous warning
        emit messageDiscarded();
        mSelectedFeature = new QgsSelectedFeature( feature.id(), vlayer, mCanvas );
        updateSelectFeature();
      }
    }
    else
    {
      // remove previous warning
      emit messageDiscarded();

      mSelectedFeature = new QgsSelectedFeature( snapResults[0].snappedAtGeometry, vlayer, mCanvas );
      updateSelectFeature();
    }
    connect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );
    connect( mSelectedFeature, SIGNAL( destroyed() ), this, SLOT( selectedFeatureDestroyed() ) );
    connect( vlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );
    connect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );
    mIsPoint = vlayer->geometryType() == QGis::Point;
    mNodeEditor = new QgsNodeEditor( vlayer, mSelectedFeature, mCanvas );
    QgisApp::instance()->addDockWidget( Qt::LeftDockWidgetArea, mNodeEditor );
    connect( mNodeEditor, SIGNAL( deleteSelectedRequested() ), this, SLOT( deleteNodeSelection() ) );
  }
  else
  {
    // remove previous warning
    emit messageDiscarded();

    QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
    Q_ASSERT( vlayer );

    // some feature already selected
    QgsPoint layerCoordPoint = toLayerCoordinates( vlayer, e->pos() );

    double tol = QgsTolerance::vertexSearchRadius( vlayer, mCanvas->mapSettings() );

    // get geometry and find if snapping is near it
    int atVertex, beforeVertex, afterVertex;
    double dist;
    QgsPoint closestLayerVertex = mSelectedFeature->geometry()->closestVertex( layerCoordPoint, atVertex, beforeVertex, afterVertex, dist );
    dist = sqrt( dist );

    mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToVertex, tol );
    if ( dist <= tol )
    {
      // some vertex selected
      mMoving = true;
      mClosestMapVertex = toMapCoordinates( vlayer, closestLayerVertex );
      if ( mMoving )
      {
        if ( mSelectedFeature->isSelected( atVertex ) )
        {
          mDeselectOnRelease = atVertex;
        }
        else if ( ctrlModifier )
        {
          mSelectedFeature->invertVertexSelection( atVertex );
        }
        else
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
    else
    {
      // no near vertex to snap
      //  unless point layer, try segment
      if ( !mIsPoint )
        mSnapper.snapToCurrentLayer( e->pos(), snapResults, QgsSnapper::SnapToSegment, tol, QList<QgsPoint>(), true );

      if ( !snapResults.isEmpty() )
      {
        // need to check all if there is a point in the feature
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
          mClosestMapVertex = toMapCoordinates( vlayer, closestLayerVertex );

          if ( mIsPoint )
          {
            if ( !ctrlModifier )
            {
              mSelectedFeature->deselectAllVertexes();
              mSelectedFeature->selectVertex( snapResult.snappedVertexNr );
            }
            else
            {
              mSelectedFeature->invertVertexSelection( snapResult.snappedVertexNr );
            }
          }
          else
          {
            if ( !ctrlModifier )
            {
              mSelectedFeature->deselectAllVertexes();
              mSelectedFeature->selectVertex( snapResult.afterVertexNr );
              mSelectedFeature->selectVertex( snapResult.beforeVertexNr );
            }
            else
            {
              mSelectedFeature->invertVertexSelection( snapResult.afterVertexNr );
              mSelectedFeature->invertVertexSelection( snapResult.beforeVertexNr );
            }
          }
        }
      }
      else if ( !ctrlModifier )
      {
        mSelectedFeature->deselectAllVertexes();

        QgsFeature feature = getFeatureAtPoint( e );
        if ( !feature.constGeometry() )
          return;

        mAnother = feature.id();
        mSelectAnother = true;
      }
    }
  }
}

void QgsMapToolNodeTool::updateSelectFeature()
{
  updateSelectFeature( *mSelectedFeature->geometry() );
}

void QgsMapToolNodeTool::updateSelectFeature( QgsGeometry &geom )
{
  delete mSelectRubberBand;

  if ( geom.geometry() )
  {
    mSelectRubberBand = new QgsGeometryRubberBand( mCanvas, mSelectedFeature->geometry()->type() );
    mSelectRubberBand->setBrushStyle( Qt::SolidPattern );

    QSettings settings;
    QColor color(
      settings.value( "/qgis/digitizing/fill_color_red", 255 ).toInt(),
      settings.value( "/qgis/digitizing/fill_color_green", 0 ).toInt(),
      settings.value( "/qgis/digitizing/fill_color_blue", 0 ).toInt() );
    double myAlpha = settings.value( "/qgis/digitizing/fill_color_alpha", 30 ).toInt() / 255.0 ;
    color.setAlphaF( myAlpha );
    mSelectRubberBand->setFillColor( color );

    QgsAbstractGeometryV2* rbGeom = geom.geometry()->clone();
    QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
    if ( mCanvas->mapSettings().layerTransform( vlayer ) )
      rbGeom->transform( *mCanvas->mapSettings().layerTransform( vlayer ) );
    mSelectRubberBand->setGeometry( rbGeom );
  }
  else
  {
    mSelectRubberBand = nullptr;
  }
}

void QgsMapToolNodeTool::selectedFeatureDestroyed()
{
  QgsDebugCall;
  cleanTool( false );
}

void QgsMapToolNodeTool::geometryChanged( QgsFeatureId fid, QgsGeometry &geom )
{
  QSettings settings;
  bool ghostLine = settings.value( "/qgis/digitizing/line_ghost", false ).toBool();
  if ( !ghostLine && mSelectedFeature && ( mSelectedFeature->featureId() == fid ) )
  {
    updateSelectFeature( geom );
  }
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

void QgsMapToolNodeTool::canvasReleaseEvent( QgsMapMouseEvent* e )
{
  if ( !mSelectedFeature )
    return;

  QgsFeatureIds movedFids( mMoveRubberBands.keys().toSet() );
  removeRubberBands();

  QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
  Q_ASSERT( vlayer );

  bool ctrlModifier = e->modifiers() & Qt::ControlModifier;

  if ( mRect )
  {
    delete mSelectionRubberBand;
    mSelectionRubberBand = nullptr;
    delete mRect;
    mRect = nullptr;
  }

  if ( mPressCoordinates == e->pos() )
  {
    if ( mSelectAnother )
    {
      // select another feature
      mSelectedFeature->setSelectedFeature( mAnother, vlayer, mCanvas );
      updateSelectFeature();
      mIsPoint = vlayer->geometryType() == QGis::Point;
      mSelectAnother = false;
    }
  }
  else
  {
    if ( mMoving )
    {
      mMoving = false;

      QList<QgsSnappingResult> snapResults;
      mSnapper.snapToBackgroundLayers( e->pos(), snapResults, QList<QgsPoint>() << mClosestMapVertex );

      QgsPoint releaseLayerCoords = toLayerCoordinates( vlayer, snapPointFromResults( snapResults, e->pos() ) );

      QgsPoint pressLayerCoords;
      if ( !snapResults.isEmpty() )
      {
        pressLayerCoords = toLayerCoordinates( vlayer, mClosestMapVertex );

        int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
        if ( topologicalEditing )
        {
          //insert vertices for snap, but don't add them to features which already has a vertex being moved
          insertSegmentVerticesForSnap( snapResults, vlayer, movedFids );
        }
      }
      else
      {
        pressLayerCoords = toLayerCoordinates( vlayer, mPressCoordinates );
      }

      mSelectedFeature->moveSelectedVertexes( releaseLayerCoords - pressLayerCoords );
      vlayer->triggerRepaint();
    }
    else // selecting vertexes by rubberband
    {
      // coordinates has to be coordinates from layer not canvas
      QgsRectangle r( toLayerCoordinates( vlayer, mPressCoordinates ),
                      toLayerCoordinates( vlayer, e->pos() ) );

      QList<QgsVertexEntry*> &vertexMap = mSelectedFeature->vertexMap();
      if ( !ctrlModifier )
      {
        mSelectedFeature->deselectAllVertexes();
      }

      QVector< int > toSelect;
      for ( int i = 0; i < vertexMap.size(); i++ )
      {
        if ( r.contains( vertexMap.at( i )->pointV1() ) )
        {
          toSelect << i;
        }
      }
      // inverting selection is enough because all were deselected if ctrl is not pressed
      mSelectedFeature->invertVertexSelection( toSelect );
    }
  }

  mMoving = false;

  if ( mDeselectOnRelease != -1 )
  {
    if ( ctrlModifier )
    {
      mSelectedFeature->invertVertexSelection( mDeselectOnRelease );
    }
    else
    {
      mSelectedFeature->deselectAllVertexes();
      mSelectedFeature->selectVertex( mDeselectOnRelease );
    }

    mDeselectOnRelease = -1;
  }
}

void QgsMapToolNodeTool::deactivate()
{
  cleanTool();

  mSelectionRubberBand = nullptr;
  mSelectAnother = false;
  mMoving = true;

  QgsMapTool::deactivate();
}

void QgsMapToolNodeTool::removeRubberBands()
{
  qDeleteAll( mMoveRubberBands );
  mMoveRubberBands.clear();
  mMoveVertices.clear();
}

void QgsMapToolNodeTool::cleanTool( bool deleteSelectedFeature )
{
  removeRubberBands();

  if ( mSelectRubberBand )
  {
    delete mSelectRubberBand;
    mSelectRubberBand = nullptr;
  }

  if ( mSelectedFeature )
  {
    QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
    Q_ASSERT( vlayer );

    disconnect( QgisApp::instance()->layerTreeView(), SIGNAL( currentLayerChanged( QgsMapLayer* ) ), this, SLOT( currentLayerChanged( QgsMapLayer* ) ) );
    disconnect( mSelectedFeature, SIGNAL( destroyed() ), this, SLOT( selectedFeatureDestroyed() ) );
    disconnect( vlayer, SIGNAL( editingStopped() ), this, SLOT( editingToggled() ) );

    if ( deleteSelectedFeature )
      delete mSelectedFeature;

    mSelectedFeature = nullptr;
  }

  if ( mNodeEditor )
  {
    delete mNodeEditor;
    mNodeEditor = nullptr;
  }
}

void QgsMapToolNodeTool::canvasDoubleClickEvent( QgsMapMouseEvent* e )
{
  if ( !mSelectedFeature )
    return;

  QgsVectorLayer *vlayer = mSelectedFeature->vlayer();
  Q_ASSERT( vlayer );

  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;

  QList<QgsSnappingResult> snapResults;
  mMoving = false;
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
  vlayer->triggerRepaint();
}

void QgsMapToolNodeTool::deleteNodeSelection()
{
  if ( mSelectedFeature )
  {
    int firstSelectedIndex = firstSelectedVertex();
    if ( firstSelectedIndex == -1 )
      return;

    mSelectedFeature->deleteSelectedVertexes();

    if ( mSelectedFeature->geometry()->isEmpty() )
    {
      emit messageEmitted( tr( "Geometry has been cleared. Use the add part tool to set geometry for this feature." ) );
    }
    else
    {
      int nextVertexToSelect = firstSelectedIndex;
      if ( mSelectedFeature->geometry()->type() == QGis::Line )
      {
        // for lines we don't wrap around vertex selection when deleting nodes from end of line
        nextVertexToSelect = qMin( nextVertexToSelect, mSelectedFeature->geometry()->geometry()->nCoordinates() - 1 );
      }

      safeSelectVertex( nextVertexToSelect );
    }
    mSelectedFeature->vlayer()->triggerRepaint();
  }
}

void QgsMapToolNodeTool::keyPressEvent( QKeyEvent* e )
{
  if ( mSelectedFeature )
  {
    if ( e->key() == Qt::Key_Backspace || e->key() == Qt::Key_Delete )
    {
      this->deleteNodeSelection();

      // Override default shortcut management in MapCanvas
      e->ignore();
    }
    else if ( e->key() == Qt::Key_Less || e->key() == Qt::Key_Comma )
    {
      int firstSelectedIndex = firstSelectedVertex();
      if ( firstSelectedIndex == -1 )
        return;

      mSelectedFeature->deselectAllVertexes();
      safeSelectVertex( firstSelectedIndex - 1 );
      e->ignore();
    }
    else if ( e->key() == Qt::Key_Greater || e->key() == Qt::Key_Period )
    {
      int firstSelectedIndex = firstSelectedVertex();
      if ( firstSelectedIndex == -1 )
        return;

      mSelectedFeature->deselectAllVertexes();
      safeSelectVertex( firstSelectedIndex + 1 );
      e->ignore();
    }
  }
}

int QgsMapToolNodeTool::firstSelectedVertex()
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

QgsPoint QgsMapToolNodeTool::snapPointFromResults( const QList<QgsSnappingResult>& snapResults, QPoint screenCoords )
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

int QgsMapToolNodeTool::insertSegmentVerticesForSnap( const QList<QgsSnappingResult>& snapResults, QgsVectorLayer* editedLayer, const QgsFeatureIds& skipFids )
{
  if ( !editedLayer || !editedLayer->isEditable() )
  {
    return 1;
  }

  //transform snaping coordinates to layer crs first
  QList<QgsSnappingResult> transformedSnapResults;
  QgsFeatureIds addedFeatures;
  QList<QgsSnappingResult>::const_iterator it = snapResults.constBegin();
  for ( ; it != snapResults.constEnd(); ++it )
  {
    //skip if snappingResult is in a different layer
    //See http://hub.qgis.org/issues/13952#note-29
    if ( it->layer != editedLayer )
      continue;

    //skip if id is in skip list or we have already added a vertex to a feature
    if ( skipFids.contains( it->snappedAtGeometry ) || addedFeatures.contains( it->snappedAtGeometry ) )
      continue;

    QgsPoint layerPoint = toLayerCoordinates( editedLayer, it->snappedVertex );
    QgsSnappingResult result( *it );
    result.snappedVertex = layerPoint;
    transformedSnapResults << result;
    addedFeatures << it->snappedAtGeometry;
  }

  return editedLayer->insertSegmentVerticesForSnap( transformedSnapResults );
}
