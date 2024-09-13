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
#include "qgsapplication.h"

QgsMapLayerStyleCategoriesModel::QgsMapLayerStyleCategoriesModel( Qgis::LayerType type, QObject *parent )
  : QAbstractListModel( parent )
{
  switch ( type )
  {
    case Qgis::LayerType::Vector:
      mCategoryList = qgsEnumList<QgsMapLayer::StyleCategory>();
      break;

    case Qgis::LayerType::VectorTile:
      mCategoryList << QgsMapLayer::StyleCategory::Symbology << QgsMapLayer::StyleCategory::Labeling << QgsMapLayer::StyleCategory::AllStyleCategories;
      break;

    case Qgis::LayerType::Raster:
      mCategoryList << QgsMapLayer::StyleCategory::LayerConfiguration
                    << QgsMapLayer::StyleCategory::Symbology
                    << QgsMapLayer::StyleCategory::MapTips
                    << QgsMapLayer::StyleCategory::Rendering
                    << QgsMapLayer::StyleCategory::CustomProperties
                    << QgsMapLayer::StyleCategory::Temporal
                    << QgsMapLayer::StyleCategory::Elevation
                    << QgsMapLayer::StyleCategory::AttributeTable
                    << QgsMapLayer::StyleCategory::Notes
                    << QgsMapLayer::StyleCategory::AllStyleCategories;
      break;
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Mesh:
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Group:
    case Qgis::LayerType::TiledScene:
      // not yet handled by the model
      break;
  }

  // move All categories to top
  int idxAllStyleCategories = mCategoryList.indexOf( QgsMapLayer::AllStyleCategories );
  if ( idxAllStyleCategories > 0 )
  {
    mCategoryList.move( idxAllStyleCategories, 0 );
  }
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
  if ( count > 0 && !mShowAllCategories )
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

  const QgsMapLayer::StyleCategory category = mCategoryList.at( index.row() + ( mShowAllCategories ? 0 : 1 ) );

  if ( role == Qt::UserRole )
  {
    return category;
  }
  if ( role == Qt::CheckStateRole )
  {
    return mCategories.testFlag( category ) ? Qt::Checked : Qt::Unchecked;
  }

  QString htmlStylePattern = QStringLiteral( "<p><b>%1</b><br/>%2</p>" );
  switch ( category )
  {
    case QgsMapLayer::StyleCategory::LayerConfiguration:
    {
      QString name = tr( "Layer Configuration" );
      QString description = tr( "Identifiable, removable, searchable, display expression, read-only, hidden" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:
          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/system.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Symbology:
    {
      QString name = tr( "Symbology" );
      QString description = tr( "Symbology" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/symbology.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Symbology3D:
    {
      QString name = tr( "3D Symbology" );
      QString description = tr( "3D Symbology" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/3d.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Labeling:
    {
      QString name = tr( "Labels" );
      QString description = tr( "Labels" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/labels.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Fields:
    {
      QString name = tr( "Fields" );
      QString description = tr( "Aliases, widgets, WMS/WFS, expressions, constraints, virtual fields" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mSourceFields.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Forms:
    {
      QString name = tr( "Attribute Form" );
      QString description = tr( "Attribute form settings, widget configuration" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionFormView.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Actions:
    {
      QString name = tr( "Actions" );
      QString description = tr( "Map layer actions" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/action.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::MapTips:
    {
      QString name = tr( "Map Tips" );
      QString description = tr( "Map Tips" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/display.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Diagrams:
    {
      QString name = tr( "Diagrams" );
      QString description = tr( "Diagrams" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/diagram.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AttributeTable:
    {
      QString name = tr( "Attribute Table Configuration" );
      QString description = tr( "Choice and order of columns, conditional styling" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOpenTable.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Rendering:
    {
      QString name = tr( "Rendering" );
      QString description = tr( "Scale visibility, simplify method, opacity" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/rendering.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::CustomProperties:
    {
      QString name = tr( "Custom Properties" );
      QString description = tr( "Variables, custom properties (often used by plugins and custom python code)" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/mActionOptions.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::GeometryOptions:
    {
      QString name = tr( "Geometry Options" );
      QString description = tr( "Geometry constraints and validity checks" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/digitizing.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Relations:
    {
      QString name = tr( "Relations" );
      QString description = tr( "Relations with other layers" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/relations.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Temporal:
    {
      QString name = tr( "Temporal Properties" );
      QString description = tr( "Temporal Properties" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/temporal.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Legend:
    {
      QString name = tr( "Legend Settings" );
      QString description = tr( "Legend Settings" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/legend.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Elevation:
    {
      QString name = tr( "Elevation Properties" );
      QString description = tr( "Elevation Properties" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( QStringLiteral( "/propertyicons/elevationscale.svg" ) );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Notes:
    {
      QString name = tr( "Notes" );
      QString description = tr( "Notes" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QIcon(); // TODO
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AllStyleCategories:
    {
      QString name = tr( "All Style Categories" );
      QString description = tr( "All Style Categories" );
      switch ( role )
      {
        case static_cast< int >( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return QVariant();
        case Qt::DecorationRole:
          return QVariant();
      }
      break;
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
    const QgsMapLayer::StyleCategory category = data( index, Qt::UserRole ).value<QgsMapLayer::StyleCategory>();
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

CategoryDisplayLabelDelegate::CategoryDisplayLabelDelegate( QObject *parent )
  : QItemDelegate( parent )
{
}

void CategoryDisplayLabelDelegate::drawDisplay( QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text ) const
{
  QLabel label;
  label.setText( text );
  label.setEnabled( option.state & QStyle::State_Enabled );
  label.setAttribute( Qt::WA_TranslucentBackground );
  label.setMargin( 3 );
  painter->save();
  painter->translate( rect.topLeft() );
  label.resize( rect.size() );
  label.render( painter );
  painter->restore();
}

QSize CategoryDisplayLabelDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QLabel label;
  QString display = index.model()->data( index, Qt::DisplayRole ).toString();
  label.setText( display );
  label.setMargin( 3 );
  return label.sizeHint();
}
