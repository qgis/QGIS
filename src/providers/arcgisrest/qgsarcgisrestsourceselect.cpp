/***************************************************************************
    qgsarcgisrestsourceselect.cpp
    -------------------------
  begin                : Nov 26, 2015
  copyright            : (C) 2015 by Sandro Mani
  email                : smani@sourcepole.ch
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsarcgisrestsourceselect.h"
#include "qgsowsconnection.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsexception.h"
#include "qgssettings.h"
#include "qgsmapcanvas.h"
#include "qgshelp.h"
#include "qgsgui.h"
#include "qgsbrowserguimodel.h"
#include "qgsarcgisrestdataitems.h"
#include "qgsnewarcgisrestconnection.h"

#include <QButtonGroup>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QRadioButton>
#include <QImageReader>

//
// QgsArcGisRestBrowserProxyModel
//

QgsArcGisRestBrowserProxyModel::QgsArcGisRestBrowserProxyModel( QObject *parent )
  : QgsBrowserProxyModel( parent )
{

}

void QgsArcGisRestBrowserProxyModel::setConnectionName( const QString &name )
{
  mConnectionName = name;
  invalidateFilter();
}

bool QgsArcGisRestBrowserProxyModel::filterAcceptsRow( int sourceRow, const QModelIndex &sourceParent ) const
{
  if ( !QgsBrowserProxyModel::filterAcceptsRow( sourceRow, sourceParent ) )
    return false;

  const QModelIndex sourceIndex = mModel->index( sourceRow, 0, sourceParent );
  if ( QgsArcGisRestConnectionItem *connectionItem = qobject_cast< QgsArcGisRestConnectionItem * >( mModel->dataItem( sourceIndex ) ) )
  {
    if ( connectionItem->name() != mConnectionName )
      return false;
  }

  return true;
}


//
// QgsArcGisRestSourceSelect
//
QgsArcGisRestSourceSelect::QgsArcGisRestSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsArcGisRestSourceSelect::cmbConnections_activated );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsArcGisRestSourceSelect::showHelp );
  setWindowTitle( QStringLiteral( "Add ArcGIS REST Layer" ) );

  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( btnNew, &QAbstractButton::clicked, this, &QgsArcGisRestSourceSelect::addEntryToServerList );
  connect( btnEdit, &QAbstractButton::clicked, this, &QgsArcGisRestSourceSelect::modifyEntryOfServerList );
  connect( btnDelete, &QAbstractButton::clicked, this, &QgsArcGisRestSourceSelect::deleteEntryOfServerList );
  connect( btnRefresh, &QAbstractButton::clicked, this, &QgsArcGisRestSourceSelect::onRefresh );
  connect( btnConnect, &QAbstractButton::clicked, this, &QgsArcGisRestSourceSelect::connectToServer );
  connect( btnSave, &QPushButton::clicked, this, &QgsArcGisRestSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsArcGisRestSourceSelect::btnLoad_clicked );
  connect( lineFilter, &QLineEdit::textChanged, this, &QgsArcGisRestSourceSelect::filterChanged );
  populateConnectionList();

  lineFilter->setShowClearButton( true );
  lineFilter->setShowSearchIcon( true );

  const QgsSettings settings;

  mImageEncodingGroup = new QButtonGroup( this );
}

QgsArcGisRestSourceSelect::~QgsArcGisRestSourceSelect()
{
}

void QgsArcGisRestSourceSelect::populateImageEncodings( const QString &formats )
{
  const QStringList availableEncodings = formats.split( ',' );

  const QString prevFormat = getSelectedImageEncoding();

  QLayoutItem *item = nullptr;
  while ( ( item = gbImageEncoding->layout()->takeAt( 0 ) ) )
  {
    delete item->widget();
    delete item;
  }
  const QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
  for ( const QString &encoding : availableEncodings )
  {
    bool supported = false;
    for ( const QByteArray &fmt : supportedFormats )
    {
      if ( encoding.startsWith( fmt, Qt::CaseInsensitive ) )
      {
        supported = true;
      }
    }
    if ( !supported )
    {
      continue;
    }

    QRadioButton *button = new QRadioButton( encoding, this );
    if ( encoding == prevFormat )
      button->setChecked( true );
    gbImageEncoding->layout()->addWidget( button );
    mImageEncodingGroup->addButton( button );
  }
  if ( !mImageEncodingGroup->checkedButton() && !mImageEncodingGroup->buttons().empty() )
    mImageEncodingGroup->buttons().value( 0 )->setChecked( true );
}

QString QgsArcGisRestSourceSelect::getSelectedImageEncoding() const
{
  return mImageEncodingGroup && mImageEncodingGroup->checkedButton() ? mImageEncodingGroup->checkedButton()->text() : QString();
}

void QgsArcGisRestSourceSelect::showEvent( QShowEvent * )
{
  if ( QgsBrowserGuiModel *model = qobject_cast< QgsBrowserGuiModel * >( browserModel() ) )
  {
    mBrowserModel = model;
  }
  else
  {
    mBrowserModel = new QgsBrowserGuiModel( this );
  }
  mBrowserModel->initialize();

  mProxyModel = new QgsArcGisRestBrowserProxyModel( this );
  mProxyModel->setBrowserModel( mBrowserModel );

  mBrowserView->setSettingsSection( objectName().toLower() ); // to distinguish 2 or more instances of the browser
  mBrowserView->setBrowserModel( mBrowserModel );
  mBrowserView->setModel( mProxyModel );
  mBrowserView->setSortingEnabled( true );
  mBrowserView->sortByColumn( 0, Qt::AscendingOrder );
  // provide a horizontal scroll bar instead of using ellipse (...) for longer items
  mBrowserView->setTextElideMode( Qt::ElideNone );

  connect( mBrowserView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsArcGisRestSourceSelect::treeWidgetCurrentRowChanged );

  mBrowserView->expand( mProxyModel->index( 0, 0 ) );
  mBrowserView->setHeaderHidden( true );

  mProxyModel->setShownDataItemProviderKeyFilter( QStringList() << QStringLiteral( "AFS" ) << QStringLiteral( "arcgisfeatureserver" )
      << QStringLiteral( "AMS" ) << QStringLiteral( "arcgismapserver" ) );

  const QModelIndex afsSourceIndex = mBrowserModel->findPath( QStringLiteral( "arcgisfeatureserver:" ) );
  mBrowserView->setRootIndex( mProxyModel->mapFromSource( afsSourceIndex ) );

  // don't show anything till connect is clicked!
  mProxyModel->setConnectionName( QString() );
}

void QgsArcGisRestSourceSelect::populateConnectionList()
{
  const QStringList conns = QgsOwsConnection::connectionList( QStringLiteral( "ARCGISFEATURESERVER" ) );
  cmbConnections->clear();
  for ( const QString &item : conns )
  {
    cmbConnections->addItem( item );
  }
  const bool connectionsAvailable = !conns.isEmpty();
  btnEdit->setEnabled( connectionsAvailable );
  btnDelete->setEnabled( connectionsAvailable );
  btnSave->setEnabled( connectionsAvailable );

  //set last used connection
  const QString selectedConnection = QgsOwsConnection::selectedConnection( QStringLiteral( "ARCGISFEATURESERVER" ) );
  const int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }
}

void QgsArcGisRestSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsArcGisRestSourceSelect::addEntryToServerList()
{
  QgsNewArcGisRestConnectionDialog nc( nullptr, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), QString() );
  nc.setWindowTitle( tr( "Create a New ArcGIS REST Server Connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsArcGisRestSourceSelect::modifyEntryOfServerList()
{
  QgsNewArcGisRestConnectionDialog nc( nullptr, QStringLiteral( "qgis/connections-arcgisfeatureserver/" ), cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify ArcGIS REST Server Connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsArcGisRestSourceSelect::deleteEntryOfServerList()
{
  const QString selectedConnection = cmbConnections->currentText();
  const QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                      .arg( selectedConnection );
  const QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result == QMessageBox::Yes )
  {
    QgsOwsConnection::deleteConnection( QStringLiteral( "ARCGISFEATURESERVER" ), selectedConnection );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    emit connectionsChanged();
    const bool connectionsAvailable = cmbConnections->count() > 0;
    btnEdit->setEnabled( connectionsAvailable );
    btnDelete->setEnabled( connectionsAvailable );
    btnSave->setEnabled( connectionsAvailable );

    if ( selectedConnection == mConnectedService )
      disconnectFromServer();
  }
}

void QgsArcGisRestSourceSelect::connectToServer()
{
  const bool haveLayers = false;
  btnConnect->setEnabled( false );

  mConnectedService = cmbConnections->currentText();
  const QgsOwsConnection connection( QStringLiteral( "ARCGISFEATURESERVER" ), mConnectedService );

  // find index of corresponding node
  if ( mBrowserModel && mProxyModel )
  {
    mProxyModel->setConnectionName( mConnectedService );

    mBrowserView->expand( mProxyModel->index( 0, 0, mBrowserView->rootIndex() ) );
    onRefresh();
  }

  btnConnect->setEnabled( true );
  emit enableButtons( haveLayers );
  updateCrsLabel();
}

void QgsArcGisRestSourceSelect::disconnectFromServer()
{
  mProxyModel->setConnectionName( QString() );
  emit enableButtons( false );
}

void QgsArcGisRestSourceSelect::addButtonClicked()
{
  if ( mBrowserView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  const QgsOwsConnection connection( QStringLiteral( "ARCGISFEATURESERVER" ), cmbConnections->currentText() );

  const QString pCrsString( labelCoordRefSys->text() );
  const QgsCoordinateReferenceSystem pCrs( pCrsString );
  //prepare canvas extent info for layers with "cache features" option not set
  QgsRectangle extent;
  QgsCoordinateReferenceSystem canvasCrs;
  if ( auto *lMapCanvas = mapCanvas() )
  {
    extent = lMapCanvas->extent();
    canvasCrs = lMapCanvas->mapSettings().destinationCrs();
  }
  //does canvas have "on the fly" reprojection set?
  if ( pCrs.isValid() && canvasCrs.isValid() )
  {
    try
    {
      QgsCoordinateTransform extentTransform = QgsCoordinateTransform( canvasCrs, pCrs, QgsProject::instance()->transformContext() );
      extentTransform.setBallparkTransformsAreAppropriate( true );
      extent = extentTransform.transformBoundingBox( extent );
      QgsDebugMsgLevel( QStringLiteral( "canvas transform: Canvas CRS=%1, Provider CRS=%2, BBOX=%3" )
                        .arg( canvasCrs.authid(), pCrs.authid(), extent.asWktCoordinates() ), 3 );
    }
    catch ( const QgsCsException & )
    {
      // Extent is not in range for specified CRS, leave extent empty.
    }
  }

  //create layers that user selected from this feature source
  const QModelIndexList list = mBrowserView->selectionModel()->selectedRows();
  for ( const QModelIndex &proxyIndex : list )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( proxyIndex );
    if ( !sourceIndex.isValid() )
    {
      continue;
    }

    QgsDataItem *item = mBrowserModel->dataItem( sourceIndex );
    if ( !item )
      continue;

    if ( QgsLayerItem *layerItem = qobject_cast< QgsLayerItem * >( item ) )
    {
      const QString layerName = layerItem->name();

      QgsRectangle layerExtent;
      if ( cbxFeatureCurrentViewExtent->isChecked() )
      {
        layerExtent = extent;
      }

      QgsDataSourceUri uri( layerItem->uri() );
      uri.setParam( QStringLiteral( "crs" ), pCrsString );
      if ( qobject_cast< QgsArcGisFeatureServiceLayerItem *>( layerItem ) )
      {
        if ( !layerExtent.isEmpty() )
        {
          uri.setParam( QStringLiteral( "bbox" ), QStringLiteral( "%1,%2,%3,%4" ).arg( layerExtent.xMinimum() ).arg( layerExtent.yMinimum() ).arg( layerExtent.xMaximum() ).arg( layerExtent.yMaximum() ) );
        }
        emit addVectorLayer( uri.uri( false ), layerName );
      }
      else if ( qobject_cast< QgsArcGisMapServiceLayerItem *>( layerItem ) )
      {
        uri.removeParam( QStringLiteral( "format" ) );
        uri.setParam( QStringLiteral( "format" ), getSelectedImageEncoding() );
        emit addRasterLayer( uri.uri( false ), layerName, QStringLiteral( "arcgismapserver" ) );
      }
    }
  }

  // Clear selection after layers have been added
  mBrowserView->selectionModel()->clearSelection();

}

void QgsArcGisRestSourceSelect::updateCrsLabel()
{
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  const QModelIndex currentIndex = mBrowserView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( currentIndex );
    if ( !sourceIndex.isValid() )
    {
      labelCoordRefSys->clear();
      return;
    }

    if ( QgsLayerItem *layerItem = qobject_cast< QgsLayerItem * >( mBrowserModel->dataItem( sourceIndex ) ) )
    {
      const QgsDataSourceUri uri( layerItem->uri() );
      labelCoordRefSys->setText( uri.param( QStringLiteral( "crs" ) ) );
    }
    else
    {
      labelCoordRefSys->clear();
    }
  }
}

void QgsArcGisRestSourceSelect::updateImageEncodings()
{
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  const QModelIndex currentIndex = mBrowserView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    const QModelIndex sourceIndex = mProxyModel->mapToSource( currentIndex );
    if ( !sourceIndex.isValid() )
    {
      return;
    }

    if ( QgsArcGisMapServiceLayerItem *layerItem = qobject_cast< QgsArcGisMapServiceLayerItem * >( mBrowserModel->dataItem( sourceIndex ) ) )
    {
      populateImageEncodings( layerItem->supportedFormats() );
    }
  }
}

void QgsArcGisRestSourceSelect::cmbConnections_activated( int index )
{
  Q_UNUSED( index )
  QgsOwsConnection::setSelectedConnection( QStringLiteral( "ARCGISFEATURESERVER" ), cmbConnections->currentText() );
}

void QgsArcGisRestSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  QgsDebugMsgLevel( QStringLiteral( "treeWidget_currentRowChanged called" ), 3 );
  updateCrsLabel();
  updateImageEncodings();

  emit enableButtons( current.isValid() );
}

void QgsArcGisRestSourceSelect::filterChanged( const QString &text )
{
  mProxyModel->setFilterString( text );
}

void QgsArcGisRestSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/index.html" ) );
}

void QgsArcGisRestSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::ArcgisFeatureServer );
  dlg.exec();
}

void QgsArcGisRestSourceSelect::btnLoad_clicked()
{
  const QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                           tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::ArcgisFeatureServer, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsArcGisRestSourceSelect::onRefresh()
{
  if ( mBrowserModel )
    refreshModel( mProxyModel->mapToSource( mBrowserView->rootIndex() ) );
}

void QgsArcGisRestSourceSelect::refreshModel( const QModelIndex &index )
{
  if ( mBrowserModel && mProxyModel )
  {
    QgsDataItem *item = mBrowserModel->dataItem( index );
    if ( item && ( item->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
    {
      mBrowserModel->refresh( index );
    }

    for ( int i = 0; i < mBrowserModel->rowCount( index ); i++ )
    {
      const QModelIndex idx = mBrowserModel->index( i, 0, index );
      const QModelIndex proxyIdx = mProxyModel->mapFromSource( idx );
      QgsDataItem *child = mBrowserModel->dataItem( idx );

      // Check also expanded descendants so that the whole expanded path does not get collapsed if one item is collapsed.
      // Fast items (usually root items) are refreshed so that when collapsed, it is obvious they are if empty (no expand symbol).
      if ( mBrowserView->isExpanded( proxyIdx ) || mBrowserView->hasExpandedDescendant( proxyIdx ) || ( child && child->capabilities2() & Qgis::BrowserItemCapability::Fast ) )
      {
        refreshModel( idx );
      }
      else
      {
        if ( child && ( child->capabilities2() & Qgis::BrowserItemCapability::Fertile ) )
        {
          child->depopulate();
        }
      }
    }
  }
}

