/***************************************************************************
                              qgsgeonodesourceselect.cpp
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Muhammad Yarjuna Rohmat, Ismail Sunni
    email                : rohmat at kartoza dot com, ismail at kartoza dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgslogger.h"
#include "qgsmessagelog.h"
#include "qgsproviderregistry.h"

#include "qgsgeonodesourceselect.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonoderequest.h"

#include "qgsgeonodenewconnection.h"
#include "qgsmanageconnectionsdialog.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QRegularExpression>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_NAME,
  MODEL_IDX_TYPE,
  MODEL_IDX_WEB_SERVICE
};

QgsGeoNodeSourceSelect::QgsGeoNodeSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsGeoNodeSourceSelect::showHelp );

  populateConnectionList();

  connect( btnNew, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::addConnectionsEntryList );
  connect( btnEdit, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::modifyConnectionsEntryList );
  connect( btnDelete, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::deleteConnectionsEntryList );
  connect( btnConnect, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::connectToGeonodeConnection );
  connect( btnSave, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::saveGeonodeConnection );
  connect( btnLoad, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::loadGeonodeConnection );
  connect( lineFilter, &QLineEdit::textChanged, this, &QgsGeoNodeSourceSelect::filterChanged );
  connect( treeView, &QTreeView::clicked, this, &QgsGeoNodeSourceSelect::treeViewSelectionChanged );

  mItemDelegate = new QgsGeonodeItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  mModel = new QStandardItemModel( this );
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( tr( "Title" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_NAME, new QStandardItem( tr( "Name" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_TYPE, new QStandardItem( tr( "Type" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_WEB_SERVICE, new QStandardItem( tr( "Web Service" ) ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );
}

QgsGeoNodeSourceSelect::~QgsGeoNodeSourceSelect()
{
  emit abortRequests();
}

void QgsGeoNodeSourceSelect::reset()
{
  treeView->clearSelection();
}

void QgsGeoNodeSourceSelect::addConnectionsEntryList()
{
  QgsGeoNodeNewConnection nc( this );

  if ( nc.exec() )
  {
    populateConnectionList( nc.name() );
    emit connectionsChanged();
  }
}

void QgsGeoNodeSourceSelect::modifyConnectionsEntryList()
{
  QgsGeoNodeNewConnection nc( this, cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify GeoNode Connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList( nc.name() );
    emit connectionsChanged();
  }
}

void QgsGeoNodeSourceSelect::deleteConnectionsEntryList()
{
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                      .arg( cmbConnections->currentText() );
  const QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Delete GeoNode Connection" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsGeoNodeConnectionUtils::deleteConnection( cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    if ( mModel )
    {
      mModel->removeRows( 0, mModel->rowCount() );
    }
    emit connectionsChanged();

    updateButtonStateForAvailableConnections();
  }
}

void QgsGeoNodeSourceSelect::populateConnectionList( const QString &selectedConnectionName )
{
  cmbConnections->clear();
  cmbConnections->addItems( QgsGeoNodeConnectionUtils::connectionList() );

  setConnectionListPosition( selectedConnectionName );
}

void QgsGeoNodeSourceSelect::setConnectionListPosition( const QString &selectedConnectionName )
{
  cmbConnections->setCurrentIndex( cmbConnections->findText( selectedConnectionName ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( selectedConnectionName.isEmpty() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }

  updateButtonStateForAvailableConnections();
}

void QgsGeoNodeSourceSelect::showHelp()
{
  //TODO - correct URL
  //QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#spatialite-layers" ) );
}

void QgsGeoNodeSourceSelect::connectToGeonodeConnection()
{
  const QgsGeoNodeConnection connection = currentConnection();

  const QString url = connection.uri().param( QStringLiteral( "url" ) );
  QgsGeoNodeRequest *geonodeRequest = new QgsGeoNodeRequest( url, true );
  connect( this, &QgsGeoNodeSourceSelect::abortRequests, geonodeRequest, &QgsGeoNodeRequest::abort );
  connect( geonodeRequest, &QgsGeoNodeRequest::requestFinished, geonodeRequest, [geonodeRequest]
  {
    QApplication::restoreOverrideCursor();
    geonodeRequest->deleteLater();
  } );
  connect( geonodeRequest, &QgsGeoNodeRequest::layersFetched, this, [ = ]( const QList< QgsGeoNodeRequest::ServiceLayerDetail > layers )
  {
    if ( !layers.empty() )
    {
      QgsDebugMsg( QStringLiteral( "Success, non empty layers %1" ).arg( layers.count( ) ) );
    }
    else
    {
      QgsMessageLog::logMessage( QStringLiteral( "Failed, empty layers" ), tr( "GeoNode" ) );
    }

    if ( mModel )
    {
      mModel->removeRows( 0, mModel->rowCount() );
    }

    if ( !layers.isEmpty() )
    {
      for ( const QgsGeoNodeRequest::ServiceLayerDetail &layer : layers )
      {
        const QUuid uuid = layer.uuid;

        const QString wmsURL = layer.wmsURL;
        const QString wfsURL = layer.wfsURL;
        const QString wcsURL = layer.wcsURL;
        const QString xyzURL = layer.xyzURL;

        if ( !wmsURL.isEmpty() )
        {
          QStandardItem *titleItem = new QStandardItem( layer.title );
          QStandardItem *nameItem = nullptr;
          if ( !layer.name.isEmpty() )
          {
            nameItem = new QStandardItem( layer.name );
          }
          else
          {
            nameItem = new QStandardItem( layer.title );
          }
          QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
          QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WMS" ) );

          const QString typeName = layer.typeName;

          titleItem->setData( uuid,  Qt::UserRole + 1 );
          titleItem->setData( wmsURL,  Qt::UserRole + 2 );
          titleItem->setData( typeName,  Qt::UserRole + 3 );
          typedef QList< QStandardItem * > StandardItemList;
          mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer %1 does not have WMS url." ).arg( layer.title ), 3 );
        }
        if ( !wfsURL.isEmpty() )
        {
          QStandardItem *titleItem = new QStandardItem( layer.title );
          QStandardItem *nameItem = nullptr;
          if ( !layer.name.isEmpty() )
          {
            nameItem = new QStandardItem( layer.name );
          }
          else
          {
            nameItem = new QStandardItem( layer.title );
          }
          QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
          QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WFS" ) );

          const QString typeName = layer.typeName;

          titleItem->setData( uuid,  Qt::UserRole + 1 );
          titleItem->setData( wfsURL,  Qt::UserRole + 2 );
          titleItem->setData( typeName,  Qt::UserRole + 3 );
          typedef QList< QStandardItem * > StandardItemList;
          mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer %1 does not have WFS url." ).arg( layer.title ), 3 );
        }

        if ( !wcsURL.isEmpty() )
        {
          QStandardItem *titleItem = new QStandardItem( layer.title );
          QStandardItem *nameItem = nullptr;
          if ( !layer.name.isEmpty() )
          {
            nameItem = new QStandardItem( layer.name );
          }
          else
          {
            nameItem = new QStandardItem( layer.title );
          }
          QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
          QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WCS" ) );

          const QString typeName = layer.typeName;

          titleItem->setData( uuid,  Qt::UserRole + 1 );
          titleItem->setData( wcsURL,  Qt::UserRole + 2 );
          titleItem->setData( typeName,  Qt::UserRole + 3 );

          typedef QList< QStandardItem * > StandardItemList;
          mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer %1 does not have WCS url." ).arg( layer.title ), 3 );
        }

        if ( !xyzURL.isEmpty() )
        {
          QStandardItem *titleItem = new QStandardItem( layer.title );
          QStandardItem *nameItem = nullptr;
          if ( !layer.name.isEmpty() )
          {
            nameItem = new QStandardItem( layer.name );
          }
          else
          {
            nameItem = new QStandardItem( layer.title );
          }
          QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
          QStandardItem *webServiceTypeItem = new QStandardItem( tr( "XYZ" ) );

          const QString typeName = layer.typeName;

          titleItem->setData( uuid,  Qt::UserRole + 1 );
          titleItem->setData( xyzURL,  Qt::UserRole + 2 );
          titleItem->setData( typeName,  Qt::UserRole + 3 );
          typedef QList< QStandardItem * > StandardItemList;
          mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
        }
        else
        {
          QgsDebugMsgLevel( QStringLiteral( "Layer %1 does not have XYZ url." ).arg( layer.title ), 3 );
        }
      }
    }

    else
    {
      QMessageBox::critical( this, tr( "Connect to GeoNode" ), tr( "Cannot get any feature services." ) );
    }

    treeView->resizeColumnToContents( MODEL_IDX_TITLE );
    treeView->resizeColumnToContents( MODEL_IDX_NAME );
    treeView->resizeColumnToContents( MODEL_IDX_TYPE );
    treeView->resizeColumnToContents( MODEL_IDX_WEB_SERVICE );
    for ( int i = MODEL_IDX_TITLE; i < MODEL_IDX_WEB_SERVICE; i++ )
    {
      if ( treeView->columnWidth( i ) > 210 )
      {
        treeView->setColumnWidth( i, 210 );
      }
    }
  } );

  QApplication::setOverrideCursor( Qt::BusyCursor );
  geonodeRequest->fetchLayers();
}

void QgsGeoNodeSourceSelect::saveGeonodeConnection()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::GeoNode );
  dlg.exec();
}

void QgsGeoNodeSourceSelect::loadGeonodeConnection()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::GeoNode, fileName );
  dlg.exec();
  populateConnectionList();
  emit connectionsChanged();
}

void QgsGeoNodeSourceSelect::filterChanged( const QString &text )
{
  const QRegularExpression regExp( text, QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( regExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

void QgsGeoNodeSourceSelect::treeViewSelectionChanged()
{
  const QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }
  addButton()->setEnabled( false );
  QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < modelIndexList.size(); i++ )
  {
    const QModelIndex idx = mModelProxy->mapToSource( modelIndexList[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    const int row = idx.row();
    const QString typeItem = mModel->item( row, MODEL_IDX_TYPE )->text();
    if ( typeItem == tr( "Layer" ) )
    {
      // Enable if there is a layer selected
      addButton()->setEnabled( true );
      return;
    }
  }

}

void QgsGeoNodeSourceSelect::addButtonClicked()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  // Get selected entry in treeview
  const QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  const QgsGeoNodeConnection connection = currentConnection();

  QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < modelIndexList.size(); i++ )
  {
    const QModelIndex idx = mModelProxy->mapToSource( modelIndexList[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    const int row = idx.row();

    const QString typeItem = mModel->item( row, MODEL_IDX_TYPE )->text();
    if ( typeItem == tr( "Map" ) )
    {
      continue;
    }
    const QString serviceURL = mModel->item( row, MODEL_IDX_TITLE )->data( Qt::UserRole + 2 ).toString();
    const QString titleName = mModel->item( row, MODEL_IDX_TITLE )->text();
    QString layerName = mModel->item( row, MODEL_IDX_NAME )->text();
    const QString webServiceType = mModel->item( row, MODEL_IDX_WEB_SERVICE )->text();

    if ( cbxUseTitleLayerName->isChecked() && !titleName.isEmpty() )
    {
      layerName = titleName;
    }

    if ( webServiceType == QLatin1String( "WMS" ) )
    {
      QgsDataSourceUri uri;
      uri.setParam( QStringLiteral( "url" ), serviceURL );

      // Set static first, to see that it works. Need to think about the UI also.
      const QString format( QStringLiteral( "image/png" ) );
      const QString crs( QStringLiteral( "EPSG:4326" ) );
      const QString styles;
      const QString contextualWMSLegend( QStringLiteral( "0" ) );

      connection.addWmsConnectionSettings( uri );

      uri.setParam( QStringLiteral( "contextualWMSLegend" ), contextualWMSLegend );
      uri.setParam( QStringLiteral( "layers" ), layerName );
      uri.setParam( QStringLiteral( "styles" ), styles );
      uri.setParam( QStringLiteral( "format" ), format );
      uri.setParam( QStringLiteral( "crs" ), crs );

      QgsDebugMsg( "Add WMS from GeoNode : " + uri.encodedUri() );
      emit addRasterLayer( uri.encodedUri(), layerName, QStringLiteral( "wms" ) );
    }
    else if ( webServiceType == QLatin1String( "WCS" ) )
    {
      QgsDataSourceUri uri;
      const QString typeName = mModel->item( row, 0 )->data( Qt::UserRole + 3 ).toString();
      uri.setParam( QStringLiteral( "url" ), serviceURL );

      connection.addWcsConnectionSettings( uri );
      uri.setParam( QStringLiteral( "identifier" ), typeName );

      QgsDebugMsg( "Add WCS from GeoNode : " + uri.encodedUri() );
      emit addRasterLayer( uri.encodedUri(), layerName, QStringLiteral( "wcs" ) );
    }
    else if ( webServiceType == QLatin1String( "WFS" ) )
    {
      // Set static first, to see that it works. Need to think about the UI also.
      const QString typeName = mModel->item( row, 0 )->data( Qt::UserRole + 3 ).toString();
      const QString crs( QStringLiteral( "EPSG:4326" ) );

      // typeName, titleName, sql,
      // Build url for WFS
      // restrictToRequestBBOX='1' srsname='EPSG:26719' typename='geonode:cab_mun' url='http://demo.geonode.org/geoserver/geonode/wms' table=\"\" sql="
      QgsDataSourceUri uri;

      uri.setParam( QStringLiteral( "restrictToRequestBBOX" ), QStringLiteral( "1" ) );
      uri.setParam( QStringLiteral( "srsname" ), crs );
      if ( serviceURL.contains( QStringLiteral( "qgis-server" ) ) )
      {
        // I need to do this since the typename used in qgis-server is without the workspace.
        const QString qgisServerTypeName = QString( typeName ).split( ':' ).last();
        uri.setParam( QStringLiteral( "typename" ), qgisServerTypeName );
      }
      else
      {
        uri.setParam( QStringLiteral( "typename" ), typeName );
      }
      uri.setParam( QStringLiteral( "url" ), serviceURL );
      //uri.setParam( QStringLiteral( "table" ), QStringLiteral( "\"\"" ) );
      //uri.setParam( QStringLiteral( "sql" ), QString() );

      connection.addWfsConnectionSettings( uri );

      QgsDebugMsg( "Add WFS from GeoNode : " + uri.uri() + " and typename: " + typeName );
      emit addVectorLayer( uri.uri(), typeName, QStringLiteral( "WFS" ) );
    }
    else if ( webServiceType == QLatin1String( "XYZ" ) )
    {
      QgsDebugMsg( "XYZ Url: " + serviceURL );
      QgsDebugMsg( "Add XYZ from GeoNode : " + serviceURL );
      QgsDataSourceUri uri;
      uri.setParam( QStringLiteral( "url" ), serviceURL );
      uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
      uri.setParam( QStringLiteral( "zmin" ), QStringLiteral( "0" ) );
      uri.setParam( QStringLiteral( "zmax" ), QStringLiteral( "18" ) );
      emit addRasterLayer( uri.encodedUri(), layerName, QStringLiteral( "wms" ) );
    }
  }

  QApplication::restoreOverrideCursor();
}

void QgsGeoNodeSourceSelect::updateButtonStateForAvailableConnections()
{
  const bool connectionsAvailable = cmbConnections->count() > 0;
  btnConnect->setEnabled( connectionsAvailable );
  btnEdit->setEnabled( connectionsAvailable );
  btnDelete->setEnabled( connectionsAvailable );
  btnSave->setEnabled( connectionsAvailable );
}

QgsGeoNodeConnection QgsGeoNodeSourceSelect::currentConnection() const
{
  return QgsGeoNodeConnection( cmbConnections->currentText() );
}
