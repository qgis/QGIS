/***************************************************************************
    qgsarcgisservicesourceselect.cpp
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

#include "qgsarcgisservicesourceselect.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"
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

#include <QButtonGroup>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QRadioButton>
#include <QImageReader>
#include "qgshelp.h"

QgsArcGisServiceSourceSelect::QgsArcGisServiceSourceSelect( const QString &serviceName, ServiceType serviceType, QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, widgetMode )
  , mServiceName( serviceName )
  , mServiceType( serviceType )
{
  setupUi( this );
  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsArcGisServiceSourceSelect::cmbConnections_activated );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsArcGisServiceSourceSelect::showHelp );
  setWindowTitle( QStringLiteral( "Add %1 Layer from a Server" ).arg( mServiceName ) );

  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton = buttonBox->addButton( tr( "&Build query" ), QDialogButtonBox::ActionRole );
    mBuildQueryButton->setDisabled( true );
    connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::buildQueryButtonClicked );
  }

  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( btnNew, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::addEntryToServerList );
  connect( btnEdit, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::modifyEntryOfServerList );
  connect( btnDelete, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::deleteEntryOfServerList );
  connect( btnConnect, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::connectToServer );
  connect( btnChangeSpatialRefSys, &QAbstractButton::clicked, this, &QgsArcGisServiceSourceSelect::changeCrs );
  connect( lineFilter, &QLineEdit::textChanged, this, &QgsArcGisServiceSourceSelect::filterChanged );
  populateConnectionList();
  mProjectionSelector = new QgsProjectionSelectionDialog( this );
  mProjectionSelector->setMessage( QString() );

  treeView->setItemDelegate( new QgsAbstractDataSourceWidgetItemDelegate( treeView ) );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/SourceSelectDialog/geometry" ) ).toByteArray() );
  cbxUseTitleLayerName->setChecked( settings.value( QStringLiteral( "Windows/SourceSelectDialog/UseTitleLayerName" ), false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( 0, new QStandardItem( QStringLiteral( "Title" ) ) );
  mModel->setHorizontalHeaderItem( 1, new QStandardItem( QStringLiteral( "Name" ) ) );
  mModel->setHorizontalHeaderItem( 2, new QStandardItem( QStringLiteral( "Abstract" ) ) );
  if ( serviceType == FeatureService )
  {
    mModel->setHorizontalHeaderItem( 3, new QStandardItem( QStringLiteral( "Filter" ) ) );
    gbImageEncoding->hide();
  }
  else
  {
    cbxFeatureCurrentViewExtent->hide();
    mImageEncodingGroup = new QButtonGroup( this );
  }

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );
  treeView->setSortingEnabled( true );

  connect( treeView, &QAbstractItemView::doubleClicked, this, &QgsArcGisServiceSourceSelect::treeWidgetItemDoubleClicked );
  connect( treeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsArcGisServiceSourceSelect::treeWidgetCurrentRowChanged );
}

QgsArcGisServiceSourceSelect::~QgsArcGisServiceSourceSelect()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/SourceSelectDialog/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/SourceSelectDialog/UseTitleLayerName" ), cbxUseTitleLayerName->isChecked() );

  delete mProjectionSelector;
  delete mModel;
  delete mModelProxy;
}


void QgsArcGisServiceSourceSelect::populateImageEncodings( const QStringList &availableEncodings )
{
  QLayoutItem *item = nullptr;
  while ( ( item = gbImageEncoding->layout()->takeAt( 0 ) ) )
  {
    delete item->widget();
    delete item;
  }
  bool first = true;
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
    button->setChecked( first );
    gbImageEncoding->layout()->addWidget( button );
    mImageEncodingGroup->addButton( button );
    first = false;
  }
}

QString QgsArcGisServiceSourceSelect::getSelectedImageEncoding() const
{
  return mImageEncodingGroup && mImageEncodingGroup->checkedButton() ? mImageEncodingGroup->checkedButton()->text() : QString();
}

void QgsArcGisServiceSourceSelect::populateConnectionList()
{
  const QStringList conns = QgsOwsConnection::connectionList( mServiceName );
  cmbConnections->clear();
  for ( const QString &item : conns )
  {
    cmbConnections->addItem( item );
  }
  bool connectionsAvailable = !conns.isEmpty();
  btnConnect->setEnabled( connectionsAvailable );
  btnEdit->setEnabled( connectionsAvailable );
  btnDelete->setEnabled( connectionsAvailable );
  btnSave->setEnabled( connectionsAvailable );

  //set last used connection
  QString selectedConnection = QgsOwsConnection::selectedConnection( mServiceName );
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }
}

QString QgsArcGisServiceSourceSelect::getPreferredCrs( const QSet<QString> &crsSet ) const
{
  if ( crsSet.size() < 1 )
  {
    return QString();
  }

  //first: project CRS
  QgsCoordinateReferenceSystem projectRefSys = QgsProject::instance()->crs();
  //convert to EPSG
  QString ProjectCRS;
  if ( projectRefSys.isValid() )
  {
    ProjectCRS = projectRefSys.authid();
  }

  if ( !ProjectCRS.isEmpty() && crsSet.contains( ProjectCRS ) )
  {
    return ProjectCRS;
  }

  //second: WGS84
  if ( crsSet.contains( GEO_EPSG_CRS_AUTHID ) )
  {
    return GEO_EPSG_CRS_AUTHID;
  }

  //third: first entry in set
  return *( crsSet.constBegin() );
}

void QgsArcGisServiceSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsArcGisServiceSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-%1/" ).arg( mServiceName.toLower() ) );
  nc.setWindowTitle( tr( "Create a New %1 Connection" ).arg( mServiceName ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsArcGisServiceSourceSelect::modifyEntryOfServerList()
{
  QgsNewHttpConnection nc( nullptr, QgsNewHttpConnection::ConnectionOther, QStringLiteral( "qgis/connections-%1/" ).arg( mServiceName.toLower() ), cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify %1 Connection" ).arg( mServiceName ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsArcGisServiceSourceSelect::deleteEntryOfServerList()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result == QMessageBox::Yes )
  {
    QgsOwsConnection::deleteConnection( mServiceName, cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    emit connectionsChanged();
    bool connectionsAvailable = cmbConnections->count() > 0;
    btnConnect->setEnabled( connectionsAvailable );
    btnEdit->setEnabled( connectionsAvailable );
    btnDelete->setEnabled( connectionsAvailable );
    btnSave->setEnabled( connectionsAvailable );
  }
}

void QgsArcGisServiceSourceSelect::connectToServer()
{
  bool haveLayers = false;
  btnConnect->setEnabled( false );
  mModel->setRowCount( 0 );
  mAvailableCRS.clear();

  QgsOwsConnection connection( mServiceName, cmbConnections->currentText() );

  setCursor( Qt::WaitCursor );
  bool success = connectToService( connection );
  unsetCursor();
  if ( success )
  {
    haveLayers = mModel->rowCount() > 0;

    if ( haveLayers )
    {
      treeView->selectionModel()->select( mModel->index( 0, 0 ), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
      treeView->setFocus();

      treeView->sortByColumn( 0, Qt::AscendingOrder );
    }
    else
    {
      QMessageBox::information( nullptr, tr( "No Layers" ), tr( "The query returned no layers." ) );
    }
  }

  btnConnect->setEnabled( true );
  emit enableButtons( haveLayers );
  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton->setEnabled( haveLayers );
  }
  btnChangeSpatialRefSys->setEnabled( haveLayers );
}

void QgsArcGisServiceSourceSelect::addButtonClicked()
{
  if ( treeView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsOwsConnection connection( mServiceName, cmbConnections->currentText() );

  QString pCrsString( labelCoordRefSys->text() );
  QgsCoordinateReferenceSystem pCrs( pCrsString );
  //prepare canvas extent info for layers with "cache features" option not set
  QgsRectangle extent;
  QgsCoordinateReferenceSystem canvasCrs;
  if ( mapCanvas() )
  {
    extent = mapCanvas()->extent();
    canvasCrs = mapCanvas()->mapSettings().destinationCrs();
  }
  //does canvas have "on the fly" reprojection set?
  if ( pCrs.isValid() && canvasCrs.isValid() )
  {
    try
    {
      Q_NOWARN_DEPRECATED_PUSH
      extent = QgsCoordinateTransform( canvasCrs, pCrs ).transform( extent );
      Q_NOWARN_DEPRECATED_POP
      QgsDebugMsg( QStringLiteral( "canvas transform: Canvas CRS=%1, Provider CRS=%2, BBOX=%3" )
                   .arg( canvasCrs.authid(), pCrs.authid(), extent.asWktCoordinates() ) );
    }
    catch ( const QgsCsException & )
    {
      // Extent is not in range for specified CRS, leave extent empty.
    }
  }

  //create layers that user selected from this feature source
  QModelIndexList list = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < list.size(); i++ )
  {
    //add a wfs layer to the map
    QModelIndex idx = mModelProxy->mapToSource( list[i] );
    if ( !idx.isValid() )
    {
      continue;
    }

    int row = idx.row();
    if ( !mModel->itemFromIndex( mModel->index( row, 0, idx.parent() ) )->data( IsLayerRole ).toBool() )
      continue;

    QString layerTitle = mModel->itemFromIndex( mModel->index( row, 0, idx.parent() ) )->text();  //layer title/id
    QString layerName = mModel->itemFromIndex( mModel->index( row, 1, idx.parent() ) )->text(); //layer name
    const QString layerUri = mModel->itemFromIndex( mModel->index( row, 0, idx.parent() ) )->data( UrlRole ).toString();
    QString filter = mServiceType == FeatureService ? mModel->itemFromIndex( mModel->index( row, 3, idx.parent() ) )->text() : QString(); //optional filter specified by user
    if ( cbxUseTitleLayerName->isChecked() && !layerTitle.isEmpty() )
    {
      layerName = layerTitle;
    }
    QgsRectangle layerExtent;
    if ( mServiceType == FeatureService && ( cbxFeatureCurrentViewExtent->isChecked() ) )
    {
      layerExtent = extent;
    }
    QString uri = getLayerURI( connection, layerUri.isEmpty() ? layerTitle : layerUri, layerName, pCrsString, filter, layerExtent );

    QgsDebugMsg( "Layer " + layerName + ", uri: " + uri );
    addServiceLayer( uri, layerName );
  }
  accept();
}

void QgsArcGisServiceSourceSelect::changeCrs()
{
  if ( mProjectionSelector->exec() )
  {
    QString crsString = mProjectionSelector->crs().authid();
    labelCoordRefSys->setText( crsString );
  }
}

void QgsArcGisServiceSourceSelect::changeCrsFilter()
{
  QgsDebugMsg( QStringLiteral( "changeCRSFilter called" ) );
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    QString currentTypename = currentIndex.sibling( currentIndex.row(), 1 ).data().toString();
    QgsDebugMsg( QStringLiteral( "the current typename is: %1" ).arg( currentTypename ) );

    QMap<QString, QStringList>::const_iterator crsIterator = mAvailableCRS.constFind( currentTypename );
    if ( crsIterator != mAvailableCRS.constEnd() )
    {
      QSet<QString> crsNames;
      const QStringList crsNamesList = crsIterator.value();
      for ( const QString &crsName : crsNamesList )
      {
        crsNames.insert( crsName );
      }
      if ( mProjectionSelector )
      {
        mProjectionSelector->setOgcWmsCrsFilter( crsNames );
        QString preferredCRS = getPreferredCrs( crsNames ); //get preferred EPSG system
        if ( !preferredCRS.isEmpty() )
        {
          QgsCoordinateReferenceSystem refSys = QgsCoordinateReferenceSystem::fromOgcWmsCrs( preferredCRS );
          mProjectionSelector->setCrs( refSys );

          labelCoordRefSys->setText( preferredCRS );
        }
      }
    }
  }
}

void QgsArcGisServiceSourceSelect::cmbConnections_activated( int index )
{
  Q_UNUSED( index );
  QgsOwsConnection::setSelectedConnection( mServiceName, cmbConnections->currentText() );
}

void QgsArcGisServiceSourceSelect::treeWidgetItemDoubleClicked( const QModelIndex &index )
{
  QgsDebugMsg( QStringLiteral( "double-click called" ) );
  QgsOwsConnection connection( mServiceName, cmbConnections->currentText() );
  buildQuery( connection, index );
}

void QgsArcGisServiceSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  QgsDebugMsg( QStringLiteral( "treeWidget_currentRowChanged called" ) );
  changeCrsFilter();
  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton->setEnabled( current.isValid() );
  }
  emit enableButtons( current.isValid() );
}

void QgsArcGisServiceSourceSelect::buildQueryButtonClicked()
{
  QgsDebugMsg( QStringLiteral( "mBuildQueryButton click called" ) );
  QgsOwsConnection connection( mServiceName, cmbConnections->currentText() );
  buildQuery( connection, treeView->selectionModel()->currentIndex() );
}

void QgsArcGisServiceSourceSelect::filterChanged( const QString &text )
{
  QgsDebugMsg( "FeatureType filter changed to :" + text );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

QSize QgsAbstractDataSourceWidgetItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  QVariant indexData = index.data( Qt::DisplayRole );
  if ( indexData.isNull() )
  {
    return QSize();
  }
  QSize size = option.fontMetrics.boundingRect( indexData.toString() ).size();
  size.setHeight( size.height() + 2 );
  return size;
}

void QgsArcGisServiceSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/index.html" ) );
}
