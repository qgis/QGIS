/***************************************************************************
    qgslockedfeature.cpp  - locked feature of vertextool
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

#include "qgslockedfeature.h"
#include "qgsvertexeditor.h"

#include "qgsfeatureiterator.h"
#include "qgspoint.h"
#include "qgssettingsregistrycore.h"
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


QgsLockedFeature::QgsLockedFeature( QgsFeatureId featureId,
                                    QgsVectorLayer *layer,
                                    QgsMapCanvas *canvas )
  : mFeatureId( featureId )
  , mLayer( layer )
  , mCanvas( canvas )
{
  replaceVertexMap();
}

QgsLockedFeature::~QgsLockedFeature()
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

void QgsLockedFeature::updateGeometry( const QgsGeometry *geom )
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

void QgsLockedFeature::beforeRollBack()
{
  disconnect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsLockedFeature::geometryChanged );
  deleteVertexMap();
}

void QgsLockedFeature::beginGeometryChange()
{
  Q_ASSERT( !mChangingGeometry );
  mChangingGeometry = true;

  disconnect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsLockedFeature::geometryChanged );
}

void QgsLockedFeature::endGeometryChange()
{
  Q_ASSERT( mChangingGeometry );
  mChangingGeometry = false;

  connect( mLayer, &QgsVectorLayer::geometryChanged, this, &QgsLockedFeature::geometryChanged );
}

void QgsLockedFeature::featureDeleted( QgsFeatureId fid )
{
  if ( fid == mFeatureId )
    deleteLater();
}

void QgsLockedFeature::geometryChanged( QgsFeatureId fid, const QgsGeometry &geom )
{
  QgsDebugCall;

  if ( !mLayer || fid != mFeatureId )
    return;

  updateGeometry( &geom );

  replaceVertexMap();
}

void QgsLockedFeature::validateGeometry( QgsGeometry *g )
{
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 0 )
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

  Qgis::GeometryValidationEngine method = Qgis::GeometryValidationEngine::QgisInternal;
  if ( QgsSettingsRegistryCore::settingsDigitizingValidateGeometries.value() == 2 )
    method = Qgis::GeometryValidationEngine::Geos;
  mValidator = new QgsGeometryValidator( *g, nullptr, method );
  connect( mValidator, &QgsGeometryValidator::errorFound, this, &QgsLockedFeature::addError );
  connect( mValidator, &QThread::finished, this, &QgsLockedFeature::validationFinished );
  mValidator->start();

  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( tr( "Validation started." ) );
}

void QgsLockedFeature::addError( QgsGeometry::Error e )
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

void QgsLockedFeature::validationFinished()
{
  QgsStatusBar *sb = QgisApp::instance()->statusBarIface();
  sb->showMessage( tr( "Validation finished (%n error(s) found).", "number of geometry errors", mGeomErrorMarkers.size() ) );
}

void QgsLockedFeature::replaceVertexMap()
{
  // delete old map
  deleteVertexMap();

  // create new map
  createVertexMap();

  // validate the geometry
  validateGeometry();

  emit vertexMapChanged();
}

void QgsLockedFeature::deleteVertexMap()
{
  const auto constMVertexMap = mVertexMap;
  for ( QgsVertexEntry *entry : constMVertexMap )
  {
    delete entry;
  }

  mVertexMap.clear();
}

bool QgsLockedFeature::isSelected( int vertexNr )
{
  return mVertexMap.at( vertexNr )->isSelected();
}

QgsGeometry *QgsLockedFeature::geometry()
{
  Q_ASSERT( mGeometry );
  return mGeometry;
}

void QgsLockedFeature::createVertexMap()
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
    mVertexMap.append( new QgsVertexEntry( pt, vertexId ) );
  }
}

void QgsLockedFeature::selectVertex( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );
  entry->setSelected( true );

  emit selectionChanged();
}

void QgsLockedFeature::deselectVertex( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );
  entry->setSelected( false );

  emit selectionChanged();
}

void QgsLockedFeature::deselectAllVertices()
{
  for ( int i = 0; i < mVertexMap.size(); i++ )
  {
    mVertexMap.at( i )->setSelected( false );
  }
  emit selectionChanged();
}

void QgsLockedFeature::invertVertexSelection( int vertexNr )
{
  if ( vertexNr < 0 || vertexNr >= mVertexMap.size() )
    return;

  QgsVertexEntry *entry = mVertexMap.at( vertexNr );

  const bool selected = !entry->isSelected();

  entry->setSelected( selected );
  emit selectionChanged();
}

void QgsLockedFeature::invertVertexSelection( const QVector<int> &vertexIndices )
{
  const auto constVertexIndices = vertexIndices;
  for ( const int index : constVertexIndices )
  {
    if ( index < 0 || index >= mVertexMap.size() )
      continue;

    QgsVertexEntry *entry = mVertexMap.at( index );
    entry->setSelected( !entry->isSelected() );
  }
  emit selectionChanged();
}

QgsFeatureId QgsLockedFeature::featureId()
{
  return mFeatureId;
}

QList<QgsVertexEntry *> &QgsLockedFeature::vertexMap()
{
  return mVertexMap;
}

QgsVectorLayer *QgsLockedFeature::layer()
{
  return mLayer;
}
