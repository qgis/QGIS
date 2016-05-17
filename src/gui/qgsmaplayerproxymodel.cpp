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
#include "qgsmaplayerregistry.h"
#include "qgsvectorlayer.h"

QgsMapLayerProxyModel::QgsMapLayerProxyModel( QObject *parent )
    : QSortFilterProxyModel( parent )
    , mFilters( All )
    , mExceptList( QList<QgsMapLayer*>() )
    , mModel( new QgsMapLayerModel( parent ) )
{
  setSourceModel( mModel );
  setDynamicSortFilter( true );
  setSortLocaleAware( true );
  setFilterCaseSensitivity( Qt::CaseInsensitive );
  sort( 0 );
}

QgsMapLayerProxyModel *QgsMapLayerProxyModel::setFilters( const Filters& filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

void QgsMapLayerProxyModel::setExceptedLayerList( const QList<QgsMapLayer*>& exceptList )
{
  if ( mExceptList == exceptList )
    return;

  mExceptList = exceptList;
  invalidateFilter();
}

void QgsMapLayerProxyModel::setExceptedLayerIds( const QStringList& ids )
{
  mExceptList.clear();

  Q_FOREACH ( const QString& id, ids )
  {
    QgsMapLayer* l = QgsMapLayerRegistry::instance()->mapLayer( id );
    if ( l )
      mExceptList << l;
  }
  invalidateFilter();
}

QStringList QgsMapLayerProxyModel::exceptedLayerIds() const
{
  QStringList lst;

  Q_FOREACH ( QgsMapLayer* l, mExceptList )
    lst << l->id();

  return lst;
}

bool QgsMapLayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilters.testFlag( All ) && mExceptList.isEmpty() )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
  if ( !layer )
    return false;

  if ( mExceptList.contains( layer ) )
    return false;

  QgsVectorLayer* vl = qobject_cast<QgsVectorLayer*>( layer );

  if ( mFilters.testFlag( WritableLayer ) && layer->readOnly() )
    return false;

  // layer type
  if (( mFilters.testFlag( RasterLayer ) && layer->type() == QgsMapLayer::RasterLayer ) ||
      ( mFilters.testFlag( VectorLayer ) && layer->type() == QgsMapLayer::VectorLayer ) ||
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
    if ( vl )
    {
      if ( mFilters.testFlag( HasGeometry ) && vl->hasGeometryType() )
        return true;
      if ( mFilters.testFlag( NoGeometry ) && vl->geometryType() == QGis::NoGeometry )
        return true;
      if ( mFilters.testFlag( PointLayer ) && vl->geometryType() == QGis::Point )
        return true;
      if ( mFilters.testFlag( LineLayer ) && vl->geometryType() == QGis::Line )
        return true;
      if ( mFilters.testFlag( PolygonLayer ) && vl->geometryType() == QGis::Polygon )
        return true;
    }
  }

  return false;
}

bool QgsMapLayerProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // default mode is alphabetical order
  QString leftStr = sourceModel()->data( left ).toString();
  QString rightStr = sourceModel()->data( right ).toString();
  return QString::localeAwareCompare( leftStr, rightStr ) < 0;
}
