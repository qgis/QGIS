/***************************************************************************
    qgsowssourceselect.cpp  -  selector for WMS,WFS,WCS
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

    generalized          : (C) 2012 Radim Blazek, based on qgswmssourceselect.cpp

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgscontexthelp.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsgenericprojectionselector.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsmessageviewer.h"
#include "qgsnewhttpconnection.h"
#include "qgsnumericsortlistviewitem.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgsowsconnection.h"
#include "qgsdataprovider.h"
#include "qgsowssourceselect.h"
#include "qgsnetworkaccessmanager.h"

#include <QButtonGroup>
#include <QFileDialog>
#include <QRadioButton>
#include <QDomDocument>
#include <QHeaderView>
#include <QImageReader>
#include <QInputDialog>
#include <QMap>
#include <QMessageBox>
#include <QPicture>
#include <QSettings>
#include <QUrl>
#include <QValidator>

#include <QNetworkRequest>
#include <QNetworkReply>

QgsOWSSourceSelect::QgsOWSSourceSelect( QString service, QWidget * parent, Qt::WFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mService( service )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mCurrentTileset( 0 )
{
  setupUi( this );

  if ( mEmbeddedMode )
  {
    mDialogButtonBox->button( QDialogButtonBox::Close )->hide();
  }

  mAddButton = mDialogButtonBox->button( QDialogButtonBox::Apply );
  mAddButton->setText( tr( "&Add" ) );
  mAddButton->setToolTip( tr( "Add selected layers to map" ) );
  mAddButton->setEnabled( false );

  mTileWidthLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );
  mTileHeightLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );
  mFeatureCountLineEdit->setValidator( new QIntValidator( 0, 9999, this ) );

  mImageFormatGroup = new QButtonGroup;

  if ( !mManagerMode )
  {
    connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) );
    //set the current project CRS if available
    long currentCRS = QgsProject::instance()->readNumEntry( "SpatialRefSys", "/ProjectCRSID", -1 );
    if ( currentCRS != -1 )
    {
      //convert CRS id to epsg
      QgsCoordinateReferenceSystem currentRefSys( currentCRS, QgsCoordinateReferenceSystem::InternalCrsId );
      if ( currentRefSys.isValid() )
      {
        mSelectedCRS = currentRefSys.authid();
      }
    }
  }
  else
  {
    mTabWidget->removeTab( mTabWidget->indexOf( mLayerOrderTab ) );
    mTabWidget->removeTab( mTabWidget->indexOf( mTilesetsTab ) );
    mImageFormatsGroupBox->hide();
    mLayersTab->layout()->removeWidget( mImageFormatsGroupBox );
    mCRSGroupBox->hide();
    mLayersTab->layout()->removeWidget( mCRSGroupBox );
    mAddButton->hide();
  }

  // set up the WMS connections we already know about
  populateConnectionList();

  QSettings settings;
  QgsDebugMsg( "restoring geometry" );
  restoreGeometry( settings.value( "/Windows/WMSSourceSelect/geometry" ).toByteArray() );
}

QgsOWSSourceSelect::~QgsOWSSourceSelect()
{
  QSettings settings;
  QgsDebugMsg( "saving geometry" );
  settings.setValue( "/Windows/WMSSourceSelect/geometry", saveGeometry() );
}

void QgsOWSSourceSelect::clearFormats()
{
  int i = 0;
  while ( QRadioButton *btn = dynamic_cast<QRadioButton*>( mImageFormatGroup->button( i++ ) ) )
  {
    btn->setVisible( false );
  }
}

void QgsOWSSourceSelect::populateFormats()
{
  QgsDebugMsg( "entered" );

  // A server may offer more similar formats, which are mapped
  // to the same GDAL format, e.g. GeoTIFF and TIFF
  // -> recreate always buttons for all available formats, enable supported

  clearFormats();

  QHBoxLayout *layout = dynamic_cast<QHBoxLayout*>( mImageFormatsGroupBox->layout() );
  if ( !layout )
  {
    layout = new QHBoxLayout;
    mImageFormatsGroupBox->setLayout( layout );
    layout->addStretch();
  }

  if ( mProviderFormats.size() == 0 )
  {
    mProviderFormats = providerFormats();
    for ( int i = 0; i < mProviderFormats.size(); i++ )
    {
      // GDAL mime types may be image/tiff, image/png, ...
      mMimeLabelMap.insert( mProviderFormats[i].format, mProviderFormats[i].label );
    }
  }

  // selectedLayersFormats may come in various forms:
  // image/tiff, GTiff, GeoTIFF, TIFF, geotiff_int16, geotiff_rgb,
  // PNG, GTOPO30, ARCGRID, IMAGEMOSAIC,
  QMap<QString, QString> formatsMap;
  formatsMap.insert( "geotiff", "tiff" );
  formatsMap.insert( "gtiff", "tiff" );
  formatsMap.insert( "tiff", "tiff" );
  formatsMap.insert( "tif", "tiff" );
  formatsMap.insert( "gif", "gif" );
  formatsMap.insert( "jpeg", "jpeg" );
  formatsMap.insert( "jpg", "jpeg" );
  formatsMap.insert( "png", "png" );

  int prefered = -1;
  int firstEnabled = -1;
  QStringList layersFormats = selectedLayersFormats();
  for ( int i = 0; i < layersFormats.size(); i++ )
  {
    QString format = layersFormats.value( i );
    QgsDebugMsg( "server format = " + format );
    QString simpleFormat = format.toLower().replace( "image/", "" ).replace( QRegExp( "_.*" ), "" );
    QgsDebugMsg( "server simpleFormat = " + simpleFormat );
    QString mimeFormat = "image/" + formatsMap.value( simpleFormat );
    QgsDebugMsg( "server mimeFormat = " + mimeFormat );

    QString label = format;
    QString tip = tr( "Server format" ) + " " + format;

    QRadioButton *btn;
    btn = dynamic_cast<QRadioButton*>( mImageFormatGroup->button( i ) );
    if ( !btn )
    {
      btn = new QRadioButton( label );
      mImageFormatGroup->addButton( btn, i );
      layout->insertWidget( layout->count() - 1, btn ); // before stretch
    }
    btn->setVisible( true );

    if ( mMimeLabelMap.contains( mimeFormat ) )
    {
      btn->setEnabled( true );
      if ( format != mMimeLabelMap.value( mimeFormat ) )
      {
        label += " / " + mMimeLabelMap.value( mimeFormat );
      }
      tip += " " + tr( "is supported by GDAL %1 driver." ).arg( mMimeLabelMap.value( mimeFormat ) );
      if ( firstEnabled < 0 ) { firstEnabled = i; }
      if ( simpleFormat.contains( "tif" ) ) // prefer *tif*
      {
        if ( prefered < 0 || simpleFormat.startsWith( "g" ) ) // prefere geotiff
        {
          prefered = i;
        }
      }
    }
    else
    {
      QgsDebugMsg( QString( "format %1 not supported." ).arg( format ) );
      btn->setEnabled( false );
      tip += " " + tr( "is not supported by GDAL" );
    }
    btn->setText( label );
    btn->setToolTip( tip );
  }
  // Set prefered
  prefered = prefered >= 0 ? prefered : firstEnabled;
  if ( prefered >= 0 )
  {
    mImageFormatGroup->button( prefered )->setChecked( true );
  }

  mImageFormatsGroupBox->setEnabled( true );
}

void QgsOWSSourceSelect::populateConnectionList()
{
  mConnectionsComboBox->clear();
  mConnectionsComboBox->addItems( QgsOWSConnection::connectionList( mService ) );

  setConnectionListPosition();

  if ( mConnectionsComboBox->count() == 0 )
  {
    // No connections - disable various buttons
    mConnectButton->setEnabled( false );
    mEditButton->setEnabled( false );
    mDeleteButton->setEnabled( false );
  }
  else
  {
    // Connections - enable various buttons
    mConnectButton->setEnabled( true );
    mEditButton->setEnabled( true );
    mDeleteButton->setEnabled( true );
  }
}
void QgsOWSSourceSelect::on_mNewButton_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, "/Qgis/connections-" + mService.toLower() + "/" );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsOWSSourceSelect::on_mEditButton_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, "/Qgis/connections-" + mService.toLower() + "/", mConnectionsComboBox->currentText() );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsOWSSourceSelect::on_mDeleteButton_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( mConnectionsComboBox->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsOWSConnection::deleteConnection( mService, mConnectionsComboBox->currentText() );
    mConnectionsComboBox->removeItem( mConnectionsComboBox->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
    emit connectionsChanged();
  }
}

void QgsOWSSourceSelect::on_mSaveButton_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WCS );
  dlg.exec();
}

void QgsOWSSourceSelect::on_mLoadButton_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WCS, fileName );
  dlg.exec();
  populateConnectionList();
  emit connectionsChanged();
}

QgsNumericSortTreeWidgetItem *QgsOWSSourceSelect::createItem(
  int id,
  const QStringList &names,
  QMap<int, QgsNumericSortTreeWidgetItem *> &items,
  int &layerAndStyleCount,
  const QMap<int, int> &layerParents,
  const QMap<int, QStringList> &layerParentNames )
{
  QgsDebugMsg( QString( "id = %1 layerAndStyleCount = %2 names = %3 " ).arg( id ).arg( layerAndStyleCount ).arg( names.join( "," ) ) );
  if ( items.contains( id ) )
    return items[id];


  QgsNumericSortTreeWidgetItem *item;
  if ( layerParents.contains( id ) )
  {
    // it has parent -> create first its parent
    int parent = layerParents[ id ];
    item = new QgsNumericSortTreeWidgetItem( createItem( parent, layerParentNames[ parent ], items, layerAndStyleCount, layerParents, layerParentNames ) );
  }
  else
    item = new QgsNumericSortTreeWidgetItem( mLayersTreeWidget );

  item->setText( 0, QString::number( ++layerAndStyleCount ) );
  item->setText( 1, names[0].simplified() );
  item->setText( 2, names[1].simplified() );
  item->setText( 3, names[2].simplified() );
  item->setToolTip( 3, "<font color=black>" + names[2].simplified()  + "</font>" );

  items[ id ] = item;

  return item;
}

void QgsOWSSourceSelect::populateLayerList( )
{
}

void QgsOWSSourceSelect::on_mConnectButton_clicked()
{
  QgsDebugMsg( "entered" );

  mLayersTreeWidget->clear();
  clearFormats();

  mConnName = mConnectionsComboBox->currentText();

  QgsOWSConnection connection( mService, mConnectionsComboBox->currentText() );
  //QgsDataProvider *theProvider = connection.provider( );
  mConnectionInfo = connection.connectionInfo();
  mUri = connection.uri();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsDebugMsg( "call populateLayerList" );
  populateLayerList();

  QApplication::restoreOverrideCursor();
}

void QgsOWSSourceSelect::addClicked()
{
  QgsDebugMsg( "entered" );
}

void QgsOWSSourceSelect::enableLayersForCrs( QTreeWidgetItem * )
{
}

void QgsOWSSourceSelect::on_mChangeCRSButton_clicked()
{
  QStringList layers;
  foreach( QTreeWidgetItem *item, mLayersTreeWidget->selectedItems() )
  {
    QString layer = item->data( 0, Qt::UserRole + 0 ).toString();
    if ( !layer.isEmpty() )
      layers << layer;
  }

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setOgcWmsCrsFilter( mSelectedLayersCRSs );

  QString myDefaultCrs = QgsProject::instance()->readEntry( "SpatialRefSys", "/ProjectCrs", GEO_EPSG_CRS_AUTHID );
  QgsCoordinateReferenceSystem defaultCRS;
  if ( defaultCRS.createFromOgcWmsCrs( myDefaultCrs ) )
  {
    mySelector->setSelectedCrsId( defaultCRS.srsid() );
  }

  if ( !mySelector->exec() )
    return;

  mSelectedCRS = mySelector->selectedAuthId();
  delete mySelector;

  mSelectedCRSLabel->setText( descriptionForAuthId( mSelectedCRS ) );

  for ( int i = 0; i < mLayersTreeWidget->topLevelItemCount(); i++ )
  {
    enableLayersForCrs( mLayersTreeWidget->topLevelItem( i ) );
  }

  updateButtons();
}

void QgsOWSSourceSelect::on_mLayersTreeWidget_itemSelectionChanged()
{
}

void QgsOWSSourceSelect::populateCRS()
{
  mSelectedLayersCRSs = selectedLayersCRSs().toSet();
  mCRSGroupBox->setTitle( tr( "Coordinate Reference System (%n available)", "crs count", mSelectedLayersCRSs.count() ) );

  mChangeCRSButton->setDisabled( mSelectedLayersCRSs.isEmpty() );

  if ( !mSelectedLayersCRSs.isEmpty() )
  {
    // check whether current CRS is supported
    // if not, use one of the available CRS
    QString defaultCRS;
    QSet<QString>::const_iterator it = mSelectedLayersCRSs.begin();
    for ( ; it != mSelectedLayersCRSs.end(); it++ )
    {
      if ( it->compare( mSelectedCRS, Qt::CaseInsensitive ) == 0 )
        break;

      // save first CRS in case the current CRS is not available
      if ( it == mSelectedLayersCRSs.begin() )
        defaultCRS = *it;

      // prefer value of DEFAULT_GEO_EPSG_CRS_ID if available
      if ( *it == GEO_EPSG_CRS_AUTHID )
        defaultCRS = *it;
    }

    if ( it == mSelectedLayersCRSs.end() )
    {
      // not found
      mSelectedCRS = defaultCRS;
    }
    mSelectedCRSLabel->setText( descriptionForAuthId( mSelectedCRS ) );
  }
  else
  {
    mSelectedCRS = "";
    mSelectedCRSLabel->setText( "" );
  }
  mChangeCRSButton->setEnabled( !mSelectedLayersCRSs.isEmpty() );
}

void QgsOWSSourceSelect::on_mTilesetsTableWidget_itemClicked( QTableWidgetItem *item )
{
  Q_UNUSED( item );

  QTableWidgetItem *rowItem = mTilesetsTableWidget->item( mTilesetsTableWidget->currentRow(), 0 );
  bool wasSelected = mCurrentTileset == rowItem;

  mTilesetsTableWidget->blockSignals( true );
  mTilesetsTableWidget->clearSelection();
  if ( !wasSelected )
  {
    QgsDebugMsg( QString( "selecting current row %1" ).arg( mTilesetsTableWidget->currentRow() ) );
    mTilesetsTableWidget->selectRow( mTilesetsTableWidget->currentRow() );
    mCurrentTileset = rowItem;
  }
  else
  {
    mCurrentTileset = 0;
  }
  mTilesetsTableWidget->blockSignals( false );

  updateButtons();
}



QString QgsOWSSourceSelect::connName()
{
  return mConnName;
}

QString QgsOWSSourceSelect::connectionInfo()
{
  return mConnectionInfo;
}

QString QgsOWSSourceSelect::selectedFormat()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.
  int id = mImageFormatGroup->checkedId();
  if ( id < 0 )
  {
    return "";
  }
  else
  {
    // TODO: do format in subclass (WMS)
    //return QUrl::toPercentEncoding( mProviderFormats[ id ].format );
    return selectedLayersFormats().value( id );
  }
}

QString QgsOWSSourceSelect::selectedCRS()
{
  return mSelectedCRS;
}

void QgsOWSSourceSelect::setConnectionListPosition()
{
  QString toSelect = QgsOWSConnection::selectedConnection( mService );

  mConnectionsComboBox->setCurrentIndex( mConnectionsComboBox->findText( toSelect ) );

  if ( mConnectionsComboBox->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      mConnectionsComboBox->setCurrentIndex( 0 );
    else
      mConnectionsComboBox->setCurrentIndex( mConnectionsComboBox->count() - 1 );
  }
  QgsOWSConnection::setSelectedConnection( mService, mConnectionsComboBox->currentText() );
}

void QgsOWSSourceSelect::showStatusMessage( QString const &theMessage )
{
  mStatusLabel->setText( theMessage );

  // update the display of this widget
  update();
}


void QgsOWSSourceSelect::showError( QString const &theTitle, QString const &theFormat, QString const &theError )
{
  QgsMessageViewer * mv = new QgsMessageViewer( this );
  mv->setWindowTitle( theTitle );

  if ( theFormat == "text/html" )
  {
    mv->setMessageAsHtml( theError );
  }
  else
  {
    mv->setMessageAsPlainText( tr( "Could not understand the response:\n%1" ).arg( theError ) );
  }
  mv->showMessage( true ); // Is deleted when closed
}

void QgsOWSSourceSelect::on_mConnectionsComboBox_activated( int )
{
  // Remember which server was selected.
  QgsOWSConnection::setSelectedConnection( mService, mConnectionsComboBox->currentText() );
}

void QgsOWSSourceSelect::on_mAddDefaultButton_clicked()
{
  addDefaultServers();
}

QString QgsOWSSourceSelect::descriptionForAuthId( QString authId )
{
  if ( mCrsNames.contains( authId ) )
    return mCrsNames[ authId ];

  QgsCoordinateReferenceSystem qgisSrs;
  qgisSrs.createFromOgcWmsCrs( authId );
  mCrsNames.insert( authId, qgisSrs.description() );
  return qgisSrs.description();
}

void QgsOWSSourceSelect::addDefaultServers()
{
  QMap<QString, QString> exampleServers;
  exampleServers["DM Solutions GMap"] = "http://www2.dmsolutions.ca/cgi-bin/mswms_gmap";
  exampleServers["Lizardtech server"] =  "http://wms.lizardtech.com/lizardtech/iserv/ows";
  // Nice to have the qgis users map, but I'm not sure of the URL at the moment.
  //  exampleServers["Qgis users map"] = "http://qgis.org/wms.cgi";

  QSettings settings;
  settings.beginGroup( "/Qgis/connections-" + mService.toLower() );
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

void QgsOWSSourceSelect::addWMSListRow( const QDomElement& item, int row )
{
  QDomElement title = item.firstChildElement( "title" );
  addWMSListItem( title, row, 0 );
  QDomElement description = item.firstChildElement( "description" );
  addWMSListItem( description, row, 1 );
  QDomElement link = item.firstChildElement( "link" );
  addWMSListItem( link, row, 2 );
}

void QgsOWSSourceSelect::addWMSListItem( const QDomElement& el, int row, int column )
{
  if ( !el.isNull() )
  {
    QTableWidgetItem* tableItem = new QTableWidgetItem( el.text() );
    // TODO: add linebreaks to long tooltips?
    tableItem->setToolTip( el.text() );
    mSearchTableWidget->setItem( row, column, tableItem );
  }
}

void QgsOWSSourceSelect::on_mSearchButton_clicked()
{
  // clear results
  mSearchTableWidget->clearContents();
  mSearchTableWidget->setRowCount( 0 );

  // disable Add WMS button
  mSearchAddButton->setEnabled( false );

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QSettings settings;
  // geopole.org (geopole.ch) 25.4.2012 : 503 Service Unavailable, archive: Recently added 20 Jul 2011
  QString mySearchUrl = settings.value( "/qgis/WMSSearchUrl", "http://geopole.org/wms/search?search=%1&type=rss" ).toString();
  QUrl url( mySearchUrl.arg( mSearchTermLineEdit->text() ) );
  QgsDebugMsg( url.toString() );

  QNetworkReply *r = QgsNetworkAccessManager::instance()->get( QNetworkRequest( url ) );
  connect( r, SIGNAL( finished() ), SLOT( searchFinished() ) );
}

void QgsOWSSourceSelect::searchFinished()
{
  QApplication::restoreOverrideCursor();

  QNetworkReply *r = qobject_cast<QNetworkReply *>( sender() );
  if ( !r )
    return;

  if ( r->error() == QNetworkReply::NoError )
  {
    // parse results
    QDomDocument doc( "RSS" );
    QByteArray res = r->readAll();
    QString error;
    int line, column;
    if ( doc.setContent( res, &error, &line, &column ) )
    {
      QDomNodeList list = doc.elementsByTagName( "item" );
      mSearchTableWidget->setRowCount( list.size() );
      for ( int i = 0; i < list.size(); i++ )
      {
        if ( list.item( i ).isElement() )
        {
          QDomElement item = list.item( i ).toElement();
          addWMSListRow( item, i );
        }
      }

      mSearchTableWidget->resizeColumnsToContents();
    }
    else
    {
      QgsDebugMsg( "setContent failed" );
      showStatusMessage( tr( "parse error at row %1, column %2: %3" ).arg( line ).arg( column ).arg( error ) );
    }
  }
  else
  {
    showStatusMessage( tr( "network error: %1" ).arg( r->error() ) );
  }

  r->deleteLater();
}

void QgsOWSSourceSelect::on_mAddWMSButton_clicked()
{
  // TODO: deactivate button if dialog is open?
  // TODO: remove from config on close?

  int selectedRow = mSearchTableWidget->currentRow();
  if ( selectedRow == -1 )
  {
    return;
  }

  QString wmsTitle = mSearchTableWidget->item( selectedRow, 0 )->text();
  QString wmsUrl = mSearchTableWidget->item( selectedRow, 2 )->text();

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
  QgsOWSConnection::setSelectedConnection( mService, wmsTitle );
  populateConnectionList();

  mTabWidget->setCurrentIndex( 0 );
}

void QgsOWSSourceSelect::on_mSearchTableWidget_itemSelectionChanged()
{
  mSearchAddButton->setEnabled( mSearchTableWidget->currentRow() != -1 );
}

void QgsOWSSourceSelect::on_mLayerUpButton_clicked()
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

void QgsOWSSourceSelect::on_mLayerDownButton_clicked()
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

QList<QgsOWSSupportedFormat> QgsOWSSourceSelect::providerFormats()
{
  return QList<QgsOWSSupportedFormat>();
}

QStringList QgsOWSSourceSelect::selectedLayersFormats()
{
  return QStringList();
}

QStringList QgsOWSSourceSelect::selectedLayersCRSs()
{
  return QStringList();
}

void QgsOWSSourceSelect::updateButtons()
{
}
