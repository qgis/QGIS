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

QgsMapLayer::StyleCategory QgsMapLayerStyleCategoriesModel::index2category( const QModelIndex &index ) const
{
  return mCategoryList.at( index.row() - ( mShowAllCategories ? 0 : 1 ) );
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

  QgsMapLayer::StyleCategory category = index2category( index );

  switch ( category )
  {
    case QgsMapLayer::LayerConfiguration:
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Layer Configuration" );
        case ToolTip:
        case Qt::ToolTipRole:
          return tr( "Identifiable, removable, searchable, display expression, read-only" );
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/system.svg" ) );
      }
    case QgsMapLayer::Symbology :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Symbology" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/symbology.svg" ) );
      }
    case QgsMapLayer::Symbology3D:
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "3D Symbology" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/3d.svg" ) );
      }
    case QgsMapLayer::Labeling :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Labels" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/labels.svg" ) );
      }
    case QgsMapLayer::Fields :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Fields" );
        case ToolTip:
        case Qt::ToolTipRole:
          return tr( "Aliases, widgets, WMS/WFS, expressions, constraints, virtual fields" );
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mSourceFields" ) );
      }
    case QgsMapLayer::Forms :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Forms" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView" ) );
      }
    case QgsMapLayer::Actions :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Actions" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/action.svg" ) );
      }
    case QgsMapLayer::MapTips :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Map Tips" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/display.svg" ) );
      }
    case QgsMapLayer::Diagrams :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Diagrams" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/diagram.svg" ) );
      }
    case QgsMapLayer::AttributeTable :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Attribute Table Settings" );
        case ToolTip:
        case Qt::ToolTipRole:
          return tr( "Choice and order of columns, conditional styling" );
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable" ) );
      }
    case QgsMapLayer::Rendering :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Rendering" );
        case ToolTip:
        case Qt::ToolTipRole:
          return tr( "Scale visibility, simplify method, opacity" );
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/rendering.svg" ) );
      }
    case QgsMapLayer::CustomProperties :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "Custom Properties" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) );
      }
    case QgsMapLayer::AllStyleCategories :
      switch ( role )
      {
        case Qt::DisplayRole:
        case ReadableCategory:
          return tr( "All Style Categories" );
        case ToolTip:
        case Qt::ToolTipRole:
          return QVariant();
        case Icon:
        case Qt::DecorationRole:
          return QVariant();
      }
  }
  return QVariant();
}

bool QgsMapLayerStyleCategoriesModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.row() >= rowCount() )
    return false;

  if ( role == Qt::CheckStateRole )
  {
    QgsMapLayer::StyleCategory category = index2category( index );
    if ( value.value<Qt::CheckState>() == Qt::Checked )
    {
      mCategories |= category;
      emit dataChanged( index, index );
      return true;
    }
    else if ( value.value<Qt::CheckState>() == Qt::Unchecked )
    {
      mCategories &= category;
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
