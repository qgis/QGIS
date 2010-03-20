/***************************************************************************
    qgswmssourceselect.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wms-c support        : (C) 2010 Jürgen E. Fischer < jef at norbit dot de >, norBIT GmbH

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
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
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

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

QgsWMSSourceSelect::QgsWMSSourceSelect( QWidget * parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  mAddButton = new QPushButton( tr( "&Add" ) );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) );

  QPushButton *pb = new QPushButton( tr( "&Save" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( saveClicked() ) );

  pb = new QPushButton( tr( "&Load" ) );
  buttonBox->addButton( pb, QDialogButtonBox::ActionRole );
  connect( pb, SIGNAL( clicked() ), this, SLOT( loadClicked() ) );

  mLayerUpButton->setIcon( QgisApp::getThemeIcon( "/mActionArrowUp.png" ) );
  mLayerDownButton->setIcon( QgisApp::getThemeIcon( "/mActionArrowDown.png" ) );

  mAddButton->setEnabled( false );
  populateConnectionList();

  QHBoxLayout *layout = new QHBoxLayout;
  mImageFormatGroup = new QButtonGroup;

  QList<QByteArray> supportedFormats = QImageReader::supportedImageFormats();

  if ( supportedFormats.contains( "png" ) )
  {
    mFormats << "image/png";
    mLabels << "PNG";

    mFormats << "image/png; mode=24bit"; // UMN mapserver
    mLabels << "PNG24";

    mFormats << "image/png8";    // used by geoserver
    mLabels << "PNG8";

    mFormats << "pngt";    // used by french IGN geoportail
    mLabels << "PNGT";
  }

  if ( supportedFormats.contains( "jpg" ) )
  {
    mFormats << "image/jpeg";
    mLabels << "JPEG";

    mFormats << "jpeg";    // used by french IGN geoportail
    mLabels << "JPEG";
  }

  if ( supportedFormats.contains( "gif" ) )
  {
    mFormats << "image/gif";
    mLabels << "GIF";
  }

  if ( supportedFormats.contains( "tiff" ) )
  {
    mFormats << "image/tiff";
    mLabels << "TIFF";
  }

  // add buttons for available encodings
  for ( int i = 0; i < mFormats.size(); i++ )
  {
    mMimeMap.insert( mFormats[i], i );

    QRadioButton *btn = new QRadioButton( mLabels[i] );
    btn->setToolTip( mFormats[i] );
    mImageFormatGroup->addButton( btn, i );
    layout->addWidget( btn );
  }

  // default to first encoding
  mImageFormatGroup->button( 0 )->setChecked( true );

  layout->addStretch();
  btnGrpImageEncoding->setLayout( layout );

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
      mCRS = currentRefSys.authid();
    }
  }

  // set up the default WMS Coordinate Reference System
  labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );

  // disable layer order and tilesets until we have some
  tabServers->setTabEnabled( tabServers->indexOf( tabLayerOrder ), false );
  tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), false );

  connect( tableWidgetWMSList, SIGNAL( itemSelectionChanged() ), this, SLOT( wmsSelectionChanged() ) );
  connect( lstTilesets, SIGNAL( itemSelectionChanged() ), this, SLOT( updateButtons() ) );
}

QgsWMSSourceSelect::~QgsWMSSourceSelect()
{

}
void QgsWMSSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/Qgis/connections-wms" );
  cmbConnections->clear();
  cmbConnections->addItems( settings.childGroups() );
  settings.endGroup();

  setConnectionListPosition();

  if ( cmbConnections->count() == 0 )
  {
    // No connections - disable various buttons
    btnConnect->setEnabled( false );
    btnEdit->setEnabled( false );
    btnDelete->setEnabled( false );
  }
  else
  {
    // Connections - enable various buttons
    btnConnect->setEnabled( true );
    btnEdit->setEnabled( true );
    btnDelete->setEnabled( true );
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
    settings.remove( "/Qgis/WMS/" + cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsWMSSourceSelect::saveClicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Save, QgsManageConnectionsDialog::WMS );
  dlg.exec();
}

void QgsWMSSourceSelect::loadClicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Load, QgsManageConnectionsDialog::WMS );
  dlg.exec();
  populateConnectionList();
}

QgsNumericSortTreeWidgetItem *QgsWMSSourceSelect::createItem(
  int id,
  const QStringList &names,
  QMap<int, QgsNumericSortTreeWidgetItem *> &items,
  int &layerAndStyleCount,
  const QMap<int, int> &layerParents,
  const QMap<int, QStringList> &layerParentNames )
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
  mCRSs.clear();

  QVector<QgsWmsLayerProperty> layers;
  if ( !wmsProvider->supportedLayers( layers ) )
    return false;

  foreach( QAbstractButton *b, mImageFormatGroup->buttons() )
  {
    b->setEnabled( false );
  }

  foreach( QString encoding, wmsProvider->supportedImageEncodings() )
  {
    int id = mMimeMap.value( encoding, -1 );
    if ( id < 0 )
    {
      QgsDebugMsg( QString( "encoding %1 not supported." ).arg( encoding ) );
      continue;
    }

    mImageFormatGroup->button( id )->setEnabled( true );
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
    lItem->setData( 0, Qt::UserRole + 2, layer->crs );

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

      lItem2->setData( 0, Qt::UserRole + 0, layer->name );
      lItem2->setData( 0, Qt::UserRole + 1, layer->style[j].name );
    }
  }

  lstLayers->sortByColumn( 0, Qt::AscendingOrder );

  QVector<QgsWmsTileSetProfile> tilesets;
  wmsProvider->supportedTileSets( tilesets );

  tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), tilesets.size() > 0 );

  if ( tilesets.size() > 0 )
  {
    lstTilesets->clearContents();
    lstTilesets->setRowCount( tilesets.size() );
    lstTilesets->setSortingEnabled( true );

    for ( int i = 0; i < tilesets.size(); i++ )
    {
      QTableWidgetItem *item = new QTableWidgetItem( tilesets[i].layers.join( ", " ) );

      item->setData( Qt::UserRole + 0, tilesets[i].layers.join( "," ) );
      item->setData( Qt::UserRole + 1, tilesets[i].styles.join( "," ) );
      item->setData( Qt::UserRole + 2, tilesets[i].format );
      item->setData( Qt::UserRole + 3, tilesets[i].crs );
      item->setData( Qt::UserRole + 4, tilesets[i].tileWidth );
      item->setData( Qt::UserRole + 5, tilesets[i].tileHeight );
      item->setData( Qt::UserRole + 6, tilesets[i].resolutions );

      lstTilesets->setItem( i, 0, item );
      lstTilesets->setItem( i, 1, new QTableWidgetItem( tilesets[i].styles.join( ", " ) ) );
      lstTilesets->setItem( i, 2, new QTableWidgetItem( QString( "%1x%2" ).arg( tilesets[i].tileWidth ).arg( tilesets[i].tileHeight ) ) );
      lstTilesets->setItem( i, 3, new QTableWidgetItem( tilesets[i].format ) );
      lstTilesets->setItem( i, 4, new QTableWidgetItem( tilesets[i].crs ) );

      if ( !mMimeMap.contains( tilesets[i].format ) )
      {
        for ( int j = 0; j < 5; j++ )
        {
          QTableWidgetItem *item = lstTilesets->item( i, j );
          item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
          item->setToolTip( tr( "encoding %1 not supported." ).arg( tilesets[i].format ) );
        }
      }

      QString crsName = descriptionForAuthId( tilesets[i].crs );
      if ( crsName.isEmpty() )
        crsName = tr( "CRS %1 not supported." ).arg( tilesets[i].crs );
      lstTilesets->item( i, 4 )->setToolTip( crsName );
    }

    lstTilesets->resizeColumnsToContents();
    lstTilesets->sortByColumn( 0, Qt::AscendingOrder );
  }

  // If we got some layers, let the user add them to the map
  if ( lstLayers->topLevelItemCount() == 1 )
  {
    lstLayers->expandItem( lstLayers->topLevelItem( 0 ) );
  }

  return true;
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

  mConnName = cmbConnections->currentText();
  mConnectionInfo = connStringParts.join( " " );

  // Check for credentials and prepend to the connection info
  QString username = settings.value( credentialsKey + "/username" ).toString();
  QString password = settings.value( credentialsKey + "/password" ).toString();
  if ( !username.isEmpty() )
  {
    // check for a password, if none prompt to get it
    if ( password.isEmpty() )
    {
      password = QInputDialog::getText( this, tr( "WMS Password for %1" ).arg( mConnName ), "Password", QLineEdit::Password );
    }
    mConnectionInfo = "username=" + username + ",password=" + password + ",url=" + mConnectionInfo;
  }


  QgsDebugMsg( QString( "Connection info: '%1'." ).arg( mConnectionInfo ) );


  // TODO: Create and bind to data provider

  // load the server data provider plugin
  QgsProviderRegistry * pReg = QgsProviderRegistry::instance();

  QgsWmsProvider *wmsProvider =
    ( QgsWmsProvider* ) pReg->getProvider( "wms", mConnectionInfo );

  if ( wmsProvider )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    connect( wmsProvider, SIGNAL( statusChanged( QString ) ), this, SLOT( showStatusMessage( QString ) ) );

    // WMS Provider all set up; let's get some layers

    if ( !populateLayerList( wmsProvider ) )
    {
      showError( wmsProvider );
    }

    delete wmsProvider;

    QApplication::restoreOverrideCursor();
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
  QStringList layers;
  QStringList styles;
  QString format;
  QString crs;
  QString connInfo = connectionInfo();

  if ( lstTilesets->selectedItems().isEmpty() )
  {
    collectSelectedLayers( layers, styles );
    crs = mCRS;
    format = mFormats[ mImageFormatGroup->checkedId()];
  }
  else
  {
    QTableWidgetItem *item = lstTilesets->selectedItems().first();
    layers = item->data( Qt::UserRole + 0 ).toStringList();
    styles = item->data( Qt::UserRole + 1 ).toStringList();
    format = item->data( Qt::UserRole + 2 ).toString();
    crs    = item->data( Qt::UserRole + 3 ).toString();

    QString connArgs = QString( "tiled=%1;%2;%3" )
                       .arg( item->data( Qt::UserRole + 4 ).toInt() )
                       .arg( item->data( Qt::UserRole + 5 ).toInt() )
                       .arg( item->data( Qt::UserRole + 6 ).toStringList().join( ";" ) );

    if ( connInfo.startsWith( "username=" ) )
    {
      connInfo.prepend( connArgs + "," );
    }
    else
    {
      connInfo.prepend( connArgs + ",url=" );
    }
  }

  QgisApp::instance()->addRasterLayer( connInfo,
                                       leLayerName->text().isEmpty() ? layers.join( "/" ) : leLayerName->text(),
                                       "wms", layers, styles, format, crs );
}

void QgsWMSSourceSelect::enableLayersForCrs( QTreeWidgetItem *item )
{
  QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
  QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();

  if ( !layerName.isEmpty() && styleName.isEmpty() )
  {
    // layer
    bool disable = !item->data( 0, Qt::UserRole + 1 ).toStringList().contains( mCRS );

    item->setDisabled( disable );

    // propagate to styles
    for ( int i = 0; i < item->childCount(); i++ )
    {
      item->child( i )->setDisabled( disable );
    }
  }
  else
  {
    // recurse to child layers
    for ( int i = 0; i < item->childCount(); i++ )
    {
      enableLayersForCrs( item->child( i ) );
    }
  }
}

void QgsWMSSourceSelect::on_btnChangeSpatialRefSys_clicked()
{
  QStringList layers;
  foreach( QTreeWidgetItem *item, lstLayers->selectedItems() )
  {
    QString layer = item->data( 0, Qt::UserRole + 0 ).toString();
    if ( !layer.isEmpty() )
      layers << layer;
  }

  QgsGenericProjectionSelector * mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setOgcWmsCrsFilter( mCRSs );

  QString myDefaultProjString = QgsProject::instance()->readEntry( "SpatialRefSys", "/ProjectCRSProj4String", GEOPROJ4 );
  QgsCoordinateReferenceSystem defaultCRS;
  if ( defaultCRS.createFromProj4( myDefaultProjString ) )
  {
    mySelector->setSelectedCrsId( defaultCRS.srsid() );
  }

  if ( !mySelector->exec() )
    return;

  mCRS = mySelector->selectedAuthId();
  delete mySelector;

  labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );

  for ( int i = 0; i < lstLayers->topLevelItemCount(); i++ )
  {
    enableLayersForCrs( lstLayers->topLevelItem( i ) );
  }

  // update the display of this widget
  update();
}

void QgsWMSSourceSelect::applySelectionConstraints( QTreeWidgetItem *item )
{
  QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
  QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();
  if ( layerName.isEmpty() )
  {
    // layer group =>
    //   process child layers and style selection first
    // then
    //   if all child layers are selected, deselect them and select the group instead
    //   if some child layers are selected, deselect the group
    //   if none child layers are selected, keep the selection state of the group
    for ( int i = 0; i < item->childCount(); i++ )
    {
      applySelectionConstraints( item->child( i ) );
    }

    int n = 0;
    for ( int i = 0; i < item->childCount(); i++ )
    {
      if ( item->child( i )->isSelected() )
        n++;
    }

    if ( n == item->childCount() )
    {
      for ( int i = 0; i < item->childCount(); i++ )
        item->child( i )->setSelected( false );
      item->setSelected( true );
      item->setExpanded( false );
    }
    else if ( n > 0 )
    {
      item->setSelected( false );
    }
  }
  else if ( styleName.isEmpty() )
  {
    // named layer =>
    //      if all styles are selected, deselect all and selected named layer
    // else if some styles are selected, deselect all, but the first and deselect the named layer
    // else if no style is selected, keep layer selection
    int n = 0;
    QTreeWidgetItem *style = 0;
    for ( int i = 0; i < item->childCount(); i++ )
    {
      if ( item->child( i )->isSelected() )
      {
        n++;
        if ( !style )
          style = item->child( i );
        else
          item->child( i )->setSelected( false );
      }
    }

    if ( n > 0 && n == item->childCount() )
    {
      // all styles were selected =>
      //   deselect all styles and select named layer
      if ( style )
        style->setSelected( false );
      item->setSelected( true );
      item->setExpanded( false );
    }
    else if ( style )
    {
      // leave first style selected
      //   and deselect named layer
      item->setSelected( false );
    }
  }
}

void QgsWMSSourceSelect::collectNamedLayers( QTreeWidgetItem *item, QStringList &layers, QStringList &styles )
{
  QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
  QString styleName = item->data( 1, Qt::UserRole + 0 ).toString();
  if ( layerName.isEmpty() )
  {
    // layer group
    for ( int i = 0; i < item->childCount(); i++ )
      collectNamedLayers( item->child( i ), layers, styles );
  }
  else if ( styleName.isEmpty() )
  {
    // named layers
    layers << layerName;
    styles << "";

    if ( mCRSs.isEmpty() )
      mCRSs = item->data( 0, Qt::UserRole + 2 ).toStringList().toSet();
    else
      mCRSs.intersect( item->data( 0, Qt::UserRole + 2 ).toStringList().toSet() );
  }
}

/**
 * retrieve selected layers
 */
