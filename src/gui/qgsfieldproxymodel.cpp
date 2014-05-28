/***************************************************************************
   qgsfieldproxymodel.cpp
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

#include "qgsfieldproxymodel.h"
#include "qgsfieldmodel.h"
#include "qgsvectorlayer.h"

QgsFieldProxyModel::QgsFieldProxyModel( QObject *parent )
    : QSortFilterProxyModel( parent )
    , mFilters( All )
    , mModel( new QgsFieldModel( this ) )
{
  setSourceModel( mModel );
}

QgsFieldProxyModel *QgsFieldProxyModel::setFilters( Filters filters )
{
  mFilters = filters;
  return this;
}

bool QgsFieldProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  if ( mFilters.testFlag( All ) )
    return true;

  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );
  QVariant typeVar = sourceModel()->data( index, QgsFieldModel::FieldTypeRole );

  // if expression, consider valid
  if ( typeVar.isNull() )
    return true;

  bool ok;
  QVariant::Type type = ( QVariant::Type )typeVar.toInt( &ok );
  if ( !ok )
    return true;

  if (( mFilters.testFlag( String ) && type == QVariant::String ) ||
      ( mFilters.testFlag( LongLong ) && type == QVariant::LongLong ) ||
      ( mFilters.testFlag( Int ) && type == QVariant::Int ) ||
      ( mFilters.testFlag( Double ) && type == QVariant::Double ) ||
      ( mFilters.testFlag( Date ) && type == QVariant::Date ) )
    return true;

  return false;
}

bool QgsFieldProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // order is field order, then expressions
  bool lok, rok;
  int leftId = sourceModel()->data( left, QgsFieldModel::FieldIndexRole ).toInt( &lok );
  int rightId = sourceModel()->data( right, QgsFieldModel::FieldIndexRole ).toInt( &rok );

  if ( !lok )
    return false;
  if ( !rok )
    return true;

  return leftId < rightId;
}
