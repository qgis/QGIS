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

QgsFieldProxyModel *QgsFieldProxyModel::setFilters( const Filters& filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

bool QgsFieldProxyModel::isReadOnly( const QModelIndex& index ) const
{
  QVariant originVariant = sourceModel()->data( index, QgsFieldModel::FieldOriginRole );
  if ( originVariant.isNull() )
  {
    //expression
    return true;
  }

  QgsFields::FieldOrigin origin = static_cast< QgsFields::FieldOrigin >( originVariant.toInt() );
  switch ( origin )
  {
    case QgsFields::OriginUnknown:
    case QgsFields::OriginJoin:
    case QgsFields::OriginExpression:
      //read only
      return true;

    case QgsFields::OriginEdit:
    case QgsFields::OriginProvider:
      //not read only
      return false;
  }
  return false; // avoid warnings
}

bool QgsFieldProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( mFilters.testFlag( HideReadOnly ) && isReadOnly( index ) )
    return false;

  if ( mFilters.testFlag( All ) )
    return true;

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
      ( mFilters.testFlag( Date ) && type == QVariant::Date ) ||
      ( mFilters.testFlag( Date ) && type == QVariant::DateTime ) ||
      ( mFilters.testFlag( Time ) && type == QVariant::Time ) )
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
