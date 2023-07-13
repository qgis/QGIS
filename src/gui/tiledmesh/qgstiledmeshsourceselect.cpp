/***************************************************************************
                         qgstiledmeshsourceselect.cpp
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

#include "qgstiledmeshsourceselect.h"
#include "qgstiledmeshconnection.h"
#include "qgsgui.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderutils.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgstiledmeshconnectiondialog.h"

#include <QMenu>
#include <QMessageBox>

///@cond PRIVATE

QgsTiledMeshSourceSelect::QgsTiledMeshSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Add Tiled Mesh Layer" ) );

  mRadioSourceService->setChecked( true );
  mStackedWidget->setCurrentIndex( 1 );

  connect( mRadioSourceFile, &QRadioButton::toggled, this, [this]
  {
    mStackedWidget->setCurrentIndex( 0 );

    emit enableButtons( !mFileWidget->filePath().isEmpty() );
  } );
  connect( mRadioSourceService, &QRadioButton::toggled, this, [this]
  {
    mStackedWidget->setCurrentIndex( 1 );

    emit enableButtons( !cmbConnections->currentText().isEmpty() );
  } );

  btnNew->setPopupMode( QToolButton::InstantPopup );
  QMenu *newMenu = new QMenu( btnNew );

  QAction *actionNew = new QAction( tr( "New Cesium 3D Tiles Connection…" ), this );
  connect( actionNew, &QAction::triggered, this, &QgsTiledMeshSourceSelect::btnNewCesium3DTiles_clicked );
  newMenu->addAction( actionNew );

  btnNew->setMenu( newMenu );

  connect( btnEdit, &QToolButton::clicked, this, &QgsTiledMeshSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QToolButton::clicked, this, &QgsTiledMeshSourceSelect::btnDelete_clicked );
  connect( btnSave, &QToolButton::clicked, this, &QgsTiledMeshSourceSelect::btnSave_clicked );
  connect( btnLoad, &QToolButton::clicked, this, &QgsTiledMeshSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsTiledMeshSourceSelect::cmbConnections_currentTextChanged );
  setupButtons( buttonBox );

  populateConnectionList();

  mFileWidget->setDialogTitle( tr( "Open Tiled Mesh Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileTiledMeshFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [ = ]( const QString & path )
  {
    emit enableButtons( !path.isEmpty() );
  } );
}

void QgsTiledMeshSourceSelect::btnNewCesium3DTiles_clicked()
{
  QgsTiledMeshConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsTiledMeshProviderConnection::Data connectionData = QgsTiledMeshProviderConnection::decodedUri( nc.connectionUri() );
    connectionData.provider = QStringLiteral( "cesiumtiles" );

    QgsTiledMeshProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsTiledMeshSourceSelect::btnEdit_clicked()
{
  const QgsTiledMeshProviderConnection::Data connection = QgsTiledMeshProviderConnection::connection( cmbConnections->currentText() );
  const QString uri = QgsTiledMeshProviderConnection::encodedUri( connection );
  const QString provider = connection.provider;

  QgsTiledMeshConnectionDialog nc( this );
  nc.setConnection( cmbConnections->currentText(), uri );
  if ( nc.exec() )
  {
    QgsTiledMeshProviderConnection::Data connectionData = QgsTiledMeshProviderConnection::decodedUri( nc.connectionUri() );
    connectionData.provider = provider;

    QgsTiledMeshProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }

}

void QgsTiledMeshSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                      .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsTiledMeshProviderConnection( QString() ).remove( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsTiledMeshSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::TiledMesh );
  dlg.exec();
}

void QgsTiledMeshSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::TiledMesh, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsTiledMeshSourceSelect::addButtonClicked()
{
  if ( mRadioSourceService->isChecked() )
  {
    const QgsTiledMeshProviderConnection::Data connection = QgsTiledMeshProviderConnection::connection( cmbConnections->currentText() );
    const QString uri = QgsTiledMeshProviderConnection::encodedUri( connection );
    emit addLayer( Qgis::LayerType::TiledMesh, uri, cmbConnections->currentText(), connection.provider );
  }
  else if ( mRadioSourceFile->isChecked() )
  {
    const QString filePath = mFileWidget->filePath();
    const QList< QgsProviderRegistry::ProviderCandidateDetails > providers = QgsProviderRegistry::instance()->preferredProvidersForUri( filePath );
    QString providerKey;
    for ( const QgsProviderRegistry::ProviderCandidateDetails &details : providers )
    {
      if ( details.layerTypes().contains( Qgis::LayerType::TiledMesh ) )
      {
        providerKey = details.metadata()->key();
      }
    }

    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), filePath );
    const QString uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );

    emit addLayer( Qgis::LayerType::TiledMesh, uri, QgsProviderUtils::suggestLayerNameFromFilePath( filePath ), providerKey );
  }
}

void QgsTiledMeshSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsTiledMeshProviderConnection::connectionList() );
  cmbConnections->blockSignals( false );

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

void QgsTiledMeshSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsTiledMeshProviderConnection::selectedConnection();

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

void QgsTiledMeshSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsTiledMeshProviderConnection::setSelectedConnection( text );
  emit enableButtons( !text.isEmpty() );
}

///@endcond
