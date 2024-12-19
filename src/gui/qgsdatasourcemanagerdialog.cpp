/***************************************************************************
    qgsdatasourcemanagerdialog.cpp - datasource manager dialog

    ---------------------
    begin                : May 19, 2017
    copyright            : (C) 2017 by Alessandro Pasotti
    email                : apasotti at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QListWidgetItem>

#include "qgsdatasourcemanagerdialog.h"
#include "moc_qgsdatasourcemanagerdialog.cpp"
#include "ui_qgsdatasourcemanagerdialog.h"
#include "qgsbrowserdockwidget.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgssourceselectprovider.h"
#include "qgssourceselectproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsmessagebar.h"
#include "qgsgui.h"
#include "qgsbrowserguimodel.h"
#include "qgsbrowserwidget.h"

QgsDataSourceManagerDialog::QgsDataSourceManagerDialog( QgsBrowserGuiModel *browserModel, QWidget *parent, QgsMapCanvas *canvas, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( tr( "Data Source Manager" ), parent, fl )
  , ui( new Ui::QgsDataSourceManagerDialog )
  , mPreviousRow( -1 )
  , mMapCanvas( canvas )
  , mBrowserModel( browserModel )
{
  ui->setupUi( this );
  ui->verticalLayout_2->setSpacing( 6 );
  ui->verticalLayout_2->setContentsMargins( 0, 0, 0, 0 );

  mMessageBar = new QgsMessageBar( this );
  mMessageBar->setSizePolicy( QSizePolicy::MinimumExpanding, QSizePolicy::Fixed );
  static_cast<QVBoxLayout *>( layout() )->insertWidget( 0, mMessageBar );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  // Bind list index to the stacked dialogs
  connect( ui->mOptionsListWidget, &QListWidget::currentRowChanged, this, &QgsDataSourceManagerDialog::setCurrentPage );

  // BROWSER Add the browser widget to the first stacked widget page
  mBrowserWidget = new QgsBrowserDockWidget( QStringLiteral( "Browser" ), mBrowserModel, this );
  mBrowserWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
  mBrowserWidget->setTitleBarWidget( new QWidget( mBrowserWidget ) );

  QWidget *browserWidgetWrapper = new QWidget( this );
  browserWidgetWrapper->setLayout( new QVBoxLayout( browserWidgetWrapper ) );
  browserWidgetWrapper->layout()->addWidget( mBrowserWidget );
  QDialogButtonBox *browserButtonBox = new QDialogButtonBox( QDialogButtonBox::StandardButton::Close | QDialogButtonBox::StandardButton::Help, browserWidgetWrapper );
  browserWidgetWrapper->layout()->addWidget( browserButtonBox );

  connect( browserButtonBox, &QDialogButtonBox::helpRequested, this, [=] {
    QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#the-browser-panel" ) );
  } );
  connect( browserButtonBox, &QDialogButtonBox::rejected, this, &QgsDataSourceManagerDialog::reject );

  ui->mOptionsStackedWidget->addWidget( browserWidgetWrapper );
  mPageProviderKeys.append( QStringLiteral( "browser" ) );
  mPageProviderNames.append( QStringLiteral( "browser" ) );

  // Forward all browser signals
  connect( mBrowserWidget, &QgsBrowserDockWidget::handleDropUriList, this, &QgsDataSourceManagerDialog::handleDropUriList );
  connect( mBrowserWidget, &QgsBrowserDockWidget::openFile, this, &QgsDataSourceManagerDialog::openFile );
  connect( mBrowserWidget, &QgsBrowserDockWidget::connectionsChanged, this, &QgsDataSourceManagerDialog::connectionsChanged );
  connect( this, &QgsDataSourceManagerDialog::updateProjectHome, mBrowserWidget->browserWidget(), &QgsBrowserWidget::updateProjectHome );

  // Add registered source select dialogs
  const QList<QgsSourceSelectProvider *> sourceSelectProviders = QgsGui::sourceSelectProviderRegistry()->providers();
  for ( QgsSourceSelectProvider *provider : sourceSelectProviders )
  {
    QgsAbstractDataSourceWidget *dlg = provider->createDataSourceWidget( this );
    if ( !dlg )
    {
      QgsMessageLog::logMessage( tr( "Cannot get %1 select dialog from source select provider %2." ).arg( provider->name(), provider->providerKey() ), QStringLiteral( "DataSourceManager" ), Qgis::MessageLevel::Critical );
      continue;
    }
    addProviderDialog( dlg, provider->providerKey(), provider->name(), provider->text(), provider->icon(), provider->toolTip() );
  }

  connect( QgsGui::sourceSelectProviderRegistry(), &QgsSourceSelectProviderRegistry::providerAdded, this, [=]( const QString &name ) {
    if ( QgsSourceSelectProvider *provider = QgsGui::sourceSelectProviderRegistry()->providerByName( name ) )
    {
      QgsAbstractDataSourceWidget *dlg = provider->createDataSourceWidget( this );
      if ( !dlg )
      {
        QgsMessageLog::logMessage( tr( "Cannot get %1 select dialog from source select provider %2." ).arg( provider->name(), provider->providerKey() ), QStringLiteral( "DataSourceManager" ), Qgis::MessageLevel::Critical );
        return;
      }
      addProviderDialog( dlg, provider->providerKey(), provider->name(), provider->text(), provider->icon(), provider->toolTip() );
    }
  } );

  connect( QgsGui::sourceSelectProviderRegistry(), &QgsSourceSelectProviderRegistry::providerRemoved, this, [=]( const QString &name ) {
    removeProviderDialog( name );
  } );

  restoreOptionsBaseUi( tr( "Data Source Manager" ) );
}

QgsDataSourceManagerDialog::~QgsDataSourceManagerDialog()
{
  delete ui;
}

void QgsDataSourceManagerDialog::openPage( const QString &pageName )
{
  // TODO -- this is actually using provider keys, not provider names!
  const int pageIdx = mPageProviderKeys.indexOf( pageName );
  if ( pageIdx != -1 )
  {
    QTimer::singleShot( 0, this, [=] { setCurrentPage( pageIdx ); } );
  }
}

QgsMessageBar *QgsDataSourceManagerDialog::messageBar() const
{
  return mMessageBar;
}

void QgsDataSourceManagerDialog::activate()
{
  raise();
  setWindowState( windowState() & ~Qt::WindowMinimized );
  activateWindow();
}

void QgsDataSourceManagerDialog::setCurrentPage( int index )
{
  mPreviousRow = ui->mOptionsStackedWidget->currentIndex();
  ui->mOptionsStackedWidget->setCurrentIndex( index );
  setWindowTitle( tr( "Data Source Manager | %1" ).arg( ui->mOptionsListWidget->currentItem()->text() ) );
  resizeAlltabs( index );
}

void QgsDataSourceManagerDialog::setPreviousPage()
{
  const int prevPage = mPreviousRow != -1 ? mPreviousRow : 0;
  setCurrentPage( prevPage );
}

void QgsDataSourceManagerDialog::refresh()
{
  mBrowserWidget->browserWidget()->refresh();
  emit providerDialogsRefreshRequested();
}

void QgsDataSourceManagerDialog::reset()
{
  const int pageCount = ui->mOptionsStackedWidget->count();
  for ( int i = 0; i < pageCount; ++i )
  {
    QWidget *widget = ui->mOptionsStackedWidget->widget( i );
    QgsAbstractDataSourceWidget *dataSourceWidget = qobject_cast<QgsAbstractDataSourceWidget *>( widget );
    if ( dataSourceWidget )
      dataSourceWidget->reset();
  }
}

void QgsDataSourceManagerDialog::configureFromUri( const QString &pageName, const QString &uri )
{
  const int pageIdx = mPageProviderNames.indexOf( pageName );
  if ( pageIdx != -1 )
  {
    QTimer::singleShot( 0, this, [this, pageIdx, uri] {
      setCurrentPage( pageIdx );
      if ( QgsAbstractDataSourceWidget *dataSourceWidget = qobject_cast<QgsAbstractDataSourceWidget *>( ui->mOptionsStackedWidget->currentWidget() ) )
      {
        dataSourceWidget->configureFromUri( uri );
      }
    } );
  }
}

void QgsDataSourceManagerDialog::rasterLayersAdded( const QStringList &layersList )
{
  emit addRasterLayers( layersList );
}

void QgsDataSourceManagerDialog::vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType )
{
  emit addVectorLayers( layerQStringList, enc, dataSourceType );
}

void QgsDataSourceManagerDialog::addProviderDialog( QgsAbstractDataSourceWidget *dlg, const QString &providerKey, const QString &providerName, const QString &text, const QIcon &icon, const QString &toolTip )
{
  mPageProviderKeys.append( providerKey );
  mPageProviderNames.append( providerName );
  ui->mOptionsStackedWidget->addWidget( dlg );
  QListWidgetItem *layerItem = new QListWidgetItem( text, ui->mOptionsListWidget );
  layerItem->setData( Qt::UserRole, providerName );
  layerItem->setToolTip( toolTip.isEmpty() ? tr( "Add %1 layer" ).arg( text ) : toolTip );
  layerItem->setIcon( icon );
  // Set crs and extent from canvas
  if ( mMapCanvas )
  {
    dlg->setMapCanvas( mMapCanvas );
  }
  dlg->setBrowserModel( mBrowserModel );

  connect( dlg, &QgsAbstractDataSourceWidget::rejected, this, &QgsDataSourceManagerDialog::reject );
  connect( dlg, &QgsAbstractDataSourceWidget::accepted, this, &QgsDataSourceManagerDialog::accept );
  makeConnections( dlg, providerKey );
}

void QgsDataSourceManagerDialog::removeProviderDialog( const QString &providerName )
{
  const int pageIdx = mPageProviderNames.indexOf( providerName );
  if ( pageIdx != -1 )
  {
    ui->mOptionsStackedWidget->removeWidget( ui->mOptionsStackedWidget->widget( pageIdx ) );
    mPageProviderKeys.removeAt( pageIdx );
    mPageProviderNames.removeAt( pageIdx );
    ui->mOptionsListWidget->removeItemWidget( ui->mOptionsListWidget->item( pageIdx ) );
  }
}

void QgsDataSourceManagerDialog::makeConnections( QgsAbstractDataSourceWidget *dlg, const QString &providerKey )
{
  // DB
  connect( dlg, &QgsAbstractDataSourceWidget::addDatabaseLayers, this, &QgsDataSourceManagerDialog::addDatabaseLayers );
  connect( dlg, &QgsAbstractDataSourceWidget::progressMessage, this, &QgsDataSourceManagerDialog::showStatusMessage );

  connect( dlg, &QgsAbstractDataSourceWidget::addLayer, this, [=]( Qgis::LayerType type, const QString &url, const QString &baseName, const QString &providerKey ) {
    Q_UNUSED( url )
    Q_UNUSED( baseName )
    Q_UNUSED( providerKey )

    switch ( type )
    {
      case Qgis::LayerType::Vector:
      case Qgis::LayerType::Raster:
      case Qgis::LayerType::Plugin:
      case Qgis::LayerType::Mesh:
      case Qgis::LayerType::VectorTile:
      case Qgis::LayerType::Annotation:
      case Qgis::LayerType::PointCloud:
      case Qgis::LayerType::Group:
        // for compatibility with older API, we ignore these signals and rely on the older granular signals (eg "addVectorLayer").
        // otherwise we will be emitting double signals for the old/new signal for these layer types
        break;

      case Qgis::LayerType::TiledScene:
        emit addLayer( type, url, baseName, providerKey );
        break;
    }
  } );

  // Vector
  Q_NOWARN_DEPRECATED_PUSH
  connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayer, this, [=]( const QString &vectorLayerPath, const QString &baseName, const QString &specifiedProvider ) {
    const QString key = specifiedProvider.isEmpty() ? providerKey : specifiedProvider;
    emit addLayer( Qgis::LayerType::Vector, vectorLayerPath, baseName, key );
  } );
  Q_NOWARN_DEPRECATED_POP
  connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayers, this, &QgsDataSourceManagerDialog::vectorLayersAdded );
  connect( dlg, &QgsAbstractDataSourceWidget::connectionsChanged, this, &QgsDataSourceManagerDialog::connectionsChanged );

  // Raster
  Q_NOWARN_DEPRECATED_PUSH
  connect( dlg, &QgsAbstractDataSourceWidget::addRasterLayer, this, [=]( const QString &rasterLayerPath, const QString &baseName, const QString &providerKey ) {
    emit addLayer( Qgis::LayerType::Raster, rasterLayerPath, baseName, providerKey );
  } );
  Q_NOWARN_DEPRECATED_POP
  connect( dlg, &QgsAbstractDataSourceWidget::addRasterLayers, this, &QgsDataSourceManagerDialog::rasterLayersAdded );

  // Mesh
  Q_NOWARN_DEPRECATED_PUSH
  connect( dlg, &QgsAbstractDataSourceWidget::addMeshLayer, this, [=]( const QString &url, const QString &baseName, const QString &providerKey ) {
    emit addLayer( Qgis::LayerType::Mesh, url, baseName, providerKey );
  } );
  Q_NOWARN_DEPRECATED_POP
  // Vector tile
  Q_NOWARN_DEPRECATED_PUSH
  connect( dlg, &QgsAbstractDataSourceWidget::addVectorTileLayer, this, [=]( const QString &url, const QString &baseName ) {
    emit addLayer( Qgis::LayerType::VectorTile, url, baseName, QString() );
  } );
  Q_NOWARN_DEPRECATED_POP
  // Point Cloud
  Q_NOWARN_DEPRECATED_PUSH
  connect( dlg, &QgsAbstractDataSourceWidget::addPointCloudLayer, this, [=]( const QString &url, const QString &baseName, const QString &providerKey ) {
    emit addLayer( Qgis::LayerType::PointCloud, url, baseName, providerKey );
  } );
  Q_NOWARN_DEPRECATED_POP
  // Virtual
  connect( dlg, &QgsAbstractDataSourceWidget::replaceVectorLayer, this, &QgsDataSourceManagerDialog::replaceSelectedVectorLayer );
  // Common
  connect( dlg, &QgsAbstractDataSourceWidget::connectionsChanged, this, &QgsDataSourceManagerDialog::connectionsChanged );
  connect( this, &QgsDataSourceManagerDialog::providerDialogsRefreshRequested, dlg, &QgsAbstractDataSourceWidget::refresh );

  // Message
  connect( dlg, &QgsAbstractDataSourceWidget::pushMessage, this, [=]( const QString &title, const QString &message, const Qgis::MessageLevel level ) {
    mMessageBar->pushMessage( title, message, level );
  } );
}

void QgsDataSourceManagerDialog::showEvent( QShowEvent *e )
{
  ui->mOptionsStackedWidget->currentWidget()->show();
  QgsOptionsDialogBase::showEvent( e );
  resizeAlltabs( ui->mOptionsStackedWidget->currentIndex() );
}
