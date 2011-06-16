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
#include "qgsnewhttpconnection.h"
#include "qgsgenericprojectionselector.h"
#include "qgscontexthelp.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgslogger.h"
#include "qgsmapcanvas.h" //for current view extent
#include "qgsnetworkaccessmanager.h"
#include "qgsmanageconnectionsdialog.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QFileDialog>

static const QString WFS_NAMESPACE = "http://www.opengis.net/wfs";

QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget* parent, QgisInterface* iface )
    : QDialog( parent )
    , mIface( iface )
    , mCapabilitiesReply( 0 )
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
}

void QgsWFSSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wfs" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();

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
  QSettings s;
  QString selectedConnection = s.value( "/Qgis/connections-wfs/selected" ).toString();
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }
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
  if ( mCapabilitiesReply->error() == QNetworkReply::NoError )
  {
    QVariant redirect = mCapabilitiesReply->attribute( QNetworkRequest::RedirectionTargetAttribute );
    if ( !redirect.isNull() )
    {
      QgsDebugMsg( "redirecting to " + redirect.toUrl().toString() );
      QNetworkRequest request( redirect.toUrl() );
      request.setAttribute( QNetworkRequest::CacheLoadControlAttribute, QNetworkRequest::PreferNetwork );
      request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );

      mCapabilitiesReply->deleteLater();
      mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );

      connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
      connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
      return;
    }

    QByteArray buffer = mCapabilitiesReply->readAll();

    QgsDebugMsg( "parsing capabilities: " + buffer );

    QString capabilitiesDocError;
    QDomDocument capabilitiesDocument;
    if ( capabilitiesDocument.setContent( buffer, true, &capabilitiesDocError ) )
    {
      QDomElement doc = capabilitiesDocument.documentElement();
      if ( doc.tagName() != "ExceptionReport" )
      {
        std::list<QString> typenames;
        std::list< std::list<QString> > crs;
        std::list<QString> titles;
        std::list<QString> abstracts;

        //get the <FeatureType> elements
        QDomNodeList featureTypeList = capabilitiesDocument.elementsByTagNameNS( WFS_NAMESPACE, "FeatureType" );
        for ( unsigned int i = 0; i < featureTypeList.length(); ++i )
        {
          QString tname, title, abstract;
          QDomElement featureTypeElem = featureTypeList.at( i ).toElement();
          std::list<QString> featureCRSList; //CRS list for this feature

          //Name
          QDomNodeList nameList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Name" );
          if ( nameList.length() > 0 )
          {
            tname = nameList.at( 0 ).toElement().text();
            //strip away namespace prefixes
            /* if ( tname.contains( ":" ) )
               {
               tname = tname.section( ":", 1, 1 );
               }*/
          }
          //Title
          QDomNodeList titleList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Title" );
          if ( titleList.length() > 0 )
          {
            title = titleList.at( 0 ).toElement().text();
          }
          //Abstract
          QDomNodeList abstractList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "Abstract" );
          if ( abstractList.length() > 0 )
          {
            abstract = abstractList.at( 0 ).toElement().text();
          }

          //DefaultSRS is always the first entry in the feature srs list
          QDomNodeList defaultCRSList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "DefaultSRS" );
          if ( defaultCRSList.length() > 0 )
          {
            featureCRSList.push_back( defaultCRSList.at( 0 ).toElement().text() );
          }

          //OtherSRS
          QDomNodeList otherCRSList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "OtherSRS" );
          for ( unsigned int i = 0; i < otherCRSList.length(); ++i )
          {
            featureCRSList.push_back( otherCRSList.at( i ).toElement().text() );
          }

          //Support <SRS> for compatibility with older versions
          QDomNodeList srsList = featureTypeElem.elementsByTagNameNS( WFS_NAMESPACE, "SRS" );
          for ( unsigned int i = 0; i < srsList.length(); ++i )
          {
            featureCRSList.push_back( srsList.at( i ).toElement().text() );
          }

          crs.push_back( featureCRSList );
          typenames.push_back( tname );
          titles.push_back( title );
          abstracts.push_back( abstract );
        }

        //insert the available CRS into mAvailableCRS
        mAvailableCRS.clear();
        std::list<QString>::const_iterator typeNameIter;
        std::list< std::list<QString> >::const_iterator crsIter;
        for ( typeNameIter = typenames.begin(), crsIter = crs.begin(); typeNameIter != typenames.end(); ++typeNameIter, ++crsIter )
        {
          std::list<QString> currentCRSList;
          for ( std::list<QString>::const_iterator it = crsIter->begin(); it != crsIter->end(); ++it )
          {
            currentCRSList.push_back( *it );
          }
          mAvailableCRS.insert( std::make_pair( *typeNameIter, currentCRSList ) );
        }

        //insert the typenames, titles and abstracts into the tree view
        std::list<QString>::const_iterator t_it = titles.begin();
        std::list<QString>::const_iterator n_it = typenames.begin();
        std::list<QString>::const_iterator a_it = abstracts.begin();
        for ( ; t_it != titles.end(); ++t_it, ++n_it, ++a_it )
        {
          QTreeWidgetItem* newItem = new QTreeWidgetItem();
          newItem->setText( 0, *t_it );
          newItem->setText( 1, *n_it );
          newItem->setText( 2, *a_it );
          treeWidget->addTopLevelItem( newItem );
        }

        if ( typenames.size() > 0 )
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
      else
      {
        QDomNode ex = doc.firstChild();
        QString exc = ex.toElement().attribute( "exceptionCode", "Exception" );
        QDomElement ext = ex.firstChild().toElement();
        QMessageBox::critical( 0, tr( "Error" ), exc + ": " + ext.firstChild().nodeValue() );
      }
    }
    else
    {
      QMessageBox::critical( 0, tr( "Capabilities document is not valid" ), capabilitiesDocError );
    }
  }
  else
  {
    QMessageBox::critical( 0, tr( "GetCapabilities Error" ), mCapabilitiesReply->errorString() );
  }

  btnConnect->setEnabled( true );
  mCapabilitiesReply->deleteLater();
  mCapabilitiesReply = 0;
}

