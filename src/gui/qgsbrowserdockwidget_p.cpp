/***************************************************************************
    qgsbrowserdockwidget_p.cpp

    Private classes for QgsBrowserDockWidget

    ---------------------
    begin                : May 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    real work done by    : (C) 2011 by Martin Dobias
    email                : a dot pasotti at itopen dot it
    ---------------------
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsbrowserdockwidget_p.h"

#include <memory>

#include <QAbstractTextDocumentLayout>
#include <QHeaderView>
#include <QTreeView>
#include <QMenu>
#include <QToolButton>
#include <QFileDialog>
#include <QPlainTextDocumentLayout>
#include <QSortFilterProxyModel>
#include <QDesktopServices>
#include <QDragEnterEvent>

#include "qgsbrowsermodel.h"
#include "qgsbrowsertreeview.h"
#include "qgslogger.h"
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgssettings.h"
#include "qgsmeshlayer.h"
#include "qgsgui.h"
#include "qgsnative.h"
#include "qgsmaptoolpan.h"
#include "qgsvectorlayercache.h"
#include "qgsvectortilelayer.h"
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include "qgsapplication.h"
#include "qgsdataitemguiproviderregistry.h"
#include "qgsdataitemguiprovider.h"
#include "qgspointcloudlayer.h"
#include "qgslayeritem.h"
#include "qgsdirectoryitem.h"

/// @cond PRIVATE


QgsBrowserPropertiesWrapLabel::QgsBrowserPropertiesWrapLabel( const QString &text, QWidget *parent )
  : QTextEdit( text, parent )
{
  setReadOnly( true );
  setFrameStyle( QFrame::NoFrame );
  setSizePolicy( QSizePolicy::Expanding, QSizePolicy::Minimum );
  QPalette pal = palette();
  pal.setColor( QPalette::Base, Qt::transparent );
  setPalette( pal );
  setLineWrapMode( QTextEdit::WidgetWidth );
  setWordWrapMode( QTextOption::WrapAnywhere );
  connect( document()->documentLayout(), &QAbstractTextDocumentLayout::documentSizeChanged,
           this, &QgsBrowserPropertiesWrapLabel::adjustHeight );
  setMaximumHeight( 20 );
}

void QgsBrowserPropertiesWrapLabel::adjustHeight( QSizeF size )
{
  const int height = static_cast<int>( size.height() ) + 2 * frameWidth();
  setMinimumHeight( height );
  setMaximumHeight( height );
}

QgsBrowserPropertiesWidget::QgsBrowserPropertiesWidget( QWidget *parent )
  : QWidget( parent )
{
}

void QgsBrowserPropertiesWidget::setWidget( QWidget *paramWidget )
{
  QVBoxLayout *layout = new QVBoxLayout( this );
  layout->setContentsMargins( 0, 0, 0, 0 );
  paramWidget->setParent( this );
  layout->addWidget( paramWidget );
}

QgsBrowserPropertiesWidget *QgsBrowserPropertiesWidget::createWidget( QgsDataItem *item, const QgsDataItemGuiContext &context, QWidget *parent )
{
  QgsBrowserPropertiesWidget *propertiesWidget = nullptr;
  // In general, we would like to show all items' paramWidget, but top level items like
  // WMS etc. have currently too large widgets which do not fit well to browser properties widget
  if ( item->type() == Qgis::BrowserItemType::Directory )
  {
    propertiesWidget = new QgsBrowserDirectoryProperties( parent );
    propertiesWidget->setItem( item );
  }
  else if ( item->type() == Qgis::BrowserItemType::Layer
            || item->type() == Qgis::BrowserItemType::Custom
            || item->type() == Qgis::BrowserItemType::Fields
            || item->type() == Qgis::BrowserItemType::Field )
  {
    // try new infrastructure of creation of layer widgets
    QWidget *paramWidget = nullptr;
    const QList< QgsDataItemGuiProvider * > providers = QgsGui::dataItemGuiProviderRegistry()->providers();
    for ( QgsDataItemGuiProvider *provider : providers )
    {
      paramWidget = provider->createParamWidget( item, context );
      if ( paramWidget )
        break;
    }
    if ( !paramWidget )
    {
      // try old infrastructure
      Q_NOWARN_DEPRECATED_PUSH
      paramWidget = item->paramWidget();
      Q_NOWARN_DEPRECATED_POP
    }

    // prefer item's widget over standard layer widget
    if ( paramWidget )
    {
      propertiesWidget = new QgsBrowserPropertiesWidget( parent );
      propertiesWidget->setWidget( paramWidget );
    }
    else if ( item->type() == Qgis::BrowserItemType::Layer )
    {
      propertiesWidget = new QgsBrowserLayerProperties( parent );
      propertiesWidget->setItem( item );
    }
  }
  return propertiesWidget;
}

QgsBrowserLayerProperties::QgsBrowserLayerProperties( QWidget *parent )
  : QgsBrowserPropertiesWidget( parent )
{
  setupUi( this );

  // we don't want links to open in the little widget, open them externally instead
  mMetadataTextBrowser->setOpenLinks( false );
  connect( mMetadataTextBrowser, &QTextBrowser::anchorClicked, this, &QgsBrowserLayerProperties::urlClicked );

  mMapCanvas->setProperty( "browser_canvas", true );
  mMapCanvas->setLayers( QList< QgsMapLayer * >() );
  mMapCanvas->setMapTool( new QgsMapToolPan( mMapCanvas ) );
  mMapCanvas->freeze( true );

  connect( mTabWidget, &QTabWidget::currentChanged, this, [ = ]
  {
    if ( mTabWidget->currentWidget() == mPreviewTab && mMapCanvas->isFrozen() )
    {
      mMapCanvas->freeze( false );
      mMapCanvas->refresh();
    }
    else if ( mTabWidget->currentWidget() == mAttributesTab )
    {
      if ( ! mAttributeTableFilterModel )
        loadAttributeTable();
    }
  } );
}

void QgsBrowserLayerProperties::setItem( QgsDataItem *item )
{
  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
  if ( !layerItem )
    return;

  mNoticeLabel->clear();

  const QgsMapLayerType type = layerItem->mapLayerType();
  QString layerMetadata = tr( "Error" );

  mLayer.reset();

  // find root item
  // we need to create a temporary layer to get metadata
  // we could use a provider but the metadata is not as complete and "pretty"  and this is easier
  QgsDebugMsgLevel( QStringLiteral( "creating temporary layer using path %1" ).arg( layerItem->path() ), 2 );
  switch ( type )
  {
    case QgsMapLayerType::RasterLayer:
    {
      QgsDebugMsgLevel( QStringLiteral( "creating raster layer" ), 2 );
      // should copy code from addLayer() to split uri ?
      QgsRasterLayer::LayerOptions options;
      options.skipCrsValidation = true;
      mLayer = std::make_unique< QgsRasterLayer >( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options );
      break;
    }

    case QgsMapLayerType::MeshLayer:
    {
      QgsDebugMsgLevel( QStringLiteral( "creating mesh layer" ), 2 );
      QgsMeshLayer::LayerOptions options { QgsProject::instance()->transformContext() };
      options.skipCrsValidation = true;
      mLayer = std::make_unique < QgsMeshLayer >( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options );
      break;
    }

    case QgsMapLayerType::VectorLayer:
    {
      QgsDebugMsgLevel( QStringLiteral( "creating vector layer" ), 2 );
      QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
      options.skipCrsValidation = true;
      mLayer = std::make_unique < QgsVectorLayer>( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options );
      break;
    }

    case QgsMapLayerType::VectorTileLayer:
    {
      QgsDebugMsgLevel( QStringLiteral( "creating vector tile layer" ), 2 );
      mLayer = std::make_unique< QgsVectorTileLayer >( layerItem->uri(), layerItem->name() );
      break;
    }

    case QgsMapLayerType::PointCloudLayer:
    {
      QgsDebugMsgLevel( QStringLiteral( "creating point cloud layer" ), 2 );
      QgsPointCloudLayer::LayerOptions options { QgsProject::instance()->transformContext() };
      options.skipCrsValidation = true;
      mLayer = std::make_unique< QgsPointCloudLayer >( layerItem->uri(), layerItem->name(), layerItem->providerKey(), options );
      break;
    }

    case QgsMapLayerType::PluginLayer:
    case QgsMapLayerType::AnnotationLayer:
    case QgsMapLayerType::GroupLayer:
    {
      // TODO: support display of properties for plugin layers
      return;
    }
  }

  mAttributeTable->setModel( nullptr );
  if ( mAttributeTableFilterModel )
  {
    // Cleanup
    mAttributeTableFilterModel->deleteLater();
    mAttributeTableFilterModel = nullptr;
  }
  if ( mLayer && mLayer->isValid() )
  {
    bool ok = false;
    mLayer->loadDefaultMetadata( ok );
    layerMetadata = mLayer->htmlMetadata();

    mMapCanvas->setDestinationCrs( mLayer->crs() );
    mMapCanvas->setLayers( QList< QgsMapLayer * >() << mLayer.get() );
    mMapCanvas->zoomToFullExtent();

    if ( mAttributesTab && mLayer->type() != QgsMapLayerType::VectorLayer )
    {
      mTabWidget->removeTab( mTabWidget->indexOf( mAttributesTab ) );
      mAttributesTab = nullptr;
    }
  }

  const QString myStyle = QgsApplication::reportStyleSheet();
  mMetadataTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mMetadataTextBrowser->setHtml( layerMetadata );

  if ( mNoticeLabel->text().isEmpty() )
  {
    mNoticeLabel->hide();
  }
}

void QgsBrowserLayerProperties::setCondensedMode( bool )
{

}

void QgsBrowserLayerProperties::urlClicked( const QUrl &url )
{
  const QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsBrowserLayerProperties::loadAttributeTable()
{
  if ( !mLayer || !mLayer->isValid() || mLayer->type() != QgsMapLayerType::VectorLayer )
    return;

  // Initialize the cache
  QgsVectorLayerCache *layerCache = new QgsVectorLayerCache( qobject_cast< QgsVectorLayer * >( mLayer.get() ), 1000, this );
  layerCache->setCacheGeometry( false );
  QgsAttributeTableModel *tableModel = new QgsAttributeTableModel( layerCache, this );
  mAttributeTableFilterModel = new QgsAttributeTableFilterModel( nullptr, tableModel, this );
  tableModel->setRequest( QgsFeatureRequest().setFlags( QgsFeatureRequest::NoGeometry ).setLimit( 100 ) );
  layerCache->setParent( tableModel );
  tableModel->setParent( mAttributeTableFilterModel );

  mAttributeTable->setModel( mAttributeTableFilterModel );
  tableModel->loadLayer();
  QFont font = mAttributeTable->font();
  int fontSize = font.pointSize();
#ifdef Q_OS_WIN
  fontSize = std::max( fontSize - 1, 8 ); // bit less on windows, due to poor rendering of small point sizes
#else
  fontSize = std::max( fontSize - 2, 6 );
#endif
  font.setPointSize( fontSize );
  mAttributeTable->setFont( font );

  // we can safely do this expensive operation here (unlike in the main attribute table), because at most we have only 100 rows...
  mAttributeTable->resizeColumnsToContents();
  mAttributeTable->resizeRowsToContents();
  mAttributeTable->verticalHeader()->setVisible( false ); // maximize valuable table space
  mAttributeTable->setAlternatingRowColors( true );
}

QgsBrowserDirectoryProperties::QgsBrowserDirectoryProperties( QWidget *parent )
  : QgsBrowserPropertiesWidget( parent )

{
  setupUi( this );

  mPathLabel = new QgsBrowserPropertiesWrapLabel( QString(), mHeaderWidget );
  mHeaderGridLayout->addItem( new QWidgetItem( mPathLabel ), 0, 1 );
}

void QgsBrowserDirectoryProperties::setItem( QgsDataItem *item )
{
  QgsDirectoryItem *directoryItem = qobject_cast<QgsDirectoryItem *>( item );
  if ( !item )
    return;

  mPathLabel->setText( QDir::toNativeSeparators( directoryItem->dirPath() ) );
  mDirectoryWidget = new QgsDirectoryParamWidget( directoryItem->dirPath(), this );
  mLayout->addWidget( mDirectoryWidget );
}

QgsBrowserPropertiesDialog::QgsBrowserPropertiesDialog( const QString &settingsSection, QWidget *parent )
  : QDialog( parent )
  , mSettingsSection( settingsSection )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );
}

void QgsBrowserPropertiesDialog::setItem( QgsDataItem *item, const QgsDataItemGuiContext &context )
{
  if ( !item )
    return;

  mPropertiesWidget = QgsBrowserPropertiesWidget::createWidget( item, context, this );
  mLayout->addWidget( mPropertiesWidget );
  setWindowTitle( item->type() == Qgis::BrowserItemType::Layer ? tr( "Layer Properties" ) : tr( "Directory Properties" ) );
}


//
// QgsDockBrowserTreeView
//

QgsDockBrowserTreeView::QgsDockBrowserTreeView( QWidget *parent ) : QgsBrowserTreeView( parent )
{
  setDragDropMode( QTreeView::DragDrop ); // sets also acceptDrops + dragEnabled
  setSelectionMode( QAbstractItemView::ExtendedSelection );
  setContextMenuPolicy( Qt::CustomContextMenu );
  setHeaderHidden( true );
  setDropIndicatorShown( true );

}

void QgsDockBrowserTreeView::setAction( QDropEvent *e )
{
  // if this mime data come from layer tree, the proposed action will be MoveAction
  // but for browser we really need CopyAction
  if ( e->mimeData()->hasFormat( QStringLiteral( "application/qgis.layertreemodeldata" ) ) &&
       e->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    e->setDropAction( Qt::CopyAction );
  }
}

void QgsDockBrowserTreeView::dragEnterEvent( QDragEnterEvent *e )
{
  setAction( e );

  // accept drag enter so that our widget will not get ignored
  // and drag events will not get passed to QgisApp
  e->accept();
}

void QgsDockBrowserTreeView::dragMoveEvent( QDragMoveEvent *e )
{
  // do not accept drops above/below items
  /*if ( dropIndicatorPosition() != QAbstractItemView::OnItem )
      {
        QgsDebugMsg("drag not on item");
        e->ignore();
        return;
      }*/

  setAction( e );
  QTreeView::dragMoveEvent( e );
  // reset action because QTreeView::dragMoveEvent() accepts proposed action
  setAction( e );

  if ( !e->mimeData()->hasFormat( QStringLiteral( "application/x-vnd.qgis.qgis.uri" ) ) )
  {
    e->ignore();
    return;
  }
}

void QgsDockBrowserTreeView::dropEvent( QDropEvent *e )
{
  setAction( e );
  QTreeView::dropEvent( e );
  // reset action because QTreeView::dropEvent() accepts proposed action
  setAction( e );
}


///@endcond
