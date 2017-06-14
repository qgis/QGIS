/***************************************************************************
                              qgsgeonodesourceselect.cpp
                              -------------------
    begin                : Feb 2017
    copyright            : (C) 2017 by Rohmat, Ismail Sunni
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

#include "qgsgeonodesourceselect.h"
#include "qgsgeonodeconnection.h"
#include "qgsgeonodenewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgslogger.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_NAME,
  MODEL_IDX_TYPE,
  MODEL_IDX_WEB_SERVICE
};

QgsGeoNodeSourceSelect::QgsGeoNodeSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool embeddedMode )
  : QDialog( parent, fl )
{
  setupUi( this );

  if ( embeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );

  populateConnectionList();

  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsGeoNodeSourceSelect::reject );
  connect( btnNew, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::addConnectionsEntryList );
  connect( btnEdit, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::modifyConnectionsEntryList );
  connect( btnDelete, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::deleteConnectionsEntryList );
  connect( btnConnect, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::connectToGeonodeConnection );
  connect( btnSave, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::saveGeonodeConnection );
  connect( btnLoad, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::loadGeonodeConnection );
  connect( lineFilter, &QLineEdit::textChanged, this, &QgsGeoNodeSourceSelect::filterChanged );
  connect( treeView, &QTreeView::clicked, this, &QgsGeoNodeSourceSelect::treeViewSelectionChanged );
  connect( mAddButton, &QPushButton::clicked, this, &QgsGeoNodeSourceSelect::addButtonClicked );

  mItemDelegate = new QgsGeonodeItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( tr( "Title" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_NAME, new QStandardItem( tr( "Name" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_TYPE, new QStandardItem( tr( "Type" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_WEB_SERVICE, new QStandardItem( tr( "Web Service" ) ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );
}

QgsGeoNodeSourceSelect::~QgsGeoNodeSourceSelect() {}

void QgsGeoNodeSourceSelect::addConnectionsEntryList()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( this );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsGeoNodeSourceSelect::modifyConnectionsEntryList()
{
  QgsGeoNodeNewConnection *nc = new QgsGeoNodeNewConnection( this, cmbConnections->currentText() );
  nc->setWindowTitle( tr( "Modify GeoNode connection" ) );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsGeoNodeSourceSelect::deleteConnectionsEntryList()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsGeoNodeConnection::deleteConnection( cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    if ( mModel )
    {
      mModel->removeRows( 0, mModel->rowCount() );
    }
    emit connectionsChanged();

    if ( cmbConnections->count() > 0 )
    {
      // Connections available - enable various buttons
      btnConnect->setEnabled( true );
      btnEdit->setEnabled( true );
      btnDelete->setEnabled( true );
      btnSave->setEnabled( true );
    }
    else
    {
      // No connections available - disable various buttons
      btnConnect->setEnabled( false );
      btnEdit->setEnabled( false );
      btnDelete->setEnabled( false );
      btnSave->setEnabled( false );
    }
  }
}

void QgsGeoNodeSourceSelect::populateConnectionList()
{
  cmbConnections->clear();
  cmbConnections->addItems( QgsGeoNodeConnection::connectionList() );

  setConnectionListPosition();
}

void QgsGeoNodeSourceSelect::setConnectionListPosition()
{
  QString toSelect = QgsGeoNodeConnection::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }

  if ( cmbConnections->count() == 0 )
  {
    // No connections - disable various buttons
    btnConnect->setEnabled( false );
    btnEdit->setEnabled( false );
    btnDelete->setEnabled( false );
    btnSave->setEnabled( false );
  }
  else
  {
    // Connections - enable various buttons
    btnConnect->setEnabled( true );
    btnEdit->setEnabled( true );
    btnDelete->setEnabled( true );
    btnSave->setEnabled( true );
  }
}

void QgsGeoNodeSourceSelect::connectToGeonodeConnection()
{
  QApplication::setOverrideCursor( Qt::BusyCursor );
  QgsGeoNodeConnection connection( cmbConnections->currentText() );

  QVariantList layers = connection.getLayers();

//  QVariantList validWMSLayer = connection.getLayers( QStringLiteral( "WMS" ) );
//  QVariantList validWFSLayer = connection.getLayers( QStringLiteral( "WFS" ) );

//  QVariantList validWMSLayerNames = QVariantList();
//  Q_FOREACH ( const QVariant &wmsLayer, validWMSLayer )
//  {
//    validWMSLayerNames.append( wmsLayer.toMap()["name"] );
//  }
//  QVariantList validWFSLayerNames = QVariantList();
//  Q_FOREACH ( const QVariant &wfsLayer, validWFSLayer )
//  {
//    validWFSLayerNames.append( wfsLayer.toMap()["name"] );
//  }

  if ( mModel )
  {
    mModel->removeRows( 0, mModel->rowCount() );
  }

  if ( !layers.isEmpty() )
  {
    Q_FOREACH ( const QVariant &layer, layers )
    {
      QString uuid = layer.toMap()["uuid"].toString();

      QString layerName = layer.toMap()["name"].toString();

      QString wmsURL = layer.toMap()["wms"].toString();
      QString wfsURL = layer.toMap()["wfs"].toString();
      QString xyzURL = layer.toMap()["xyz"].toString();

      if ( wmsURL.length() > 0 )
      {
        QStandardItem *titleItem = new QStandardItem( layer.toMap()["title"].toString() );
        QStandardItem *nameItem;
        if ( layer.toMap()["name"].toString().length() > 0 )
        {
          nameItem = new QStandardItem( layer.toMap()["name"].toString() );
        }
        else
        {
          nameItem = new QStandardItem( layer.toMap()["title"].toString() );
        }
        QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
        QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WMS" ) );

        QString typeName = layer.toMap()["typename"].toString();

        titleItem->setData( uuid,  Qt::UserRole + 1 );
        titleItem->setData( wmsURL,  Qt::UserRole + 2 );
        titleItem->setData( typeName,  Qt::UserRole + 3 );
        typedef QList< QStandardItem * > StandardItemList;
        mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
      }
      else
      {
        qDebug() << "Layer " << layer.toMap()["title"].toString() << " does not have WMS url.";
      }
      if ( wfsURL.length() > 0 )
      {
        QStandardItem *titleItem = new QStandardItem( layer.toMap()["title"].toString() );
        QStandardItem *nameItem;
        if ( layer.toMap()["name"].toString().length() > 0 )
        {
          nameItem = new QStandardItem( layer.toMap()["name"].toString() );
        }
        else
        {
          nameItem = new QStandardItem( layer.toMap()["title"].toString() );
        }
        QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
        QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WFS" ) );

        QString typeName = layer.toMap()["typename"].toString();

        titleItem->setData( uuid,  Qt::UserRole + 1 );
        titleItem->setData( wfsURL,  Qt::UserRole + 2 );
        titleItem->setData( typeName,  Qt::UserRole + 3 );
        typedef QList< QStandardItem * > StandardItemList;
        mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
      }
      else
      {
        qDebug() << "Layer " << layer.toMap()["title"].toString() << " does not have WFS url.";
      }
      if ( xyzURL.length() > 0 )
      {
        QStandardItem *titleItem = new QStandardItem( layer.toMap()["title"].toString() );
        QStandardItem *nameItem;
        if ( layer.toMap()["name"].toString().length() > 0 )
        {
          nameItem = new QStandardItem( layer.toMap()["name"].toString() );
        }
        else
        {
          nameItem = new QStandardItem( layer.toMap()["title"].toString() );
        }
        QStandardItem *serviceTypeItem = new QStandardItem( tr( "Layer" ) );
        QStandardItem *webServiceTypeItem = new QStandardItem( tr( "XYZ" ) );

        QString typeName = layer.toMap()["typename"].toString();

        titleItem->setData( uuid,  Qt::UserRole + 1 );
        titleItem->setData( xyzURL,  Qt::UserRole + 2 );
        titleItem->setData( typeName,  Qt::UserRole + 3 );
        typedef QList< QStandardItem * > StandardItemList;
        mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
      }
      else
      {
        qDebug() << "Layer " << layer.toMap()["title"].toString() << " does not have XYZ url.";
      }
    }
  }

  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Cannot get any feature services" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( QStringLiteral( "GeonodeCapabilitiesErrorBox" ) );
    box->open();
  }

  // Currently we don't have obvious way to add Map, comment these lines until the time comes.
//  QVariantList maps = connection.getMaps();
//  if ( !maps.isEmpty() )
//  {
//    Q_FOREACH ( const QVariant &map, maps )
//    {
//      QStandardItem *titleItem = new QStandardItem( map.toMap()["title"].toString() );
//      QStandardItem *nameItem = new QStandardItem( "-" );
//      QStandardItem *serviceTypeItem = new QStandardItem( tr( "Map" ) );
//      QStandardItem *webServiceTypeItem = new QStandardItem( tr( "WMS" ) );
//      typedef QList< QStandardItem * > StandardItemList;
//      mModel->appendRow( StandardItemList() << titleItem << nameItem << serviceTypeItem << webServiceTypeItem );
//    }
//  }

//  else
//  {
//    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Cannot get any map services" ), QMessageBox::Ok, this );
//    box->setAttribute( Qt::WA_DeleteOnClose );
//    box->setModal( true );
//    box->setObjectName( QStringLiteral( "GeonodeCapabilitiesErrorBox" ) );
//    box->open();
//  }

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
  QApplication::restoreOverrideCursor();
}

void QgsGeoNodeSourceSelect::saveGeonodeConnection()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::GeoNode );
  dlg.exec();
}

void QgsGeoNodeSourceSelect::loadGeonodeConnection()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *XML)" ) );
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
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

void QgsGeoNodeSourceSelect::treeViewSelectionChanged()
{
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    qDebug() << "Current index is invalid";
    return;
  }
  mAddButton->setEnabled( false );
  QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < modelIndexList.size(); i++ )
  {
    QModelIndex idx = mModelProxy->mapToSource( modelIndexList[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    int row = idx.row();
    QString typeItem = mModel->item( row, MODEL_IDX_TYPE )->text();
    if ( typeItem == tr( "Layer" ) )
    {
      // Enable if there is a layer selected
      mAddButton->setEnabled( true );
      return;
    }
  }

}

void QgsGeoNodeSourceSelect::addButtonClicked()
{
  qDebug() << "Add button clicked";
  QApplication::setOverrideCursor( Qt::BusyCursor );
  // Get selected entry in treeview
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    qDebug() << "Current index is invalid";
    return;
  }

  QgsGeoNodeConnection connection( cmbConnections->currentText() );
  QModelIndexList modelIndexList = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < modelIndexList.size(); i++ )
  {
    QModelIndex idx = mModelProxy->mapToSource( modelIndexList[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    int row = idx.row();

    qDebug() << "Model index row " << row;

    QString typeItem = mModel->item( row, MODEL_IDX_TYPE )->text();
    if ( typeItem == tr( "Map" ) )
    {
      qDebug() << "Skip adding map.";
      continue;
    }
    QString serviceURL = mModel->item( row, MODEL_IDX_TITLE )->data( Qt::UserRole + 2 ).toString();
    QString titleName = mModel->item( row, MODEL_IDX_TITLE )->text();
    QString layerName = mModel->item( row, MODEL_IDX_NAME )->text();
    QString webServiceType = mModel->item( row, MODEL_IDX_WEB_SERVICE )->text();

    if ( cbxUseTitleLayerName->isChecked() && !titleName.isEmpty() )
    {
      QString layerName = titleName;
    }

    qDebug() << "Layer name: " << layerName << " Type: " << webServiceType;

    if ( webServiceType == "WMS" )
    {
      qDebug() << "Adding WMS layer of " << layerName;
      QgsDataSourceUri uri;
      uri.setParam( QStringLiteral( "url" ), serviceURL );

      // Set static first, to see that it works. Need to think about the UI also.
      QString format( "image/png" );
      QString crs( "EPSG:4326" );
      QString styles( "" );
      QString contextualWMSLegend( "0" );

      uri.setParam( QStringLiteral( "contextualWMSLegend" ), contextualWMSLegend );
      uri.setParam( QStringLiteral( "layers" ), layerName );
      uri.setParam( QStringLiteral( "styles" ), styles );
      uri.setParam( QStringLiteral( "format" ), format );
      uri.setParam( QStringLiteral( "crs" ), crs );

      QgsDebugMsg( "Add WMS from GeoNode : " + uri.encodedUri() );
      emit addRasterLayer( uri.encodedUri(), layerName, QStringLiteral( "wms" ) );
    }
    else if ( webServiceType == "WFS" )
    {
      qDebug() << "Adding WFS layer of " << layerName;

      // Set static first, to see that it works. Need to think about the UI also.
      QString typeName = mModel->item( row, 0 )->data( Qt::UserRole + 3 ).toString();
      QString crs( "EPSG:4326" );

      // typeName, titleName, sql,
      // Build url for WFS
      // restrictToRequestBBOX='1' srsname='EPSG:26719' typename='geonode:cab_mun' url='http://demo.geonode.org/geoserver/geonode/wms' table=\"\" sql="
      QString uri;
      uri += QStringLiteral( " srsname='%1'" ).arg( crs ) ;
      uri += QStringLiteral( " typename='%1'" ).arg( typeName ) ;
      uri += QStringLiteral( " url='%1'" ).arg( serviceURL );

      QgsDebugMsg( "Add WFS from GeoNode : " + uri );
      emit addWfsLayer( uri, typeName );
    }
    else if ( webServiceType == "XYZ" )
    {
      QgsDebugMsg( "XYZ Url: " + serviceURL );
      QgsDebugMsg( "Add XYZ from GeoNode : " + serviceURL );
      QgsDataSourceUri uri;
      uri.setParam( QStringLiteral( "url" ), serviceURL );
      uri.setParam( QStringLiteral( "type" ), QStringLiteral( "xyz" ) );
      uri.setParam( QStringLiteral( "zmin" ), "0" );
      uri.setParam( QStringLiteral( "zmax" ), "18" );
      emit addRasterLayer( uri.encodedUri(), layerName, QStringLiteral( "wms" ) );
    }
  }

  QApplication::restoreOverrideCursor();

  if ( !mHoldDialogOpen->isChecked() )
  {
    accept();
  }
}
