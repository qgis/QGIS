/***************************************************************************
                         qgsvectortilesourceselect.cpp
                         ---------------------------------
    begin                : April 2020
    copyright            : (C) 2020 by Alexander Bruy
    email                : alexander dot bruy at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsvectortilesourceselect.h"
#include "moc_qgsvectortilesourceselect.cpp"
#include "qgsvectortileconnection.h"
#include "qgsvectortileconnectiondialog.h"
#include "qgsarcgisvectortileconnectiondialog.h"
#include "qgsprovidermetadata.h"
#include "qgsproviderutils.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>
#include <QMenu>
#include <QAction>

///@cond PRIVATE

QgsVectorTileSourceSelect::QgsVectorTileSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  QgsGui::enableAutoGeometryRestore( this );

  setWindowTitle( tr( "Add Vector Tile Layer" ) );

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

  QAction *actionNew = new QAction( tr( "New Generic Connection…" ), this );
  connect( actionNew, &QAction::triggered, this, &QgsVectorTileSourceSelect::btnNew_clicked );
  newMenu->addAction( actionNew );

  QAction *actionNewArcGISConnection = new QAction( tr( "New ArcGIS Vector Tile Service Connection…" ), this );
  connect( actionNewArcGISConnection, &QAction::triggered, this, &QgsVectorTileSourceSelect::newArcgisVectorTileServerConnection );
  newMenu->addAction( actionNewArcGISConnection );

  btnNew->setMenu( newMenu );

  connect( btnEdit, &QToolButton::clicked, this, &QgsVectorTileSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QToolButton::clicked, this, &QgsVectorTileSourceSelect::btnDelete_clicked );
  connect( btnSave, &QToolButton::clicked, this, &QgsVectorTileSourceSelect::btnSave_clicked );
  connect( btnLoad, &QToolButton::clicked, this, &QgsVectorTileSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsVectorTileSourceSelect::cmbConnections_currentTextChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileSourceSelect::showHelp );
  setupButtons( buttonBox );

  populateConnectionList();

  mFileWidget->setDialogTitle( tr( "Open Vector Tile Dataset" ) );
  mFileWidget->setFilter( QgsProviderRegistry::instance()->fileVectorTileFilters() );
  mFileWidget->setStorageMode( QgsFileWidget::GetFile );
  mFileWidget->setOptions( QFileDialog::HideNameFilterDetails );
  connect( mFileWidget, &QgsFileWidget::fileChanged, this, [=]( const QString &path ) {
    emit enableButtons( !path.isEmpty() );
  } );
}

void QgsVectorTileSourceSelect::btnNew_clicked()
{
  QgsVectorTileConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
    populateConnectionList();
    QgsVectorTileProviderConnection::setSelectedConnection( nc.connectionName() );
    setConnectionListPosition();
    emit connectionsChanged();
  }
}

void QgsVectorTileSourceSelect::newArcgisVectorTileServerConnection()
{
  QgsArcgisVectorTileConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsVectorTileSourceSelect::btnEdit_clicked()
{
  const QgsVectorTileProviderConnection::Data connection = QgsVectorTileProviderConnection::connection( cmbConnections->currentText() );
  const QString uri = QgsVectorTileProviderConnection::encodedUri( connection );

  switch ( connection.serviceType )
  {
    case QgsVectorTileProviderConnection::Generic:
    {
      QgsVectorTileConnectionDialog nc( this );
      nc.setConnection( cmbConnections->currentText(), uri );
      if ( nc.exec() )
      {
        QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
        populateConnectionList();
        emit connectionsChanged();
      }
      break;
    }

    case QgsVectorTileProviderConnection::ArcgisVectorTileService:
    {
      QgsArcgisVectorTileConnectionDialog nc( this );

      nc.setConnection( cmbConnections->currentText(), uri );
      if ( nc.exec() )
      {
        QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
        populateConnectionList();
        emit connectionsChanged();
      }
      break;
    }
  }
}

void QgsVectorTileSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                        .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsVectorTileProviderConnection::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsVectorTileSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::VectorTile );
  dlg.exec();
}

void QgsVectorTileSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::VectorTile, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsVectorTileSourceSelect::addButtonClicked()
{
  if ( mRadioSourceService->isChecked() )
  {
    const QString uri = QgsVectorTileProviderConnection::encodedUri( QgsVectorTileProviderConnection::connection( cmbConnections->currentText() ) );
    Q_NOWARN_DEPRECATED_PUSH
    emit addVectorTileLayer( uri, cmbConnections->currentText() );
    Q_NOWARN_DEPRECATED_POP
    emit addLayer( Qgis::LayerType::VectorTile, uri, cmbConnections->currentText(), QString() );
  }
  else if ( mRadioSourceFile->isChecked() )
  {
    const QString filePath = mFileWidget->filePath();
    const QList<QgsProviderRegistry::ProviderCandidateDetails> providers = QgsProviderRegistry::instance()->preferredProvidersForUri( filePath );
    QString providerKey;
    for ( const QgsProviderRegistry::ProviderCandidateDetails &details : providers )
    {
      if ( details.layerTypes().contains( Qgis::LayerType::VectorTile ) )
      {
        providerKey = details.metadata()->key();
      }
    }

    QVariantMap parts;
    parts.insert( QStringLiteral( "path" ), filePath );
    const QString uri = QgsProviderRegistry::instance()->encodeUri( providerKey, parts );

    Q_NOWARN_DEPRECATED_PUSH
    emit addVectorTileLayer( uri, QgsProviderUtils::suggestLayerNameFromFilePath( filePath ) );
    Q_NOWARN_DEPRECATED_POP
    emit addLayer( Qgis::LayerType::VectorTile, uri, cmbConnections->currentText(), QString() );
  }
}

void QgsVectorTileSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsVectorTileProviderConnection::connectionList() );
  cmbConnections->blockSignals( false );

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

void QgsVectorTileSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsVectorTileProviderConnection::selectedConnection();

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

void QgsVectorTileSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsVectorTileProviderConnection::setSelectedConnection( text );
  emit enableButtons( !text.isEmpty() );
}

void QgsVectorTileSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#using-vector-tiles-services" ) );
}

///@endcond
