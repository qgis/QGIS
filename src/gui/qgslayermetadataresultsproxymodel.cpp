/***************************************************************************
  qgslayermetadataresultsproxymodel.cpp - QgsLayerMetadataResultsProxyModel

 ---------------------
 begin                : 1.9.2022
 copyright            : (C) 2022 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgslayermetadataresultsproxymodel.h"
#include "qgsabstractlayermetadataprovider.h"

QgsLayerMetadataResultsProxyModel::QgsLayerMetadataResultsProxyModel( QObject *parent ) : QSortFilterProxyModel( parent )
{
}

void QgsLayerMetadataResultsProxyModel::setFilterExtent( const QgsRectangle &extent )
{
  mFilterExtent = extent;
  invalidateFilter();
}

void QgsLayerMetadataResultsProxyModel::setFilterGeometryType( const QgsWkbTypes::GeometryType geometryType )
{
  mFilterGeometryType = geometryType;
  invalidateFilter();
}

void QgsLayerMetadataResultsProxyModel::setFilterString( const QString &filterString )
{
  mFilterString = filterString;
  invalidateFilter();
}

void QgsLayerMetadataResultsProxyModel::setFilterMapLayerType( const QgsMapLayerType mapLayerType )
{
  mFilterMapLayerType = mapLayerType;
  invalidateFilter();
}

bool QgsLayerMetadataResultsProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QModelIndex index0 = sourceModel()->index( sourceRow, 0, sourceParent );
  bool result { QSortFilterProxyModel::filterAcceptsRow( sourceRow, sourceParent ) };

  if ( result )
  {
    const QgsLayerMetadataProviderResult &metadataResult { sourceModel()->data( index0, Qt::ItemDataRole::UserRole ).value<QgsLayerMetadataProviderResult>( ) };

    if ( ! mFilterString.isEmpty() )
    {
      result = result && metadataResult.contains( mFilterString );
    }

    if ( result && ! mFilterExtent.isEmpty() )
    {
      // Exclude aspatial from extent filter
      result = result && ( metadataResult.geometryType() != QgsWkbTypes::UnknownGeometry && metadataResult.geometryType() != QgsWkbTypes::NullGeometry ) && mFilterExtent.intersects( metadataResult.geographicExtent().boundingBox() );
    }

    if ( result && mFilterMapLayerTypeEnabled )
    {
      result = result && metadataResult.layerType() == mFilterMapLayerType;
    }

    if ( result && mFilterGeometryTypeEnabled )
    {
      if ( mFilterGeometryType == QgsWkbTypes::UnknownGeometry || mFilterGeometryType == QgsWkbTypes::NullGeometry )
      {
        result = result && ( metadataResult.geometryType() == QgsWkbTypes::UnknownGeometry || metadataResult.geometryType() == QgsWkbTypes::NullGeometry );
      }
      else
      {
        result = result &&  metadataResult.geometryType() == mFilterGeometryType;
      }
    }
  }

  return result;
}

void QgsLayerMetadataResultsProxyModel::setFilterMapLayerTypeEnabled( bool enabled )
{
  mFilterMapLayerTypeEnabled = enabled;
  invalidateFilter();
}

void QgsLayerMetadataResultsProxyModel::setFilterGeometryTypeEnabled( bool enabled )
{
  mFilterGeometryTypeEnabled = enabled;
  invalidateFilter();
}

const QString QgsLayerMetadataResultsProxyModel::filterString() const
{
  return mFilterString;
}

