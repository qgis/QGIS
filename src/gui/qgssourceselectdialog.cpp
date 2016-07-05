/***************************************************************************
    qgssourceselectdialog.cpp
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

#include "qgssourceselectdialog.h"
#include "qgsowsconnection.h"
#include "qgsnewhttpconnection.h"
#include "qgsgenericprojectionselector.h"
#include "qgsexpressionbuilderdialog.h"
#include "qgscontexthelp.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgscrscache.h"

#include <QItemDelegate>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QRadioButton>
#include <QImageReader>

/** \ingroup gui
 * Item delegate with tweaked sizeHint.
 * @note not available in Python bindings */
class QgsSourceSelectItemDelegate : public QItemDelegate
{
  public:
    /** Constructor */
    QgsSourceSelectItemDelegate( QObject *parent = 0 ) : QItemDelegate( parent ) { }
    QSize sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const override;
};


QgsSourceSelectDialog::QgsSourceSelectDialog( const QString& serviceName, ServiceType serviceType, QWidget* parent, Qt::WindowFlags fl )
    : QDialog( parent, fl ), mServiceName( serviceName ), mServiceType( serviceType ), mBuildQueryButton( 0 ), mImageEncodingGroup( 0 )
{
  setupUi( this );
  setWindowTitle( QString( "Add %1 Layer from a Server" ).arg( mServiceName ) );

  mAddButton = buttonBox->addButton( tr( "&Add" ), QDialogButtonBox::ActionRole );
  mAddButton->setEnabled( false );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addButtonClicked() ) );

  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton = buttonBox->addButton( tr( "&Build query" ), QDialogButtonBox::ActionRole );
    mBuildQueryButton->setDisabled( true );
    connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQueryButtonClicked() ) );
  }

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

  treeView->setItemDelegate( new QgsSourceSelectItemDelegate( treeView ) );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SourceSelectDialog/geometry" ).toByteArray() );
  cbxUseTitleLayerName->setChecked( settings.value( "/Windows/SourceSelectDialog/UseTitleLayerName", false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( 0, new QStandardItem( "Title" ) );
  mModel->setHorizontalHeaderItem( 1, new QStandardItem( "Name" ) );
  mModel->setHorizontalHeaderItem( 2, new QStandardItem( "Abstract" ) );
  if ( serviceType == FeatureService )
  {
    mModel->setHorizontalHeaderItem( 3, new QStandardItem( "Cache Feature" ) );
    mModel->setHorizontalHeaderItem( 4, new QStandardItem( "Filter" ) );
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

  connect( treeView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( treeWidgetItemDoubleClicked( const QModelIndex& ) ) );
  connect( treeView->selectionModel(), SIGNAL( currentRowChanged( QModelIndex, QModelIndex ) ), this, SLOT( treeWidgetCurrentRowChanged( const QModelIndex&, const QModelIndex& ) ) );
}

QgsSourceSelectDialog::~QgsSourceSelectDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/SourceSelectDialog/geometry", saveGeometry() );
  settings.setValue( "/Windows/SourceSelectDialog/UseTitleLayerName", cbxUseTitleLayerName->isChecked() );

  delete mProjectionSelector;
  delete mModel;
  delete mModelProxy;
}

void QgsSourceSelectDialog::setCurrentExtentAndCrs( const QgsRectangle& canvasExtent, const QgsCoordinateReferenceSystem& canvasCrs )
{
  mCanvasExtent = canvasExtent;
  mCanvasCrs = canvasCrs;
}

void QgsSourceSelectDialog::populateImageEncodings( const QStringList& availableEncodings )
{
  QLayoutItem* item;
  while (( item = gbImageEncoding->layout()->takeAt( 0 ) ) != nullptr )
  {
    delete item->widget();
    delete item;
  }
  bool first = true;
  QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();
  foreach ( const QString& encoding, availableEncodings )
  {
    bool supported = false;
    foreach ( const QByteArray& fmt, supportedFormats )
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

    QRadioButton* button = new QRadioButton( encoding, this );
    button->setChecked( first );
    gbImageEncoding->layout()->addWidget( button );
    mImageEncodingGroup->addButton( button );
    first = false;
  }
}

QString QgsSourceSelectDialog::getSelectedImageEncoding() const
{
  return mImageEncodingGroup ? mImageEncodingGroup->checkedButton()->text() : QString();
}

void QgsSourceSelectDialog::populateConnectionList()
{
  QStringList conns = QgsOWSConnection::connectionList( mServiceName );
  cmbConnections->clear();
  foreach ( const QString& item, conns )
  {
    cmbConnections->addItem( item );
  }
  bool connectionsAvailable = !conns.isEmpty();
  btnConnect->setEnabled( connectionsAvailable );
  btnEdit->setEnabled( connectionsAvailable );
  btnDelete->setEnabled( connectionsAvailable );
  btnSave->setEnabled( connectionsAvailable );

  //set last used connection
  QString selectedConnection = QgsOWSConnection::selectedConnection( mServiceName );
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }
}

