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
#include "qgsvectortileconnection.h"
#include "qgsvectortileconnectiondialog.h"

#include <QFileDialog>
#include <QMessageBox>

///@cond PRIVATE

QgsVectorTileSourceSelect::QgsVectorTileSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  setWindowTitle( tr( "Add Vector Tile Layer" ) );
  mConnectionsGroupBox->setTitle( tr( "Vector Tile Connections" ) );

  QgsGui::enableAutoGeometryRestore( this );

  connect( btnNew, &QPushButton::clicked, this, &QgsVectorTileSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsVectorTileSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsVectorTileSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsVectorTileSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsVectorTileSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsVectorTileSourceSelect::cmbConnections_currentTextChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsVectorTileSourceSelect::showHelp );
  setupButtons( buttonBox );

  // disable help button until we got an entry in the docs
  buttonBox->button( QDialogButtonBox::Help )->setEnabled( false );

  populateConnectionList();
}

void QgsVectorTileSourceSelect::btnNew_clicked()
{
  QgsVectorTileConnectionDialog nc( this );
  if ( nc.exec() )
  {
    QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsVectorTileSourceSelect::btnEdit_clicked()
{
  QgsVectorTileConnectionDialog nc( this );
  QString uri = QgsVectorTileProviderConnection::encodedUri( QgsVectorTileProviderConnection::connection( cmbConnections->currentText() ) );
  nc.setConnection( cmbConnections->currentText(), uri );
  if ( nc.exec() )
  {
    QgsVectorTileProviderConnection::addConnection( nc.connectionName(), QgsVectorTileProviderConnection::decodedUri( nc.connectionUri() ) );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsVectorTileSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
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
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
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
  QString uri = QgsVectorTileProviderConnection::encodedUri( QgsVectorTileProviderConnection::connection( cmbConnections->currentText() ) );
  emit addVectorTileLayer( uri, cmbConnections->currentText() );
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
  QString toSelect = QgsVectorTileProviderConnection::selectedConnection();

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
}

///@endcond
