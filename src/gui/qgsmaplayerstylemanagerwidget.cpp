/***************************************************************************
    qgsmaplayerstylemanagerwidget.cpp
    ---------------------
    begin                : June 2016
    copyright            : (C) 2016 by Nathan Woodrow
    email                : woodrow dot nathan at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsmaplayerstylemanagerwidget.h"

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayer.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsmeshlayerproperties.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerproperties.h"
#include "qgsrasterlayerproperties.h"
#include "qgstiledscenelayer.h"
#include "qgstiledscenelayerproperties.h"
#include "qgsvectorlayer.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilelayerproperties.h"

#include <QAction>
#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QString>
#include <QToolBar>
#include <QVBoxLayout>

#include "moc_qgsmaplayerstylemanagerwidget.cpp"

using namespace Qt::StringLiterals;

//
// QgsMapLayerStyleModel
//
QgsMapLayerStyleModel::QgsMapLayerStyleModel( QgsMapLayerStyleManager *manager, QObject *parent )
  : QAbstractListModel( parent )
  , mManager( manager )
{
  if ( mManager )
  {
    connect( mManager, &QgsMapLayerStyleManager::styleAdded, this, &QgsMapLayerStyleModel::styleAdded );
    connect( mManager, &QgsMapLayerStyleManager::styleRemoved, this, &QgsMapLayerStyleModel::styleRemoved );
    connect( mManager, &QgsMapLayerStyleManager::styleRenamed, this, &QgsMapLayerStyleModel::styleRenamed );

    mStyleNames = mManager->styles();
  }
}

int QgsMapLayerStyleModel::rowCount( const QModelIndex &parent ) const
{
  if ( parent.isValid() )
    return 0;

  return static_cast< int >( mStyleNames.size() );
}

QVariant QgsMapLayerStyleModel::data( const QModelIndex &index, int role ) const
{
  if ( index.row() < 0 || index.row() >= rowCount( QModelIndex() ) )
    return QVariant();

  switch ( role )
  {
    case Qt::DisplayRole:
    case Qt::ToolTipRole:
    case Qt::EditRole:
      return mStyleNames.at( index.row() );

    default:
      break;
  }
  return QVariant();
}

bool QgsMapLayerStyleModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !mManager || !index.isValid() || role != Qt::EditRole )
  {
    return false;
  }
  if ( index.row() >= mStyleNames.size() )
  {
    return false;
  }

  if ( value.toString().isEmpty() )
    return false;

  //has name changed?
  const bool changed = mStyleNames.at( index.row() ) != value.toString();
  if ( !changed )
    return true;

  //check if name already exists
  if ( mStyleNames.contains( value.toString() ) )
    return false;

  mManager->renameStyle( mStyleNames[index.row()], value.toString() );
  mStyleNames[index.row()] = value.toString();
  return true;
}

Qt::ItemFlags QgsMapLayerStyleModel::flags( const QModelIndex &index ) const
{
  Qt::ItemFlags flags = QAbstractListModel::flags( index );
  if ( index.isValid() )
  {
    return flags | Qt::ItemIsEditable;
  }
  else
  {
    return flags;
  }
}

QModelIndex QgsMapLayerStyleModel::indexForName( const QString &name ) const
{
  const int row = static_cast< int >( mStyleNames.indexOf( name ) );
  if ( row >= 0 )
  {
    return index( row, 0, QModelIndex() );
  }
  return QModelIndex();
}

void QgsMapLayerStyleModel::styleAdded( const QString &name )
{
  beginInsertRows( QModelIndex(), static_cast< int >( mStyleNames.size() ), static_cast< int >( mStyleNames.size() ) );
  mStyleNames << name;
  endInsertRows();
}

void QgsMapLayerStyleModel::styleRemoved( const QString &name )
{
  const int row = static_cast< int >( mStyleNames.indexOf( name ) );
  if ( row >= 0 )
  {
    beginRemoveRows( QModelIndex(), row, row );
    mStyleNames.remove( row );
    endRemoveRows();
  }
}

void QgsMapLayerStyleModel::styleRenamed( const QString &oldname, const QString &newname )
{
  const QModelIndex styleIndex = indexForName( oldname );
  mStyleNames[styleIndex.row()] = newname;
  emit dataChanged( styleIndex, styleIndex );
}

//
// QgsMapLayerStyleManagerWidget
//

QgsMapLayerStyleManagerWidget::QgsMapLayerStyleManagerWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  mModel = new QgsMapLayerStyleModel( layer->styleManager(), this );
  mStyleList = new QListView( this );
  mStyleList->setModel( mModel );
  mStyleList->setViewMode( QListView::ListMode );
  mStyleList->setResizeMode( QListView::Adjust );

  QToolBar *toolbar = new QToolBar( this );
  QAction *addAction = toolbar->addAction( tr( "Add" ) );
  addAction->setIcon( QgsApplication::getThemeIcon( u"symbologyAdd.svg"_s ) );
  connect( addAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::addStyle );
  QAction *removeAction = toolbar->addAction( tr( "Remove Current" ) );
  removeAction->setIcon( QgsApplication::getThemeIcon( u"symbologyRemove.svg"_s ) );
  connect( removeAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::removeStyle );
  QAction *loadFromFileAction = toolbar->addAction( tr( "Load Style" ) );
  loadFromFileAction->setIcon( QgsApplication::getThemeIcon( u"/mActionFileOpen.svg"_s ) );
  connect( loadFromFileAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadStyle );
  QAction *saveAction = toolbar->addAction( tr( "Save Style" ) );
  saveAction->setIcon( QgsApplication::getThemeIcon( u"mActionFileSave.svg"_s ) );
  connect( saveAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::saveStyle );
  QAction *saveAsDefaultAction = toolbar->addAction( tr( "Save as Default" ) );
  connect( saveAsDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::saveAsDefault );
  QAction *loadDefaultAction = toolbar->addAction( tr( "Restore Default" ) );
  connect( loadDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadDefault );

  connect( mStyleList->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMapLayerStyleManagerWidget::selectionChanged );

  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->addWidget( toolbar );
  layout()->addWidget( mStyleList );

  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMapLayerStyleManagerWidget::currentStyleChanged );

  const QString active = mLayer->styleManager()->currentStyle();
  currentStyleChanged( active );
}

void QgsMapLayerStyleManagerWidget::selectionChanged( const QItemSelection &selected, const QItemSelection & )
{
  if ( !mLayer || selected.empty() )
    return;

  const QModelIndex index = selected.indexes().first();
  if ( !index.isValid() )
    return;

  const QString name = index.data().toString();
  mLayer->styleManager()->setCurrentStyle( name );
}

void QgsMapLayerStyleManagerWidget::currentStyleChanged( const QString &name )
{
  const QModelIndex listIndex = mModel->indexForName( name );
  if ( listIndex.isValid() )
  {
    mStyleList->selectionModel()->select( listIndex, QItemSelectionModel::SelectionFlag::ClearAndSelect );
  }
}

void QgsMapLayerStyleManagerWidget::addStyle()
{
  bool ok;
  const QString text = QInputDialog::getText( nullptr, tr( "New Style" ), tr( "Style name:" ), QLineEdit::Normal, u"new style"_s, &ok );
  if ( !ok || text.isEmpty() )
    return;

  const bool res = mLayer->styleManager()->addStyleFromLayer( text );
  if ( res ) // make it active!
  {
    mLayer->styleManager()->setCurrentStyle( text );
  }
  else
  {
    QgsDebugError( "Failed to add style: " + text );
  }
}

void QgsMapLayerStyleManagerWidget::removeStyle()
{
  const QString current = mLayer->styleManager()->currentStyle();
  const bool res = mLayer->styleManager()->removeStyle( current );
  if ( !res )
    QgsDebugError( u"Failed to remove current style"_s );
}

void QgsMapLayerStyleManagerWidget::saveAsDefault()
{
  if ( !mLayer )
    return;

  switch ( mLayer->type() )
  {
    case Qgis::LayerType::Vector:
      QgsVectorLayerProperties( mMapCanvas, mMapLayerConfigWidgetContext.messageBar(), qobject_cast<QgsVectorLayer *>( mLayer ) ).saveDefaultStyle();
      break;

    case Qgis::LayerType::Raster:
      QgsRasterLayerProperties( mLayer, mMapCanvas ).saveStyleAsDefault();
      break;

    case Qgis::LayerType::Mesh:
      QgsMeshLayerProperties( mLayer, mMapCanvas ).saveStyleAsDefault();
      break;

    case Qgis::LayerType::VectorTile:
      QgsVectorTileLayerProperties( qobject_cast<QgsVectorTileLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleAsDefault();
      break;

    case Qgis::LayerType::PointCloud:
      QgsPointCloudLayerProperties( qobject_cast<QgsPointCloudLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleAsDefault();
      break;

    case Qgis::LayerType::TiledScene:
      QgsTiledSceneLayerProperties( qobject_cast<QgsTiledSceneLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleAsDefault();
      break;

    // Not available for these
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
}

void QgsMapLayerStyleManagerWidget::loadDefault()
{
  if ( !mLayer )
    return;

  switch ( mLayer->type() )
  {
    case Qgis::LayerType::Vector:
      QgsVectorLayerProperties( mMapCanvas, mMapLayerConfigWidgetContext.messageBar(), qobject_cast<QgsVectorLayer *>( mLayer ) ).loadDefaultStyle();
      break;

    case Qgis::LayerType::Raster:
      QgsRasterLayerProperties( mLayer, mMapCanvas ).loadDefaultStyle();
      break;

    case Qgis::LayerType::Mesh:
      QgsMeshLayerProperties( mLayer, mMapCanvas ).loadDefaultStyle();
      break;

    case Qgis::LayerType::VectorTile:
      QgsVectorTileLayerProperties( qobject_cast<QgsVectorTileLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadDefaultStyle();
      break;

    case Qgis::LayerType::PointCloud:
      QgsPointCloudLayerProperties( qobject_cast<QgsPointCloudLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadDefaultStyle();
      break;

    case Qgis::LayerType::TiledScene:
      QgsTiledSceneLayerProperties( qobject_cast<QgsTiledSceneLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadDefaultStyle();
      break;

    // Not available for these
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
}

void QgsMapLayerStyleManagerWidget::saveStyle()
{
  if ( !mLayer )
    return;

  switch ( mLayer->type() )
  {
    case Qgis::LayerType::Vector:
      QgsVectorLayerProperties( mMapCanvas, mMapLayerConfigWidgetContext.messageBar(), qobject_cast<QgsVectorLayer *>( mLayer ) ).saveStyleAs();
      break;

    case Qgis::LayerType::Raster:
      QgsRasterLayerProperties( mLayer, mMapCanvas ).saveStyleAs();
      break;

    case Qgis::LayerType::Mesh:
      QgsMeshLayerProperties( mLayer, mMapCanvas ).saveStyleToFile();
      break;

    case Qgis::LayerType::VectorTile:
      QgsVectorTileLayerProperties( qobject_cast<QgsVectorTileLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleToFile();
      break;

    case Qgis::LayerType::PointCloud:
      QgsPointCloudLayerProperties( qobject_cast<QgsPointCloudLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleToFile();
      break;

    case Qgis::LayerType::TiledScene:
      QgsTiledSceneLayerProperties( qobject_cast<QgsTiledSceneLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).saveStyleToFile();
      break;

    // Not available for these
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
}

void QgsMapLayerStyleManagerWidget::loadStyle()
{
  if ( !mLayer )
    return;

  switch ( mLayer->type() )
  {
    case Qgis::LayerType::Vector:
      QgsVectorLayerProperties( mMapCanvas, mMapLayerConfigWidgetContext.messageBar(), qobject_cast<QgsVectorLayer *>( mLayer ) ).loadStyle();
      break;

    case Qgis::LayerType::Raster:
      QgsRasterLayerProperties( mLayer, mMapCanvas ).loadStyleFromFile();
      break;

    case Qgis::LayerType::Mesh:
      QgsMeshLayerProperties( mLayer, mMapCanvas ).loadStyleFromFile();
      break;

    case Qgis::LayerType::VectorTile:
      QgsVectorTileLayerProperties( qobject_cast<QgsVectorTileLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadStyle();
      break;

    case Qgis::LayerType::PointCloud:
      QgsPointCloudLayerProperties( qobject_cast<QgsPointCloudLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadStyleFromFile();
      break;

    case Qgis::LayerType::TiledScene:
      QgsTiledSceneLayerProperties( qobject_cast<QgsTiledSceneLayer *>( mLayer ), mMapCanvas, mMapLayerConfigWidgetContext.messageBar() ).loadStyleFromFile();
      break;

    // Not available for these
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
}