QString QgsSourceSelectDialog::getPreferredCrs( const QSet<QString>& crsSet ) const
{
  if ( crsSet.size() < 1 )
  {
    return "";
  }

  //first: project CRS
  long ProjectCRSID = QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectCRSID", -1 );
  //convert to EPSG
  QgsCoordinateReferenceSystem projectRefSys = QgsCRSCache::instance()->crsBySrsId( ProjectCRSID );
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

void QgsSourceSelectDialog::addEntryToServerList()
{

  QgsNewHttpConnection nc( 0, QString( "/Qgis/connections-%1/" ).arg( mServiceName.toLower() ) );
  nc.setWindowTitle( tr( "Create a new %1 connection" ).arg( mServiceName ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsSourceSelectDialog::modifyEntryOfServerList()
{
  QgsNewHttpConnection nc( 0, QString( "/Qgis/connections-%1/" ).arg( mServiceName.toLower() ), cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify %1 connection" ).arg( mServiceName ) );

  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsSourceSelectDialog::deleteEntryOfServerList()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsOWSConnection::deleteConnection( mServiceName, cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    emit connectionsChanged();
    bool connectionsAvailable = cmbConnections->count() > 0;
    btnConnect->setEnabled( connectionsAvailable );
    btnEdit->setEnabled( connectionsAvailable );
    btnDelete->setEnabled( connectionsAvailable );
    btnSave->setEnabled( connectionsAvailable );
  }
}

void QgsSourceSelectDialog::connectToServer()
{
  bool haveLayers = false;
  btnConnect->setEnabled( false );
  mModel->setRowCount( 0 );
  mAvailableCRS.clear();

  QgsOWSConnection connection( mServiceName, cmbConnections->currentText() );

  setCursor( Qt::WaitCursor );
  bool success = connectToService( connection );
  unsetCursor();
  if ( success )
  {
    haveLayers = mModel->rowCount() > 0;

    if ( haveLayers )
    {
      for ( int i = 0; i < treeView->header()->count(); ++i )
      {
        treeView->resizeColumnToContents( i );
        if ( i < 2 && treeView->columnWidth( i ) > 300 )
        {
          treeView->setColumnWidth( i, 300 );
        }
      }
      treeView->selectionModel()->select( mModel->index( 0, 0 ), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
      treeView->setFocus();
    }
    else
    {
      QMessageBox::information( 0, tr( "No Layers" ), tr( "The query returned no layers." ) );
    }
  }

  btnConnect->setEnabled( true );
  mAddButton->setEnabled( haveLayers );
  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton->setEnabled( haveLayers );
  }
  btnChangeSpatialRefSys->setEnabled( haveLayers );
}

void QgsSourceSelectDialog::addButtonClicked()
{
  if ( treeView->selectionModel()->selectedRows().isEmpty() )
  {
    return;
  }

  QgsOWSConnection connection( mServiceName, cmbConnections->currentText() );

  QString pCrsString( labelCoordRefSys->text() );
  QgsCoordinateReferenceSystem pCrs( pCrsString );
  //prepare canvas extent info for layers with "cache features" option not set
  QgsRectangle extent = mCanvasExtent;
  //does canvas have "on the fly" reprojection set?
  if ( pCrs.isValid() && mCanvasCrs.isValid() )
  {
    try
    {
      extent = QgsCoordinateTransform( mCanvasCrs, pCrs ).transform( extent );
      QgsDebugMsg( QString( "canvas transform: Canvas CRS=%1, Provider CRS=%2, BBOX=%3" )
                   .arg( mCanvasCrs.authid(), pCrs.authid(), extent.asWktCoordinates() ) );
    }
    catch ( const QgsCsException& )
    {
      // Extent is not in range for specified CRS, leave extent empty.
    }
  }

  //create layers that user selected from this feature source
  QModelIndexList list = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < list.size(); i++ )
  { //add a wfs layer to the map
    QModelIndex idx = mModelProxy->mapToSource( list[i] );
    if ( !idx.isValid() )
    {
      continue;
    }
    int row = idx.row();
    QString layerTitle = mModel->item( row, 0 )->text(); //layer title/id
    QString layerName = mModel->item( row, 1 )->text(); //layer name
    bool cacheFeatures = mServiceType == FeatureService ? mModel->item( row, 3 )->checkState() == Qt::Checked : false;
    QString filter = mServiceType == FeatureService ? mModel->item( row, 4 )->text() : ""; //optional filter specified by user
    if ( cbxUseTitleLayerName->isChecked() && !layerTitle.isEmpty() )
    {
      layerName = layerTitle;
    }
    QgsRectangle layerExtent;
    if ( mServiceType == FeatureService && ( cbxFeatureCurrentViewExtent->isChecked() || !cacheFeatures ) )
    {
      layerExtent = extent;
    }
    QString uri = getLayerURI( connection, layerTitle, layerName, pCrsString, filter, layerExtent );

    QgsDebugMsg( "Layer " + layerName + ", uri: " + uri );
    emit addLayer( uri, layerName );
  }
  accept();
}

void QgsSourceSelectDialog::changeCRS()
{
  if ( mProjectionSelector->exec() )
  {
    QString crsString = mProjectionSelector->selectedAuthId();
    labelCoordRefSys->setText( crsString );
  }
}

void QgsSourceSelectDialog::changeCRSFilter()
{
  QgsDebugMsg( "changeCRSFilter called" );
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    QString currentTypename = currentIndex.sibling( currentIndex.row(), 1 ).data().toString();
    QgsDebugMsg( QString( "the current typename is: %1" ).arg( currentTypename ) );

    QMap<QString, QStringList>::const_iterator crsIterator = mAvailableCRS.find( currentTypename );
    if ( crsIterator != mAvailableCRS.end() )
    {
      QSet<QString> crsNames;
      foreach ( const QString& crsName, crsIterator.value() )
      {
        crsNames.insert( crsName );
      }
      if ( mProjectionSelector )
      {
        mProjectionSelector->setOgcWmsCrsFilter( crsNames );
        QString preferredCRS = getPreferredCrs( crsNames ); //get preferred EPSG system
        if ( !preferredCRS.isEmpty() )
        {
          QgsCoordinateReferenceSystem refSys = QgsCRSCache::instance()->crsByOgcWmsCrs( preferredCRS );
          mProjectionSelector->setSelectedCrsId( refSys.srsid() );

          labelCoordRefSys->setText( preferredCRS );
        }
      }
    }
  }
}

void QgsSourceSelectDialog::on_cmbConnections_activated( int index )
{
  Q_UNUSED( index );
  QgsOWSConnection::setSelectedConnection( mServiceName, cmbConnections->currentText() );
}

void QgsSourceSelectDialog::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WFS );
  dlg.exec();
}

void QgsSourceSelectDialog::on_btnLoad_clicked()
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

void QgsSourceSelectDialog::treeWidgetItemDoubleClicked( const QModelIndex& index )
{
  QgsDebugMsg( "double click called" );
  QgsOWSConnection connection( mServiceName, cmbConnections->currentText() );
  buildQuery( connection, index );
}

void QgsSourceSelectDialog::treeWidgetCurrentRowChanged( const QModelIndex & current, const QModelIndex & previous )
{
  Q_UNUSED( previous )
  QgsDebugMsg( "treeWidget_currentRowChanged called" );
  changeCRSFilter();
  if ( mServiceType == FeatureService )
  {
    mBuildQueryButton->setEnabled( current.isValid() );
  }
  mAddButton->setEnabled( current.isValid() );
}

void QgsSourceSelectDialog::buildQueryButtonClicked()
{
  QgsDebugMsg( "mBuildQueryButton click called" );
  QgsOWSConnection connection( mServiceName, cmbConnections->currentText() );
  buildQuery( connection, treeView->selectionModel()->currentIndex() );
}

void QgsSourceSelectDialog::filterChanged( QString text )
{
  QgsDebugMsg( "FeatureType filter changed to :" + text );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

QSize QgsSourceSelectItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
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

void QgsSourceSelectDialog::on_buttonBox_helpRequested() const
{
  QgsContextHelp::run( metaObject()->className() );
}
