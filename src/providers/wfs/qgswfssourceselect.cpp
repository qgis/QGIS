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

#include "qgisinterface.h"
#include "qgswfssourceselect.h"
#include "qgswfsconnection.h"
#include "qgsnewhttpconnection.h"
#include "qgsgenericprojectionselector.h"
#include "qgscontexthelp.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h" //for current view extent
#include "qgsmanageconnectionsdialog.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>


QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget* parent, Qt::WFlags fl )
    : QDialog( parent, fl )
    , mConn( NULL )
{
  setupUi( this );
  btnAdd = buttonBox->button( QDialogButtonBox::Ok );
  btnAdd->setEnabled( false );

  connect( buttonBox, SIGNAL( accepted() ), this, SLOT( addLayer() ) );
  connect( buttonBox, SIGNAL( rejected() ), this, SLOT( reject() ) );
  connect( btnNew, SIGNAL( clicked() ), this, SLOT( addEntryToServerList() ) );
  connect( btnEdit, SIGNAL( clicked() ), this, SLOT( modifyEntryOfServerList() ) );
  connect( btnDelete, SIGNAL( clicked() ), this, SLOT( deleteEntryOfServerList() ) );
  connect( btnConnect, SIGNAL( clicked() ), this, SLOT( connectToServer() ) );
  connect( btnChangeSpatialRefSys, SIGNAL( clicked() ), this, SLOT( changeCRS() ) );
  connect( treeWidget, SIGNAL( currentItemChanged( QTreeWidgetItem*, QTreeWidgetItem* ) ), this, SLOT( changeCRSFilter() ) );
  populateConnectionList();
  mProjectionSelector = new QgsGenericProjectionSelector( this );
  mProjectionSelector->setMessage();
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  delete mProjectionSelector;

  delete mConn;
}

void QgsWFSSourceSelect::populateConnectionList()
{
  QStringList keys = QgsWFSConnection::connectionList();

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
  }

  else
  {
    // No connections available - disable various buttons
    btnConnect->setEnabled( false );
    btnEdit->setEnabled( false );
    btnDelete->setEnabled( false );
  }

  //set last used connection
  QString selectedConnection = QgsWFSConnection::selectedConnection();
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }

  delete mConn;
  mConn = new QgsWFSConnection( cmbConnections->currentText() );
  connect( mConn, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
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

  if ( !mConn )
    return;
  QgsWFSConnection::ErrorCode err = mConn->errorCode();
  if ( err != QgsWFSConnection::NoError )
  {
    QString title;
    switch ( err )
    {
      case QgsWFSConnection::NetworkError: title = tr( "Network Error" ); break;
      case QgsWFSConnection::XmlError:     title = tr( "Capabilities document is not valid" ); break;
      case QgsWFSConnection::ServerExceptionError: title = tr( "Server Exception" ); break;
      default: tr( "Error" ); break;
    }
    // handle errors
    QMessageBox::critical( 0, title, mConn->errorMessage() );

    btnAdd->setEnabled( false );
    return;
  }

  QgsWFSConnection::GetCapabilities caps = mConn->capabilities();

  mAvailableCRS.clear();
  foreach( QgsWFSConnection::FeatureType featureType, caps.featureTypes )
  {
    // insert the typenames, titles and abstracts into the tree view
    QTreeWidgetItem* newItem = new QTreeWidgetItem();
    newItem->setText( 0, featureType.title );
    newItem->setText( 1, featureType.name );
    newItem->setText( 2, featureType.abstract );
    treeWidget->addTopLevelItem( newItem );

    // insert the available CRS into mAvailableCRS
    std::list<QString> currentCRSList;
    foreach( QString crs, featureType.crslist )
    {
      currentCRSList.push_back( crs );
    }
    mAvailableCRS.insert( std::make_pair( featureType.name, currentCRSList ) );
  }

  if ( caps.featureTypes.count() > 0 )
  {
    btnAdd->setEnabled( true );
    treeWidget->setCurrentItem( treeWidget->topLevelItem( 0 ) );
    btnChangeSpatialRefSys->setEnabled( true );
  }
  else
  {
    QMessageBox::information( 0, tr( "No Layers" ), tr( "capabilities document contained no layers." ) );
    btnAdd->setEnabled( false );
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
    QgsWFSConnection::deleteConnection( cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    emit connectionsChanged();
  }
}

void QgsWFSSourceSelect::connectToServer()
{
  btnConnect->setEnabled( false );
  treeWidget->clear();

  if ( mConn )
  {
    mConn->requestCapabilities();
  }
}


void QgsWFSSourceSelect::addLayer()
{
  //get selected entry in lstWidget
  QTreeWidgetItem* tItem = treeWidget->currentItem();
  if ( !tItem )
  {
    return;
  }

  QString typeName = tItem->text( 1 );
  QString crs = labelCoordRefSys->text();
  QString filter = mFilterLineEdit->text();
  QgsRectangle bBox;

#if 0
  // TODO: resolve [MD]
  //get current extent
  QgsMapCanvas* canvas = mIface->mapCanvas();
  if ( canvas && mBboxCheckBox->isChecked() )
  {
    QgsRectangle currentExtent = canvas->extent();
  }
#endif

  //add a wfs layer to the map
  QgsWFSConnection conn( cmbConnections->currentText() );
  QString uri = conn.uriGetFeature( typeName, crs, filter, bBox );

  emit addWfsLayer( uri, typeName );

  accept();
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
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QTreeWidgetItem* currentTreeItem = treeWidget->currentItem();
  if ( currentTreeItem )
  {
    QString currentTypename = currentTreeItem->text( 1 );
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
  QgsWFSConnection::setSelectedConnection( cmbConnections->currentText() );

  delete mConn;
  mConn = new QgsWFSConnection( cmbConnections->currentText() );
  connect( mConn, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
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
