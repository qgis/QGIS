/***************************************************************************
   qgsmaplayerproxymodel.cpp
    --------------------------------------
   Date                 : 01.04.2014
   Copyright            : (C) 2014 Denis Rouzaud
   Email                : denis.rouzaud@gmail.com
***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "qgsmaplayerproxymodel.h"
#include "qgsmaplayermodel.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsmeshlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsrasterdataprovider.h"
#include "qgsmeshdataprovider.h"

QgsMapLayerProxyModel::QgsMapLayerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mFilters( All )
  , mModel( new QgsMapLayerModel( parent ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

QgsMapLayerProxyModel *QgsMapLayerProxyModel::setFilters( Filters filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

void QgsMapLayerProxyModel::setLayerWhitelist( const QList<QgsMapLayer *> &layers )
{
  if ( mLayerWhitelist == layers )
    return;

  mLayerWhitelist = layers;
  invalidateFilter();
}

void QgsMapLayerProxyModel::setExceptedLayerList( const QList<QgsMapLayer *> &exceptList )
{
  if ( mExceptList == exceptList )
    return;

  mExceptList = exceptList;
  invalidateFilter();
}

void QgsMapLayerProxyModel::setExceptedLayerIds( const QStringList &ids )
{
  mExceptList.clear();

  Q_FOREACH ( const QString &id, ids )
  {
    QgsMapLayer *l = QgsProject::instance()->mapLayer( id );
    if ( l )
      mExceptList << l;
  }
  invalidateFilter();
}

QStringList QgsMapLayerProxyModel::exceptedLayerIds() const
{
  QStringList lst;

  Q_FOREACH ( QgsMapLayer *l, mExceptList )
    lst << l->id();

  return lst;
}

void QgsMapLayerProxyModel::setExcludedProviders( const QStringList &providers )
{
  mExcludedProviders = providers;
  invalidateFilter();
}

void QgsMapLayerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsMapLayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilters.testFlag( All ) && mExceptList.isEmpty() && mLayerWhitelist.isEmpty() && mExcludedProviders.isEmpty() && mFilterString.isEmpty() )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( sourceModel()->data( index, QgsMapLayerModel::EmptyRole ).toBool()
       || sourceModel()->data( index, QgsMapLayerModel::AdditionalRole ).toBool() )
    return true;

  QgsMapLayer *layer = static_cast<QgsMapLayer *>( index.internalPointer() );
  if ( !layer )
    return false;

  if ( !mLayerWhitelist.isEmpty() && !mLayerWhitelist.contains( layer ) )
    return false;

  if ( mExceptList.contains( layer ) )
    return false;

  if ( layer->dataProvider() && mExcludedProviders.contains( layer->dataProvider()->name() ) )
    return false;

  if ( mFilters.testFlag( WritableLayer ) && layer->readOnly() )
    return false;

  if ( !layer->name().contains( mFilterString, Qt::CaseInsensitive ) )
    return false;

  // layer type
  if ( ( mFilters.testFlag( RasterLayer ) && layer->type() == QgsMapLayer::RasterLayer ) ||
       ( mFilters.testFlag( VectorLayer ) && layer->type() == QgsMapLayer::VectorLayer ) ||
       ( mFilters.testFlag( MeshLayer ) && layer->type() == QgsMapLayer::MeshLayer ) ||
       ( mFilters.testFlag( PluginLayer ) && layer->type() == QgsMapLayer::PluginLayer ) )
    return true;

  // geometry type
  bool detectGeometry = mFilters.testFlag( NoGeometry ) ||
                        mFilters.testFlag( PointLayer ) ||
                        mFilters.testFlag( LineLayer ) ||
                        mFilters.testFlag( PolygonLayer ) ||
                        mFilters.testFlag( HasGeometry );
  if ( detectGeometry && layer->type() == QgsMapLayer::VectorLayer )
  {
    if ( QgsVectorLayer *vl = qobject_cast< QgsVectorLayer *>( layer ) )
    {
      if ( mFilters.testFlag( HasGeometry ) && vl->isSpatial() )
        return true;
      if ( mFilters.testFlag( NoGeometry ) && vl->geometryType() == QgsWkbTypes::NullGeometry )
        return true;
      if ( mFilters.testFlag( PointLayer ) && vl->geometryType() == QgsWkbTypes::PointGeometry )
        return true;
      if ( mFilters.testFlag( LineLayer ) && vl->geometryType() == QgsWkbTypes::LineGeometry )
        return true;
      if ( mFilters.testFlag( PolygonLayer ) && vl->geometryType() == QgsWkbTypes::PolygonGeometry )
        return true;
    }
  }

  return false;
}

bool QgsMapLayerProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty row is always first
  if ( sourceModel()->data( left, QgsMapLayerModel::EmptyRole ).toBool() )
    return true;
  else if ( sourceModel()->data( right, QgsMapLayerModel::EmptyRole ).toBool() )
    return false;

  // additional rows are always last
  bool leftAdditional = sourceModel()->data( left, QgsMapLayerModel::AdditionalRole ).toBool();
  bool rightAdditional = sourceModel()->data( right, QgsMapLayerModel::AdditionalRole ).toBool();

  if ( leftAdditional && !rightAdditional )
    return false;
  else if ( rightAdditional && !leftAdditional )
    return true;

  // default mode is alphabetical order
  QString leftStr = sourceModel()->data( left ).toString();
  QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
