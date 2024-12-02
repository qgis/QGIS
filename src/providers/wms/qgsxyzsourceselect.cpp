/***************************************************************************
                         qgsxyzsourceselect.cpp
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
#include "qgsxyzsourceselect.h"
#include "moc_qgsxyzsourceselect.cpp"
#include "qgsxyzconnection.h"
#include "qgsxyzconnectiondialog.h"
#include "qgsowsconnection.h"
#include "qgsxyzsourcewidget.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

QgsXyzSourceSelect::QgsXyzSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  setWindowTitle( tr( "Add XYZ Layer" ) );
  mConnectionsGroupBox->setTitle( tr( "XYZ Connections" ) );

  mSourceWidget = new QgsXyzSourceWidget();
  QHBoxLayout *hlayout = new QHBoxLayout();
  hlayout->setContentsMargins( 0, 0, 0, 0 );
  hlayout->addWidget( mSourceWidget );
  mSourceContainerWidget->setLayout( hlayout );

  connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, &QgsXyzSourceSelect::enableButtons );
  connect( mSourceWidget, &QgsProviderSourceWidget::changed, this, [this] {
    if ( mBlockChanges )
      return;

    mBlockChanges++;
    cmbConnections->setCurrentIndex( cmbConnections->findData( QStringLiteral( "~~custom~~" ) ) );
    mBlockChanges--;
  } );

  QgsGui::enableAutoGeometryRestore( this );

  connect( btnNew, &QPushButton::clicked, this, &QgsXyzSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsXyzSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsXyzSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsXyzSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsXyzSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsXyzSourceSelect::cmbConnections_currentTextChanged );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsXyzSourceSelect::showHelp );
  setupButtons( buttonBox );
  populateConnectionList();
}

void QgsXyzSourceSelect::btnNew_clicked()
{
  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );

  QgsXyzConnectionDialog nc( this );
  if ( isCustom )
  {
    // when creating a new connection, default to the current connection parameters
    nc.sourceWidget()->setSourceUri( mSourceWidget->sourceUri() );
  }
  if ( nc.exec() )
  {
    QgsXyzConnectionUtils::addConnection( nc.connection() );

    QgsXyzConnectionSettings::sTreeXyzConnections->setSelectedItem( nc.connection().name );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsXyzSourceSelect::btnEdit_clicked()
{
  QgsXyzConnectionDialog nc( this );
  nc.setConnection( QgsXyzConnectionUtils::connection( cmbConnections->currentText() ) );
  if ( nc.exec() )
  {
    QgsXyzConnectionUtils::addConnection( nc.connection() );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsXyzSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                        .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsXyzConnectionUtils::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsXyzSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::XyzTiles );
  dlg.exec();
}

void QgsXyzSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::XyzTiles, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsXyzSourceSelect::addButtonClicked()
{
  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );
  Q_NOWARN_DEPRECATED_PUSH
  emit addRasterLayer( mSourceWidget->sourceUri(), isCustom ? tr( "XYZ Layer" ) : cmbConnections->currentText(), QStringLiteral( "wms" ) );
  Q_NOWARN_DEPRECATED_POP
  emit addLayer( Qgis::LayerType::Raster, mSourceWidget->sourceUri(), isCustom ? tr( "XYZ Layer" ) : cmbConnections->currentText(), QStringLiteral( "wms" ) );
}

void QgsXyzSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItem( tr( "Custom" ), QStringLiteral( "~~custom~~" ) );
  cmbConnections->addItems( QgsXyzConnectionUtils::connectionList() );
  cmbConnections->blockSignals( false );

  btnSave->setDisabled( cmbConnections->count() == 1 );

  setConnectionListPosition();
}

void QgsXyzSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsXyzConnectionSettings::sTreeXyzConnections->selectedItem();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }

  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );
  btnEdit->setDisabled( isCustom );
  btnDelete->setDisabled( isCustom );
}

void QgsXyzSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsXyzConnectionSettings::sTreeXyzConnections->setSelectedItem( text );

  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );
  btnEdit->setDisabled( isCustom );
  btnDelete->setDisabled( isCustom );

  if ( !mBlockChanges )
  {
    mBlockChanges++;
    if ( isCustom )
    {
      mSourceWidget->setSourceUri( QString() );
    }
    else
    {
      mSourceWidget->setSourceUri( QgsXyzConnectionUtils::connection( cmbConnections->currentText() ).encodedUri() );
    }
    mBlockChanges--;
  }
}

void QgsXyzSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#using-xyz-tile-services" ) );
}
