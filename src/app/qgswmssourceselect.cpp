/***************************************************************************
    qgswmssourceselect.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG


 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
/* $Id$ */

#include "../providers/wms/qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID 
#include "qgisapp.h" //for getThemeIcon
#include "qgscontexthelp.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgenericprojectionselector.h"
#include "qgshttptransaction.h"
#include "qgslogger.h"
#include "qgsmessageviewer.h"
#include "qgsnewhttpconnection.h"
#include "qgsnumericsortlistviewitem.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmssourceselect.h"
#include <qgisinterface.h>


#include <QButtonGroup>
#include <QDomDocument>
#include <QHeaderView>
#include <QImageReader>
#include <QInputDialog>
#include <QMap>
#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QUrl>




QgsWMSSourceSelect::QgsWMSSourceSelect( QWidget * parent, Qt::WFlags fl )
    : QDialog( parent, fl ),
    m_Epsg( GEO_EPSG_CRS_ID ),
    mWmsProvider( 0 )
{
  setupUi( this );

  mAddButton = new QPushButton( tr( "&Add" ) );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) );

  mLayerUpButton->setIcon( QgisApp::getThemeIcon( "/mActionArrowUp.png" ) );
  mLayerDownButton->setIcon( QgisApp::getThemeIcon( "/mActionArrowDown.png" ) );

  mAddButton->setEnabled( false );
  populateConnectionList();

  // Qt Designer 4.1 doesn't let us use a QButtonGroup, so it has to
  // be done manually... Unless I'm missing something, it's a whole
  // lot harder to do groups of radio buttons in Qt4 than Qt3.
  m_imageFormatGroup = new QButtonGroup;
  m_imageFormatLayout = new QHBoxLayout;
  // Populate it with buttons for all of the formats that we can support. The
  // value is the user visible name, and a unique integer, and the key is the
  // mime type.
  QMap<QString, QPair<QString, int> > formats;
  // Note - the "PNG", etc text should match the Qt format strings as given by
  // a called QImageReader::supportedImageFormats(), but case doesn't matter
  m_PotentialFormats.insert( "image/png",             qMakePair( QString( "PNG" ),  1 ) );
  m_PotentialFormats.insert( "image/jpeg",            qMakePair( QString( "JPEG" ), 2 ) );
  m_PotentialFormats.insert( "image/gif",             qMakePair( QString( "GIF" ),  4 ) );
  // Desipte the Qt docs saying that it supports bmp, it practise it doesn't work...
  //m_PotentialFormats.insert("image/wbmp",            qMakePair(QString("BMP"),  5));
  // Some wms servers will support tiff, but by default Qt doesn't, but leave
  // this in for those versions of Qt that might support tiff
  m_PotentialFormats.insert( "image/tiff",            qMakePair( QString( "TIFF" ), 6 ) );
  //MH: UMN mapserver often offers png 24 bit
  m_PotentialFormats.insert( "image/png; mode=24bit", qMakePair( QString( "PNG" ), 7 ) );

  QMap<QString, QPair<QString, int> >::const_iterator iter = m_PotentialFormats.constBegin();
  int i = 1;
  while ( iter != m_PotentialFormats.end() )
  {
    QRadioButton* btn = new QRadioButton( iter.value().first );
    m_imageFormatGroup->addButton( btn, iter.value().second );
    m_imageFormatLayout->addWidget( btn );
    if ( i == 1 )
    {
      btn->setChecked( true );
    }
    iter++;
    i++;
  }

  m_imageFormatLayout->addStretch();
  btnGrpImageEncoding->setLayout( m_imageFormatLayout );

  // set up the WMS connections we already know about
  populateConnectionList();

  //set the current project CRS if available
  long currentCRS = QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectCRSID", -1 );
  if ( currentCRS != -1 )
  {
    //convert CRS id to epsg
    QgsCoordinateReferenceSystem currentRefSys( currentCRS, QgsCoordinateReferenceSystem::InternalCrsId );
    if ( currentRefSys.isValid() )
    {
      m_Epsg = currentRefSys.epsg();
    }
  }

  // set up the default WMS Coordinate Reference System
  labelCoordRefSys->setText( descriptionForEpsg( m_Epsg ) );


  //
  // For wms search tab
  //
  tableWidgetWMSList->setColumnWidth( 0, 250 );
  tableWidgetWMSList->setColumnWidth( 1, 150 );
  tableWidgetWMSList->setColumnWidth( 2, 250 );
  tableWidgetWMSList->verticalHeader()->hide();

  connect( tableWidgetWMSList, SIGNAL( itemSelectionChanged() ), this, SLOT( wmsSelectionChanged() ) );
}

