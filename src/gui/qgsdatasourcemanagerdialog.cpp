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
#include "ui_qgsdatasourcemanagerdialog.h"
#include "qgsbrowserdockwidget.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgssourceselectprovider.h"
#include "qgssourceselectproviderregistry.h"
#include "qgsabstractdatasourcewidget.h"
#include "qgsmapcanvas.h"
#include "qgsmessagelog.h"
#include "qgsgui.h"

QgsDataSourceManagerDialog::QgsDataSourceManagerDialog( QgsBrowserModel *browserModel, QWidget *parent, QgsMapCanvas *canvas, Qt::WindowFlags fl )
  : QgsOptionsDialogBase( QStringLiteral( "Data Source Manager" ), parent, fl )
  , ui( new Ui::QgsDataSourceManagerDialog )
  , mPreviousRow( -1 )
  , mMapCanvas( canvas )
{

  ui->setupUi( this );
  ui->verticalLayout_2->setSpacing( 6 );
  ui->verticalLayout_2->setMargin( 0 );
  ui->verticalLayout_2->setContentsMargins( 0, 0, 0, 0 );

  // QgsOptionsDialogBase handles saving/restoring of geometry, splitter and current tab states,
  // switching vertical tabs between icon/text to icon-only modes (splitter collapsed to left),
  // and connecting QDialogButtonBox's accepted/rejected signals to dialog's accept/reject slots
  initOptionsBase( false );

  // Bind list index to the stacked dialogs
  connect( ui->mOptionsListWidget, &QListWidget::currentRowChanged, this, &QgsDataSourceManagerDialog::setCurrentPage );

  // BROWSER Add the browser widget to the first stacked widget page
  mBrowserWidget = new QgsBrowserDockWidget( QStringLiteral( "Browser" ), browserModel, this );
  mBrowserWidget->setFeatures( QDockWidget::NoDockWidgetFeatures );
  ui->mOptionsStackedWidget->addWidget( mBrowserWidget );
  mPageNames.append( QStringLiteral( "browser" ) );
  // Forward all browser signals
  connect( mBrowserWidget, &QgsBrowserDockWidget::handleDropUriList, this, &QgsDataSourceManagerDialog::handleDropUriList );
  connect( mBrowserWidget, &QgsBrowserDockWidget::openFile, this, &QgsDataSourceManagerDialog::openFile );
  connect( mBrowserWidget, &QgsBrowserDockWidget::connectionsChanged, this, &QgsDataSourceManagerDialog::connectionsChanged );
  connect( this, &QgsDataSourceManagerDialog::updateProjectHome, mBrowserWidget, &QgsBrowserDockWidget::updateProjectHome );

  // Add provider dialogs
  const QList<QgsSourceSelectProvider *> sourceSelectProviders = QgsGui::sourceSelectProviderRegistry()->providers( );
  for ( const auto &provider : sourceSelectProviders )
  {
    QgsAbstractDataSourceWidget *dlg = provider->createDataSourceWidget( this );
    if ( !dlg )
    {
      QgsMessageLog::logMessage( tr( "Cannot get %1 select dialog from source select provider %2." ).arg( provider->name(), provider->providerKey() ), QStringLiteral( "DataSourceManager" ), Qgis::Critical );
      continue;
    }
    addProviderDialog( dlg, provider->providerKey(), provider->text(), provider->icon( ), provider->toolTip( ) );
  }

  restoreOptionsBaseUi( QStringLiteral( "Data Source Manager" ) );
}

QgsDataSourceManagerDialog::~QgsDataSourceManagerDialog()
{
  delete ui;
}

void QgsDataSourceManagerDialog::openPage( const QString &pageName )
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


void QgsDataSourceManagerDialog::addProviderDialog( QgsAbstractDataSourceWidget *dlg, const QString &providerKey, const QString &providerName, const QIcon &icon, const QString &toolTip )
{
  mPageNames.append( providerKey );
  ui->mOptionsStackedWidget->addWidget( dlg );
  QListWidgetItem *layerItem = new QListWidgetItem( providerName, ui->mOptionsListWidget );
  layerItem->setToolTip( toolTip.isEmpty() ? tr( "Add %1 layer" ).arg( providerName ) : toolTip );
  layerItem->setIcon( icon );
  // Set crs and extent from canvas
  if ( mMapCanvas )
  {
    dlg->setMapCanvas( mMapCanvas );
  }
  connect( dlg, &QgsAbstractDataSourceWidget::rejected, this, &QgsDataSourceManagerDialog::reject );
  connect( dlg, &QgsAbstractDataSourceWidget::accepted, this, &QgsDataSourceManagerDialog::accept );
  makeConnections( dlg, providerKey );
}

void QgsDataSourceManagerDialog::makeConnections( QgsAbstractDataSourceWidget *dlg, const QString &providerKey )
{
  // DB
  connect( dlg, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ),
           this, SIGNAL( addDatabaseLayers( QStringList const &, QString const & ) ) );
  connect( dlg, SIGNAL( progressMessage( QString ) ),
           this, SIGNAL( showStatusMessage( QString ) ) );
  // Vector
  connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayer, this, [ = ]( const QString & vectorLayerPath, const QString & baseName, const QString & specifiedProvider )
  {
    QString key = specifiedProvider.isEmpty() ? providerKey : specifiedProvider;
    this->vectorLayerAdded( vectorLayerPath, baseName, key );
  }
         );
  connect( dlg, &QgsAbstractDataSourceWidget::addVectorLayers,
           this, &QgsDataSourceManagerDialog::vectorLayersAdded );
  connect( dlg, SIGNAL( connectionsChanged() ), this, SIGNAL( connectionsChanged() ) );
  // Raster
  connect( dlg, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ),
           this, SIGNAL( addRasterLayer( QString const &, QString const &, QString const & ) ) );
  // Mesh
  connect( dlg, &QgsAbstractDataSourceWidget::addMeshLayer, this, &QgsDataSourceManagerDialog::addMeshLayer );

  // Virtual
  connect( dlg, SIGNAL( replaceVectorLayer( QString, QString, QString, QString ) ),
           this, SIGNAL( replaceSelectedVectorLayer( QString, QString, QString, QString ) ) );
  // Common
  connect( dlg, SIGNAL( connectionsChanged() ), this, SIGNAL( connectionsChanged() ) );
  connect( this, SIGNAL( providerDialogsRefreshRequested() ), dlg, SLOT( refresh() ) );
}

void QgsDataSourceManagerDialog::showEvent( QShowEvent *e )
{
  ui->mOptionsStackedWidget->currentWidget()->show();
  QgsOptionsDialogBase::showEvent( e );
}
