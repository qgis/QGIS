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
#include "moc_qgsmaplayerproxymodel.cpp"
#include "qgsmaplayermodel.h"
#include "qgsmaplayer.h"
#include "qgsproject.h"
#include "qgsvectorlayer.h"

QgsMapLayerProxyModel::QgsMapLayerProxyModel( QObject *parent )
  : QSortFilterProxyModel( parent )
  , mFilters( Qgis::LayerFilter::All )
  , mModel( new QgsMapLayerModel( parent ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

QgsMapLayerProxyModel *QgsMapLayerProxyModel::setFilters( Qgis::LayerFilters filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

bool QgsMapLayerProxyModel::layerMatchesFilters( const QgsMapLayer *layer, const Qgis::LayerFilters &filters )
{
  if ( filters.testFlag( Qgis::LayerFilter::All ) )
    return true;

  // layer type
  if ( ( filters.testFlag( Qgis::LayerFilter::RasterLayer ) && layer->type() == Qgis::LayerType::Raster ) ||
       ( filters.testFlag( Qgis::LayerFilter::VectorLayer ) && layer->type() == Qgis::LayerType::Vector ) ||
       ( filters.testFlag( Qgis::LayerFilter::MeshLayer ) && layer->type() == Qgis::LayerType::Mesh ) ||
       ( filters.testFlag( Qgis::LayerFilter::VectorTileLayer ) && layer->type() == Qgis::LayerType::VectorTile ) ||
       ( filters.testFlag( Qgis::LayerFilter::PointCloudLayer ) && layer->type() == Qgis::LayerType::PointCloud ) ||
       ( filters.testFlag( Qgis::LayerFilter::AnnotationLayer ) && layer->type() == Qgis::LayerType::Annotation ) ||
       ( filters.testFlag( Qgis::LayerFilter::TiledSceneLayer ) && layer->type() == Qgis::LayerType::TiledScene ) ||
       ( filters.testFlag( Qgis::LayerFilter::PluginLayer ) && layer->type() == Qgis::LayerType::Plugin ) )
    return true;

  // geometry type
  const bool detectGeometry = filters.testFlag( Qgis::LayerFilter::NoGeometry ) ||
                              filters.testFlag( Qgis::LayerFilter::PointLayer ) ||
                              filters.testFlag( Qgis::LayerFilter::LineLayer ) ||
                              filters.testFlag( Qgis::LayerFilter::PolygonLayer ) ||
                              filters.testFlag( Qgis::LayerFilter::HasGeometry );
  if ( detectGeometry && layer->type() == Qgis::LayerType::Vector )
  {
    if ( const QgsVectorLayer *vl = qobject_cast<const QgsVectorLayer *>( layer ) )
    {
      if ( filters.testFlag( Qgis::LayerFilter::HasGeometry ) && vl->isSpatial() )
        return true;
      if ( filters.testFlag( Qgis::LayerFilter::NoGeometry ) && vl->geometryType() == Qgis::GeometryType::Null )
        return true;
      if ( filters.testFlag( Qgis::LayerFilter::PointLayer ) && vl->geometryType() == Qgis::GeometryType::Point )
        return true;
      if ( filters.testFlag( Qgis::LayerFilter::LineLayer ) && vl->geometryType() == Qgis::GeometryType::Line )
        return true;
      if ( filters.testFlag( Qgis::LayerFilter::PolygonLayer ) && vl->geometryType() == Qgis::GeometryType::Polygon )
        return true;
    }
  }

  return false;
}

void QgsMapLayerProxyModel::setLayerWhitelist( const QList<QgsMapLayer *> &layers )
{
  setLayerAllowlist( layers );
}

void QgsMapLayerProxyModel::setLayerAllowlist( const QList<QgsMapLayer *> &layers )
{
  if ( mLayerAllowlist == layers )
    return;

  mLayerAllowlist = layers;
  invalidateFilter();
}

void QgsMapLayerProxyModel::setExceptedLayerList( const QList<QgsMapLayer *> &exceptList )
{
  if ( mExceptList == exceptList )
    return;

  mExceptList = exceptList;
  invalidateFilter();
}

void  QgsMapLayerProxyModel::setProject( QgsProject *project )
{
  mModel->setProject( project );
}

void QgsMapLayerProxyModel::setExceptedLayerIds( const QStringList &ids )
{
  mExceptList.clear();

  const auto constIds = ids;
  for ( const QString &id : constIds )
  {
    QgsMapLayer *l = QgsProject::instance()->mapLayer( id ); // skip-keyword-check
    if ( l )
      mExceptList << l;
  }
  invalidateFilter();
}

QStringList QgsMapLayerProxyModel::exceptedLayerIds() const
{
  QStringList lst;

  const auto constMExceptList = mExceptList;
  for ( QgsMapLayer *l : constMExceptList )
    lst << l->id();

  return lst;
}

void QgsMapLayerProxyModel::setExcludedProviders( const QStringList &providers )
{
  mExcludedProviders = providers;
  invalidateFilter();
}

bool QgsMapLayerProxyModel::acceptsLayer( QgsMapLayer *layer ) const
{
  if ( !layer )
    return false;

  if ( !mLayerAllowlist.isEmpty() && !mLayerAllowlist.contains( layer ) )
    return false;

  if ( mExceptList.contains( layer ) )
    return false;

  if ( layer->dataProvider() && mExcludedProviders.contains( layer->providerType() ) )
    return false;

  if ( mFilters.testFlag( Qgis::LayerFilter::WritableLayer ) && layer->readOnly() )
    return false;

  if ( !layer->name().contains( mFilterString, Qt::CaseInsensitive ) )
    return false;

  return layerMatchesFilters( layer, mFilters );
}

void QgsMapLayerProxyModel::setFilterString( const QString &filter )
{
  mFilterString = filter;
  invalidateFilter();
}

bool QgsMapLayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilters.testFlag( Qgis::LayerFilter::All ) && mExceptList.isEmpty() && mLayerAllowlist.isEmpty() && mExcludedProviders.isEmpty() && mFilterString.isEmpty() )
    return true;

  const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( sourceModel()->data( index, static_cast< int >( QgsMapLayerModel::CustomRole::Empty ) ).toBool()
       || sourceModel()->data( index, static_cast< int >( QgsMapLayerModel::CustomRole::Additional ) ).toBool() )
    return true;

  return acceptsLayer( static_cast<QgsMapLayer *>( index.internalPointer() ) );
}

bool QgsMapLayerProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty row is always first
  if ( sourceModel()->data( left, static_cast< int >( QgsMapLayerModel::CustomRole::Empty ) ).toBool() )
    return true;
  else if ( sourceModel()->data( right, static_cast< int >( QgsMapLayerModel::CustomRole::Empty ) ).toBool() )
    return false;

  // additional rows are always last
  const bool leftAdditional = sourceModel()->data( left, static_cast< int >( QgsMapLayerModel::CustomRole::Additional ) ).toBool();
  const bool rightAdditional = sourceModel()->data( right, static_cast< int >( QgsMapLayerModel::CustomRole::Additional ) ).toBool();

  if ( leftAdditional && !rightAdditional )
    return false;
  else if ( rightAdditional && !leftAdditional )
    return true;

  // default mode is alphabetical order
  const QString leftStr = sourceModel()->data( left ).toString();
  const QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