void QgsWMSSourceSelect::on_lstLayers_itemSelectionChanged()
{
  lstLayers->blockSignals( true );
  for ( int i = 0; i < lstLayers->topLevelItemCount(); i++ )
  {
    applySelectionConstraints( lstLayers->topLevelItem( i ) );
  }
  lstLayers->blockSignals( false );

  // selected layers with styles
  QStringList layers;
  QStringList styles;

  mCRSs.clear();

  // determine selected layers and styles and set of crses that are available for all layers
  foreach( QTreeWidgetItem *item, lstLayers->selectedItems() )
  {
    QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
    QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();

    if ( layerName.isEmpty() )
    {
      // layers groups: collect named layers of group and add using the default style
      collectNamedLayers( item, layers, styles );
    }
    else if ( styleName.isEmpty() )
    {
      // named layer: add using default style
      layers << layerName;
      styles << "";
      if ( mCRSs.isEmpty() )
        mCRSs = item->data( 0, Qt::UserRole + 2 ).toStringList().toSet();
      else
        mCRSs.intersect( item->data( 0, Qt::UserRole + 2 ).toStringList().toSet() );
    }
    else
    {
      // style: add named layer with selected style
      layers << layerName;
      styles << styleName;
      if ( mCRSs.isEmpty() )
        mCRSs = item->parent()->data( 0, Qt::UserRole + 2 ).toStringList().toSet();
      else
        mCRSs.intersect( item->parent()->data( 0, Qt::UserRole + 2 ).toStringList().toSet() );
    }
  }

  gbCRS->setTitle( tr( "Coordinate Reference System (%n available)", "crs count", mCRSs.count() ) );
  btnChangeSpatialRefSys->setDisabled( mCRSs.isEmpty() );

  if ( !layers.isEmpty() && !mCRSs.isEmpty() )
  {

    // check whether current CRS is supported
    // if not, use one of the available CRS
    QString defaultCRS;
    QSet<QString>::const_iterator it = mCRSs.begin();
    for ( ; it != mCRSs.end(); it++ )
    {
      if ( it->compare( mCRS, Qt::CaseInsensitive ) == 0 )
        break;

      // save first CRS in case the current CRS is not available
      if ( it == mCRSs.begin() )
        defaultCRS = *it;

      // prefer value of DEFAULT_GEO_EPSG_CRS_ID if available
      if ( *it == GEO_EPSG_CRS_AUTHID )
        defaultCRS = *it;
    }

    if ( it == mCRSs.end() )
    {
      // not found
      mCRS = defaultCRS;
      labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );
    }

  }
  else if ( mCRSs.isEmpty() )
  {
    mCRS = "";
    labelCoordRefSys->setText( "" );
  }

  updateLayerOrderTab( layers, styles );
  updateButtons();
}

