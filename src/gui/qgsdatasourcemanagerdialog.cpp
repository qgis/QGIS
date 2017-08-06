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

#include <QMessageBox>
#include <QListWidgetItem>

#include "qgsdatasourcemanagerdialog.h"
#include "ui_qgsdatasourcemanagerdialog.h"
#include "qgsbrowserdockwidget.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsmapcanvas.h"
#include "qgsgeonodesourceselect.h"

QgsDataSourceManagerDialog::QgsDataSourceManagerDialog( QWidget *parent, QgsMapCanvas *canvas, Qt::WindowFlags fl ) :
  QgsOptionsDialogBase( QStringLiteral( "Data Source Manager" ), parent, fl ),
  ui( new Ui::QgsDataSourceManagerDialog ),
  mPreviousRow( -1 ),
  mMapCanvas( canvas )
{

  ui->setupUi( this );
  ui->verticalLayout_2->setSpacing( 6 );
  ui->verticalLayout_2->setMargin( 0 );
  ui->verticalLayout_2->setContentsMargins( 0, 0, 0, 0 );
  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( true );

  // Bind list index to the stacked dialogs
  connect( ui->mOptionsListWidget, &QListWidget::currentRowChanged, this, &QgsDataSourceManagerDialog::setCurrentPage );

  // BROWSER Add the browser widget to the first stacked widget page
  mBrowserWidget = new QgsBrowserDockWidget( QStringLiteral( "Browser" ), this );
  mBrowserWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
  ui->mOptionsStackedWidget->addWidget( mBrowserWidget );
  mPageNames.append( QStringLiteral( "browser" ) );
  // Forward all browser signals
  connect( mBrowserWidget, &QgsBrowserDockWidget::handleDropUriList, this, &QgsDataSourceManagerDialog::handleDropUriList );
  connect( mBrowserWidget, &QgsBrowserDockWidget::openFile, this, &QgsDataSourceManagerDialog::openFile );
  connect( mBrowserWidget, &QgsBrowserDockWidget::connectionsChanged, this, &QgsDataSourceManagerDialog::connectionsChanged );
  connect( this, &QgsDataSourceManagerDialog::updateProjectHome, mBrowserWidget, &QgsBrowserDockWidget::updateProjectHome );

  // Add data provider dialogs
  QWidget *dlg = nullptr;

  addVectorProviderDialog( QStringLiteral( "ogr" ), tr( "Vector" ), QStringLiteral( "/mActionAddOgrLayer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "gdal" ), tr( "Raster" ), QStringLiteral( "/mActionAddRasterLayer.svg" ) );

  addVectorProviderDialog( QStringLiteral( "delimitedtext" ), tr( "Delimited Text" ), QStringLiteral( "/mActionAddDelimitedTextLayer.svg" ) );

#ifdef HAVE_POSTGRESQL
  addDbProviderDialog( QStringLiteral( "postgres" ), tr( "PostgreSQL" ), QStringLiteral( "/mActionAddPostgisLayer.svg" ) );
#endif

  addDbProviderDialog( QStringLiteral( "spatialite" ), tr( "SpatiaLite" ), QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );

  addDbProviderDialog( QStringLiteral( "mssql" ), tr( "MSSQL" ), QStringLiteral( "/mActionAddMssqlLayer.svg" ) );

  addDbProviderDialog( QStringLiteral( "DB2" ), tr( "DB2" ), QStringLiteral( "/mActionAddDb2Layer.svg" ) );

#ifdef HAVE_ORACLE
  addDbProviderDialog( QStringLiteral( "oracle" ), tr( "Oracle" ), QStringLiteral( "/mActionAddOracleLayer.svg" ) );
#endif

  dlg = addVectorProviderDialog( QStringLiteral( "virtual" ), tr( "Virtual Layer" ), QStringLiteral( "/mActionAddVirtualLayer.svg" ) );

  // Apparently this is the only provider using replaceVectorLayer, we should
  // move this in to the base abstract class when it is used by at least one
  // additional provider.
  if ( dlg )
  {
    connect( dlg, SIGNAL( replaceVectorLayer( QString, QString, QString, QString ) ), this, SIGNAL( replaceSelectedVectorLayer( QString, QString, QString, QString ) ) );
  }

  addRasterProviderDialog( QStringLiteral( "wms" ), tr( "WMS" ), QStringLiteral( "/mActionAddWmsLayer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "wcs" ), tr( "WCS" ), QStringLiteral( "/mActionAddWcsLayer.svg" ) );

  addVectorProviderDialog( QStringLiteral( "WFS" ), tr( "WFS" ), QStringLiteral( "/mActionAddWfsLayer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "arcgismapserver" ), tr( "ArcGIS Map Server" ), QStringLiteral( "/mActionAddAmsLayer.svg" ) );

  addVectorProviderDialog( QStringLiteral( "arcgisfeatureserver" ), tr( "ArcGIS Feature Server" ), QStringLiteral( "/mActionAddAfsLayer.svg" ) );

  QDialog *geonodeDialog = new QgsGeoNodeSourceSelect( this, Qt::Widget, QgsProviderRegistry::WidgetMode::Embedded );
  dlg = addDialog( geonodeDialog, QStringLiteral( "geonode" ), tr( "GeoNode" ), QStringLiteral( "/mActionAddGeonodeLayer.svg" ) );

  if ( dlg )
  {
    connect( dlg, SIGNAL( addRasterLayer( QString, QString, QString ) ), this, SLOT( rasterLayerAdded( QString, QString, QString ) ) );
    connect( dlg, SIGNAL( addWfsLayer( QString, QString, QString ) ), this, SLOT( vectorLayerAdded( QString, QString, QString ) ) );
  }
}

QgsDataSourceManagerDialog::~QgsDataSourceManagerDialog()
{
  delete ui;
}

void QgsDataSourceManagerDialog::openPage( QString pageName )
{
  int pageIdx = mPageNames.indexOf( pageName );
  if ( pageIdx != -1 )
  {
    QTimer::singleShot( 0, this, [ = ] { setCurrentPage( pageIdx ); } );
  }
}

void QgsDataSourceManagerDialog::setCurrentPage( int index )
{
  mPreviousRow = ui->mOptionsStackedWidget->currentIndex();
  ui->mOptionsStackedWidget->setCurrentIndex( index );
  setWindowTitle( tr( "Data Source Manager | %1" ).arg( ui->mOptionsListWidget->currentItem()->text() ) );
}

void QgsDataSourceManagerDialog::setPreviousPage()
{
  int prevPage = mPreviousRow != -1 ? mPreviousRow : 0;
  setCurrentPage( prevPage );
}

void QgsDataSourceManagerDialog::refresh()
{
  mBrowserWidget->refresh();
  emit providerDialogsRefreshRequested();
}

void QgsDataSourceManagerDialog::rasterLayerAdded( const QString &uri, const QString &baseName, const QString &providerKey )
{
  emit addRasterLayer( uri, baseName, providerKey );
}

void QgsDataSourceManagerDialog::vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey )
{
  emit addVectorLayer( vectorLayerPath, baseName, providerKey );
}

void QgsDataSourceManagerDialog::vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType )
{
  emit addVectorLayers( layerQStringList, enc, dataSourceType );
}

QDialog *QgsDataSourceManagerDialog::addDialog( QDialog *dialog, QString const key, QString const name, QString const icon, QString title )
{
  mPageNames.append( key );
  ui->mOptionsStackedWidget->addWidget( dialog );
  QListWidgetItem *layerItem = new QListWidgetItem( name, ui->mOptionsListWidget );
  layerItem->setToolTip( title.isEmpty() ? tr( "Add %1 layer" ).arg( name ) : title );
  layerItem->setIcon( QgsApplication::getThemeIcon( icon ) );
  return dialog;
}

QgsAbstractDataSourceWidget *QgsDataSourceManagerDialog::providerDialog( const QString providerKey, const QString providerName, const QString icon, QString title )
{
  QgsAbstractDataSourceWidget *dlg = dynamic_cast<QgsAbstractDataSourceWidget *>( QgsProviderRegistry::instance()->createSelectionWidget( providerKey, this, Qt::Widget, QgsProviderRegistry::WidgetMode::Embedded ) );
  if ( !dlg )
  {
    QMessageBox::warning( this, providerName, tr( "Cannot get %1 select dialog from provider %2." ).arg( providerName, providerKey ) );
    return nullptr;
  }
  else
  {
    mPageNames.append( providerKey );
    ui->mOptionsStackedWidget->addWidget( dlg );
    QListWidgetItem *layerItem = new QListWidgetItem( providerName, ui->mOptionsListWidget );
    layerItem->setToolTip( title.isEmpty() ? tr( "Add %1 layer" ).arg( providerName ) : title );
    layerItem->setIcon( QgsApplication::getThemeIcon( icon ) );
    // Set crs and extent from canvas
    if ( mMapCanvas )
    {
      dlg->setMapCanvas( mMapCanvas );
    }
    connect( dlg, &QgsAbstractDataSourceWidget::rejected, this, &QgsDataSourceManagerDialog::reject );
    connect( dlg, &QgsAbstractDataSourceWidget::accepted, this, &QgsDataSourceManagerDialog::accept );
    return dlg;
  }
}

QgsAbstractDataSourceWidget *QgsDataSourceManagerDialog::addDbProviderDialog( const QString providerKey, const QString providerName, const QString icon, QString title )
{
  QgsAbstractDataSourceWidget *dlg = providerDialog( providerKey, providerName, icon, title );
  if ( dlg )
  {
    connect( dlg, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
             this, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ) );
    connect( dlg, SIGNAL( progress( int, int ) ),
             this, SIGNAL( showProgress( int, int ) ) );
    connect( dlg, SIGNAL( progressMessage( QString ) ),
             this, SIGNAL( showStatusMessage( QString ) ) );
    connect( dlg, SIGNAL( connectionsChanged() ), this, SIGNAL( connectionsChanged() ) );
    connect( this, SIGNAL( providerDialogsRefreshRequested() ), dlg, SLOT( refresh() ) );
  }
  return dlg;
}

QgsAbstractDataSourceWidget *QgsDataSourceManagerDialog::addRasterProviderDialog( const QString providerKey, const QString providerName, const QString icon, QString title )
{
  QgsAbstractDataSourceWidget *dlg = providerDialog( providerKey, providerName, icon, title );
  if ( dlg )
  {
    connect( dlg, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
             this, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ) );
    connect( dlg, SIGNAL( connectionsChanged() ), this, SIGNAL( connectionsChanged() ) );
    connect( this,  SIGNAL( providerDialogsRefreshRequested() ), dlg, SLOT( refresh() ) );
  }
  return dlg;
}

QgsAbstractDataSourceWidget *QgsDataSourceManagerDialog::addVectorProviderDialog( const QString providerKey, const QString providerName, const QString icon, QString title )
{
  QgsAbstractDataSourceWidget *dlg = providerDialog( providerKey, providerName, icon, title );
  if ( dlg )
  {
    connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayer, this, [ = ]( const QString & vectorLayerPath, const QString & baseName )
    { this->vectorLayerAdded( vectorLayerPath, baseName, providerKey ); } );
    connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayers, this, &QgsDataSourceManagerDialog::vectorLayersAdded );
    connect( this,  SIGNAL( providerDialogsRefreshRequested() ), dlg, SLOT( refresh() ) );
  }
  return dlg;
}

void QgsDataSourceManagerDialog::showEvent( QShowEvent *e )
{
  ui->mOptionsStackedWidget->currentWidget()->show();
  QDialog::showEvent( e );
}
