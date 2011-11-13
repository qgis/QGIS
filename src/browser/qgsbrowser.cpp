/***************************************************************************
                             qgsbrowser.cpp  -
                             -------------------
    begin                : 2011-04-01
    copyright            : (C) 2011 Radim Blazek
    email                : radim dot blazek at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QSettings>
#include <QMessageBox>
#include <QKeyEvent>
#include <QMetaObject>

#include "qgsapplication.h"
#include "qgsdataitem.h"
#include "qgsbrowser.h"
#include "qgsbrowsermodel.h"
#include "qgsencodingfiledialog.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgsmaplayerregistry.h"
#include "qgsproviderregistry.h"
#include "qgsvectorlayer.h"
#include "qgsrasterlayer.h"
#include "qgsnewvectorlayerdialog.h"


QgsBrowser::QgsBrowser( QWidget *parent, Qt::WFlags flags )
    : QMainWindow( parent, flags )
    , mDirtyMetadata( true )
    , mDirtyPreview( true )
    , mDirtyAttributes( true )
    , mLayer( 0 )
    , mParamWidget( 0 )
{
  setupUi( this );

  // Disable tabs by default
  tabWidget->setTabEnabled( tabWidget->indexOf( paramTab ), false );
  tabWidget->setTabEnabled( tabWidget->indexOf( metaTab ), false );
  tabWidget->setTabEnabled( tabWidget->indexOf( previewTab ), false );
  tabWidget->setTabEnabled( tabWidget->indexOf( attributesTab ), false );

  mModel = new QgsBrowserModel( treeView );
  treeView->setModel( mModel );

  // Last expanded is stored, don't cover whole height with file system
  //treeView->expand( mModel->index(0,0) );

  connect( treeView, SIGNAL( clicked( const QModelIndex& ) ), this, SLOT( itemClicked( const QModelIndex& ) ) );

  treeView->setExpandsOnDoubleClick( false );
  connect( treeView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( itemDoubleClicked( const QModelIndex& ) ) );
  connect( treeView, SIGNAL( expanded( const QModelIndex& ) ), this, SLOT( itemExpanded( const QModelIndex& ) ) );

  connect( tabWidget, SIGNAL( currentChanged( int ) ), this, SLOT( tabChanged() ) );

  connect( mActionNewVectorLayer, SIGNAL( triggered() ), this, SLOT( newVectorLayer() ) );

  connect( stopRenderingButton, SIGNAL( clicked() ), this, SLOT( stopRendering() ) );

  mapCanvas->setCanvasColor( Qt::white );

  QSettings settings;
  QString lastPath =  settings.value( "/Browser/lastExpanded" ).toString();
  QgsDebugMsg( "lastPath = " + lastPath );
  if ( !lastPath.isEmpty() )
  {
    expandPath( lastPath );
  }
}

QgsBrowser::~QgsBrowser()
{

}

void QgsBrowser::expandPath( QString path )
{
  QModelIndex idx = mModel->findPath( path );
  if ( idx.isValid() )
  {
    treeView->expand( idx );
    treeView->scrollTo( idx, QAbstractItemView::PositionAtTop );
  }
}

void QgsBrowser::itemClicked( const QModelIndex& index )
{
  mIndex = index;

  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  // Disable preview, attributes tab

  bool paramEnable = false;
  bool metaEnable = false;
  bool previewEnable = false;
  bool attributesEnable = false;

  // mark all tabs as dirty
  mDirtyMetadata = true;
  mDirtyPreview = true;
  mDirtyAttributes = true;

  // clear the previous stuff
  attributeTable->setLayer( NULL );
  QList<QgsMapCanvasLayer> nolayers;
  mapCanvas->setLayerSet( nolayers );
  metaTextBrowser->clear();
  if ( mParamWidget )
  {
    paramLayout->removeWidget( mParamWidget );
    mParamWidget->hide();
    delete mParamWidget;
    mParamWidget = 0;
  }

  // QgsMapLayerRegistry deletes the previous layer(s) for us
  // TODO: in future we could cache the layers in the registry
  QgsMapLayerRegistry::instance()->removeAllMapLayers();
  mLayer = 0;

  // this should probably go to the model and only emit signal when a layer is clicked
  mParamWidget = item->paramWidget();
  if ( mParamWidget )
  {
    paramLayout->addWidget( mParamWidget );
    mParamWidget->show();
    paramEnable = true;
  }

  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( mModel->dataItem( index ) );
  if ( layerItem )
  {
    bool res = layerClicked( layerItem );

    if ( res )
    {
      metaEnable = true;
      previewEnable = true;
      if ( mLayer->type() == QgsMapLayer::VectorLayer )
      {
        attributesEnable = true;
      }
    }
  }
  else
  {
    mActionSetProjection->setEnabled( false );
  }

  // force update of the current tab
  updateCurrentTab();

  int selected = -1;
  if ( mLastTab.contains( item->metaObject()->className() ) )
  {
    selected = mLastTab[ item->metaObject()->className()];
  }

  // Enabling tabs call tabChanged !
  tabWidget->setTabEnabled( tabWidget->indexOf( paramTab ), paramEnable );
  tabWidget->setTabEnabled( tabWidget->indexOf( metaTab ), metaEnable );
  tabWidget->setTabEnabled( tabWidget->indexOf( previewTab ), previewEnable );
  tabWidget->setTabEnabled( tabWidget->indexOf( attributesTab ), attributesEnable );

  // select tab according last selection for this data item
  if ( selected >= 0 )
  {
    QgsDebugMsg( QString( "set tab %1 %2" ).arg( item->metaObject()->className() ).arg( selected ) );
    tabWidget->setCurrentIndex( selected );
  }

  QgsDebugMsg( QString( "clicked: %1 %2 %3" ).arg( index.row() ).arg( index.column() ).arg( item->name() ) );
}

bool QgsBrowser::layerClicked( QgsLayerItem *item )
{
  if ( !item )
    return false;

  mActionSetProjection->setEnabled( item->capabilities() & QgsLayerItem::SetCrs );

  QString uri = item->uri();
  if ( !uri.isEmpty() )
  {
    QgsMapLayer::LayerType type = item->mapLayerType();
    QString providerKey = item->providerKey();

    QgsDebugMsg( providerKey + " : " + uri );
    if ( type == QgsMapLayer::VectorLayer )
    {
      mLayer = new QgsVectorLayer( uri, QString(), providerKey );
    }
    if ( type == QgsMapLayer::RasterLayer )
    {
      // This should go to WMS provider
      QStringList URIParts = uri.split( "|" );
      QString rasterLayerPath = URIParts.at( 0 );
      QStringList layers;
      QStringList styles;
      QString format;
      QString crs;
      for ( int i = 1 ; i < URIParts.size(); i++ )
      {
        QString part = URIParts.at( i );
        int pos = part.indexOf( "=" );
        QString field = part.left( pos );
        QString value = part.mid( pos + 1 );

        if ( field == "layers" )
          layers = value.split( "," );
        if ( field == "styles" )
          styles = value.split( "," );
        if ( field == "format" )
          format = value;
        if ( field == "crs" )
          crs = value;
      }
      QgsDebugMsg( "rasterLayerPath = " + rasterLayerPath );
      QgsDebugMsg( "layers = " + layers.join( " " ) );

      mLayer = new QgsRasterLayer( 0, rasterLayerPath, "", providerKey, layers, styles, format, crs );
    }
  }

  if ( !mLayer || !mLayer->isValid() )
  {
    qDebug( "No layer" );
    return false;
  }

  QgsDebugMsg( "Layer created" );

  QgsMapLayerRegistry::instance()->addMapLayer( mLayer );

  return true;
}


void QgsBrowser::itemDoubleClicked( const QModelIndex& index )
{
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

  // Currently doing nothing
  QgsDebugMsg( QString( "%1 %2 %3" ).arg( index.row() ).arg( index.column() ).arg( item->name() ) );
}

void QgsBrowser::itemExpanded( const QModelIndex& index )
{
  QSettings settings;
  QgsDataItem *item = mModel->dataItem( index );
  if ( !item )
    return;

#if 0
  if ( item->mType == QgsDataItem::Directory || item->mType == QgsDataItem::Collection )
  {
    QgsDirectoryItem *i = qobject_cast<QgsDirectoryItem*>( item );
    settings.setValue( "/Browser/lastExpandedDir", i->mPath );
  }
#endif

  // TODO: save separately each type (FS, WMS)
  settings.setValue( "/Browser/lastExpanded", item->path() );
  QgsDebugMsg( "last expanded: " + item->path() );
}

void QgsBrowser::newVectorLayer()
{
  // Set file dialog to last selected dir
  QSettings settings;
  QString lastPath =  settings.value( "/Browser/lastExpanded" ).toString();
  if ( !lastPath.isEmpty() )
  {
    settings.setValue( "/UI/lastVectorFileFilterDir", lastPath );
  }

  QString fileName = QgsNewVectorLayerDialog::runAndCreateLayer( this );

  if ( !fileName.isEmpty() )
  {
    QgsDebugMsg( "New vector layer: " + fileName );
    expandPath( fileName );
    QFileInfo fileInfo( fileName );
    QString dirPath = fileInfo.absoluteDir().path();
    mModel->refresh( dirPath );
  }
}

void QgsBrowser::on_mActionWmsConnections_triggered()
{
  QDialog *wmss = dynamic_cast<QDialog*>( QgsProviderRegistry::instance()->selectWidget( QString( "wms" ), this ) );
  if ( !wmss )
  {
    QMessageBox::warning( this, tr( "WMS" ), tr( "Cannot get WMS select dialog from provider." ) );
    return;
  }
  wmss->exec();
  delete wmss;
  // TODO: refresh only WMS
  refresh();
}

void QgsBrowser::on_mActionSetProjection_triggered()
{
  if ( !mLayer )
    return;

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setSelectedCrsId( mLayer->crs().srsid() );
  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs( mySelector->selectedCrsId(), QgsCoordinateReferenceSystem::InternalCrsId );

    // TODO: open data source in write mode set crs and save
#if 0
    mLayer->setCrs( srs );
    // Is this safe?
    // selectedIndexes() is protected
#endif

    QgsDataItem *item = mModel->dataItem( mIndex );
    QgsLayerItem *layerItem = qobject_cast<QgsLayerItem*>( item );
    if ( layerItem )
    {
      if ( !layerItem->setCrs( srs ) )
      {
        QMessageBox::critical( this, tr( "CRS" ), tr( "Cannot set layer CRS" ) );
      }
    }
    QgsDebugMsg( srs.authid() + " - " + srs.description() );
  }
  else
  {
    QApplication::restoreOverrideCursor();
  }

  delete mySelector;
}

void QgsBrowser::saveWindowState()
{
  QSettings settings;
  settings.setValue( "/Windows/Browser/state", saveState() );
  settings.setValue( "/Windows/Browser/geometry", saveGeometry() );
  settings.setValue( "/Windows/Browser/sizes/0", splitter->sizes()[0] );
  settings.setValue( "/Windows/Browser/sizes/1", splitter->sizes()[1] );
}

void QgsBrowser::restoreWindowState()
{
  QSettings settings;
  if ( !restoreState( settings.value( "/Windows/Browser/state" ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI state failed" );
  }
  if ( !restoreGeometry( settings.value( "/Windows/Browser/geometry" ).toByteArray() ) )
  {
    QgsDebugMsg( "restore of UI geometry failed" );
  }
  int size0 = settings.value( "/Windows/Browser/sizes/0" ).toInt();
  if ( size0 > 0 )
  {

    QList<int> sizes;
    sizes << size0;
    sizes << settings.value( "/Windows/Browser/sizes/1" ).toInt();
    QgsDebugMsg( QString( "set splitter sizes to %1 %2" ).arg( sizes[0] ).arg( sizes[1] ) );
    splitter->setSizes( sizes );
  }
}

void QgsBrowser::keyPressEvent( QKeyEvent * e )
{
  QgsDebugMsg( "Entered" );
  if ( e->key() == Qt::Key_Escape )
  {
    stopRendering();
  }
  else
  {
    e->ignore();
  }
}

void QgsBrowser::stopRendering()
{
  // you might have seen this already in QgisApp
  QgsDebugMsg( "Entered" );
  if ( mapCanvas )
  {
    QgsMapRenderer* mypMapRenderer = mapCanvas->mapRenderer();
    if ( mypMapRenderer )
    {
      QgsRenderContext* mypRenderContext = mypMapRenderer->rendererContext();
      if ( mypRenderContext )
      {
        mypRenderContext->setRenderingStopped( true );
      }
    }
  }
}

QgsBrowser::Tab QgsBrowser::activeTab()
{
  QWidget* curr = tabWidget->currentWidget();
  if ( curr == metaTab )
    return Metadata;
  if ( curr == previewTab )
    return Preview;
  return Attributes;
}

void QgsBrowser::updateCurrentTab()
{
  // update contents of the current tab

  Tab current = activeTab();

  if ( current == Metadata && mDirtyMetadata )
  {
    if ( mLayer && mLayer->isValid() )
    {
      // Set meta
      QString myStyle = QgsApplication::reportStyleSheet();

      metaTextBrowser->document()->setDefaultStyleSheet( myStyle );
      metaTextBrowser->setHtml( mLayer->metadata() );
    }
    else
    {
      metaTextBrowser->setHtml( QString() );
    }
    mDirtyMetadata = false;
  }

  if ( current == Preview && mDirtyPreview )
  {
    if ( mLayer && mLayer->isValid() )
    {
      // Create preview: add to map canvas
      QList<QgsMapCanvasLayer> layers;
      layers << QgsMapCanvasLayer( mLayer );
      mapCanvas->setLayerSet( layers );
      QgsRectangle fullExtent = mLayer->extent();
      fullExtent.scale( 1.05 ); // add some border
      mapCanvas->setExtent( fullExtent );
      mapCanvas->refresh();

      QgsRasterLayer *rlayer = qobject_cast< QgsRasterLayer * >( mLayer );
      if ( rlayer )
      {
        connect( rlayer->dataProvider(), SIGNAL( dataChanged() ), rlayer, SLOT( clearCacheImage() ) );
        connect( rlayer->dataProvider(), SIGNAL( dataChanged() ), mapCanvas, SLOT( refresh() ) );
      }
    }
    mDirtyPreview = false;
  }

  if ( current == Attributes && mDirtyAttributes )
  {
    if ( mLayer  && mLayer->isValid() && mLayer->type() == QgsMapLayer::VectorLayer )
    {
      QgsVectorLayer* vlayer = qobject_cast<QgsVectorLayer*>( mLayer );
      QApplication::setOverrideCursor( Qt::WaitCursor );
      attributeTable->setLayer( vlayer );
      QApplication::restoreOverrideCursor();
    }
    else
    {
      attributeTable->setLayer( NULL );
    }
    mDirtyAttributes = false;
  }
}

void QgsBrowser::tabChanged()
{
  updateCurrentTab();
  // Store last selected tab for selected data item
  if ( mIndex.isValid() )
  {
    QgsDataItem *item = mModel->dataItem( mIndex );
    if ( !item )
      return;

    QgsDebugMsg( QString( "save last tab %1 : %2" ).arg( item->metaObject()->className() ).arg( tabWidget->currentIndex() ) );
    mLastTab[ item->metaObject()->className()] = tabWidget->currentIndex();
  }
}

void QgsBrowser::on_mActionRefresh_triggered()
{
  QgsDebugMsg( "Entered" );
  refresh();
}

void QgsBrowser::refresh( const QModelIndex& index )
{
  QgsDebugMsg( "Entered" );
  if ( index.isValid() )
  {
    QgsDataItem *item = mModel->dataItem( index );
    if ( item )
    {
      QgsDebugMsg( "path = " + item->path() );
    }
    else
    {
      QgsDebugMsg( "invalid item" );
    }
  }

  mModel->refresh( index );

  for ( int i = 0 ; i < mModel->rowCount( index ); i++ )
  {
    QModelIndex idx = mModel->index( i, 0, index );
    if ( treeView->isExpanded( idx ) || !mModel->hasChildren( idx ) )
    {
      refresh( idx );
    }
  }
}
