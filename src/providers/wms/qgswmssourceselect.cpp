/***************************************************************************
    qgswmssourceselect.cpp  -  selector for WMS servers, etc.
                             -------------------
    begin                : 3 April 2005
    copyright            :
    original             : (C) 2005 by Brendan Morley email  : morb at ozemail dot com dot au
    wms search           : (C) 2009 Mathias Walker <mwa at sourcepole.ch>, Sourcepole AG
    wmts/wms-c support   : (C) 2010-2012 Juergen E. Fischer < jef at norbit dot de >, norBIT GmbH

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgswmsprovider.h"
#include "qgis.h" // GEO_EPSG_CRS_ID
#include "qgscoordinatereferencesystem.h"
#include "qgsdatasourceuri.h"
#include "qgsprojectionselectiondialog.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsmessageviewer.h"
#include "qgsnewhttpconnection.h"
#include "qgstreewidgetitem.h"
#include "qgsproject.h"
#include "qgsproviderregistry.h"
#include "qgswmsconnection.h"
#include "qgswmssourceselect.h"
#include "qgswmtsdimensions.h"
#include "qgsnetworkaccessmanager.h"
#include "qgswmscapabilities.h"
#include "qgsapplication.h"
#include "qgssettings.h"

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
#include <QUrl>
#include <QValidator>
#include <QNetworkRequest>
#include <QNetworkReply>

QgsWMSSourceSelect::QgsWMSSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
  , mDefaultCRS( GEO_EPSG_CRS_AUTHID )
  , mCurrentTileset( nullptr )
{
  setupUi( this );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsWMSSourceSelect::showHelp );

  // Creates and connects standard ok/apply buttons
  setupButtons( buttonBox );

  mTileWidth->setValidator( new QIntValidator( 0, 9999, this ) );
  mTileHeight->setValidator( new QIntValidator( 0, 9999, this ) );
  mStepWidth->setValidator( new QIntValidator( 0, 999999, this ) );
  mStepHeight->setValidator( new QIntValidator( 0, 999999, this ) );
  mFeatureCount->setValidator( new QIntValidator( 0, 9999, this ) );

  mImageFormatGroup = new QButtonGroup;

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::Manager )
  {

    QHBoxLayout *layout = new QHBoxLayout;

    mFormats = QgsWmsProvider::supportedFormats();

    // add buttons for available encodings
    for ( int i = 0; i < mFormats.size(); i++ )
    {
      mMimeMap.insert( mFormats[i].format, i );

      QRadioButton *btn = new QRadioButton( mFormats.at( i ).label );
      btn->setToolTip( mFormats[i].format );
      btn->setHidden( true );
      mImageFormatGroup->addButton( btn, i );
      layout->addWidget( btn );
    }

    // default to first encoding
    mImageFormatGroup->button( 0 )->setChecked( true );
    btnGrpImageEncoding->setDisabled( true );

    layout->addStretch();
    btnGrpImageEncoding->setLayout( layout );
    setTabOrder( lstLayers, mImageFormatGroup->button( 0 ) );

    //set the current project CRS if available
    QgsCoordinateReferenceSystem currentRefSys = QgsProject::instance()->crs();
    //convert CRS id to epsg
    if ( currentRefSys.isValid() )
    {
      mDefaultCRS = mCRS = currentRefSys.authid();
    }

    // set up the default WMS Coordinate Reference System
    labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );

    // disable layer order and tilesets until we have some
    tabServers->setTabEnabled( tabServers->indexOf( tabLayerOrder ), false );
    tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), false );
  }
  else
  {
    tabServers->removeTab( tabServers->indexOf( tabLayerOrder ) );
    tabServers->removeTab( tabServers->indexOf( tabTilesets ) );
    btnGrpImageEncoding->hide();
    tabLayers->layout()->removeWidget( btnGrpImageEncoding );
    gbCRS->hide();
    tabLayers->layout()->removeWidget( gbCRS );
  }

  clear();

  // set up the WMS connections we already know about
  populateConnectionList();

  QgsSettings settings;
  QgsDebugMsg( "restoring geometry" );
  restoreGeometry( settings.value( QStringLiteral( "Windows/WMSSourceSelect/geometry" ) ).toByteArray() );
}

QgsWMSSourceSelect::~QgsWMSSourceSelect()
{
  QgsSettings settings;
  QgsDebugMsg( "saving geometry" );
  settings.setValue( QStringLiteral( "Windows/WMSSourceSelect/geometry" ), saveGeometry() );
}

void QgsWMSSourceSelect::refresh()
{
  // Reload WMS connections and update the GUI
  QgsDebugMsg( "Refreshing WMS connections ..." );
  populateConnectionList();
}


void QgsWMSSourceSelect::populateConnectionList()
{
  cmbConnections->clear();
  cmbConnections->addItems( QgsWMSConnection::connectionList() );

  setConnectionListPosition();
}

void QgsWMSSourceSelect::on_btnNew_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsWMSSourceSelect::on_btnEdit_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "qgis/connections-wms/" ), cmbConnections->currentText() );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsWMSSourceSelect::on_btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    QgsWMSConnection::deleteConnection( cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    setConnectionListPosition();
    emit connectionsChanged();
  }
}

void QgsWMSSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WMS );
  dlg.exec();
}

void QgsWMSSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WMS, fileName );
  dlg.exec();
  populateConnectionList();
  emit connectionsChanged();
}

QgsTreeWidgetItem *QgsWMSSourceSelect::createItem(
  int id,
  const QStringList &names,
  QMap<int, QgsTreeWidgetItem *> &items,
  int &layerAndStyleCount,
  const QMap<int, int> &layerParents,
  const QMap<int, QStringList> &layerParentNames )
{
  if ( items.contains( id ) )
    return items[id];

  QgsTreeWidgetItem *item = nullptr;
  if ( layerParents.contains( id ) )
  {
    int parent = layerParents[ id ];
    item = new QgsTreeWidgetItem( createItem( parent, layerParentNames[ parent ], items, layerAndStyleCount, layerParents, layerParentNames ) );
  }
  else
    item = new QgsTreeWidgetItem( lstLayers );

  item->setText( 0, QString::number( ++layerAndStyleCount ) );
  item->setText( 1, names[0].simplified() );
  item->setText( 2, names[1].simplified() );
  item->setText( 3, names[2].simplified() );
  item->setToolTip( 3, "<font color=black>" + names[2].simplified()  + "</font>" );

  items[ id ] = item;

  return item;
}

void QgsWMSSourceSelect::clear()
{
  lstLayers->clear();
  lstTilesets->clearContents();

  mCRSs.clear();

  Q_FOREACH ( QAbstractButton *b, mImageFormatGroup->buttons() )
  {
    b->setHidden( true );
  }

  mFeatureCount->setEnabled( false );
}

bool QgsWMSSourceSelect::populateLayerList( const QgsWmsCapabilities &capabilities )
{
  QVector<QgsWmsLayerProperty> layers = capabilities.supportedLayers();

  bool first = true;
  Q_FOREACH ( const QString &encoding, capabilities.supportedImageEncodings() )
  {
    int id = mMimeMap.value( encoding, -1 );
    if ( id < 0 )
    {
      QgsDebugMsg( QString( "encoding %1 not supported." ).arg( encoding ) );
      continue;
    }

    mImageFormatGroup->button( id )->setVisible( true );
    if ( first )
    {
      mImageFormatGroup->button( id )->setChecked( true );
      first = false;
    }
  }

  btnGrpImageEncoding->setEnabled( true );

  QMap<int, QgsTreeWidgetItem *> items;
  QMap<int, int> layerParents;
  QMap<int, QStringList> layerParentNames;
  capabilities.layerParents( layerParents, layerParentNames );

  lstLayers->setSortingEnabled( true );

  int layerAndStyleCount = -1;

  for ( QVector<QgsWmsLayerProperty>::iterator layer = layers.begin();
        layer != layers.end();
        ++layer )
  {
    QgsTreeWidgetItem *lItem = createItem( layer->orderId, QStringList() << layer->name << layer->title << layer->abstract, items, layerAndStyleCount, layerParents, layerParentNames );

    lItem->setData( 0, Qt::UserRole + 0, layer->name );
    lItem->setData( 0, Qt::UserRole + 1, "" );
    lItem->setData( 0, Qt::UserRole + 2, layer->crs );
    lItem->setData( 0, Qt::UserRole + 3, layer->title.isEmpty() ? layer->name : layer->title );

    // Also insert the styles
    // Layer Styles
    for ( int j = 0; j < layer->style.size(); j++ )
    {
      QgsDebugMsg( QString( "got style name %1 and title '%2'." ).arg( layer->style.at( j ).name, layer->style.at( j ).title ) );

      QgsTreeWidgetItem *lItem2 = new QgsTreeWidgetItem( lItem );
      lItem2->setText( 0, QString::number( ++layerAndStyleCount ) );
      lItem2->setText( 1, layer->style.at( j ).name.simplified() );
      lItem2->setText( 2, layer->style.at( j ).title.simplified() );
      lItem2->setText( 3, layer->style.at( j ).abstract.simplified() );

      lItem2->setData( 0, Qt::UserRole + 0, layer->name );
      lItem2->setData( 0, Qt::UserRole + 1, layer->style.at( j ).name );
      lItem2->setData( 0, Qt::UserRole + 3, layer->style.at( j ).title.isEmpty() ? layer->style.at( j ).name : layer->style.at( j ).title );
    }
  }

  lstLayers->sortByColumn( 0, Qt::AscendingOrder );

  mTileLayers = capabilities.supportedTileLayers();

  tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), !mTileLayers.isEmpty() );
  if ( tabServers->isTabEnabled( tabServers->indexOf( tabTilesets ) ) )
    tabServers->setCurrentWidget( tabTilesets );

  if ( !mTileLayers.isEmpty() )
  {
    QHash<QString, QgsWmtsTileMatrixSet> tileMatrixSets = capabilities.supportedTileMatrixSets();

    int rows = 0;
    Q_FOREACH ( const QgsWmtsTileLayer &l, mTileLayers )
    {
      rows += l.styles.size() * l.setLinks.size() * l.formats.size();
    }

    lstTilesets->clearContents();
    lstTilesets->setRowCount( rows );
    lstTilesets->setSortingEnabled( false );

    int row = 0;
    Q_FOREACH ( const QgsWmtsTileLayer &l, mTileLayers )
    {
      Q_FOREACH ( const QgsWmtsStyle &style, l.styles )
      {
        Q_FOREACH ( const QgsWmtsTileMatrixSetLink &setLink, l.setLinks )
        {
          Q_FOREACH ( const QString &format, l.formats )
          {
            QTableWidgetItem *item = new QTableWidgetItem( l.identifier );
            item->setData( Qt::UserRole + 0, l.identifier );
            item->setData( Qt::UserRole + 1, format );
            item->setData( Qt::UserRole + 2, style.identifier );
            item->setData( Qt::UserRole + 3, setLink.tileMatrixSet );
            item->setData( Qt::UserRole + 4, tileMatrixSets[ setLink.tileMatrixSet ].crs );
            item->setData( Qt::UserRole + 5, l.title );

            lstTilesets->setItem( row, 0, item );
            lstTilesets->setItem( row, 1, new QTableWidgetItem( format ) );

            QTableWidgetItem *titleItem = new QTableWidgetItem( l.title.isEmpty() ? l.identifier : l.title );
            if ( !l.abstract.isEmpty() )
              titleItem->setToolTip( "<p>" + l.abstract + "</p>" );
            lstTilesets->setItem( row, 2, titleItem );

            QTableWidgetItem *styleItem = new QTableWidgetItem( style.title.isEmpty() ? style.identifier : style.title );
            if ( !style.abstract.isEmpty() )
              titleItem->setToolTip( "<p>" + style.abstract + "</p>" );
            lstTilesets->setItem( row, 3, styleItem );

            lstTilesets->setItem( row, 4, new QTableWidgetItem( setLink.tileMatrixSet ) );
            lstTilesets->setItem( row, 5, new QTableWidgetItem( tileMatrixSets[ setLink.tileMatrixSet ].crs ) );

            if ( !mMimeMap.contains( format ) )
            {
              for ( int i = 0; i < lstTilesets->columnCount(); i++ )
              {
                QTableWidgetItem *item = lstTilesets->item( row, i );
                item->setFlags( item->flags() & ~Qt::ItemIsEnabled );
                item->setToolTip( tr( "encoding %1 not supported." ).arg( format ) );
              }
            }

            row++;
          }
        }
      }
    }

    lstTilesets->resizeColumnsToContents();
    lstTilesets->setSortingEnabled( true );
    lstTilesets->sortByColumn( 0, Qt::AscendingOrder );
  }
  else
  {
    lstTilesets->setRowCount( 0 );
  }

  // If we got some layers, let the user add them to the map
  if ( lstLayers->topLevelItemCount() == 1 )
  {
    lstLayers->expandItem( lstLayers->topLevelItem( 0 ) );
  }

  on_lstLayers_itemSelectionChanged();

  return true;
}


void QgsWMSSourceSelect::on_btnConnect_clicked()
{
  clear();

  mConnName = cmbConnections->currentText();

  QgsWMSConnection connection( cmbConnections->currentText() );
  mUri = connection.uri();

  QgsWmsSettings wmsSettings;
  if ( !wmsSettings.parseUri( mUri.encodedUri() ) )
  {
    QMessageBox::warning(
      this,
      tr( "WMS Provider" ),
      tr( "Failed to parse WMS URI" )
    );
    return;
  }

  QgsWmsCapabilitiesDownload capDownload( wmsSettings.baseUrl(), wmsSettings.authorization(), true );
  connect( &capDownload, &QgsWmsCapabilitiesDownload::statusChanged, this, &QgsWMSSourceSelect::showStatusMessage );

  QApplication::setOverrideCursor( Qt::WaitCursor );
  bool res = capDownload.downloadCapabilities();
  QApplication::restoreOverrideCursor();

  if ( !res )
  {
    QMessageBox::warning(
      this,
      tr( "WMS Provider" ),
      tr( "Failed to download capabilities:\n" ) + capDownload.lastError()
    );
    return;
  }

  QgsWmsCapabilities caps;
  if ( !caps.parseResponse( capDownload.response(), wmsSettings.parserSettings() ) )
  {
    QMessageBox msgBox( QMessageBox::Warning, tr( "WMS Provider" ),
                        tr( "The server you are trying to connect to does not seem to be a WMS server. Please check the URL." ),
                        QMessageBox::Ok, this );
    msgBox.setDetailedText( tr( "Instead of the capabilities string that was expected, the following response has been received:\n\n%1" ).arg( caps.lastError() ) );
    msgBox.exec();
    return;
  }

  mFeatureCount->setEnabled( caps.identifyCapabilities() != QgsRasterInterface::NoCapabilities );

  populateLayerList( caps );
}

void QgsWMSSourceSelect::addButtonClicked()
{
  QStringList layers;
  QStringList styles;
  QStringList titles;
  QString format;
  QString crs;

  QgsDataSourceUri uri = mUri;

  if ( mTileWidth->text().toInt() > 0 && mTileHeight->text().toInt() > 0 )
  {
    uri.setParam( QStringLiteral( "maxWidth" ), mTileWidth->text() );
    uri.setParam( QStringLiteral( "maxHeight" ), mTileHeight->text() );
  }

  if ( mStepWidth->text().toInt() > 0 && mStepHeight->text().toInt() > 0 )
  {
    uri.setParam( QStringLiteral( "stepWidth" ), mStepWidth->text() );
    uri.setParam( QStringLiteral( "stepHeight" ), mStepHeight->text() );
  }

  if ( lstTilesets->selectedItems().isEmpty() )
  {
    collectSelectedLayers( layers, styles, titles );
    crs = mCRS;
    format = mFormats[ mImageFormatGroup->checkedId()].format;
  }
  else
  {
    QTableWidgetItem *item = lstTilesets->selectedItems().first();

    layers = QStringList( item->data( Qt::UserRole + 0 ).toString() );
    format = item->data( Qt::UserRole + 1 ).toString();
    styles = QStringList( item->data( Qt::UserRole + 2 ).toString() );
    crs    = item->data( Qt::UserRole + 4 ).toString();
    titles = QStringList( item->data( Qt::UserRole + 5 ).toString() );

    uri.setParam( QStringLiteral( "tileMatrixSet" ), item->data( Qt::UserRole + 3 ).toStringList() );

    const QgsWmtsTileLayer *layer = nullptr;

    Q_FOREACH ( const QgsWmtsTileLayer &l, mTileLayers )
    {
      if ( l.identifier == layers.join( QStringLiteral( "," ) ) )
      {
        layer = &l;
        break;
      }
    }

    if ( !layer )
      return;

    if ( !layer->dimensions.isEmpty() )
    {
      QgsWmtsDimensions *dlg = new QgsWmtsDimensions( *layer, this );
      if ( dlg->exec() != QDialog::Accepted )
      {
        delete dlg;
        return;
      }

      QHash<QString, QString> dims;
      dlg->selectedDimensions( dims );

      QString dimString;
      QString delim;

      for ( QHash<QString, QString>::const_iterator it = dims.constBegin();
            it != dims.constEnd();
            ++it )
      {
        dimString += delim + it.key() + '=' + it.value();
        delim = ';';
      }

      delete dlg;

      uri.setParam( QStringLiteral( "tileDimensions" ), dimString );
    }
  }

  uri.setParam( QStringLiteral( "layers" ), layers );
  uri.setParam( QStringLiteral( "styles" ), styles );
  uri.setParam( QStringLiteral( "format" ), format );
  uri.setParam( QStringLiteral( "crs" ), crs );
  QgsDebugMsg( QString( "crs=%2 " ).arg( crs ) );

  if ( mFeatureCount->text().toInt() > 0 )
  {
    uri.setParam( QStringLiteral( "featureCount" ), mFeatureCount->text() );
  }

  uri.setParam( QStringLiteral( "contextualWMSLegend" ), mContextualLegendCheckbox->isChecked() ? "1" : "0" );

  emit addRasterLayer( uri.encodedUri(),
                       leLayerName->text().isEmpty() ? titles.join( QStringLiteral( "/" ) ) : leLayerName->text(),
                       QStringLiteral( "wms" ) );
}

void QgsWMSSourceSelect::enableLayersForCrs( QTreeWidgetItem *item )
{
  QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
  QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();

  if ( !layerName.isEmpty() && styleName.isEmpty() )
  {
    // layer
    bool disable = !item->data( 0, Qt::UserRole + 2 ).toStringList().contains( mCRS, Qt::CaseInsensitive );

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
  Q_FOREACH ( QTreeWidgetItem *item, lstLayers->selectedItems() )
  {
    QString layer = item->data( 0, Qt::UserRole + 0 ).toString();
    if ( !layer.isEmpty() )
      layers << layer;
  }

  QgsProjectionSelectionDialog *mySelector = new QgsProjectionSelectionDialog( this );
  mySelector->setMessage( QString() );
  mySelector->setOgcWmsCrsFilter( mCRSs );

  QgsCoordinateReferenceSystem defaultCRS = QgsProject::instance()->crs();
  if ( defaultCRS.isValid() )
  {
    mySelector->setCrs( defaultCRS );
  }

  if ( !mySelector->exec() )
    return;

  mCRS = mySelector->crs().authid();
  delete mySelector;

  labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );

  for ( int i = 0; i < lstLayers->topLevelItemCount(); i++ )
  {
    enableLayersForCrs( lstLayers->topLevelItem( i ) );
  }

  updateButtons();

  // update the display of this widget
  update();
}

void QgsWMSSourceSelect::applySelectionConstraints( QTreeWidgetItem *item )
{
  if ( item->childCount() == 0 )
  {
    return;
  }

  int styles = 0;
  for ( int i = 0; i < item->childCount(); i++ )
  {
    QTreeWidgetItem *child = item->child( i );
    QString style = child->data( 0, Qt::UserRole + 1 ).toString();
    if ( !style.isEmpty() )
      styles++;
  }

  if ( styles > 0 )
  {
    if ( styles < item->childCount() )
    {
      return;
    }

    QTreeWidgetItem *style = nullptr;
    QTreeWidgetItem *firstNewStyle = nullptr;
    for ( int i = 0; i < item->childCount(); i++ )
    {
      QTreeWidgetItem *child = item->child( i );
      if ( child->isSelected() )
      {
        if ( !firstNewStyle && !mCurrentSelection.contains( child ) )
          firstNewStyle = child;

        if ( !style )
          style = child;

        child->setSelected( false );
      }
    }

    if ( firstNewStyle || style )
    {
      // individual style selected => deselect layer and all parent groups
      QTreeWidgetItem *parent = item;
      while ( parent )
      {
        parent->setSelected( false );
        parent = parent->parent();
      }

      if ( firstNewStyle )
        firstNewStyle->setSelected( true );
      else if ( style )
        style->setSelected( true );
    }
  }
  else
  {
    // no styles => layer or layer group =>
    //   process child layers and style selection first
    // then
    //   if some child layers are selected, deselect the group and all parents
    //   otherwise keep the selection state of the group
    int n = 0;
    for ( int i = 0; i < item->childCount(); i++ )
    {
      QTreeWidgetItem *child = item->child( i );
      applySelectionConstraints( child );
      if ( child->isSelected() )
        n++;
    }

    if ( n > 0 )
    {
      if ( item->isSelected() )
      {
        for ( int i = 0; i < n; i++ )
        {
          QTreeWidgetItem *child = item->child( i );
          child->setSelected( false );
        }
        item->setExpanded( false );
      }
      else
      {
        for ( QTreeWidgetItem *parent = item->parent(); parent;  parent = parent->parent() )
        {
          parent->setSelected( false );
        }
      }
    }
  }
}

void QgsWMSSourceSelect::collectNamedLayers( QTreeWidgetItem *item, QStringList &layers, QStringList &styles, QStringList &titles )
{
  QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
  QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();
  QString titleName = item->data( 0, Qt::UserRole + 3 ).toString();
  if ( layerName.isEmpty() )
  {
    // layer group
    for ( int i = 0; i < item->childCount(); i++ )
      collectNamedLayers( item->child( i ), layers, styles, titles );
  }
  else if ( styleName.isEmpty() )
  {
    // named layers
    layers << layerName;
    styles << QLatin1String( "" );
    titles << titleName;

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
  mCurrentSelection = lstLayers->selectedItems();
  lstLayers->blockSignals( false );

  // selected layers with styles
  QStringList layers;
  QStringList styles;
  QStringList titles;

  mCRSs.clear();

  // determine selected layers and styles and set of crses that are available for all layers
  Q_FOREACH ( QTreeWidgetItem *item, lstLayers->selectedItems() )
  {
    QString layerName = item->data( 0, Qt::UserRole + 0 ).toString();
    QString styleName = item->data( 0, Qt::UserRole + 1 ).toString();
    QString titleName = item->data( 0, Qt::UserRole + 3 ).toString();

    if ( layerName.isEmpty() )
    {
      // layers groups: collect named layers of group and add using the default style
      collectNamedLayers( item, layers, styles, titles );
    }
    else if ( styleName.isEmpty() )
    {
      // named layer: add using default style
      layers << layerName;
      styles << QLatin1String( "" );
      titles << titleName;
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
      titles << titleName;
      if ( mCRSs.isEmpty() )
        mCRSs = item->parent()->data( 0, Qt::UserRole + 2 ).toStringList().toSet();
      else
        mCRSs.intersect( item->parent()->data( 0, Qt::UserRole + 2 ).toStringList().toSet() );
    }
  }

  gbCRS->setTitle( tr( "Options (%n coordinate reference systems available)", "crs count", mCRSs.count() ) );
  btnChangeSpatialRefSys->setDisabled( mCRSs.isEmpty() );

  if ( !layers.isEmpty() && !mCRSs.isEmpty() )
  {

    // check whether current CRS is supported
    // if not, use one of the available CRS
    QString defaultCRS;
    QSet<QString>::const_iterator it = mCRSs.constBegin();
    for ( ; it != mCRSs.constEnd(); ++it )
    {
      if ( it->compare( mCRS, Qt::CaseInsensitive ) == 0 )
        break;

      // save first CRS in case the current CRS is not available
      if ( it == mCRSs.constBegin() )
        defaultCRS = *it;

      // prefer value of DEFAULT_GEO_EPSG_CRS_ID if available
      if ( *it == mDefaultCRS )
        defaultCRS = *it;
    }

    if ( it == mCRSs.constEnd() )
    {
      // not found
      mCRS = defaultCRS;
      labelCoordRefSys->setText( descriptionForAuthId( mCRS ) );
    }

  }
  else if ( layers.isEmpty() || mCRSs.isEmpty() )
  {
    mCRS = QLatin1String( "" );
    labelCoordRefSys->setText( QLatin1String( "" ) );
  }

  updateLayerOrderTab( layers, styles, titles );
  updateButtons();
}

void QgsWMSSourceSelect::on_lstTilesets_itemClicked( QTableWidgetItem *item )
{
  Q_UNUSED( item );

  QTableWidgetItem *rowItem = lstTilesets->item( lstTilesets->currentRow(), 0 );
  bool wasSelected = mCurrentTileset == rowItem;

  lstTilesets->blockSignals( true );
  lstTilesets->clearSelection();
  if ( !wasSelected )
  {
    QgsDebugMsg( QString( "selecting current row %1" ).arg( lstTilesets->currentRow() ) );
    lstTilesets->selectRow( lstTilesets->currentRow() );
    mCurrentTileset = rowItem;
  }
  else
  {
    mCurrentTileset = nullptr;
  }
  lstTilesets->blockSignals( false );

  updateButtons();
}

void QgsWMSSourceSelect::updateButtons()
{
  if ( !lstTilesets->selectedItems().isEmpty() )
  {
    // tileset selected => disable layer selection and layer order
    lstLayers->setEnabled( false );
    tabServers->setTabEnabled( tabServers->indexOf( tabLayerOrder ), false );
    tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), lstTilesets->rowCount() > 0 );
    btnGrpImageEncoding->setEnabled( false );
  }
  else
  {
    // no tileset selected =>
    //   disable layerorder, when no layers selected
    //   disable tilesets, when layer are selected or no tilesets available
    lstLayers->setEnabled( true );
    tabServers->setTabEnabled( tabServers->indexOf( tabLayerOrder ), mLayerOrderTreeWidget->topLevelItemCount() > 0 );
    tabServers->setTabEnabled( tabServers->indexOf( tabTilesets ), mLayerOrderTreeWidget->topLevelItemCount() == 0  && lstTilesets->rowCount() > 0 );
    btnGrpImageEncoding->setEnabled( true );
  }

  if ( lstTilesets->selectedItems().isEmpty() && mLayerOrderTreeWidget->topLevelItemCount() == 0 )
  {
    if ( lstTilesets->rowCount() == 0 )
      labelStatus->setText( tr( "Select layer(s)" ) );
    else
      labelStatus->setText( tr( "Select layer(s) or a tileset" ) );
    emit enableButtons( false );
  }
  else if ( !lstTilesets->selectedItems().isEmpty() && mLayerOrderTreeWidget->topLevelItemCount() > 0 )
  {
    labelStatus->setText( tr( "Select either layer(s) or a tileset" ) );
    emit enableButtons( false );
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
        emit enableButtons( false );
      }
      else if ( mCRS.isEmpty() )
      {
        labelStatus->setText( tr( "No CRS selected" ) );
        emit enableButtons( false );
      }
      else if ( mImageFormatGroup->checkedId() == -1 )
      {
        labelStatus->setText( tr( "No image encoding selected" ) );
        emit enableButtons( false );
      }
      else
      {
        labelStatus->setText( tr( "%n Layer(s) selected", "selected layer count", mLayerOrderTreeWidget->topLevelItemCount() ) );
        emit enableButtons( true );
      }
    }
    else
    {
      labelStatus->setText( tr( "Tileset selected" ) );
      emit enableButtons( true );
    }
  }

  if ( leLayerName->text().isEmpty() || leLayerName->text() == mLastLayerName )
  {
    if ( addButton()->isEnabled() )
    {
      if ( !lstTilesets->selectedItems().isEmpty() )
      {
        QTableWidgetItem *item = lstTilesets->selectedItems().first();
        mLastLayerName = item->data( Qt::UserRole + 5 ).toString();
        if ( mLastLayerName.isEmpty() )
          mLastLayerName = item->data( Qt::UserRole + 0 ).toString();
        leLayerName->setText( mLastLayerName );
      }
      else
      {
        QStringList layers, styles, titles;
        collectSelectedLayers( layers, styles, titles );
        mLastLayerName = titles.join( QStringLiteral( "/" ) );
        leLayerName->setText( mLastLayerName );
      }
    }
    else
    {
      mLastLayerName = QLatin1String( "" );
      leLayerName->setText( mLastLayerName );
    }
  }
}


QString QgsWMSSourceSelect::connName()
{
  return mConnName;
}

void QgsWMSSourceSelect::collectSelectedLayers( QStringList &layers, QStringList &styles, QStringList &titles )
{
  //go through list in layer order tab
  QStringList selectedLayerList;
  for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
  {
    layers << mLayerOrderTreeWidget->topLevelItem( i )->text( 0 );
    styles << mLayerOrderTreeWidget->topLevelItem( i )->text( 1 );
    titles << mLayerOrderTreeWidget->topLevelItem( i )->text( 2 );
  }
}

QString QgsWMSSourceSelect::selectedImageEncoding()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.
  int id = mImageFormatGroup->checkedId();
  if ( id < 0 )
  {
    return QLatin1String( "" );
  }
  else
  {
    return QUrl::toPercentEncoding( mFormats.at( id ).format );
  }
}


void QgsWMSSourceSelect::setConnectionListPosition()
{
  QString toSelect = QgsWMSConnection::selectedConnection();

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

void QgsWMSSourceSelect::showStatusMessage( QString const &message )
{
  labelStatus->setText( message );

  // update the display of this widget
  update();
}


void QgsWMSSourceSelect::showError( QgsWmsProvider *wms )
{
  QgsMessageViewer *mv = new QgsMessageViewer( this );
  mv->setWindowTitle( wms->lastErrorTitle() );

  if ( wms->lastErrorFormat() == QLatin1String( "text/html" ) )
  {
    mv->setMessageAsHtml( wms->lastError() );
  }
  else
  {
    mv->setMessageAsPlainText( tr( "Could not understand the response. The %1 provider said:\n%2" ).arg( wms->name(), wms->lastError() ) );
  }
  mv->showMessage( true ); // Is deleted when closed
}

void QgsWMSSourceSelect::on_cmbConnections_activated( int )
{
  // Remember which server was selected.
  QgsWMSConnection::setSelectedConnection( cmbConnections->currentText() );
}

void QgsWMSSourceSelect::on_btnAddDefault_clicked()
{
  addDefaultServers();
}

QString QgsWMSSourceSelect::descriptionForAuthId( const QString &authId )
{
  if ( mCrsNames.contains( authId ) )
    return mCrsNames[ authId ];

  QgsCoordinateReferenceSystem qgisSrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( authId );
  mCrsNames.insert( authId, qgisSrs.description() );
  return qgisSrs.description();
}

void QgsWMSSourceSelect::addDefaultServers()
{
  QMap<QString, QString> exampleServers;
  exampleServers[QStringLiteral( "QGIS Server Demo - Alaska" )] = QStringLiteral( "http://demo.qgis.org/cgi-bin/qgis_mapserv.fcgi?map=/web/demos/alaska/alaska_map.qgs" );
  exampleServers[QStringLiteral( "GeoServer Demo - World" )] = QStringLiteral( "http://tiles.boundlessgeo.com/" );

  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "qgis/connections-wms" ) );
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

void QgsWMSSourceSelect::addWMSListRow( const QDomElement &item, int row )
{
  QDomElement title = item.firstChildElement( QStringLiteral( "title" ) );
  addWMSListItem( title, row, 0 );
  QDomElement description = item.firstChildElement( QStringLiteral( "description" ) );
  addWMSListItem( description, row, 1 );
  QDomElement link = item.firstChildElement( QStringLiteral( "link" ) );
  addWMSListItem( link, row, 2 );
}

void QgsWMSSourceSelect::addWMSListItem( const QDomElement &el, int row, int column )
{
  if ( !el.isNull() )
  {
    QTableWidgetItem *tableItem = new QTableWidgetItem( el.text() );
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

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsSettings settings;
  QString mySearchUrl = settings.value( QStringLiteral( "qgis/WMSSearchUrl" ), "http://geopole.org/wms/search?search=%1&type=rss" ).toString();
  QUrl url( mySearchUrl.arg( leSearchTerm->text() ) );
  QgsDebugMsg( url.toString() );

  QNetworkReply *r = QgsNetworkAccessManager::instance()->get( QNetworkRequest( url ) );
  connect( r, &QNetworkReply::finished, this, &QgsWMSSourceSelect::searchFinished );
}

void QgsWMSSourceSelect::searchFinished()
{
  QApplication::restoreOverrideCursor();

  QNetworkReply *r = qobject_cast<QNetworkReply *>( sender() );
  if ( !r )
    return;

  if ( r->error() == QNetworkReply::NoError )
  {
    // parse results
    QDomDocument doc( QStringLiteral( "RSS" ) );
    QByteArray res = r->readAll();
    QString error;
    int line, column;
    if ( doc.setContent( res, &error, &line, &column ) )
    {
      QDomNodeList list = doc.elementsByTagName( QStringLiteral( "item" ) );
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

  QgsSettings settings;
  if ( settings.contains( QStringLiteral( "qgis/connections-wms/%1/url" ).arg( wmsTitle ) ) )
  {
    QString msg = tr( "The %1 connection already exists. Do you want to overwrite it?" ).arg( wmsTitle );
    QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Overwrite" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
    if ( result != QMessageBox::Ok )
    {
      return;
    }
  }

  // add selected WMS to config and mark as current
  settings.setValue( QStringLiteral( "qgis/connections-wms/%1/url" ).arg( wmsTitle ), wmsUrl );
  QgsWMSConnection::setSelectedConnection( wmsTitle );
  populateConnectionList();

  tabServers->setCurrentIndex( 0 );
}

void QgsWMSSourceSelect::on_tableWidgetWMSList_itemSelectionChanged()
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

  QTreeWidgetItem *selectedItem = mLayerOrderTreeWidget->takeTopLevelItem( selectedIndex );
  mLayerOrderTreeWidget->insertTopLevelItem( selectedIndex - 1, selectedItem );
  mLayerOrderTreeWidget->clearSelection();
  selectedItem->setSelected( true );

  updateButtons();
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

  QTreeWidgetItem *selectedItem = mLayerOrderTreeWidget->takeTopLevelItem( selectedIndex );
  mLayerOrderTreeWidget->insertTopLevelItem( selectedIndex + 1, selectedItem );
  mLayerOrderTreeWidget->clearSelection();
  selectedItem->setSelected( true );

  updateButtons();
}

void QgsWMSSourceSelect::updateLayerOrderTab( const QStringList &newLayerList, const QStringList &newStyleList, const QStringList &newTitleList )
{
  //check, if each layer / style combination is already contained in the  layer order tab
  //if not, add it to the top of the list

  QStringList::const_iterator layerListIt = newLayerList.constBegin();
  QStringList::const_iterator styleListIt = newStyleList.constBegin();
  QStringList::const_iterator titleListIt = newTitleList.constBegin();

  for ( ; layerListIt != newLayerList.constEnd(); ++layerListIt, ++styleListIt, ++titleListIt )
  {
    bool combinationExists = false;
    for ( int i = 0; i < mLayerOrderTreeWidget->topLevelItemCount(); ++i )
    {
      QTreeWidgetItem *currentItem = mLayerOrderTreeWidget->topLevelItem( i );
      if ( currentItem->text( 0 ) == *layerListIt && currentItem->text( 1 ) == *styleListIt )
      {
        combinationExists = true;
        break;
      }
    }

    if ( !combinationExists )
    {
      QTreeWidgetItem *newItem = new QTreeWidgetItem();
      newItem->setText( 0, *layerListIt );
      newItem->setText( 1, *styleListIt );
      newItem->setText( 2, *titleListIt );
      mLayerOrderTreeWidget->addTopLevelItem( newItem );
    }

  }

  //check, if each layer style combination in the layer order tab is still in newLayerList / newStyleList
  //if not: remove it from the tree widget

  if ( mLayerOrderTreeWidget->topLevelItemCount() > 0 )
  {
    for ( int i = mLayerOrderTreeWidget->topLevelItemCount() - 1; i >= 0; --i )
    {
      QTreeWidgetItem *currentItem = mLayerOrderTreeWidget->topLevelItem( i );
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

void QgsWMSSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/ogc_client_support.html" ) );
}
