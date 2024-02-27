/***************************************************************************
                         qgssensorthingssourceselect.cpp
                         ---------------------------------
    Date                 : December 2023
    Copyright            : (C) 2023 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include "qgssensorthingssourceselect.h"
#include "qgssensorthingsconnection.h"
#include "qgssensorthingsconnectionwidget.h"
#include "qgssensorthingsconnectiondialog.h"
#include "qgssensorthingssourcewidget.h"
#include "qgssensorthingsprovider.h"
#include "qgssensorthingssubseteditor.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QPushButton>

///@cond PRIVATE

QgsSensorThingsSourceSelect::QgsSensorThingsSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  setWindowTitle( tr( "Add SensorThings Layer" ) );
  mConnectionsGroupBox->setTitle( tr( "SensorThings Connections" ) );

  mConnectionWidget = new QgsSensorThingsConnectionWidget();
  QVBoxLayout *vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );
  vlayout->addWidget( mConnectionWidget );
  mConnectionContainerWidget->setLayout( vlayout );

  mSourceWidget = new QgsSensorThingsSourceWidget();
  vlayout = new QVBoxLayout();
  vlayout->setContentsMargins( 0, 0, 0, 0 );
  vlayout->addWidget( mSourceWidget );
  mLayerSettingsContainerWidget->setLayout( vlayout );

  txtSubsetSQL->setWrapMode( QgsCodeEditor::WrapWord );
  connect( pbnQueryBuilder, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::buildFilter );

  connect( mSourceWidget, &QgsProviderSourceWidget::validChanged, this, &QgsSensorThingsSourceSelect::validate );
  connect( mConnectionWidget, &QgsSensorThingsConnectionWidget::validChanged, this, &QgsSensorThingsSourceSelect::validate );

  connect( mConnectionWidget, &QgsSensorThingsConnectionWidget::changed, this, [this]
  {
    if ( mBlockChanges )
      return;

    mBlockChanges++;
    cmbConnections->setCurrentIndex( cmbConnections->findData( QStringLiteral( "~~custom~~" ) ) );
    mSourceWidget->setSourceUri( mConnectionWidget->sourceUri() );
    mBlockChanges--;

  } );

  QgsGui::enableAutoGeometryRestore( this );

  connect( btnNew, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsSensorThingsSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsSensorThingsSourceSelect::cmbConnections_currentTextChanged );
#if 0 // TODO when documentation page is available
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsSensorThingsSourceSelect::showHelp );
#endif
  setupButtons( buttonBox );
  populateConnectionList();
}

void QgsSensorThingsSourceSelect::btnNew_clicked()
{
  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );
  QgsSensorThingsConnectionDialog nc( this );
  if ( isCustom )
  {
    // when creating a new connection, default to the current connection parameters
    nc.connectionWidget()->setSourceUri( mConnectionWidget->sourceUri() );
  }
  if ( nc.exec() )
  {
    QgsSensorThingsProviderConnection::Data connectionData = QgsSensorThingsProviderConnection::decodedUri( nc.connectionUri() );

    QgsSensorThingsProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsSensorThingsSourceSelect::btnEdit_clicked()
{
  const QString oldName = cmbConnections->currentText();
  const QgsSensorThingsProviderConnection::Data connection = QgsSensorThingsProviderConnection::connection( oldName );
  const QString uri = QgsSensorThingsProviderConnection::encodedUri( connection );

  QgsSensorThingsConnectionDialog nc( this );
  nc.setConnection( oldName, uri );
  if ( nc.exec() )
  {
    QgsSensorThingsProviderConnection::Data connectionData = QgsSensorThingsProviderConnection::decodedUri( nc.connectionUri() );

    QgsSensorThingsProviderConnection( QString() ).remove( oldName );

    QgsSensorThingsProviderConnection::addConnection( nc.connectionName(), connectionData );
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsSensorThingsSourceSelect::btnDelete_clicked()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                      .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsSensorThingsProviderConnection( QString() ).remove( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsSensorThingsSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::SensorThings );
  dlg.exec();
}

void QgsSensorThingsSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::SensorThings, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsSensorThingsSourceSelect::addButtonClicked()
{
  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );

  const QString providerUri = mConnectionWidget->sourceUri();
  QString layerUri = mSourceWidget->updateUriFromGui( providerUri );

  QVariantMap uriParts = QgsProviderRegistry::instance()->decodeUri(
                           QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
                           layerUri
                         );

  if ( !txtSubsetSQL->text().isEmpty() )
    uriParts.insert( QStringLiteral( "sql" ), txtSubsetSQL->text() );

  layerUri = QgsProviderRegistry::instance()->encodeUri(
               QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY,
               uriParts
             );

  const Qgis::SensorThingsEntity type = QgsSensorThingsUtils::stringToEntity( uriParts.value( QStringLiteral( "entity" ) ).toString() );

  QString baseName;
  if ( QgsSensorThingsUtils::entityTypeHasGeometry( type ) )
  {
    const QString geometryType = uriParts.value( QStringLiteral( "geometryType" ) ).toString();
    QString geometryNamePart;
    if ( geometryType.compare( QLatin1String( "point" ), Qt::CaseInsensitive ) == 0 ||
         geometryType.compare( QLatin1String( "multipoint" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Points" );
    }
    else if ( geometryType.compare( QLatin1String( "line" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Lines" );
    }
    else if ( geometryType.compare( QLatin1String( "polygon" ), Qt::CaseInsensitive ) == 0 )
    {
      geometryNamePart = tr( "Polygons" );
    }

    if ( !geometryNamePart.isEmpty() )
    {
      baseName = QStringLiteral( "%1 - %2 (%3)" ).arg( isCustom ? tr( "SensorThings" ) : cmbConnections->currentText(),
                 QgsSensorThingsUtils::displayString( type, true ),
                 geometryNamePart );
    }
    else
    {
      baseName = QStringLiteral( "%1 - %2" ).arg( isCustom ? tr( "SensorThings" ) : cmbConnections->currentText(),
                 QgsSensorThingsUtils::displayString( type, true ) );
    }
  }
  else
  {
    baseName = QStringLiteral( "%1 - %2" ).arg( isCustom ? tr( "SensorThings" ) : cmbConnections->currentText(),
               QgsSensorThingsUtils::displayString( type, true ) );
  }

  Q_NOWARN_DEPRECATED_PUSH
  emit addVectorLayer( layerUri, baseName, QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY );
  Q_NOWARN_DEPRECATED_POP

  emit addLayer( Qgis::LayerType::Vector, layerUri, baseName, QgsSensorThingsProvider::SENSORTHINGS_PROVIDER_KEY );
}

void QgsSensorThingsSourceSelect::setMapCanvas( QgsMapCanvas *mapCanvas )
{
  QgsAbstractDataSourceWidget::setMapCanvas( mapCanvas );
  mSourceWidget->setMapCanvas( mapCanvas );
}

void QgsSensorThingsSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItem( tr( "Custom" ), QStringLiteral( "~~custom~~" ) );
  cmbConnections->addItems( QgsSensorThingsProviderConnection::connectionList() );
  cmbConnections->blockSignals( false );

  btnSave->setDisabled( cmbConnections->count() == 1 );

  setConnectionListPosition();
}

void QgsSensorThingsSourceSelect::setConnectionListPosition()
{
  const QString toSelect = QgsSensorThingsProviderConnection::selectedConnection();

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

void QgsSensorThingsSourceSelect::cmbConnections_currentTextChanged( const QString &text )
{
  QgsSensorThingsProviderConnection::setSelectedConnection( text );

  const bool isCustom = cmbConnections->currentData().toString() == QLatin1String( "~~custom~~" );
  btnEdit->setDisabled( isCustom );
  btnDelete->setDisabled( isCustom );

  if ( !mBlockChanges )
  {
    mBlockChanges++;
    if ( isCustom )
    {
      mConnectionWidget->setSourceUri( QString() );
    }
    else
    {
      mConnectionWidget->setSourceUri( QgsSensorThingsProviderConnection::encodedLayerUri( QgsSensorThingsProviderConnection::connection( cmbConnections->currentText() ) ) );
      mSourceWidget->setSourceUri( mConnectionWidget->sourceUri() );
    }
    mBlockChanges--;
  }
}

void QgsSensorThingsSourceSelect::buildFilter()
{
  const QgsFields fields = QgsSensorThingsUtils::fieldsForEntityType( mSourceWidget->currentEntityType() );
  QgsSensorThingsSubsetEditor subsetEditor( nullptr, fields );
  subsetEditor.setSubsetString( txtSubsetSQL->text( ) );
  if ( subsetEditor.exec() )
  {
    txtSubsetSQL->setText( subsetEditor.subsetString() );
  }
}

void QgsSensorThingsSourceSelect::validate()
{
  const bool isValid = mConnectionWidget->isValid() && mSourceWidget->isValid();
  emit enableButtons( isValid );
}

#if 0 // TODO
void QgsSensorThingsSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#using-xyz-tile-services" ) );
}
#endif

///@endcond PRIVATE