QgsWMSSourceSelect::~QgsWMSSourceSelect()
{

}
void QgsWMSSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wms" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();

  if ( keys.begin() == keys.end() )
  {
    // No connections - disable various buttons
    btnConnect->setEnabled( FALSE );
    btnEdit->setEnabled( FALSE );
    btnDelete->setEnabled( FALSE );
  }
  else
  {
    // Connections - enable various buttons
    btnConnect->setEnabled( TRUE );
    btnEdit->setEnabled( TRUE );
    btnDelete->setEnabled( TRUE );
  }
}
void QgsWMSSourceSelect::on_btnNew_clicked()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this );

  if ( nc->exec() )
  {
    populateConnectionList();
  }
}

void QgsWMSSourceSelect::on_btnEdit_clicked()
{

  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, "/Qgis/connections-wms/", cmbConnections->currentText() );

  if ( nc->exec() )
  {
    populateConnectionList();
  }
}

void QgsWMSSourceSelect::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key );
    cmbConnections->removeItem( cmbConnections->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
  }
}

QgsNumericSortTreeWidgetItem *QgsWMSSourceSelect::createItem(
  int id, const QStringList &names, QMap<int, QgsNumericSortTreeWidgetItem *> &items, int &layerAndStyleCount,
  const QMap<int, int> &layerParents, const QMap<int, QStringList> &layerParentNames )

{
  if ( items.contains( id ) )
    return items[id];

  QgsNumericSortTreeWidgetItem *item;
  if ( layerParents.contains( id ) )
  {
    int parent = layerParents[ id ];
    item = new QgsNumericSortTreeWidgetItem( createItem( parent, layerParentNames[ parent ], items, layerAndStyleCount, layerParents, layerParentNames ) );
  }
  else
    item = new QgsNumericSortTreeWidgetItem( lstLayers );

  item->setText( 0, QString::number( ++layerAndStyleCount ) );
  item->setText( 1, names[0].simplified() );
  item->setText( 2, names[1].simplified() );
  item->setText( 3, names[2].simplified() );

  items[ id ] = item;

  return item;
}

bool QgsWMSSourceSelect::populateLayerList( QgsWmsProvider *wmsProvider )
{
  QVector<QgsWmsLayerProperty> layers;

  if ( !wmsProvider->supportedLayers( layers ) )
  {
    return FALSE;
  }

  QMap<int, QgsNumericSortTreeWidgetItem *> items;
  QMap<int, int> layerParents;
  QMap<int, QStringList> layerParentNames;
  wmsProvider->layerParents( layerParents, layerParentNames );

  lstLayers->clear();
  lstLayers->setSortingEnabled( true );

  int layerAndStyleCount = -1;

  for ( QVector<QgsWmsLayerProperty>::iterator layer = layers.begin();
        layer != layers.end();
        layer++ )
  {
    QgsNumericSortTreeWidgetItem *lItem = createItem( layer->orderId, QStringList() << layer->name << layer->title << layer->abstract, items, layerAndStyleCount, layerParents, layerParentNames );

    lItem->setData( 0, Qt::UserRole, layer->name );
    lItem->setData( 0, Qt::UserRole + 1, "" );

    // Also insert the styles
    // Layer Styles
    for ( int j = 0; j < layer->style.size(); j++ )
    {
      QgsDebugMsg( QString( "got style name %1 and title '%2'." ).arg( layer->style[j].name ).arg( layer->style[j].title ) );

      QgsNumericSortTreeWidgetItem *lItem2 = new QgsNumericSortTreeWidgetItem( lItem );
      lItem2->setText( 0, QString::number( ++layerAndStyleCount ) );
      lItem2->setText( 1, layer->style[j].name.simplified() );
      lItem2->setText( 2, layer->style[j].title.simplified() );
      lItem2->setText( 3, layer->style[j].abstract.simplified() );

      lItem2->setData( 0, Qt::UserRole, layer->name );
      lItem2->setData( 0, Qt::UserRole + 1, layer->style[j].name );
    }
  }

  // If we got some layers, let the user add them to the map
  if ( lstLayers->topLevelItemCount() > 0 )
  {
    mAddButton->setEnabled( TRUE );

    if ( lstLayers->topLevelItemCount() == 1 )
    {
      lstLayers->expandItem( lstLayers->topLevelItem( 0 ) );
    }
  }
  else
  {
    mAddButton->setEnabled( FALSE );
  }

  return TRUE;
}


