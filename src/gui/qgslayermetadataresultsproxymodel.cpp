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

void QgsLayerMetadataResultsProxyModel::setFilterString( const QString &filterString )
{
  mFilterString = filterString;
  invalidateFilter();
}

bool QgsLayerMetadataResultsProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  QModelIndex index0 = sourceModel()->index( sourceRow, 0, sourceParent );
  bool result { QSortFilterProxyModel::filterAcceptsRow( sourceRow, sourceParent ) };
  if ( result && ! mFilterString.isEmpty() )
  {
    result = result && sourceModel()->data( index0, Qt::ItemDataRole::UserRole ).value<QgsLayerMetadataProviderResult>( ).contains( mFilterString );
  }
  if ( result && ! mFilterExtent.isEmpty() )
  {
    result = result && mFilterExtent.intersects( sourceModel()->data( index0, Qt::ItemDataRole::UserRole ).value<QgsLayerMetadataProviderResult>( ).geographicExtent().boundingBox() );
  }
  return result;
}

const QString QgsLayerMetadataResultsProxyModel::filterString() const
{
  return mFilterString;
}
