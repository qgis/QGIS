/***************************************************************************
    qgsselectedfeature.cpp  - selected feature of vertextool
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

#include "vertextool/qgsselectedfeature.h"
#include "vertextool/qgsvertexentry.h"

#include "qgsfeatureiterator.h"
#include "qgspoint.h"
#include "qgssettings.h"
#include "qgslogger.h"
#include "qgsvertexmarker.h"
#include "qgsgeometryvalidator.h"
#include "qgsguiutils.h"
#include "qgsvectorlayer.h"
#include "qgsrubberband.h"
#include "qgisapp.h"
#include "qgslayertreeview.h"
#include "qgsproject.h"
#include "qgsstatusbar.h"
#include "qgsmapcanvas.h"


QgsSelectedFeature::QgsSelectedFeature( QgsFeatureId featureId,
                                        QgsVectorLayer *vlayer,
                                        QgsMapCanvas *canvas )
  : mFeatureId( featureId )
  , mChangingGeometry( false )
{
  setSelectedFeature( featureId, vlayer, canvas );
}

QgsSelectedFeature::~QgsSelectedFeature()
{
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
    mValidator = nullptr;
  }

  delete mGeometry;
}

void QgsSelectedFeature::currentLayerChanged( QgsMapLayer *layer )
{
  if ( layer == mLayer )
    deleteLater();
}

void QgsSelectedFeature::updateGeometry( const QgsGeometry *geom )
{
  delete mGeometry;

  if ( !geom )
  {
    QgsFeature f;
    mLayer->getFeatures( QgsFeatureRequest().setFilterFid( mFeatureId ) ).nextFeature( f );
    if ( f.hasGeometry() )
      mGeometry = new QgsGeometry( f.geometry() );
    else
      mGeometry = new QgsGeometry();
  }
  else
  {
    mGeometry = new QgsGeometry( *geom );
  }
}

void QgsSelectedFeature::setSelectedFeature( QgsFeatureId featureId, QgsVectorLayer *layer, QgsMapCanvas *canvas )
{
  mFeatureId = featureId;
  mLayer = layer;
  mCanvas = canvas;

  delete mGeometry;
  mGeometry = nullptr;

  // signal changing of current layer
  connect( QgisApp::instance()->layerTreeView(), &QgsLayerTreeView::currentLayerChanged, this, &QgsSelectedFeature::currentLayerChanged );

  // feature was deleted
  connect( mLayer, &QgsVectorLayer::featureDeleted, this, &QgsSelectedFeature::featureDeleted );

  // rolling back
  connect( mLayer, &QgsVectorLayer::beforeRollBack, this, &QgsSelectedFeature::beforeRollBack );

  // projection or extents changed
  connect( canvas, &QgsMapCanvas::destinationCrsChanged, this, &QgsSelectedFeature::updateVertexMarkersPosition );
  connect( canvas, &QgsMapCanvas::extentsChanged, this, &QgsSelectedFeature::updateVertexMarkersPosition );

  // geometry was changed
  connect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsSelectedFeature::geometryChanged );

  replaceVertexMap();
}

void QgsSelectedFeature::beforeRollBack()
{
  disconnect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsSelectedFeature::geometryChanged );
  deleteVertexMap();
}

void QgsSelectedFeature::beginGeometryChange()
{
  Q_ASSERT( !mChangingGeometry );
  mChangingGeometry = true;

  disconnect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsSelectedFeature::geometryChanged );
}

void QgsSelectedFeature::endGeometryChange()
{
  Q_ASSERT( mChangingGeometry );
  mChangingGeometry = false;

  connect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsSelectedFeature::geometryChanged );
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

void QgsSelectedFeature::geometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  QgsDebugCall;

  if ( !mLayer || fid != mFeatureId )
    return;

  updateGeometry( &geom );

  replaceVertexMap();
}

void QgsSelectedFeature::validateGeometry( QgsGeometry *g )
{
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 0 )
    return;

  if ( !g )
    g = mGeometry;

  mTip.clear();

  if ( mValidator )
  {
    mValidator->stop();
    mValidator->wait();
    mValidator->deleteLater();
    mValidator = nullptr;
  }

  mGeomErrors.clear();
  while ( !mGeomErrorMarkers.isEmpty() )
  {
    QgsVertexMarker *vm = mGeomErrorMarkers.takeFirst();
    QgsDebugMsg( "deleting " + vm->toolTip() );
    delete vm;
  }

  QgsGeometry::ValidationMethod method = QgsGeometry::ValidatorQgisInternal;
  if ( settings.value( QStringLiteral( "qgis/digitizing/validate_geometries" ), 1 ).toInt() == 2 )
    method = QgsGeometry::ValidatorGeos;
  mValidator = new QgsGeometryValidator( *g, nullptr, method );
  connect( mValidator, &QgsGeometryValidator::errorFound, this, &QgsSelectedFeature::addError );
  connect( mValidator, &QThread::finished, this, &QgsSelectedFeature::validationFinished );
  mValidator->start();

  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( tr( "Validation started." ) );
}

void QgsSelectedFeature::addError( QgsGeometry::Error e )
{
  mGeomErrors << e;
  if ( !mTip.isEmpty() )
    mTip += '\n';
  mTip += e.what();

  if ( e.hasWhere() )
  {
    QgsVertexMarker *marker = new QgsVertexMarker( mCanvas );
    marker->setCenter( mCanvas->mapSettings().layerToMapCoordinates( mLayer, e.where() ) );
    marker->setIconType( QgsVertexMarker::ICON_X );
    marker->setColor( Qt::green );
    marker->setZValue( marker->zValue() + 1 );
    marker->setIconSize( QgsGuiUtils::scaleIconSize( 10 ) );
    marker->setPenWidth( QgsGuiUtils::scaleIconSize( 2 ) );
    marker->setToolTip( e.what() );
    mGeomErrorMarkers << marker;
  }

  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( e.what() );
  sb->setToolTip( mTip );
}

void QgsSelectedFeature::validationFinished()
{
  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( tr( "Validation finished (%n error(s) found).", "number of geometry errors", mGeomErrorMarkers.size() ) );
}

void QgsSelectedFeature::replaceVertexMap()
{
  // delete old map
  deleteVertexMap();

  // create new map
  createVertexMap();

  // validate the geometry
  validateGeometry();

  emit vertexMapChanged();
}

void QgsSelectedFeature::deleteVertexMap()
{
  Q_FOREACH ( QgsVertexEntry *entry, mVertexMap )
  {
    delete entry;
  }

  mVertexMap.clear();
}

bool QgsSelectedFeature::isSelected( int vertexNr )
{
  return mVertexMap.at( vertexNr )->isSelected();
}

QgsGeometry *QgsSelectedFeature::geometry()
{
  Q_ASSERT( mGeometry );
  return mGeometry;
}

void QgsSelectedFeature::createVertexMap()
{

  if ( !mGeometry )
  {
    QgsDebugMsg( QStringLiteral( "Loading feature" ) );
    updateGeometry( nullptr );
  }

  if ( !mGeometry )
  {
    return;
  }

  const QgsAbstractGeometry *geom = mGeometry->constGet();
  if ( !geom )
  {
    return;
  }

  QgsVertexId vertexId;
  QgsPoint pt;
  while ( geom->nextVertex( vertexId, pt ) )
  {
    mVertexMap.append( new QgsVertexEntry( mCanvas, mLayer, pt, vertexId, tr( "ring %1, vertex %2" ).arg( vertexId.ring ).arg( vertexId.vertex ) ) );
  }
}

void QgsSelectedFeature::selectVertex( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );
  entry->setSelected();

  emit selectionChanged();
}

void QgsSelectedFeature::deselectVertex( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );
  entry->setSelected( false );

  emit selectionChanged();
}

void QgsSelectedFeature::deselectAllVertices()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap.at( i )->setSelected( false );
  }
  emit selectionChanged();
}

void QgsSelectedFeature::invertVertexSelection( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );

  bool selected = !entry->isSelected();

  entry->setSelected( selected );
  emit selectionChanged();
}

void QgsSelectedFeature::invertVertexSelection( const QVector<int> &vertexIndices )
{
  Q_FOREACH ( int index, vertexIndices )
  {
    if ( index < 0 || index >= mVertexMap.size() )
      continue;

    QgsVertexEntry *entry = mVertexMap.at( index );
    entry->setSelected( !entry->isSelected() );
  }
  emit selectionChanged();
}

void QgsSelectedFeature::updateVertexMarkersPosition()
{
  Q_FOREACH ( QgsVertexEntry *vertexEntry, mVertexMap )
  {
    vertexEntry->placeMarker();
  }
}

QgsFeatureId QgsSelectedFeature::featureId()
{
  return mFeatureId;
}

QList<QgsVertexEntry *> &QgsSelectedFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer *QgsSelectedFeature::layer()
{
  return mLayer;
}