void QgsWMSSourceSelect::populateImageEncodingGroup( QgsWmsProvider* wmsProvider )
{
  QStringList formats;

  formats = wmsProvider->supportedImageEncodings();

  //
  // Remove old group of buttons
  //
  QList<QAbstractButton*> btns = m_imageFormatGroup->buttons();
  for ( int i = 0; i < btns.count(); ++i )
  {
    btns.at( i )->hide();
  }

  //
  // Collect capabilities reported by Qt itself
  //
  QList<QByteArray> qtImageFormats = QImageReader::supportedImageFormats();

#ifdef QGISDEBUG
  QList<QByteArray>::const_iterator it = qtImageFormats.begin();
  while ( it != qtImageFormats.end() )
  {
    QgsDebugMsg( QString( "can support input of '%1'." ).arg(( *it ).data() ) );
    ++it;
  }
#endif

  //
  // Add new group of buttons
  //

  bool first = true;
  for ( QStringList::Iterator format  = formats.begin();
        format != formats.end();
        ++format )
  {
    QgsDebugMsg( QString( "got image format %1." ).arg(( *format ) ) );

    QMap<QString, QPair<QString, int> >::const_iterator iter =
      m_PotentialFormats.find( *format );

    // The formats in qtImageFormats are lowercase, so force ours to lowercase too.
    if ( iter != m_PotentialFormats.end() &&
         qtImageFormats.contains( iter.value().first.toLower().toLocal8Bit() ) )
    {
      m_imageFormatGroup->button( iter.value().second )->show();
      if ( first )
      {
        first = false;
        m_imageFormatGroup->button( iter.value().second )->setChecked( true );
      }
    }
    else
    {
      QgsDebugMsg( QString( "Unsupported type of %1" ).arg( format->toLocal8Bit().data() ) );
    }
  }
}


void QgsWMSSourceSelect::on_btnConnect_clicked()
{
  // populate the table list
  QSettings settings;

  QString key = "/Qgis/connections-wms/" + cmbConnections->currentText();
  QString credentialsKey = "/Qgis/WMS/" + cmbConnections->currentText();

  QStringList connStringParts;
  QString part;

  connStringParts += settings.value( key + "/url" ).toString();

  m_connName = cmbConnections->currentText();
  m_connectionInfo = connStringParts.join( " " );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    if ( password.isEmpty() )
    {
      password = QInputDialog::getText( this, tr( "WMS Password for %1" ).arg( m_connName ), "Password", QLineEdit::Password );

    }
    m_connectionInfo = "username=" + username + ",password=" + password + ",url=" + m_connectionInfo;
  }


  QgsDebugMsg( QString( "Connection info: '%1'." ).arg( m_connectionInfo ) );


  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  mWmsProvider =
    ( QgsWmsProvider* ) pReg->getProvider( "wms", m_connectionInfo );

  if ( mWmsProvider )
  {
    connect( mWmsProvider, SIGNAL( statusChanged( QString ) ), this, SLOT( showStatusMessage( QString ) ) );

    // WMS Provider all set up; let's get some layers

    if ( !populateLayerList( mWmsProvider ) )
    {
      showError( mWmsProvider );
    }
    populateImageEncodingGroup( mWmsProvider );
  }
  else
  {
    // Let user know we couldn't initialise the WMS provider
    QMessageBox::warning(
      this,
      tr( "WMS Provider" ),
      tr( "Could not open the WMS Provider" )
    );
  }

}

