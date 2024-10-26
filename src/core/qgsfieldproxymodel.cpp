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
#include "moc_qgsfieldproxymodel.cpp"
#include "qgsfieldmodel.h"
#include "qgsvariantutils.h"

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
  const QVariant originVariant = sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldOrigin ) );
  if ( QgsVariantUtils::isNull( originVariant ) )
  {
    //expression
    return true;
  }

  const Qgis::FieldOrigin origin = static_cast< Qgis::FieldOrigin >( originVariant.toInt() );
  switch ( origin )
  {
    case Qgis::FieldOrigin::Join:
    {
      // show joined fields (e.g. auxiliary fields) only if they have a non-hidden editor widget.
      // This enables them to be bulk field-calculated when a user needs to, but hides them by default
      // (since there's often MANY of these, e.g. after using the label properties tool on a layer)
      if ( sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::EditorWidgetType ) ).toString() == QLatin1String( "Hidden" ) )
        return true;

      return !sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::JoinedFieldIsEditable ) ).toBool();
    }

    case Qgis::FieldOrigin::Unknown:
    case Qgis::FieldOrigin::Expression:
      //read only
      return true;

    case Qgis::FieldOrigin::Edit:
    case Qgis::FieldOrigin::Provider:
    {
      if ( !sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldIsWidgetEditable ) ).toBool() )
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

  if ( mFilters.testFlag( QgsFieldProxyModel::OriginProvider ) )
  {
    const Qgis::FieldOrigin origin = static_cast< Qgis::FieldOrigin >( sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldOrigin ) ).toInt() );
    switch ( origin )
    {
      case Qgis::FieldOrigin::Unknown:
      case Qgis::FieldOrigin::Join:
      case Qgis::FieldOrigin::Edit:
      case Qgis::FieldOrigin::Expression:
        return false;

      case Qgis::FieldOrigin::Provider:
        break;
    }
  }

  if ( mFilters.testFlag( AllTypes ) )
    return true;

  const QVariant typeVar = sourceModel()->data( index, static_cast< int >( QgsFieldModel::CustomRole::FieldType ) );

  // if expression, consider valid
  if ( QgsVariantUtils::isNull( typeVar ) )
    return true;

  bool ok;
  const QMetaType::Type type = static_cast<QMetaType::Type>( typeVar.toInt( &ok ) );
  if ( !ok )
    return true;

  if ( ( mFilters.testFlag( String ) && type == QMetaType::Type::QString ) ||
       ( mFilters.testFlag( LongLong ) && type == QMetaType::Type::LongLong ) ||
       ( mFilters.testFlag( Int ) && type == QMetaType::Type::Int ) ||
       ( mFilters.testFlag( Double ) && type == QMetaType::Type::Double ) ||
       ( mFilters.testFlag( Date ) && type == QMetaType::Type::QDate ) ||
       ( mFilters.testFlag( Date ) && type == QMetaType::Type::QDateTime ) ||
       ( mFilters.testFlag( DateTime ) && type == QMetaType::Type::QDateTime ) ||
       ( mFilters.testFlag( Time ) && type == QMetaType::Type::QTime ) ||
       ( mFilters.testFlag( Binary ) && type == QMetaType::Type::QByteArray ) ||
       ( mFilters.testFlag( Boolean ) && type == QMetaType::Type::Bool ) )
    return true;

  return false;
}

bool QgsFieldProxyModel::lessThan( const QModelIndex &left, const QModelIndex &right ) const
{
  // empty field is always first
  if ( sourceModel()->data( left, static_cast< int >( QgsFieldModel::CustomRole::IsEmpty ) ).toBool() )
    return true;
  else if ( sourceModel()->data( right, static_cast< int >( QgsFieldModel::CustomRole::IsEmpty ) ).toBool() )
    return false;

  // order is field order, then expressions
  bool lok, rok;
  const int leftId = sourceModel()->data( left, static_cast< int >( QgsFieldModel::CustomRole::FieldIndex ) ).toInt( &lok );
  const int rightId = sourceModel()->data( right, static_cast< int >( QgsFieldModel::CustomRole::FieldIndex ) ).toInt( &rok );

  if ( !lok )
    return false;
  if ( !rok )
    return true;

  return leftId < rightId;
}
