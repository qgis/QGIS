/***************************************************************************
    qgsselectedfeature.cpp  - selected feature of nodetool
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

#include "nodetool/qgsselectedfeature.h"
#include "nodetool/qgsvertexentry.h"

#include <qgslogger.h>
#include <qgsvertexmarker.h>
#include <qgsgeometryvalidator.h>
#include <qgsvectorlayer.h>
#include <qgsrubberband.h>
#include <qgisapp.h>
#include <qgsmaprenderer.h>
#include <qgslegend.h>
#include <qgsproject.h>

QgsSelectedFeature::QgsSelectedFeature( QgsFeatureId featureId,
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

QgsSelectedFeature::~QgsSelectedFeature()
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

void QgsSelectedFeature::currentLayerChanged( QgsMapLayer *layer )
{
  QgsDebugMsg( "Entering." );
  if ( layer == mVlayer )
    deleteLater();
}

void QgsSelectedFeature::updateGeometry( QgsGeometry *geom )
{
  QgsDebugMsg( "Entering." );

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

void QgsSelectedFeature::cleanRubberBandsData()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setRubberBandValues( false, 0, 0 );
  }
}

void QgsSelectedFeature::setSelectedFeature( QgsFeatureId featureId, QgsVectorLayer* vlayer, QgsMapCanvas* canvas )
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

  // projection or extents changed
  connect( canvas->mapRenderer(), SIGNAL( destinationSrsChanged() ), this, SLOT( updateVertexMarkersPosition() ) );
  connect( canvas, SIGNAL( extentsChanged() ), this, SLOT( updateVertexMarkersPosition() ) );

  // geometry was changed
  connect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );

  replaceVertexMap();
}

void QgsSelectedFeature::beginGeometryChange()
{
  Q_ASSERT( !mChangingGeometry );
  mChangingGeometry = true;

  disconnect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );
}

void QgsSelectedFeature::endGeometryChange()
{
  Q_ASSERT( mChangingGeometry );
  mChangingGeometry = false;

  connect( mVlayer, SIGNAL( geometryChanged( QgsFeatureId, QgsGeometry & ) ), this, SLOT( geometryChanged( QgsFeatureId, QgsGeometry & ) ) );
}

void QgsSelectedFeature::canvasLayersChanged()
{
  currentLayerChanged( mCanvas->currentLayer() );
}

void QgsSelectedFeature::featureDeleted( QgsFeatureId fid )
{
  if ( fid == mFeatureId )
    deleteLater();
}

void QgsSelectedFeature::geometryChanged( QgsFeatureId fid, QgsGeometry &geom )
{
  QgsDebugMsg( "Entering." );

  if ( !mVlayer || fid != mFeatureId )
    return;

  updateGeometry( &geom );

  replaceVertexMap();
}

void QgsSelectedFeature::validateGeometry( QgsGeometry *g )
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

void QgsSelectedFeature::addError( QgsGeometry::Error e )
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

void QgsSelectedFeature::validationFinished()
{
  QStatusBar *sb = QgisApp::instance()->statusBar();
  sb->showMessage( tr( "Validation finished (%n error(s) found).", "number of geometry errors", mGeomErrorMarkers.size() ) );
}

void QgsSelectedFeature::deleteSelectedVertexes()
{
  int nSelected = 0;
  foreach ( QgsVertexEntry *entry, mVertexMap )
  {
    if ( entry->isSelected() )
      nSelected++;
  }

  if ( nSelected == 0 )
    return;

  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );
  QMultiMap<double, QgsSnappingResult> currentResultList;

  mVlayer->beginEditCommand( QObject::tr( "Deleted vertices" ) );

  beginGeometryChange();

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

      // only last update should trigger the geometry update
      // as vertex selection gets lost on the update
      if ( --nSelected == 0 )
        endGeometryChange();

      if ( !mVlayer->deleteVertex( mFeatureId, i ) )
      {
        count = 0;
        QgsDebugMsg( QString( "Deleting vertex %1 failed - resetting" ).arg( i ) );
        break;
      }

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

void QgsSelectedFeature::moveSelectedVertexes( const QgsVector &v )
{
  int nUpdates = 0;
  foreach ( QgsVertexEntry *entry, mVertexMap )
  {
    if ( entry->isSelected() )
      nUpdates++;
  }

  if ( nUpdates == 0 )
    return;

  mVlayer->beginEditCommand( QObject::tr( "Moved vertices" ) );
  int topologicalEditing = QgsProject::instance()->readNumEntry( "Digitizing", "/TopologicalEditing", 0 );

  beginGeometryChange();

  QMultiMap<double, QgsSnappingResult> currentResultList;
  for ( int i = mVertexMap.size() - 1; i > -1 && nUpdates > 0; i-- )
  {
    QgsVertexEntry *entry = mVertexMap.value( i, 0 );
    if ( !entry || !entry->isSelected() )
      continue;

    if ( topologicalEditing )
    {
      // snap from current vertex
      currentResultList.clear();
      mVlayer->snapWithContext( entry->point(), ZERO_TOLERANCE, currentResultList, QgsSnapper::SnapToVertex );
    }

    // only last update should trigger the geometry update
    // as vertex selection gets lost on the update
    if ( --nUpdates == 0 )
      endGeometryChange();

    QgsPoint p = entry->point() + v;
    mVlayer->moveVertex( p.x(), p.y(), mFeatureId, i );

    if ( topologicalEditing )
    {
      QMultiMap<double, QgsSnappingResult>::iterator resultIt =  currentResultList.begin();

      for ( ; resultIt != currentResultList.end(); ++resultIt )
      {
        // move all other
        if ( mFeatureId !=  resultIt.value().snappedAtGeometry )
          mVlayer->moveVertex( p.x(), p.y(),
                               resultIt.value().snappedAtGeometry, resultIt.value().snappedVertexNr );
      }
    }
  }

  mVlayer->endEditCommand();
}

void QgsSelectedFeature::replaceVertexMap()
{
  // delete old map
  deleteVertexMap();

  // create new map
  createVertexMap();

  // validate the geometry
  validateGeometry();
}

void QgsSelectedFeature::deleteVertexMap()
{
  foreach ( QgsVertexEntry *entry, mVertexMap )
  {
    delete entry;
  }

  mVertexMap.clear();
}

bool QgsSelectedFeature::isSelected( int vertexNr )
{
  return mVertexMap[vertexNr]->isSelected();
}

QgsGeometry *QgsSelectedFeature::geometry()
{
  Q_ASSERT( mGeometry );
  return mGeometry;
}

void QgsSelectedFeature::createVertexMapPolygon()
{
  int y = 0;
  QgsPolygon polygon = mGeometry->asPolygon();
  if ( !polygon.empty() )
  {
    // polygon
    for ( int i2 = 0; i2 < polygon.size(); i2++ )
    {
      const QgsPolyline& poly = polygon[i2];
      for ( int i = 0; i < poly.size(); i++ )
      {
        mVertexMap.insert( y + i, new QgsVertexEntry( mCanvas, mVlayer, poly[i], i, tr( "ring %1, vertex %2" ).arg( i2 ).arg( i ) ) );
      }
      mVertexMap[y + poly.size() - 1 ]->setEqual( y );
      mVertexMap[y]->setEqual( y + poly.size() - 1 );
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
        for ( int i = 0; i < poly.size(); i++ )
        {
          mVertexMap.insert( y + i, new QgsVertexEntry( mCanvas, mVlayer, poly[i], y + i - 1, tr( "polygon %1, ring %2, vertex %3" ).arg( i2 ).arg( i3 ).arg( i ) ) );
        }
        mVertexMap[y + poly.size() - 1]->setEqual( y );
        mVertexMap[y]->setEqual( y + poly.size() - 1 );
        y += poly.size();
      }
    }
  }
}

void QgsSelectedFeature::createVertexMapLine()
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
      for ( int i = 0; i < poly.size(); i++ )
      {
        mVertexMap.insert( y + i, new QgsVertexEntry( mCanvas, mVlayer, poly[i], i, tr( "polyline %1, vertex %2" ).arg( i2 ).arg( i ) ) );
      }
      y += poly.size();
    }
  }
  else
  {
    QgsPolyline poly = mGeometry->asPolyline();
    for ( int i = 0; i < poly.size(); i++ )
    {
      mVertexMap.insert( i, new QgsVertexEntry( mCanvas, mVlayer, poly[i], i, tr( "vertex %1" ).arg( i ) ) );
    }
  }
}

void QgsSelectedFeature::createVertexMapPoint()
{
  Q_ASSERT( mGeometry );

  if ( mGeometry->isMultipart() )
  {
    // multipoint
    QgsMultiPoint poly = mGeometry->asMultiPoint();
    for ( int i = 0; i < poly.size(); i++ )
    {
      mVertexMap.insert( i, new QgsVertexEntry( mCanvas, mVlayer, poly[i], 1, tr( "point %1" ).arg( i ) ) );
    }
  }
  else
  {
    // single point
    mVertexMap.insert( 1, new QgsVertexEntry( mCanvas, mVlayer, mGeometry->asPoint(), 1, tr( "single point" ) ) );
  }
}

void QgsSelectedFeature::createVertexMap()
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

void QgsSelectedFeature::selectVertex( int vertexNr )
{
  QgsVertexEntry *entry = mVertexMap[vertexNr];
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

void QgsSelectedFeature::deselectVertex( int vertexNr )
{
  QgsVertexEntry *entry = mVertexMap[vertexNr];
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

void QgsSelectedFeature::deselectAllVertexes()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap[i]->setSelected( false );
    mVertexMap[i]->update();
  }
}

void QgsSelectedFeature::invertVertexSelection( int vertexNr, bool invert )
{
  QgsVertexEntry *entry = mVertexMap[vertexNr];

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

void QgsSelectedFeature::updateVertexMarkersPosition()
{
  // function for on-line updating vertex markers without refresh of canvas
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    QgsVertexEntry *entry = mVertexMap[i];
    entry->setCenter( entry->point() );
    entry->update();
  }
}

QgsFeatureId QgsSelectedFeature::featureId()
{
  return mFeatureId;
}

QList<QgsVertexEntry*> &QgsSelectedFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer* QgsSelectedFeature::vlayer()
{
  return mVlayer;
}