void QgsWMSSourceSelect::addClicked()
{
  if ( selectedLayers().empty() == TRUE )
  {
    QMessageBox::information( this, tr( "Select Layer" ), tr( "You must select at least one leaf layer first." ) );
  }
  else if ( mWmsProvider->supportedCrsForLayers( selectedLayers() ).size() == 0 )
  {
    QMessageBox::information( this, tr( "Coordinate Reference System" ), tr( "There are no available coordinate reference system for the set of layers you've selected." ) );
  }
  else
  {
    QgisApp::instance()->addRasterLayer(
      connectionInfo(),
      leLayerName->text().isEmpty() ? selectedLayers().join( "/" ) : leLayerName->text(),
      "wms",
      selectedLayers(),
      selectedStylesForSelectedLayers(),
      selectedImageEncoding(),
      selectedCrs() );
  }
}


void QgsWMSSourceSelect::on_btnChangeSpatialRefSys_clicked()
{
  if ( !mWmsProvider )
  {
    return;
  }

  QSet<QString> crsFilter = mWmsProvider->supportedCrsForLayers( selectedLayers() );

  QgsGenericProjectionSelector * mySelector =
    new QgsGenericProjectionSelector( this );
  mySelector->setMessage();

  mySelector->setOgcWmsCrsFilter( crsFilter );

  QString myDefaultProjString = QgsProject::instance()->readEntry( "SpatialRefSys", "/ProjectCRSProj4String", GEOPROJ4 );
  QgsCoordinateReferenceSystem defaultCRS;
  if ( defaultCRS.createFromProj4( myDefaultProjString ) )
  {
    mySelector->setSelectedCrsId( defaultCRS.srsid() );
  }

  if ( mySelector->exec() )
  {
    m_Epsg = mySelector->selectedEpsg();
  }
  else
  {
    // NOOP
  }

  labelCoordRefSys->setText( descriptionForEpsg( m_Epsg ) );
//  labelCoordRefSys->setText( mySelector->selectedProj4String() );

  delete mySelector;

  // update the display of this widget
  this->update();
}


/**
 * This function is used to:
 * 1. Store the list of selected layers and visual styles as appropriate.
 * 2. Ensure that only one style is selected per layer.
 *    If more than one is found, the most recently selected style wins.
 */
