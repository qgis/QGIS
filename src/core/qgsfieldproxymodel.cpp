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
  , mFilters( AllTypes )
  , mModel( new QgsFieldModel( this ) )
{
  setSourceModel( mModel );
}

QgsFieldProxyModel *QgsFieldProxyModel::setFilters( QgsFieldProxyModel::Filters filters )
{
  mFilters = filters;
  invalidateFilter();
  return this;
}

bool QgsFieldProxyModel::isReadOnly( const QModelIndex &index ) const
{
  const QVariant originVariant = sourceModel()->data( index, QgsFieldModel::FieldOriginRole );
  if ( originVariant.isNull() )
  {
    //expression
    return true;
  }

  const QgsFields::FieldOrigin origin = static_cast< QgsFields::FieldOrigin >( originVariant.toInt() );
  switch ( origin )
  {
    case QgsFields::OriginJoin:
    {
      // show joined fields (e.g. auxiliary fields) only if they have a non-hidden editor widget.
      // This enables them to be bulk field-calculated when a user needs to, but hides them by default
      // (since there's often MANY of these, e.g. after using the label properties tool on a layer)
      if ( sourceModel()->data( index, QgsFieldModel::EditorWidgetType ).toString() == QLatin1String( "Hidden" ) )
        return true;

      return !sourceModel()->data( index, QgsFieldModel::JoinedFieldIsEditable ).toBool();
    }

    case QgsFields::OriginUnknown:
    case QgsFields::OriginExpression:
      //read only
      return true;

    case QgsFields::OriginEdit:
    case QgsFields::OriginProvider:
    {
      if ( !sourceModel()->data( index, QgsFieldModel::FieldIsWidgetEditable ).toBool() )
      {
        return true;
      }
      else
      {
        //not read only
        return false;
      }
    }

  }
  return false; // avoid warnings
}

bool QgsFieldProxyModel::filterAcceptsRow( int source_row, const QModelIndex &source_parent ) const
{
  const QModelIndex index = sourceModel()->index( source_row, 0, source_parent );

  if ( mFilters.testFlag( HideReadOnly ) && isReadOnly( index ) )
    return false;

  if ( mFilters.testFlag( AllTypes ) )
    return true;

  const QVariant typeVar = sourceModel()->data( index, QgsFieldModel::FieldTypeRole );

  // if expression, consider valid
  if ( typeVar.isNull() )
    return true;

  bool ok;
  const QVariant::Type type = ( QVariant::Type )typeVar.toInt( &ok );
  if ( !ok )
    return true;

  if ( ( mFilters.testFlag( String ) && type == QVariant::String ) ||
       ( mFilters.testFlag( LongLong ) && type == QVariant::LongLong ) ||
       ( mFilters.testFlag( Int ) && type == QVariant::Int ) ||
       ( mFilters.testFlag( Double ) && type == QVariant::Double ) ||
       ( mFilters.testFlag( Date ) && type == QVariant::Date ) ||
       ( mFilters.testFlag( Date ) && type == QVariant::DateTime ) ||
       ( mFilters.testFlag( DateTime ) && type == QVariant::DateTime ) ||
       ( mFilters.testFlag( Time ) && type == QVariant::Time ) )
    return true;

  return false;
}

bool QgsFieldProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty field is always first
  if ( sourceModel()->data( left, QgsFieldModel::IsEmptyRole ).toBool() )
    return true;
  else if ( sourceModel()->data( right, QgsFieldModel::IsEmptyRole ).toBool() )
    return false;

  // order is field order, then expressions
  bool lok, rok;
  const int leftId = sourceModel()->data( left, QgsFieldModel::FieldIndexRole ).toInt( &lok );
  const int rightId = sourceModel()->data( right, QgsFieldModel::FieldIndexRole ).toInt( &rok );

  if ( !lok )
    return false;
  if ( !rok )
    return true;

  return leftId < rightId;
}
