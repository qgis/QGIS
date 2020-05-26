/***************************************************************************
  qgsquickvaluerelationlistmodel.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsquickvaluerelationlistmodel.h"

#include "qgslogger.h"


QgsQuickValueRelationListModel::QgsQuickValueRelationListModel( QObject *parent )
  : QAbstractListModel( parent )
{
}

void QgsQuickValueRelationListModel::populate( const QVariantMap &config, const QgsFeature &formFeature )
{
  beginResetModel();
  mCache = QgsValueRelationFieldFormatter::createCache( config, formFeature );
  endResetModel();
}

QVariant QgsQuickValueRelationListModel::keyForRow( int row ) const
{
  if ( row < 0 || row >= mCache.count() )
  {
    QgsDebugMsg( "keyForRow: access outside of range " + QString::number( row ) );
    return QVariant();
  }
  return mCache[row].key;
}

int QgsQuickValueRelationListModel::rowForKey( const QVariant &key ) const
{
  for ( int i = 0; i < mCache.count(); ++i )
  {
    if ( mCache[i].key == key )
      return i;
  }
  QgsDebugMsg( "rowForKey: key not found: " + key.toString() );
  return -1;
}

int QgsQuickValueRelationListModel::rowCount( const QModelIndex & ) const
{
  return mCache.count();
}

QVariant QgsQuickValueRelationListModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() )
    return QVariant();

  int row = index.row();
  if ( row < 0 || row >= mCache.count() )
    return QVariant();

  if ( role == Qt::DisplayRole )
    return mCache[row].value;

  return QVariant();
}