void QgsWMSSourceSelect::on_lstLayers_itemSelectionChanged()
{
  QStringList newSelectedLayers;
  QStringList newSelectedStylesForSelectedLayers;

  QMap<QString, QString> newSelectedStyleIdForLayer;

  // Iterate through the layers
  QList<QTreeWidgetItem *> selected( lstLayers->selectedItems() );
  QList<QTreeWidgetItem *>::iterator it;
  for ( it = selected.begin(); it != selected.end(); ++it )
  {
    QTreeWidgetItem *item = *it;

    QString layerName = item->data( 0, Qt::UserRole ).toString();
    if ( layerName.isEmpty() )
      continue;

    newSelectedLayers << layerName;
    newSelectedStylesForSelectedLayers << item->data( 0, Qt::UserRole + 1 ).toString();

    newSelectedStyleIdForLayer[layerName] = item->text( 0 );

    // Check if multiple styles have now been selected
    if (
      ( !( m_selectedStyleIdForLayer[layerName].isNull() ) ) &&  // not just isEmpty()
      ( newSelectedStyleIdForLayer[layerName] != m_selectedStyleIdForLayer[layerName] )
    )
    {
      // Remove old style selection
      lstLayers->findItems( m_selectedStyleIdForLayer[layerName], Qt::MatchRecursive ).first()->setSelected( false );
      // Remove old layer/style pair also from newSelectedLayers / newSelectedStylesForSelectedLayers
      int oldIndex = newSelectedLayers.indexOf( layerName );
      if ( oldIndex != -1 )
      {
        newSelectedLayers.removeAt( oldIndex );
        newSelectedStylesForSelectedLayers.removeAt( oldIndex );
      }
    }

    QgsDebugMsg( QString( "Added %1" ).arg( item->text( 0 ) ) );

  }

  // If we got some selected items, let the user play with projections
  if ( newSelectedLayers.count() > 0 )
  {
    // Determine how many CRSs there are to play with
    if ( mWmsProvider )
    {
      QSet<QString> crsFilter = mWmsProvider->supportedCrsForLayers( newSelectedLayers );

      gbCRS->setTitle( tr( "Coordinate Reference System (%n available)", "crs count", crsFilter.count() ) );

      // check whether current CRS is supported
      // if not, use one of the available CRS
      bool isThere = false;
      long defaultEpsg = 0;
      for ( QSet<QString>::const_iterator i = crsFilter.begin(); i != crsFilter.end(); ++i )
      {
        QStringList parts = i->split( ":" );

        if ( parts.at( 0 ).compare( "EPSG", Qt::CaseInsensitive ) == 0 )
        {
          long epsg = atol( parts.at( 1 ).toUtf8() );
          if ( epsg == m_Epsg )
          {
            isThere = true;
            break;
          }

          // save first CRS in case we current m_Epsg is not available
          if ( i == crsFilter.begin() )
            defaultEpsg = epsg;
          // prefer value of DEFAULT_GEO_EPSG_CRS_ID if available
          if ( epsg == GEO_EPSG_CRS_ID )
            defaultEpsg = epsg;
        }
      }

      // we have to change selected CRS to default one
      if ( !isThere && crsFilter.size() > 0 )
      {
        m_Epsg = defaultEpsg;
        labelCoordRefSys->setText( descriptionForEpsg( m_Epsg ) );
      }
    }

    btnChangeSpatialRefSys->setEnabled( TRUE );
  }
  else
  {
    btnChangeSpatialRefSys->setEnabled( FALSE );
  }

  m_selectedStyleIdForLayer = newSelectedStyleIdForLayer;
  updateLayerOrderTab( newSelectedLayers, newSelectedStylesForSelectedLayers );
}


QString QgsWMSSourceSelect::connName()
{
  return m_connName;
}

QString QgsWMSSourceSelect::connectionInfo()
{
  return m_connectionInfo;
}

QStringList QgsWMSSourceSelect::selectedLayers()
{
  //go through list in layer order tab
  QStringList selectedLayerList;
  for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
  {
    selectedLayerList << mLayerOrderTreeWidget->topLevelItem( i )->text( 0 );
  }
  return selectedLayerList;
}

QStringList QgsWMSSourceSelect::selectedStylesForSelectedLayers()
{
  //go through list in layer order tab
  QStringList selectedStyleList;
  for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
  {
    selectedStyleList << mLayerOrderTreeWidget->topLevelItem( i )->text( 1 );
  }
  return selectedStyleList;
}


QString QgsWMSSourceSelect::selectedImageEncoding()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.

  int id = m_imageFormatGroup->checkedId();
  QString label = m_imageFormatGroup->button( id )->text();

  // The way that we construct the buttons, this call to key() will never have
  // a value that isn't in the map, hence we don't need to check for the value
  // not being found.

  QString imageEncoding = m_PotentialFormats.key( qMakePair( label, id ) );

  //substitute blanks with %20 (e.g. in "image/png; mode=24bit")
  imageEncoding.replace( QRegExp( " " ), "%20" );
  return imageEncoding;
}


QString QgsWMSSourceSelect::selectedCrs()
{
  if ( m_Epsg )
  {
    return QString( "EPSG:%1" )
           .arg( m_Epsg );
  }
  else
  {
    return QString();
  }
}

void QgsWMSSourceSelect::serverChanged()
{
  // Remember which server was selected.
  QSettings settings;
  settings.setValue( "/Qgis/connections-wms/selected",
                     cmbConnections->currentText() );
}

void QgsWMSSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  QString toSelect = settings.value( "/Qgis/connections-wms/selected" ).toString();
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for ( int i = 0; i < cmbConnections->count(); ++i )
    if ( cmbConnections->itemText( i ) == toSelect )
    {
      cmbConnections->setCurrentIndex( i );
      set = true;
      break;
    }
  // If we couldn't find the stored item, but there are some,
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if ( !set && cmbConnections->count() > 0 )
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}
void QgsWMSSourceSelect::showStatusMessage( QString const & theMessage )
{
  labelStatus->setText( theMessage );

  // update the display of this widget
  this->update();
}


void QgsWMSSourceSelect::showError( QgsWmsProvider * wms )
{
#if 0
  QMessageBox::warning(
    this,
    wms->lastErrorTitle(),
    tr( "Could not understand the response.  The %1 provider said:\n%2", "COMMENTED OUT" )
    .arg( wms->name() ).arg( wms->lastError() )
  );
#endif

  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( wms->lastErrorTitle() );
  mv->setMessageAsPlainText( tr( "Could not understand the response.  The %1 provider said:\n%2" )
                             .arg( wms->name() ).arg( wms->lastError() )
                           );
  mv->showMessage( true ); // Is deleted when closed
}

void QgsWMSSourceSelect::on_cmbConnections_activated( int )
{
  serverChanged();
}

void QgsWMSSourceSelect::on_btnAddDefault_clicked()
{
  addDefaultServers();
}

QString QgsWMSSourceSelect::descriptionForEpsg( long epsg )
{
  // We'll assume this function isn't called very often,
  // so please forgive the lack of caching of results

  QgsCoordinateReferenceSystem qgisSrs = QgsCoordinateReferenceSystem( epsg, QgsCoordinateReferenceSystem::EpsgCrsId );

  return qgisSrs.description();
}

void QgsWMSSourceSelect::addDefaultServers()
{
  QMap<QString, QString> exampleServers;
  exampleServers["NASA (JPL)"] = "http://wms.jpl.nasa.gov/wms.cgi";
  exampleServers["DM Solutions GMap"] = "http://www2.dmsolutions.ca/cgi-bin/mswms_gmap";
  exampleServers["Lizardtech server"] =  "http://wms.lizardtech.com/lizardtech/iserv/ows";
  // Nice to have the qgis users map, but I'm not sure of the URL at the moment.
  //  exampleServers["Qgis users map"] = "http://qgis.org/wms.cgi";

  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wms" );
  QMap<QString, QString>::const_iterator i = exampleServers.constBegin();
  for ( ; i != exampleServers.constEnd(); ++i )
  {
    // Only do a server if it's name doesn't already exist.
    QStringList keys = settings.childGroups();
    if ( !keys.contains( i.key() ) )
    {
      QString path = i.key();
      settings.setValue( path + "/url", i.value() );
    }
  }
  settings.endGroup();
  populateConnectionList();

  QMessageBox::information( this, tr( "WMS proxies" ), "<p>" + tr( "Several WMS servers have "
                            "been added to the server list. Note that if "
                            "you access the internet via a web proxy, you will "
                            "need to set the proxy settings in the QGIS options dialog." ) + "</p>" );
}

