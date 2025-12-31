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

#include "moc_qgsmaplayerstylecategoriesmodel.cpp"

QgsMapLayerStyleCategoriesModel::QgsMapLayerStyleCategoriesModel( Qgis::LayerType type, QObject *parent )
  : QAbstractListModel( parent )
  , mLayerType( type )
{
  switch ( type )
  {
    case Qgis::LayerType::Vector:
      mCategoryList = qgsEnumList<QgsMapLayer::StyleCategory>();
      break;

    case Qgis::LayerType::VectorTile:
      mCategoryList << QgsMapLayer::StyleCategory::Symbology
                    << QgsMapLayer::StyleCategory::Labeling
                    << QgsMapLayer::StyleCategory::AllVisualStyleCategories
                    << QgsMapLayer::StyleCategory::AllStyleCategories;
      break;

    case Qgis::LayerType::Raster:
      mCategoryList << QgsMapLayer::StyleCategory::LayerConfiguration
                    << QgsMapLayer::StyleCategory::Symbology
                    << QgsMapLayer::StyleCategory::Labeling
                    << QgsMapLayer::StyleCategory::MapTips
                    << QgsMapLayer::StyleCategory::Rendering
                    << QgsMapLayer::StyleCategory::CustomProperties
                    << QgsMapLayer::StyleCategory::Temporal
                    << QgsMapLayer::StyleCategory::Elevation
                    << QgsMapLayer::StyleCategory::AttributeTable
                    << QgsMapLayer::StyleCategory::Notes
                    << QgsMapLayer::StyleCategory::Legend
                    << QgsMapLayer::StyleCategory::AllVisualStyleCategories
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

  // move the grouped categories to top
  int idxCategory = mCategoryList.indexOf( QgsMapLayer::AllAttributeCategories );
  if ( idxCategory > 0 )
  {
    mCategoryList.move( idxCategory, 0 );
  }
  idxCategory = mCategoryList.indexOf( QgsMapLayer::AllVisualStyleCategories );
  if ( idxCategory > 0 )
  {
    mCategoryList.move( idxCategory, 0 );
  }
  idxCategory = mCategoryList.indexOf( QgsMapLayer::AllStyleCategories );
  if ( idxCategory > 0 )
  {
    mCategoryList.move( idxCategory, 0 );
  }
}

void QgsMapLayerStyleCategoriesModel::setCategories( QgsMapLayer::StyleCategories categories )
{
  if ( mCategories == categories )
    return;

  // filter the categories and only preserve the categories supported by the current layer type
  QgsMapLayer::StyleCategories allowedCategories;
  for ( QgsMapLayer::StyleCategory category : std::as_const( mCategoryList ) )
  {
    allowedCategories |= category;
  }
  categories &= allowedCategories;

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
  {
    if ( mCategoryList.contains( QgsMapLayer::AllAttributeCategories ) )
    {
      count--;
    }
    if ( mCategoryList.contains( QgsMapLayer::AllVisualStyleCategories ) )
    {
      count--;
    }
    if ( mCategoryList.contains( QgsMapLayer::AllStyleCategories ) )
    {
      count--;
    }
  }

  return count;
}

int QgsMapLayerStyleCategoriesModel::columnCount( const QModelIndex & ) const
{
  return 1;
}

QVariant QgsMapLayerStyleCategoriesModel::data( const QModelIndex &index, int role ) const
{
  const int filteredRowCount = rowCount();

  if ( !index.isValid() || index.row() >= filteredRowCount )
    return QVariant();

  const QgsMapLayer::StyleCategory category = mCategoryList.at( index.row() + ( mShowAllCategories ? 0 : mCategoryList.count() - filteredRowCount ) );

  if ( role == Qt::UserRole )
  {
    return category;
  }
  if ( role == Qt::CheckStateRole )
  {
    return mCategories.testFlag( category ) ? Qt::Checked : Qt::Unchecked;
  }

  QString htmlStylePattern = u"<p><b>%1</b><br/><span style='color:gray;'>%2</span></p>"_s;
  switch ( category )
  {
    case QgsMapLayer::StyleCategory::LayerConfiguration:
    {
      QString name = tr( "Layer Configuration" );
      QString description = tr( "The layers display expression and the datasource flags: identifiable, removable, searchable, read-only and hidden from the project settings" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:
          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/layerconfiguration.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Symbology:
    {
      QString name = tr( "Symbology" );
      QString description = tr( "Everything from the symbology section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/symbology.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Symbology3D:
    {
      QString name = tr( "3D Symbology" );
      QString description = tr( "Everything from the 3D symbology section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/3d.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Labeling:
    {
      QString name = tr( "Labels" );
      QString description = tr( "Everything from the labels section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/labels.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Fields:
    {
      QString name = tr( "Fields" );
      QString description = tr( "Virtual fields, aliases, default value expressions and constraints from the form section and WMS/WFS exposure" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/sourcefieldsandforms.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Forms:
    {
      QString name = tr( "Attribute Form" );
      QString description = tr( "Form layout and widget configuration (no constraints and default value expressions)" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/mActionFormView.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Actions:
    {
      QString name = tr( "Actions" );
      QString description = tr( "Everything from the actions section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/action.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::MapTips:
    {
      QString name = tr( "Map Tips" );
      QString description = tr( "Map tips settings (no layer display expression)" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/display.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Diagrams:
    {
      QString name = tr( "Diagrams" );
      QString description = tr( "Everything from the diagram section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/diagram.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AttributeTable:
    {
      QString name = tr( "Attribute Table Configuration" );
      QString description = tr( "Attribute table settings: choice and order of columns and conditional styling" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/mActionOpenTable.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Rendering:
    {
      QString name = tr( "Rendering" );
      QString description = tr( "Everything from the rendering section: Scale visibility, simplify method, opacity, auto refresh etc." );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/rendering.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::CustomProperties:
    {
      QString name = tr( "Custom Properties" );
      QString description = tr( "Layer variables and embedded legend widgets as well as all the custom properties (often used by plugins and custom python code)" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/mActionOptions.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::GeometryOptions:
    {
      QString name = tr( "Geometry Options" );
      QString description = tr( "Geometry constraints and validity checks" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/digitizing.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Relations:
    {
      QString name = tr( "Relations" );
      QString description = tr( "The relations this layer has with other layers" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/relations.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Temporal:
    {
      QString name = tr( "Temporal Properties" );
      QString description = tr( "Everything from the temporal section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/temporal.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Legend:
    {
      QString name = tr( "Legend Settings" );
      QString description = tr( "Legend settings (no embedded legend widgets)" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/legend.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Elevation:
    {
      QString name = tr( "Elevation Properties" );
      QString description = tr( "Everything from the elevation section" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/elevationscale.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::Notes:
    {
      QString name = tr( "Notes" );
      QString description = tr( "The layer notes" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:

          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QgsApplication::getThemeIcon( u"/propertyicons/notes.svg"_s );
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AllAttributeCategories:
    {
      QString name = tr( "All Fields and Attribute Form Categories" );
      QString description = tr( "All fields and attribute form categories" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:
          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QVariant();
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AllVisualStyleCategories:
    {
      QString name = tr( "All Symbology and Labeling Categories" );
      QString description = mLayerType == Qgis::LayerType::Vector ? tr( "All symbology, labeling and diagram categories" ) : tr( "All symbology and labeling categories" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:
          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
          return description;
        case Qt::DecorationRole:
          return QVariant();
      }
      break;
    }
    case QgsMapLayer::StyleCategory::AllStyleCategories:
    {
      QString name = tr( "All Style Categories" );
      QString description = tr( "All style categories" );
      switch ( role )
      {
        case static_cast<int>( Role::NameRole ):
          return name;
        case Qt::DisplayRole:
          return htmlStylePattern.arg( name ).arg( description );
        case Qt::ToolTipRole:
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

QgsCategoryDisplayLabelDelegate::QgsCategoryDisplayLabelDelegate( QObject *parent )
  : QItemDelegate( parent )
{
}

void QgsCategoryDisplayLabelDelegate::drawDisplay( QPainter *painter, const QStyleOptionViewItem &option, const QRect &rect, const QString &text ) const
{
  QLabel label;
  label.setText( text );
  label.setEnabled( option.state & QStyle::State_Enabled );
  label.setAttribute( Qt::WA_TranslucentBackground );
  label.setMargin( 3 );
  label.setWordWrap( true );
  painter->save();
  painter->translate( rect.topLeft() );
  label.resize( rect.size() );
  label.render( painter );
  painter->restore();
}

QSize QgsCategoryDisplayLabelDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )
  QLabel label;
  QString display = index.model()->data( index, Qt::DisplayRole ).toString();
  label.setText( display );
  label.setWordWrap( true );
  label.setFixedWidth( option.widget->size().width() );
  label.setMargin( 3 );
  return label.sizeHint();
}
