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
#include "qgsopenvectorlayerdialog.h"
#include "qgssourceselectdialog.h"
#include "qgsmapcanvas.h"


QgsDataSourceManagerDialog::QgsDataSourceManagerDialog( QgsMapCanvas *mapCanvas, QWidget *parent ) :
  QDialog( parent ),
  ui( new Ui::QgsDataSourceManagerDialog ),
  mMapCanvas( mapCanvas )
{
  ui->setupUi( this );

  // More setup
  int size = QgsSettings().value( QStringLiteral( "/IconSize" ), 24 ).toInt();
  // buffer size to match displayed icon size in toolbars, and expected geometry restore
  // newWidth (above) may need adjusted if you adjust iconBuffer here
  int iconBuffer = 4;
  ui->mList->setIconSize( QSize( size + iconBuffer, size + iconBuffer ) );
  ui->mList->setFrameStyle( QFrame::NoFrame );
  ui->mListFrame->layout()->setContentsMargins( 0, 3, 3, 3 );

  // Bind list index to the stacked dialogs
  connect( ui->mList, SIGNAL( currentRowChanged( int ) ), this, SLOT( setCurrentPage( int ) ) );

  // BROWSER Add the browser widget to the first stacked widget page
  mBrowserWidget = new QgsBrowserDockWidget( QStringLiteral( "Browser" ), this );
  mBrowserWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
  ui->mStackedWidget->addWidget( mBrowserWidget );

  // VECTOR Layers (completely different interface: it's not a provider)
  QgsOpenVectorLayerDialog *ovl = new QgsOpenVectorLayerDialog( this, Qt::Widget, true );
  ui->mStackedWidget->addWidget( ovl );
  QListWidgetItem *ogrItem = new QListWidgetItem( tr( "Vector files" ), ui->mList );
  ogrItem->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionAddOgrLayer.svg" ) ) );
  connect( ovl, &QgsOpenVectorLayerDialog::addVectorLayers, this, &QgsDataSourceManagerDialog::vectorLayersAdded );

  // Add data provider dialogs
  QDialog *dlg;

#ifdef HAVE_POSTGRESQL
  addDbProviderDialog( QStringLiteral( "postgres" ), tr( "PostgreSQL" ), QStringLiteral( "/mActionAddPostgisLayer.svg" ) );
#endif

  addDbProviderDialog( QStringLiteral( "spatialite" ), tr( "Spatialite" ), QStringLiteral( "/mActionAddSpatiaLiteLayer.svg" ) );

  addDbProviderDialog( QStringLiteral( "mssql" ), tr( "MSSQL" ), QStringLiteral( "/mActionAddMssqlLayer.svg" ) );

  addDbProviderDialog( QStringLiteral( "DB2" ), tr( "DB2" ), QStringLiteral( "/mActionAddDb2Layer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "wms" ), tr( "WMS" ), QStringLiteral( "/mActionAddWmsLayer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "arcgismapserver" ), tr( "ArcGIS Map Server" ), QStringLiteral( "/mActionAddAmsLayer.svg" ) );

  addRasterProviderDialog( QStringLiteral( "wcs" ), tr( "WCS" ), QStringLiteral( "/mActionAddWcsLayer.svg" ) );

  dlg = providerDialog( QStringLiteral( "WFS" ), tr( "WFS" ), QStringLiteral( "/mActionAddWfsLayer.svg" ) );

  if ( dlg )
  {
    // Forward (if only a common interface for the signals had been used in the providers ...)
    connect( dlg, SIGNAL( addWfsLayer( QString, QString ) ), this, SIGNAL( addWfsLayer( QString, QString ) ) );
    connect( this, &QgsDataSourceManagerDialog::addWfsLayer, this,  [ = ]( const QString & vectorLayerPath, const QString & baseName )
    {
      this->vectorLayerAdded( vectorLayerPath, baseName, QStringLiteral( "WFS" ) );
    } );
  }

  QgsSourceSelectDialog *afss = dynamic_cast<QgsSourceSelectDialog *>( providerDialog( QStringLiteral( "arcgisfeatureserver" ),
                                tr( "ArcGIS Feature Server" ),
                                QStringLiteral( "/mActionAddAfsLayer.svg" ) ) );
  if ( afss && mMapCanvas )
  {
    afss->setCurrentExtentAndCrs( mMapCanvas->extent(), mMapCanvas->mapSettings().destinationCrs() );
    // Forward (if only a common interface for the signals had been used in the providers ...)
    connect( afss, SIGNAL( addLayer( QString, QString ) ), this, SIGNAL( addAfsLayer( QString, QString ) ) );
    connect( this, &QgsDataSourceManagerDialog::addAfsLayer,
             this, [ = ]( const QString & vectorLayerPath, const QString & baseName )
    { this->vectorLayerAdded( vectorLayerPath, baseName, QStringLiteral( "arcgisfeatureserver" ) ); } );
  }

}

QgsDataSourceManagerDialog::~QgsDataSourceManagerDialog()
{
  delete ui;
}

void QgsDataSourceManagerDialog::setCurrentPage( int index )
{
  ui->mStackedWidget->setCurrentIndex( index );
  setWindowTitle( tr( "Data Source Manager | %1" ).arg( ui->mList->currentItem()->text( ) ) );
}

void QgsDataSourceManagerDialog::rasterLayerAdded( const QString &uri, const QString &baseName, const QString &providerKey )
{
  emit( addRasterLayer( uri, baseName, providerKey ) );
}

void QgsDataSourceManagerDialog::vectorLayerAdded( const QString &vectorLayerPath, const QString &baseName, const QString &providerKey )
{
  emit( addVectorLayer( vectorLayerPath, baseName, providerKey ) );
}

void QgsDataSourceManagerDialog::vectorLayersAdded( const QStringList &layerQStringList, const QString &enc, const QString &dataSourceType )
{
  emit addVectorLayers( layerQStringList, enc, dataSourceType );
}


QDialog *QgsDataSourceManagerDialog::providerDialog( const QString providerKey, const QString providerName, const QString icon )
{
  QDialog *dlg = dynamic_cast<QDialog *>( QgsProviderRegistry::instance()->createSelectionWidget( providerKey, this, Qt::Widget, true ) );
  if ( !dlg )
  {
    QMessageBox::warning( this, providerName, tr( "Cannot get %1 select dialog from provider %2." ).arg( providerName, providerKey ) );
    return nullptr;
  }
  else
  {
    ui->mStackedWidget->addWidget( dlg );
    QListWidgetItem *wmsItem = new QListWidgetItem( providerName, ui->mList );
    wmsItem->setIcon( QgsApplication::getThemeIcon( icon ) );
    return dlg;
  }
}

void QgsDataSourceManagerDialog::addDbProviderDialog( const QString providerKey, const QString providerName, const QString icon )
{
  QDialog *dlg = providerDialog( providerKey, providerName, icon );
  if ( dlg )
  {
    connect( dlg, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
             this, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ) );
    connect( dlg, SIGNAL( progress( int, int ) ),
             this, SIGNAL( showProgress( int, int ) ) );
    connect( dlg, SIGNAL( progressMessage( QString ) ),
             this, SIGNAL( showStatusMessage( QString ) ) );
  }
}

void QgsDataSourceManagerDialog::addRasterProviderDialog( const QString providerKey, const QString providerName, const QString icon )
{
  QDialog *dlg = providerDialog( providerKey, providerName, icon );
  if ( dlg )
  {
    // Forward
    connect( dlg, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
             this, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ) );
  }
}
