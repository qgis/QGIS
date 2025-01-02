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
#include <QAction>
#include <QVBoxLayout>
#include <QToolBar>
#include <QInputDialog>
#include <QMessageBox>
#include <QFileDialog>

#include "qgsmaplayerstylemanagerwidget.h"
#include "moc_qgsmaplayerstylemanagerwidget.cpp"
#include "qgslogger.h"
#include "qgsmaplayer.h"
#include "qgsmapcanvas.h"
#include "qgsmaplayerconfigwidget.h"
#include "qgsmaplayerstylemanager.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilelayer.h"
#include "qgsapplication.h"
#include "qgsvectorlayerproperties.h"
#include "qgsvectortilelayerproperties.h"
#include "qgsrasterlayerproperties.h"
#include "qgsmeshlayerproperties.h"

QgsMapLayerStyleManagerWidget::QgsMapLayerStyleManagerWidget( QgsMapLayer *layer, QgsMapCanvas *canvas, QWidget *parent )
  : QgsMapLayerConfigWidget( layer, canvas, parent )
{
  mModel = new QStandardItemModel( this );
  mStyleList = new QListView( this );
  mStyleList->setModel( mModel );
  mStyleList->setViewMode( QListView::ListMode );
  mStyleList->setResizeMode( QListView::Adjust );

  QToolBar *toolbar = new QToolBar( this );
  QAction *addAction = toolbar->addAction( tr( "Add" ) );
  addAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "symbologyAdd.svg" ) ) );
  connect( addAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::addStyle );
  QAction *removeAction = toolbar->addAction( tr( "Remove Current" ) );
  removeAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "symbologyRemove.svg" ) ) );
  connect( removeAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::removeStyle );
  QAction *loadFromFileAction = toolbar->addAction( tr( "Load Style" ) );
  loadFromFileAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionFileOpen.svg" ) ) );
  connect( loadFromFileAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadStyle );
  QAction *saveAction = toolbar->addAction( tr( "Save Style" ) );
  saveAction->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "mActionFileSave.svg" ) ) );
  connect( saveAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::saveStyle );
  QAction *saveAsDefaultAction = toolbar->addAction( tr( "Save as Default" ) );
  connect( saveAsDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::saveAsDefault );
  QAction *loadDefaultAction = toolbar->addAction( tr( "Restore Default" ) );
  connect( loadDefaultAction, &QAction::triggered, this, &QgsMapLayerStyleManagerWidget::loadDefault );

  //broken connect - not sure what the purpose of this was?
  //  connect( canvas, &QgsMapCanvas::mapCanvasRefreshed, this, SLOT( updateCurrent() ) );

  connect( mStyleList, &QAbstractItemView::clicked, this, &QgsMapLayerStyleManagerWidget::styleClicked );

  setLayout( new QVBoxLayout() );
  layout()->setContentsMargins( 0, 0, 0, 0 );
  layout()->addWidget( toolbar );
  layout()->addWidget( mStyleList );

  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::currentStyleChanged, this, &QgsMapLayerStyleManagerWidget::currentStyleChanged );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleAdded, this, &QgsMapLayerStyleManagerWidget::styleAdded );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleRemoved, this, &QgsMapLayerStyleManagerWidget::styleRemoved );
  connect( mLayer->styleManager(), &QgsMapLayerStyleManager::styleRenamed, this, &QgsMapLayerStyleManagerWidget::styleRenamed );

  mModel->clear();

  const QStringList styles = mLayer->styleManager()->styles();
  for ( const QString &styleName : styles )
  {
    QStandardItem *item = new QStandardItem( styleName );
    item->setData( styleName );
    mModel->appendRow( item );
  }

  const QString active = mLayer->styleManager()->currentStyle();
  currentStyleChanged( active );

  connect( mModel, &QStandardItemModel::itemChanged, this, &QgsMapLayerStyleManagerWidget::renameStyle );
}

void QgsMapLayerStyleManagerWidget::styleClicked( const QModelIndex &index )
{
  if ( !mLayer || !index.isValid() )
    return;

  const QString name = index.data().toString();
  mLayer->styleManager()->setCurrentStyle( name );
}

void QgsMapLayerStyleManagerWidget::currentStyleChanged( const QString &name )
{
  const QList<QStandardItem *> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );

  mStyleList->setCurrentIndex( item->index() );
}

void QgsMapLayerStyleManagerWidget::styleAdded( const QString &name )
{
  QgsDebugMsgLevel( QStringLiteral( "Style added" ), 2 );
  QStandardItem *item = new QStandardItem( name );
  item->setData( name );
  mModel->appendRow( item );
}

void QgsMapLayerStyleManagerWidget::styleRemoved( const QString &name )
{
  const QList<QStandardItem *> items = mModel->findItems( name );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );
  mModel->removeRow( item->row() );
}

void QgsMapLayerStyleManagerWidget::styleRenamed( const QString &oldname, const QString &newname )
{
  const QList<QStandardItem *> items = mModel->findItems( oldname );
  if ( items.isEmpty() )
    return;

  QStandardItem *item = items.at( 0 );
  item->setText( newname );
  item->setData( newname );
}

void QgsMapLayerStyleManagerWidget::addStyle()
{
  bool ok;
  const QString text = QInputDialog::getText( nullptr, tr( "New Style" ), tr( "Style name:" ), QLineEdit::Normal, QStringLiteral( "new style" ), &ok );
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
    QgsDebugError( QStringLiteral( "Failed to remove current style" ) );
}

void QgsMapLayerStyleManagerWidget::renameStyle( QStandardItem *item )
{
  const QString oldName = item->data().toString();
  const QString newName = item->text();
  item->setData( newName );
  whileBlocking( this )->mLayer->styleManager()->renameStyle( oldName, newName );
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

    // Not available for these
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::TiledScene:
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

    // Not available for these
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::TiledScene:
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

    // Not available for these
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::TiledScene:
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

    // Not available for these
    case Qgis::LayerType::PointCloud:
    case Qgis::LayerType::Annotation:
    case Qgis::LayerType::TiledScene:
    case Qgis::LayerType::Plugin:
    case Qgis::LayerType::Group:
      break;
  }
}
