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
#include "qgsattributetablemodel.h"
#include "qgsattributetablefiltermodel.h"
#include <QDesktopServices>

#include <QDragEnterEvent>


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
  int height = size.height() + 2 * frameWidth();
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
  paramWidget->setParent( this );
  layout->addWidget( paramWidget );
}

QgsBrowserPropertiesWidget *QgsBrowserPropertiesWidget::createWidget( QgsDataItem *item, QWidget *parent )
{
  QgsBrowserPropertiesWidget *propertiesWidget = nullptr;
  // In general, we would like to show all items' paramWidget, but top level items like
  // WMS etc. have currently too large widgets which do not fit well to browser properties widget
  if ( item->type() == QgsDataItem::Directory )
  {
    propertiesWidget = new QgsBrowserDirectoryProperties( parent );
    propertiesWidget->setItem( item );
  }
  else if ( item->type() == QgsDataItem::Layer )
  {
    // prefer item's widget over standard layer widget
    QWidget *paramWidget = item->paramWidget();
    if ( paramWidget )
    {
      propertiesWidget = new QgsBrowserPropertiesWidget( parent );
      propertiesWidget->setWidget( paramWidget );
    }
    else
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

class ProjectionSettingRestorer
{
  public:

    ProjectionSettingRestorer()
    {
      QgsSettings settings;
      previousSetting = settings.value( QStringLiteral( "/Projections/defaultBehavior" ) ).toString();
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), QStringLiteral( "useProject" ) );
    }

    ~ProjectionSettingRestorer()
    {
      QgsSettings settings;
      settings.setValue( QStringLiteral( "/Projections/defaultBehavior" ), previousSetting );
    }

    QString previousSetting;
};

void QgsBrowserLayerProperties::setItem( QgsDataItem *item )
{
  QgsLayerItem *layerItem = qobject_cast<QgsLayerItem *>( item );
  if ( !layerItem )
    return;

  mNoticeLabel->clear();

  QgsMapLayer::LayerType type = layerItem->mapLayerType();
  QString layerMetadata = tr( "Error" );
  QgsCoordinateReferenceSystem layerCrs;

  QString defaultProjectionOption = QgsSettings().value( QStringLiteral( "Projections/defaultBehavior" ), "prompt" ).toString();
  // temporarily override /Projections/defaultBehavior to avoid dialog prompt
  // TODO - remove when there is a cleaner way to block the unknown projection dialog!
  ProjectionSettingRestorer restorer;
  ( void )restorer; // no warnings

  mLayer.reset();

  // find root item
  // we need to create a temporary layer to get metadata
  // we could use a provider but the metadata is not as complete and "pretty"  and this is easier
  QgsDebugMsg( QStringLiteral( "creating temporary layer using path %1" ).arg( layerItem->path() ) );
  if ( type == QgsMapLayer::RasterLayer )
  {
    QgsDebugMsg( QStringLiteral( "creating raster layer" ) );
    // should copy code from addLayer() to split uri ?
    mLayer = qgis::make_unique< QgsRasterLayer >( layerItem->uri(), layerItem->name(), layerItem->providerKey() );
  }
  else if ( type == QgsMapLayer::MeshLayer )
  {
    QgsDebugMsg( QStringLiteral( "creating mesh layer" ) );
    mLayer = qgis::make_unique < QgsMeshLayer >( layerItem->uri(), layerItem->name(), layerItem->providerKey() );
  }
  else if ( type == QgsMapLayer::VectorLayer )
  {
    QgsDebugMsg( QStringLiteral( "creating vector layer" ) );
    mLayer = qgis::make_unique < QgsVectorLayer>( layerItem->uri(), layerItem->name(), layerItem->providerKey() );
  }
  else if ( type == QgsMapLayer::PluginLayer )
  {
    // TODO: support display of properties for plugin layers
    return;
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
    layerCrs = mLayer->crs();
    layerMetadata = mLayer->htmlMetadata();

    mMapCanvas->setDestinationCrs( mLayer->crs() );
    mMapCanvas->setLayers( QList< QgsMapLayer * >() << mLayer.get() );
    mMapCanvas->zoomToFullExtent();

    if ( mAttributesTab && mLayer->type() != QgsMapLayer::VectorLayer )
    {
      mTabWidget->removeTab( mTabWidget->indexOf( mAttributesTab ) );
      mAttributesTab = nullptr;
    }
  }

  QString myStyle = QgsApplication::reportStyleSheet();
  mMetadataTextBrowser->document()->setDefaultStyleSheet( myStyle );
  mMetadataTextBrowser->setHtml( layerMetadata );

// report if layer was set to to project crs without prompt (may give a false positive)
  if ( defaultProjectionOption == QLatin1String( "prompt" ) )
  {
    QgsCoordinateReferenceSystem defaultCrs =
      QgsProject::instance()->crs();
    if ( layerCrs == defaultCrs )
      mNoticeLabel->setText( "NOTICE: Layer CRS set from project (" + defaultCrs.authid() + ')' );
  }

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
  QFileInfo file( url.toLocalFile() );
  if ( file.exists() && !file.isDir() )
    QgsGui::instance()->nativePlatformInterface()->openFileExplorerAndSelectFile( url.toLocalFile() );
  else
    QDesktopServices::openUrl( url );
}

void QgsBrowserLayerProperties::loadAttributeTable()
{
  if ( !mLayer || !mLayer->isValid() || mLayer->type() != QgsMapLayer::VectorLayer )
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
  QgsSettings settings;
  restoreGeometry( settings.value( mSettingsSection + "/propertiesDialog/geometry" ).toByteArray() );
}

QgsBrowserPropertiesDialog::~QgsBrowserPropertiesDialog()
{
  QgsSettings settings;
  settings.setValue( mSettingsSection + "/propertiesDialog/geometry", saveGeometry() );
}

void QgsBrowserPropertiesDialog::setItem( QgsDataItem *item )
{
  if ( !item )
    return;

  mPropertiesWidget = QgsBrowserPropertiesWidget::createWidget( item, this );
  mLayout->addWidget( mPropertiesWidget );
  setWindowTitle( item->type() == QgsDataItem::Layer ? tr( "Layer Properties" ) : tr( "Directory Properties" ) );
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
