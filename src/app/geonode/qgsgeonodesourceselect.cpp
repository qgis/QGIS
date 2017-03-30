/***************************************************************************
                              qgsgeonodesourceselect.cpp
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2016 by Marco Hugentobler
  email                : marco dot hugentobler at karto dot baug dot ethz dot ch
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

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_TYPE
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

  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( btnNew, SIGNAL( clicked() ), this, SLOT( addConnectionsEntryList() ) );
  connect( btnEdit, SIGNAL( clicked() ), this, SLOT( modifyConnectionsEntryList() ) );
  connect( btnDelete, SIGNAL( clicked() ), this, SLOT( deleteConnectionsEntryList() ) );
  connect( btnConnect, SIGNAL( clicked() ), this, SLOT( connectToGeonodeConnection() ) );
  connect( btnSave, SIGNAL( clicked() ), this, SLOT( saveGeonodeConnection() ) );
  connect( btnLoad, SIGNAL( clicked() ), this, SLOT( loadGeonodeConnection() ) );
  connect( lineFilter, SIGNAL( textChanged( QString ) ), this, SLOT( filterChanged( QString ) ) );

  mItemDelegate = new QgsGeonodeItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( QStringLiteral( "Title" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_TYPE, new QStandardItem( QStringLiteral( "Service Type" ) ) );

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
  QgsGeoNodeConnection connection( cmbConnections->currentText() );
  QVariantList layers = connection.getLayers();
  QVariantList maps = connection.getMaps();

  if ( mModel )
  {
    mModel->removeRows( 0, mModel->rowCount() );
  }

  if ( !layers.isEmpty() )
  {
    Q_FOREACH ( const QVariant &layer, layers )
    {
      QStandardItem *titleItem = new QStandardItem( layer.toMap()["title"].toString() );
      QStandardItem *serviceTypeItem = new QStandardItem( "Feature Service" );
      typedef QList< QStandardItem * > StandardItemList;
      mModel->appendRow( StandardItemList() << titleItem << serviceTypeItem );
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

  if ( !maps.isEmpty() )
  {
    Q_FOREACH ( const QVariant &map, maps )
    {
      QStandardItem *titleItem = new QStandardItem( map.toMap()["title"].toString() );
      QStandardItem *serviceTypeItem = new QStandardItem( "Map Service" );
      typedef QList< QStandardItem * > StandardItemList;
      mModel->appendRow( StandardItemList() << titleItem << serviceTypeItem );
    }
  }

  else
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), tr( "Cannot get any map services" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( QStringLiteral( "GeonodeCapabilitiesErrorBox" ) );
    box->open();
  }

  treeView->resizeColumnToContents( MODEL_IDX_TITLE );
  treeView->resizeColumnToContents( MODEL_IDX_TYPE );
  for ( int i = MODEL_IDX_TITLE; i < MODEL_IDX_TYPE; i++ )
  {
    if ( treeView->columnWidth( i ) > 300 )
    {
      treeView->setColumnWidth( i, 300 );
    }
  }
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
