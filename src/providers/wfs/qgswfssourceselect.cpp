/***************************************************************************
                              qgswfssourceselect.cpp
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2006 by Marco Hugentobler
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

#include "qgswfssourceselect.h"
#include "qgsowsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsprovider.h"
#include "qgsnewhttpconnection.h"
#include "qgsgenericprojectionselector.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscontexthelp.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h" //for current view extent
#include "qgsmanageconnectionsdialog.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QPainter>


QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget* parent, Qt::WindowFlags fl, bool embeddedMode )
    : QDialog( parent, fl )
    , mCapabilities( NULL )
{
  setupUi( this );

  if ( embeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  mBuildQueryButton->setDisabled( true );


  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addLayer() ) );

  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQueryButtonClicked() ) );

  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( btnNew, SIGNAL( clicked() ), this, SLOT( addEntryToServerList() ) );
  connect( btnEdit, SIGNAL( clicked() ), this, SLOT( modifyEntryOfServerList() ) );
  connect( btnDelete, SIGNAL( clicked() ), this, SLOT( deleteEntryOfServerList() ) );
  connect( btnConnect, SIGNAL( clicked() ), this, SLOT( connectToServer() ) );
  connect( btnChangeSpatialRefSys, SIGNAL( clicked() ), this, SLOT( changeCRS() ) );
  connect( lineFilter, SIGNAL( textChanged( QString ) ), this, SLOT( filterChanged( QString ) ) );
  populateConnectionList();
  mProjectionSelector = new QgsGenericProjectionSelector( this );
  mProjectionSelector->setMessage();

  mItemDelegate = new QgsWFSItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  QSettings settings;
  QgsDebugMsg( "restoring settings" );
  restoreGeometry( settings.value( "/Windows/WFSSourceSelect/geometry" ).toByteArray() );
  cbxUseTitleLayerName->setChecked( settings.value( "/Windows/WFSSourceSelect/UseTitleLayerName", false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( 0, new QStandardItem( "Title" ) );
  mModel->setHorizontalHeaderItem( 1, new QStandardItem( "Name" ) );
  mModel->setHorizontalHeaderItem( 2, new QStandardItem( "Abstract" ) );
  mModel->setHorizontalHeaderItem( 3, new QStandardItem( "Cache Feature" ) );
  mModel->setHorizontalHeaderItem( 4, new QStandardItem( "Filter" ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );

  connect( treeView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( treeWidgetItemDoubleClicked( const QModelIndex& ) ) );
  connect( treeView->selectionModel(), SIGNAL( currentRowChanged( QModelIndex, QModelIndex ) ), this, SLOT( treeWidgetCurrentRowChanged( const QModelIndex&, const QModelIndex& ) ) );
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  QSettings settings;
  QgsDebugMsg( "saving settings" );
  settings.setValue( "/Windows/WFSSourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/WFSSourceSelect/UseTitleLayerName", cbxUseTitleLayerName->isChecked() );

  delete mItemDelegate;
  delete mProjectionSelector;
  delete mCapabilities;
  delete mModel;
  delete mModelProxy;
  delete mAddButton;
  delete mBuildQueryButton;
}

void QgsWFSSourceSelect::populateConnectionList()
{
  QStringList keys = QgsOWSConnection::connectionList( "WFS" );

  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }

  if ( keys.begin() != keys.end() )
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

  //set last used connection
  QString selectedConnection = QgsOWSConnection::selectedConnection( "WFS" );
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }

  QgsOWSConnection connection( "WFS", cmbConnections->currentText() );
  delete mCapabilities;
  mCapabilities = new QgsWFSCapabilities( connection.uri().encodedUri() );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
}

QString QgsWFSSourceSelect::getPreferredCrs( const QSet<QString>& crsSet ) const
{
  if ( crsSet.size() < 1 )
  {
    return "";
  }

  //first: project CRS
  long ProjectCRSID = QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectCRSID", -1 );
  //convert to EPSG
  QgsCoordinateReferenceSystem projectRefSys( ProjectCRSID, QgsCoordinateReferenceSystem::InternalCrsId );
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

void QgsWFSSourceSelect::capabilitiesReplyFinished()
{
  btnConnect->setEnabled( true );

  if ( !mCapabilities )
    return;
  QgsWFSCapabilities::ErrorCode err = mCapabilities->errorCode();
  if ( err != QgsWFSCapabilities::NoError )
  {
    QString title;
    switch ( err )
    {
      case QgsWFSCapabilities::NetworkError: title = tr( "Network Error" ); break;
      case QgsWFSCapabilities::XmlError:     title = tr( "Capabilities document is not valid" ); break;
      case QgsWFSCapabilities::ServerExceptionError: title = tr( "Server Exception" ); break;
      default: tr( "Error" ); break;
    }
    // handle errors
    QMessageBox::critical( 0, title, mCapabilities->errorMessage() );

    mAddButton->setEnabled( false );
    return;
  }

  QgsWFSCapabilities::GetCapabilities caps = mCapabilities->capabilities();

  mAvailableCRS.clear();
  foreach ( QgsWFSCapabilities::FeatureType featureType, caps.featureTypes )
  {
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem* titleItem = new QStandardItem( featureType.title );
    QStandardItem* nameItem = new QStandardItem( featureType.name );
    QStandardItem* abstractItem = new QStandardItem( featureType.abstract );
    abstractItem->setToolTip( "<font color=black>" + featureType.abstract  + "</font>" );
    abstractItem->setTextAlignment( Qt::AlignLeft | Qt::AlignTop );
    QStandardItem* cachedItem = new QStandardItem();
    QStandardItem* filterItem = new QStandardItem();
    cachedItem->setCheckable( true );
    cachedItem->setCheckState( Qt::Checked );

    typedef QList< QStandardItem* > StandardItemList;
    mModel->appendRow( StandardItemList() << titleItem << nameItem << abstractItem << cachedItem << filterItem );

    // insert the available CRS into mAvailableCRS
    std::list<QString> currentCRSList;
    foreach ( QString crs, featureType.crslist )
    {
      currentCRSList.push_back( crs );
    }
    mAvailableCRS.insert( std::make_pair( featureType.name, currentCRSList ) );
  }

  if ( caps.featureTypes.count() > 0 )
  {
    treeView->resizeColumnToContents( 0 );
    treeView->resizeColumnToContents( 1 );
    treeView->resizeColumnToContents( 2 );
    treeView->resizeColumnToContents( 3 );
    for ( int i = 0; i < 2; i++ )
    {
      if ( treeView->columnWidth( i ) > 300 )
      {
        treeView->setColumnWidth( i, 300 );
      }
    }
    if ( treeView->columnWidth( 2 ) > 150 )
    {
      treeView->setColumnWidth( 2, 150 );
    }
    btnChangeSpatialRefSys->setEnabled( true );
    treeView->selectionModel()->select( mModel->index( 0, 0 ), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
    treeView->setFocus();
  }
  else
  {
    QMessageBox::information( 0, tr( "No Layers" ), tr( "capabilities document contained no layers." ) );
    mAddButton->setEnabled( false );
    mBuildQueryButton->setEnabled( false );
  }
}

void QgsWFSSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/" );
  nc.setWindowTitle( tr( "Create a new WFS connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsWFSSourceSelect::modifyEntryOfServerList()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/", cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify WFS connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsWFSSourceSelect::deleteEntryOfServerList()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsOWSConnection::deleteConnection( "WFS", cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
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

void QgsWFSSourceSelect::connectToServer()
{
  btnConnect->setEnabled( false );
  if ( mModel )
  {
    mModel->removeRows( 0, mModel->rowCount() );
  }
  if ( mCapabilities )
  {
    mCapabilities->requestCapabilities();
  }
}


void QgsWFSSourceSelect::addLayer()
{
  //get selected entry in treeview
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsOWSConnection connection( "WFS", cmbConnections->currentText() );
  QgsWFSCapabilities conn( connection.uri().encodedUri() );

  QString pCrsString( labelCoordRefSys->text() );
  QgsCoordinateReferenceSystem pCrs( pCrsString );
  //prepare canvas extent info for layers with "cache features" option not set
  QgsRectangle extent;
  QVariant extentVariant = property( "MapExtent" );
  if ( extentVariant.isValid() )
  {
    QString extentString = extentVariant.toString();
    QStringList minMaxSplit = extentString.split( ":" );
    if ( minMaxSplit.size() > 1 )
    {
      QStringList xyMinSplit = minMaxSplit[0].split( "," );
      QStringList xyMaxSplit = minMaxSplit[1].split( "," );
      if ( xyMinSplit.size() > 1 && xyMaxSplit.size() > 1 )
      {
        extent.set( xyMinSplit[0].toDouble(), xyMinSplit[1].toDouble(),
                    xyMaxSplit[0].toDouble(), xyMaxSplit[1].toDouble() );
      }
    }
    //does canvas have "on the fly" reprojection set?
    QVariant crsVariant = property( "MapCRS" );
    if ( crsVariant.isValid() )
    { //transform between provider CRS set in source select dialog and canvas CRS
      QgsCoordinateReferenceSystem cCrs( crsVariant.toString() );
      if ( pCrs.isValid() && cCrs.isValid() )
      {
        QgsCoordinateTransform xform( pCrs, cCrs );
        extent = xform.transformBoundingBox( extent, QgsCoordinateTransform::ReverseTransform );
        QgsDebugMsg( QString( "canvas transform: Canvas CRS=%1, Provider CRS=%2, BBOX=%3" )
                     .arg( cCrs.authid(), pCrs.authid(), extent.asWktCoordinates() ) );
      }
    }
  }

  //create layers that user selected from this WFS source
  QModelIndexList list = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < list.size(); i++ )
  { //add a wfs layer to the map
    QModelIndex idx = mModelProxy->mapToSource( list[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    int row = idx.row();
    QString typeName = mModel->item( row, 1 )->text(); //WFS repository's name for layer
    QString titleName = mModel->item( row, 0 )->text(); //WFS type name title for layer name (if option is set)
    QString filter = mModel->item( row, 4 )->text(); //optional filter specified by user
    QString layerName = typeName;
    if ( cbxUseTitleLayerName->isChecked() && !titleName.isEmpty() )
    {
      layerName = titleName;
    }
    QgsDebugMsg( "Layer " + typeName + " Filter is " + filter );

    //is "cache features" checked?
    //non-cached mode does not work anymore
    if ( !cbxFeatureCurrentViewExtent->isChecked() && mModel->item( row, 3 )->checkState() == Qt::Checked )
    { //yes: entire WFS layer will be retrieved and cached
      mUri = conn.uriGetFeature( typeName, pCrsString, filter );
    }
    else
    { //no: include BBOX of current canvas extent in URI
      mUri = conn.uriGetFeature( typeName, pCrsString, filter, extent );
    }
    emit addWfsLayer( mUri, layerName );
  }
  accept();
}

void QgsWFSSourceSelect::buildQuery( const QModelIndex& index )
{
  if ( !index.isValid() )
  {
    return;
  }
  QModelIndex filterIndex = index.sibling( index.row(), 4 );
  QString typeName = index.sibling( index.row(), 1 ).data().toString();

  //get available fields for wfs layer
  QgsWFSProvider p( "" );  //bypasses most provider instantiation logic
  QgsOWSConnection connection( "WFS", cmbConnections->currentText() );
  QgsWFSCapabilities conn( connection.uri().encodedUri() );
  QString uri = conn.uriDescribeFeatureType( typeName );

  QgsFields fields;
  QString geometryAttribute;
  QGis::WkbType geomType;
  if ( p.describeFeatureType( uri, geometryAttribute, fields, geomType ) != 0 )
  {
    return;
  }

  //show expression builder
  QgsExpressionBuilderDialog d( 0, filterIndex.data().toString() );

  //add available attributes to expression builder
  QgsExpressionBuilderWidget* w = d.expressionBuilder();
  if ( !w )
  {
    return;
  }

  w->loadFieldNames( fields );

  if ( d.exec() == QDialog::Accepted )
  {
    QgsDebugMsg( "Expression text = " + w->expressionText() );
    mModelProxy->setData( filterIndex, QVariant( w->expressionText() ) );
  }
}

void QgsWFSSourceSelect::changeCRS()
{
  if ( mProjectionSelector->exec() )
  {
    QString crsString = mProjectionSelector->selectedAuthId();
    labelCoordRefSys->setText( crsString );
  }
}

void QgsWFSSourceSelect::changeCRSFilter()
{
  QgsDebugMsg( "changeCRSFilter called" );
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    QString currentTypename = currentIndex.sibling( currentIndex.row(), 1 ).data().toString();
    QgsDebugMsg( QString( "the current typename is: %1" ).arg( currentTypename ) );

    std::map<QString, std::list<QString> >::const_iterator crsIterator = mAvailableCRS.find( currentTypename );
    if ( crsIterator != mAvailableCRS.end() )
    {
      std::list<QString> crsList = crsIterator->second;

      QSet<QString> crsNames;

      for ( std::list<QString>::const_iterator it = crsList.begin(); it != crsList.end(); ++it )
      {
        crsNames.insert( *it );
      }
      if ( mProjectionSelector )
      {
        mProjectionSelector->setOgcWmsCrsFilter( crsNames );
        QString preferredCRS = getPreferredCrs( crsNames ); //get preferred EPSG system
        if ( !preferredCRS.isEmpty() )
        {
          QgsCoordinateReferenceSystem refSys;
          refSys.createFromOgcWmsCrs( preferredCRS );
          mProjectionSelector->setSelectedCrsId( refSys.srsid() );

          labelCoordRefSys->setText( preferredCRS );
        }
      }
    }
  }
}

void QgsWFSSourceSelect::on_cmbConnections_activated( int index )
{
  Q_UNUSED( index );
  QgsOWSConnection::setSelectedConnection( "WFS", cmbConnections->currentText() );

  QgsOWSConnection connection( "WFS", cmbConnections->currentText() );

  delete mCapabilities;
  mCapabilities = new QgsWFSCapabilities( connection.uri().encodedUri() );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
}

void QgsWFSSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WFS );
  dlg.exec();
}

void QgsWFSSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WFS, fileName );
  dlg.exec();
  populateConnectionList();
  emit connectionsChanged();
}

void QgsWFSSourceSelect::treeWidgetItemDoubleClicked( const QModelIndex& index )
{
  QgsDebugMsg( "double click called" );
  buildQuery( index );
}

void QgsWFSSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex & current, const QModelIndex & previous )
{
  Q_UNUSED( previous )
  QgsDebugMsg( "treeWidget_currentRowChanged called" );
  changeCRSFilter();
  mBuildQueryButton->setEnabled( current.isValid() );
  mAddButton->setEnabled( current.isValid() );
}

void QgsWFSSourceSelect::buildQueryButtonClicked()
{
  QgsDebugMsg( "mBuildQueryButton click called" );
  buildQuery( treeView->selectionModel()->currentIndex() );
}

void QgsWFSSourceSelect::filterChanged( QString text )
{
  QgsDebugMsg( "WFS FeatureType filter changed to :" + text );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

QSize QgsWFSItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
{
  QVariant indexData;
  indexData = index.data( Qt::DisplayRole );
  if ( indexData.isNull() )
  {
    return QSize();
  }
  QString data = indexData.toString();
  QSize size = option.fontMetrics.boundingRect( data ).size();
  size.setHeight( size.height() + 2 );
  return size;
}