bool QgsWMSSourceSelect::retrieveSearchResults( const QString& searchTerm, QByteArray& httpResponse )
{
  // TODO: test proxy
  // read proxy settings: code from QgsWmsProvider::retrieveUrl()
  QSettings settings;
  QString proxyHost, proxyUser, proxyPassword;
  int proxyPort = 0;
  QNetworkProxy::ProxyType proxyType = QNetworkProxy::NoProxy;

  bool proxyEnabled = settings.value( "proxy/proxyEnabled", "0" ).toBool();
  if ( proxyEnabled )
  {
    proxyHost = settings.value( "proxy/proxyHost", "" ).toString();
    proxyPort = settings.value( "proxy/proxyPort", "" ).toString().toInt();
    proxyUser = settings.value( "proxy/proxyUser", "" ).toString();
    proxyPassword = settings.value( "proxy/proxyPassword", "" ).toString();
    QString proxyTypeString =  settings.value( "proxy/proxyType", "" ).toString();
    if ( proxyTypeString == "DefaultProxy" )
    {
      proxyType = QNetworkProxy::DefaultProxy;
    }
    else if ( proxyTypeString == "Socks5Proxy" )
    {
      proxyType = QNetworkProxy::Socks5Proxy;
    }
    else if ( proxyTypeString == "HttpProxy" )
    {
      proxyType = QNetworkProxy::HttpProxy;
    }
#if QT_VERSION >= 0x040400
    else if ( proxyTypeString == "HttpCachingProxy" )
    {
      proxyType = QNetworkProxy::HttpCachingProxy;
    }
    else if ( proxyTypeString == "FtpCachingProxy" )
    {
      proxyType = QNetworkProxy::FtpCachingProxy;
    }
#endif
  }
  // Get username/password from settings for protected WMS

  QUrl url( QString( "http://geopole.org/wms/search?search=%1&type=rss" ).arg( searchTerm ) );
  QgsHttpTransaction http( url.toEncoded(),
                           proxyHost, proxyPort, proxyUser, proxyPassword, proxyType );

  bool httpOk = http.getSynchronously( httpResponse );
  if ( !httpOk )
  {
    // TODO: error handling
    return false;
  }

  // TODO: check doctype?

  return true;
}

void QgsWMSSourceSelect::addWMSListRow( const QDomElement& item, int row )
{
  QDomElement title = item.firstChildElement( "title" );
  addWMSListItem( title, row, 0 );
  QDomElement link = item.firstChildElement( "link" );
  addWMSListItem( link, row, 1 );
  QDomElement description = item.firstChildElement( "description" );
  addWMSListItem( description, row, 2 );
}

void QgsWMSSourceSelect::addWMSListItem( const QDomElement& el, int row, int column )
{
  if ( !el.isNull() )
  {
    QTableWidgetItem* tableItem = new QTableWidgetItem( el.text() );
    // TODO: add linebreaks to long tooltips?
    tableItem->setToolTip( el.text() );
    tableWidgetWMSList->setItem( row, column, tableItem );
  }
}

void QgsWMSSourceSelect::on_btnSearch_clicked()
{
  // clear results
  tableWidgetWMSList->clearContents();
  tableWidgetWMSList->setRowCount( 0 );

  // disable Add WMS button
  btnAddWMS->setEnabled( false );

  // retrieve search results
  QByteArray httpResponse;
  bool success = retrieveSearchResults( leSearchTerm->text(), httpResponse );
  if ( !success )
  {
    // TODO: error handling
    return;
  }

  // parse results
  QDomDocument doc( "RSS" );
  if ( !doc.setContent( httpResponse ) )
  {
    // TODO: error handling
    return;
  }

  QDomNodeList list = doc.elementsByTagName( "item" );
  tableWidgetWMSList->setRowCount( list.size() );
  for ( int i = 0; i < list.size(); i++ )
  {
    if ( list.item( i ).isElement() )
    {
      QDomElement item = list.item( i ).toElement();
      addWMSListRow( item, i );
    }
  }
}

void QgsWMSSourceSelect::on_btnAddWMS_clicked()
{
  // TODO: deactivate button if dialog is open?
  // TODO: remove from config on close?

  int selectedRow = tableWidgetWMSList->currentRow();
  if ( selectedRow == -1 )
  {
    return;
  }

  QString wmsTitle = tableWidgetWMSList->item( selectedRow, 0 )->text();
  QString wmsUrl = tableWidgetWMSList->item( selectedRow, 1 )->text();

  QSettings settings;
  if ( settings.contains( QString( "Qgis/connections-wms/%1/url" ).arg( wmsTitle ) ) )
  {
    QString msg = tr( "The %1 connection already exists. Do you want to overwrite it?" ).arg( wmsTitle );
    QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Overwrite" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
    if ( result != QMessageBox::Ok )
    {
      return;
    }
  }

  // add selected WMS to config and mark as current
  settings.setValue( QString( "Qgis/connections-wms/%1/url" ).arg( wmsTitle ), wmsUrl );
  settings.setValue( "/Qgis/connections-wms/selected", wmsTitle );
  populateConnectionList();
  tabWidget->setCurrentIndex( 0 );
}