void QgsWFSSourceSelect::capabilitiesReplyProgress( qint64, qint64 )
{
}

void QgsWFSSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/" );
  nc.setWindowTitle( tr( "Create a new WFS connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
  }
}

void QgsWFSSourceSelect::modifyEntryOfServerList()
{
  QgsNewHttpConnection nc( 0, "/Qgis/connections-wfs/", cmbConnections->currentText() );
  nc.setWindowTitle( tr( "Modify WFS connection" ) );

  if ( nc.exec() )
  {
    populateConnectionList();
  }
}

void QgsWFSSourceSelect::deleteEntryOfServerList()
{
  QSettings settings;
  QString key = "/Qgis/connections-wfs/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
  }
}

void QgsWFSSourceSelect::connectToServer()
{
  //find out the server URL
  QSettings settings;
  QString key = "/Qgis/connections-wfs/" + cmbConnections->currentText() + "/url";
  mUri = settings.value( key ).toString();
  QgsDebugMsg( QString( "url is: %1" ).arg( mUri ) );

  //make a GetCapabilities request
  //modify mUri to add '?' or '&' at the end if it is not already there
  if ( !( mUri.contains( "?" ) ) )
  {
    mUri.append( "?" );
  }
  else if (( mUri.right( 1 ) != "?" ) && ( mUri.right( 1 ) != "&" ) )
  {
    mUri.append( "&" );
  }

  btnConnect->setEnabled( false );
  treeWidget->clear();

  QNetworkRequest request( mUri + "SERVICE=WFS&REQUEST=GetCapabilities&VERSION=1.0.0" );
  request.setAttribute( QNetworkRequest::CacheSaveControlAttribute, true );
  mCapabilitiesReply = QgsNetworkAccessManager::instance()->get( request );
  connect( mCapabilitiesReply, SIGNAL( finished() ), this, SLOT( capabilitiesReplyFinished() ) );
  connect( mCapabilitiesReply, SIGNAL( downloadProgress( qint64, qint64 ) ), this, SLOT( capabilitiesReplyProgress( qint64, qint64 ) ) );
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

  QString uri = mUri;
  if ( !( uri.contains( "?" ) ) )
  {
    uri.append( "?" );
  }
  QgsDebugMsg( QString( "%1SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=%2" ).arg( uri ).arg( typeName ) );

  //get CRS
  QString crsString = labelCoordRefSys->text();
  if ( !crsString.isEmpty() )
  {
    crsString.prepend( "&SRSNAME=" );
  }

  QString filterString;
  if ( !mFilterLineEdit->text().isEmpty() )
  {
    filterString = ( "&FILTER=" + mFilterLineEdit->text() );
  }

  //add a wfs layer to the map
  if ( mIface )
  {
    //get current extent
    QgsMapCanvas* canvas = mIface->mapCanvas();
    QString bBoxString;
    if ( canvas && mBboxCheckBox->isChecked() )
    {
      QgsRectangle currentExtent = canvas->extent();
      bBoxString = QString( "&BBOX=%1,%2,%3,%4" )
                   .arg( currentExtent.xMinimum(), 0, 'f' )
                   .arg( currentExtent.yMinimum(), 0, 'f' )
                   .arg( currentExtent.xMaximum(), 0, 'f' )
                   .arg( currentExtent.yMaximum(), 0, 'f' );
    }
    mIface->addVectorLayer( uri + "SERVICE=WFS&VERSION=1.0.0&REQUEST=GetFeature&TYPENAME=" + typeName + crsString + bBoxString + filterString, typeName, "WFS" );
  }
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
  QSettings s;
  s.setValue( "/Qgis/connections-wfs/selected", cmbConnections->currentText() );
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
}