void QgsWMSSourceSelect::updateButtons()
{
  if ( lstTilesets->selectedItems().isEmpty() && mLayerOrderTreeWidget->topLevelItemCount() == 0 )
  {
    if ( lstTilesets->rowCount() == 0 )
      labelStatus->setText( tr( "Select layer(s)" ) );
    else
      labelStatus->setText( tr( "Select layer(s) or a tileset" ) );
    mAddButton->setEnabled( false );
  }
  else if ( !lstTilesets->selectedItems().isEmpty() && mLayerOrderTreeWidget->topLevelItemCount() > 0 )
  {
    labelStatus->setText( tr( "Select either layer(s) or a tileset" ) );
    mAddButton->setEnabled( false );
  }
  else
  {
    gbCRS->setTitle( tr( "Coordinate Reference System (%n available)", "crs count", mCRSs.count() ) );
    btnChangeSpatialRefSys->setEnabled( !mCRSs.isEmpty() );

    if ( lstTilesets->selectedItems().isEmpty() )
    {
      if ( mCRSs.isEmpty() )
      {
        labelStatus->setText( tr( "No common CRS for selected layers." ) );
        mAddButton->setEnabled( false );
      }
      else if ( mCRS.isEmpty() )
      {
        labelStatus->setText( tr( "No CRS selected" ) );
        mAddButton->setEnabled( false );
      }
      else if ( mImageFormatGroup->checkedId() == -1 )
      {
        labelStatus->setText( tr( "No image encoding selected" ) );
        mAddButton->setEnabled( false );
      }
      else
      {
        labelStatus->setText( tr( "%n Layer(s) selected", "selected layer count", mLayerOrderTreeWidget->topLevelItemCount() ) );
        mAddButton->setEnabled( true );
      }
    }
    else
    {
      labelStatus->setText( tr( "Tileset selected" ) );
      mAddButton->setEnabled( true );
    }
  }
}