void QgsWMSSourceSelect::wmsSelectionChanged()
{
  btnAddWMS->setEnabled( tableWidgetWMSList->currentRow() != -1 );
}

void QgsWMSSourceSelect::on_mLayerUpButton_clicked()
{
  QList<QTreeWidgetItem *> selectionList = mLayerOrderTreeWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    return;
  }
  int selectedIndex = mLayerOrderTreeWidget->indexOfTopLevelItem( selectionList[0] );
  if ( selectedIndex < 1 )
  {
    return; //item not existing or already on top
  }

  QTreeWidgetItem* selectedItem = mLayerOrderTreeWidget->takeTopLevelItem( selectedIndex );
  mLayerOrderTreeWidget->insertTopLevelItem( selectedIndex - 1, selectedItem );
  mLayerOrderTreeWidget->clearSelection();
  selectedItem->setSelected( true );
}

void QgsWMSSourceSelect::on_mLayerDownButton_clicked()
{
  QList<QTreeWidgetItem *> selectionList = mLayerOrderTreeWidget->selectedItems();
  if ( selectionList.size() < 1 )
  {
    return;
  }
  int selectedIndex = mLayerOrderTreeWidget->indexOfTopLevelItem( selectionList[0] );
  if ( selectedIndex < 0 || selectedIndex > mLayerOrderTreeWidget->topLevelItemCount() - 2 )
  {
    return; //item not existing or already at bottom
  }

  QTreeWidgetItem* selectedItem = mLayerOrderTreeWidget->takeTopLevelItem( selectedIndex );
  mLayerOrderTreeWidget->insertTopLevelItem( selectedIndex + 1, selectedItem );
  mLayerOrderTreeWidget->clearSelection();
  selectedItem->setSelected( true );
}

void QgsWMSSourceSelect::updateLayerOrderTab( const QStringList& newLayerList, const QStringList& newStyleList )
{
  //check, if each layer / style combination is already contained in the  layer order tab
  //if not, add it to the top of the list

  QStringList::const_iterator layerListIt = newLayerList.constBegin();
  QStringList::const_iterator styleListIt = newStyleList.constBegin();

  for ( ; layerListIt != newLayerList.constEnd(); ++layerListIt, ++styleListIt )
  {
    bool combinationExists = false;
    for ( int i = 0; i < mLayerOrderTreeWidget->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem* currentItem = mLayerOrderTreeWidget->topLevelItem( i );
      if ( currentItem->text( 0 ) == *layerListIt && currentItem->text( 1 ) == *styleListIt )
      {
        combinationExists = true;
        break;
      }
    }

    if ( !combinationExists )
    {
      QTreeWidgetItem* newItem = new QTreeWidgetItem();
      newItem->setText( 0, *layerListIt );
      newItem->setText( 1, *styleListIt );
      mLayerOrderTreeWidget->addTopLevelItem( newItem );
    }

  }

  //check, if each layer style combination in the layer order tab is still in newLayerList / newStyleList
  //if not: remove it from the tree widget

  if ( mLayerOrderTreeWidget->topLevelItemCount() > 0 )
  {
    for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
    {
      QTreeWidgetItem* currentItem = mLayerOrderTreeWidget->topLevelItem( i );
      bool combinationExists = false;

      QStringList::const_iterator llIt = newLayerList.constBegin();
      QStringList::const_iterator slIt = newStyleList.constBegin();
      for ( ; llIt != newLayerList.constEnd(); ++llIt, ++slIt )
      {
        if ( *llIt == currentItem->text( 0 ) && *slIt == currentItem->text( 1 ) )
        {
          combinationExists = true;
          break;
        }
      }

      if ( !combinationExists )
      {
        mLayerOrderTreeWidget->takeTopLevelItem( i );
      }
    }
  }

}

//
//
// ENDS
