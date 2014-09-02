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
#include "qgsvectorlayer.h"

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

bool QgsMapLayerProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilters.testFlag( All ) )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QgsMapLayer* layer = static_cast<QgsMapLayer*>( index.internalPointer() );
  if ( !layer )
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
    QgsVectorLayer* vl = dynamic_cast<QgsVectorLayer*>( layer );
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
