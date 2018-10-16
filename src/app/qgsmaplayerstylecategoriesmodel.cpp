/***************************************************************************
  qgsmaplayerstylecategoriesmodel.cpp
  --------------------------------------
  Date                 : September 2018
  Copyright            : (C) 2018 by Denis Rouzaud
  Email                : denis@opengis.ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmaplayerstylecategoriesmodel.h"

QgsMapLayerStyleCategoriesModel::QgsMapLayerStyleCategoriesModel( QObject *parent )
  : QAbstractListModel( parent )
{
  mCategoryList = qgsEnumMap<QgsMapLayer::StyleCategory>().keys();
  // move All categories to top
  mCategoryList.move( mCategoryList.indexOf( QgsMapLayer::AllStyleCategories ), 0 );
}

void QgsMapLayerStyleCategoriesModel::setCategories( QgsMapLayer::StyleCategories categories )
{
  if ( mCategories == categories )
    return;

  beginResetModel();
  mCategories = categories;
  endResetModel();
}

QgsMapLayer::StyleCategories QgsMapLayerStyleCategoriesModel::categories() const
{
  return mCategories;
}

void QgsMapLayerStyleCategoriesModel::setShowAllCategories( bool showAll )
{
  beginResetModel();
  mShowAllCategories = showAll;
  endResetModel();
}

int QgsMapLayerStyleCategoriesModel::rowCount( const QModelIndex & ) const
{
  int count = mCategoryList.count();
  if ( !mShowAllCategories )
    count--;
  return count;
}

int QgsMapLayerStyleCategoriesModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QVariant QgsMapLayerStyleCategoriesModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= rowCount() )
    return QVariant();

  QgsMapLayer::StyleCategory category = mCategoryList.at( index.row() + ( mShowAllCategories ? 0 : 1 ) );

  if ( role == Qt::UserRole )
  {
    return category;
  }
  if ( role == Qt::CheckStateRole )
  {
    return mCategories.testFlag( category ) ? Qt::Checked : Qt::Unchecked;
  }

  switch ( category )
  {
    case QgsMapLayer::LayerConfiguration:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Layer Configuration" );
        case Qt::ToolTipRole:
          return tr( "Identifiable, removable, searchable, display expression, read-only" );
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/system.svg" ) );
      }
      break;
    case QgsMapLayer::Symbology:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Symbology" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/symbology.svg" ) );
      }
      break;
    case QgsMapLayer::Symbology3D:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "3D Symbology" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/3d.svg" ) );
      }
      break;
    case QgsMapLayer::Labeling:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Labels" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/labels.svg" ) );
      }
      break;
    case QgsMapLayer::Fields:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Fields" );
        case Qt::ToolTipRole:
          return tr( "Aliases, widgets, WMS/WFS, expressions, constraints, virtual fields" );
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mSourceFields.svg" ) );
      }
      break;
    case QgsMapLayer::Forms:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Forms" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      break;
    case QgsMapLayer::Actions:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Actions" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/action.svg" ) );
      }
      break;
    case QgsMapLayer::MapTips:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Map Tips" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/display.svg" ) );
      }
      break;
    case QgsMapLayer::Diagrams:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Diagrams" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/diagram.svg" ) );
      }
      break;
    case QgsMapLayer::AttributeTable:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Attribute Table Settings" );
        case Qt::ToolTipRole:
          return tr( "Choice and order of columns, conditional styling" );
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) );
      }
      break;
    case QgsMapLayer::Rendering:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Rendering" );
        case Qt::ToolTipRole:
          return tr( "Scale visibility, simplify method, opacity" );
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/rendering.svg" ) );
      }
      break;
    case QgsMapLayer::CustomProperties:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Custom Properties" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) );
      }
      break;
    case QgsMapLayer::GeometryOptions:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "Geometry Options" );
        case Qt::ToolTipRole:
          return tr( "Geometry constraints and validity checks" );
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/digitizing.svg" ) );
      }
      break;
    case QgsMapLayer::AllStyleCategories:
      switch ( role )
      {
        case Qt::DisplayRole:
          return tr( "All Style Categories" );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QVariant();
      }
      break;

  }
  return QVariant();
}

bool QgsMapLayerStyleCategoriesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.row() >= rowCount() )
    return false;

  if ( role == Qt::CheckStateRole )
  {
    QgsMapLayer::StyleCategory category = data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
    if ( value.value<Qt::CheckState>() == Qt::Checked )
    {
      mCategories |= category;
      emit dataChanged( index, index );
      return true;
    }
    else if ( value.value<Qt::CheckState>() == Qt::Unchecked )
    {
      mCategories &= ~category;
      emit dataChanged( index, index );
      return true;
    }
  }
  return false;
}


Qt::ItemFlags QgsMapLayerStyleCategoriesModel::flags( const QModelIndex & ) const
{
  return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
}