QString QgsWMSSourceSelect::connName()
{
  return mConnName;
}

QString QgsWMSSourceSelect::connectionInfo()
{
  return mConnectionInfo;
}

void QgsWMSSourceSelect::collectSelectedLayers( QStringList &layers, QStringList &styles )
{
  //go through list in layer order tab
  QStringList selectedLayerList;
  for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
  {
    layers << mLayerOrderTreeWidget->topLevelItem( i )->text( 0 );
    styles << mLayerOrderTreeWidget->topLevelItem( i )->text( 1 );
  }
}

QString QgsWMSSourceSelect::selectedImageEncoding()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.
  int id = mImageFormatGroup->checkedId();
  if ( id < 0 )
  {
    return "";
  }
  else
  {
    return QUrl::toPercentEncoding( mFormats[ id ] );
  }
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
void QgsWMSSourceSelect::showStatusMessage( QString const &theMessage )
{
  labelStatus->setText( theMessage );

  // update the display of this widget
  update();
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
  // Remember which server was selected.
  QSettings settings;
  settings.setValue( "/Qgis/connections-wms/selected",
                     cmbConnections->currentText() );
}

void QgsWMSSourceSelect::on_btnAddDefault_clicked()
{
  addDefaultServers();
}

QString QgsWMSSourceSelect::descriptionForAuthId( QString authId )
{
  if ( mCrsNames.contains( authId ) )
    return mCrsNames[ authId ];

  QgsCoordinateReferenceSystem qgisSrs;
  qgisSrs.createFromOgcWmsCrs( authId );
  mCrsNames.insert( authId, qgisSrs.description() );
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

void QgsWMSSourceSelect::addWMSListRow( const QDomElement& item, int row )
{
  QDomElement title = item.firstChildElement( "title" );
  addWMSListItem( title, row, 0 );
  QDomElement description = item.firstChildElement( "description" );
  addWMSListItem( description, row, 1 );
  QDomElement link = item.firstChildElement( "link" );
  addWMSListItem( link, row, 2 );
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

  QSettings settings;
  QString mySearchUrl = settings.value( "/qgis/WMSSearchUrl", "http://geopole.org/wms/search?search=%1&type=rss" ).toString();
  QUrl url( mySearchUrl.arg( leSearchTerm->text() ) );
  QgsDebugMsg( url.toString() );

  QNetworkReply *r = QgisApp::instance()->nam()->get( QNetworkRequest( url ) );
  connect( r, SIGNAL( finished() ), SLOT( searchFinished() ) );
}

void QgsWMSSourceSelect::searchFinished()
{
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
      tableWidgetWMSList->setRowCount( list.size() );
      for ( int i = 0; i < list.size(); i++ )
      {
        if ( list.item( i ).isElement() )
        {
          QDomElement item = list.item( i ).toElement();
          addWMSListRow( item, i );
        }
      }

      tableWidgetWMSList->resizeColumnsToContents();
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
  QString wmsUrl = tableWidgetWMSList->item( selectedRow, 2 )->text();

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

  tabServers->setCurrentIndex( 0 );
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

  tabServers->setTabEnabled( tabServers->indexOf( tabLayerOrder ), mLayerOrderTreeWidget->topLevelItemCount() > 0 );
}
