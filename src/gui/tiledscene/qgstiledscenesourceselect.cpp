/***************************************************************************
                         qgstiledscenesourceselect.cpp
                         ---------------------------------
    begin                : June 2023
    copyright            : (C) 2023 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstiledscenesourceselect.h"
#include "moc_qgstiledscenesourceselect.cpp"
#include "qgstiledsceneconnection.h"
#include "qgsgui.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderutils.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgstiledsceneconnectiondialog.h"
#include "qgshelp.h"

#include <QMenu>
#include <QMessageBox>

///@cond PRIVATE

QgsTiledSceneSourceSelect::QgsTiledSceneSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Add Scene Layer" ) );

  mRadioSourceService->setChecked( true );
  mStackedWidget->setCurrentIndex( 1 );

  connect( mRadioSourceFile, &QRadioButton::toggled, this, [this] {
    mStackedWidget->setCurrentIndex( 0 );

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  } );
  connect( mRadioSourceService, &QRadioButton::toggled, this, [this] {
    mStackedWidget->setCurrentIndex( 1 );

    emit enableButtons( !cmbConnections->currentText().isEmpty() );
  } );

  btnNew->setPopupMode( QToolButton::InstantPopup );
  QMenu *newMenu = new QMenu( btnNew );

  QAction *actionNew = new QAction( tr( "New Cesium 3D Tiles Connection…" ), this );
  connect( actionNew, &QAction::triggered, this, [this]() { newConnection( "cesiumtiles" ); } );
  newMenu->addAction( actionNew );

  actionNew = new QAction( tr( "New Quantized Mesh Connection…" ), this );
  connect( actionNew, &QAction::triggered, this, [this]() { newConnection( "quantizedmesh" ); } );
  newMenu->addAction( actionNew );

  btnNew->setMenu( newMenu );

  connect( btnEdit, &QToolButton::clicked, this, &QgsTiledSceneSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QToolButton::clicked, this, &QgsTiledSceneSourceSelect::btnDelete_clicked );
  connect( btnSave, &QToolButton::clicked, this, &QgsTiledSceneSourceSelect::btnSave_clicked );
  connect( btnLoad, &QToolButton::clicked, this, &QgsTiledSceneSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsTiledSceneSourceSelect::cmbConnections_currentTextChanged );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsTiledSceneSourceSelect::showHelp );

  populateConnectionList();

  mFileWidget->setDialogTitle( tr( "Open Scene Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileTiledSceneFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    emit enableButtons( !path.isEmpty() );
  } );
}

void QgsTiledSceneSourceSelect::btnEdit_clicked()
{
  const QgsTiledSceneProviderConnection::Data connection = QgsTiledSceneProviderConnection::connection( cmbConnections->currentText() );
  const QString uri = QgsTiledSceneProviderConnection::encodedUri( connection );
  const QString provider = connection.provider;

  QgsTiledSceneConnectionDialog nc( this );
  nc.setConnection( cmbConnections->currentText(), uri );
  if ( nc.exec() )
  {
    QgsTiledSceneProviderConnection::Data connectionData = QgsTiledSceneProviderConnection::decodedUri( nc.connectionUri() );
    connectionData.provider = provider;

    QgsTiledSceneProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsTiledSceneSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                        .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsTiledSceneProviderConnection( QString() ).remove( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsTiledSceneSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::TiledScene );
  dlg.exec();
}

void QgsTiledSceneSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::TiledScene, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsTiledSceneSourceSelect::addButtonClicked()
{
  if ( mRadioSourceService->isChecked() )
  {
    const QgsTiledSceneProviderConnection::Data connection = QgsTiledSceneProviderConnection::connection( cmbConnections->currentText() );
    const QString uri = QgsTiledSceneProviderConnection::encodedUri( connection );
    emit addLayer( Qgis::LayerType::TiledScene, uri, cmbConnections->currentText(), connection.provider );
  }
  else if ( mRadioSourceFile->isChecked() )
  {
    const QString filePath = mFileWidget->filePath();
    const QList<QgsProviderRegistry::ProviderCandidateDetails> providers = QgsProviderRegistry::instance()->preferredProvidersForUri( filePath );
    QString providerKey;
    for ( const QgsProviderRegistry::ProviderCandidateDetails &details : providers )
    {
      if ( details.layerTypes().contains( Qgis::LayerType::TiledScene ) )
      {
        providerKey = details.metadata()->key();
      }
    }

    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), filePath );
    const QString uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );

    emit addLayer( Qgis::LayerType::TiledScene, uri, QgsProviderUtils::suggestLayerNameFromFilePath( filePath ), providerKey );
  }
}

void QgsTiledSceneSourceSelect::newConnection( QString provider )
{
  QgsTiledSceneConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsTiledSceneProviderConnection::Data connectionData = QgsTiledSceneProviderConnection::decodedUri( nc.connectionUri() );
    connectionData.provider = provider;

    QgsTiledSceneProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    QgsTiledSceneProviderConnection::setSelectedConnection( nc.connectionName() );
    setConnectionListPosition();
    emit connectionsChanged();
  }
}


void QgsTiledSceneSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsTiledSceneProviderConnection::connectionList() );
  cmbConnections->blockSignals( false );

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

void QgsTiledSceneSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsTiledSceneProviderConnection::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }

  emit enableButtons( !cmbConnections->currentText().isEmpty() );
}

void QgsTiledSceneSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsTiledSceneProviderConnection::setSelectedConnection( text );
  emit enableButtons( !text.isEmpty() );
}

void QgsTiledSceneSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html" ) );
}

///@endcond
