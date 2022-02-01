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
#include "qgsgui.h"

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
  , mDefaultCRS( geoEpsgCrsAuthId() )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnNew, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnLoad_clicked );
  connect( btnConnect, &QPushButton::clicked, this, &QgsWMSSourceSelect::btnConnect_clicked );
  connect( mCrsSelector, &QgsProjectionSelectionWidget::crsChanged, this, &QgsWMSSourceSelect::crsSelectorChanged );
  connect( lstLayers, &QTreeWidget::itemSelectionChanged, this, &QgsWMSSourceSelect::lstLayers_itemSelectionChanged );
  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsWMSSourceSelect::cmbConnections_activated );
  connect( lstTilesets, &QTableWidget::itemClicked, this, &QgsWMSSourceSelect::lstTilesets_itemClicked );
  connect( mLayerUpButton, &QPushButton::clicked, this, &QgsWMSSourceSelect::mLayerUpButton_clicked );
  connect( mLayerDownButton, &QPushButton::clicked, this, &QgsWMSSourceSelect::mLayerDownButton_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsWMSSourceSelect::showHelp );

  connect( mLayersFilterLineEdit, &QgsFilterLineEdit::textChanged, this, &QgsWMSSourceSelect::filterLayers );
  connect( mTilesetsFilterLineEdit, &QgsFilterLineEdit::textChanged, this, &QgsWMSSourceSelect::filterTiles );

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
      mCrsSelector->setCrs( currentRefSys );
    }

    // set up the default WMS Coordinate Reference System
    labelCoordRefSys->setDisabled( true );
    mCrsSelector->setDisabled( true );

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


  mInterpretationCombo = new QgsWmsInterpretationComboBox( this );
  mInterpretationLayout->addWidget( mInterpretationCombo );

  clear();

  // set up the WMS connections we already know about
  populateConnectionList();

}


void QgsWMSSourceSelect::refresh()
{
  // Reload WMS connections and update the GUI
  QgsDebugMsgLevel( QStringLiteral( "Refreshing WMS connections ..." ), 2 );
  populateConnectionList();
}


void QgsWMSSourceSelect::populateConnectionList()
{
  cmbConnections->clear();
  cmbConnections->addItems( QgsWMSConnection::connectionList() );

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

void QgsWMSSourceSelect::btnNew_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "qgis/connections-wms/" ), QString(), QgsNewHttpConnection::FlagShowHttpSettings );


  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsWMSSourceSelect::btnEdit_clicked()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, QgsNewHttpConnection::ConnectionWms, QStringLiteral( "qgis/connections-wms/" ), cmbConnections->currentText(), QgsNewHttpConnection::FlagShowHttpSettings );

  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }

  delete nc;
}

void QgsWMSSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result == QMessageBox::Yes )
  {
    QgsWMSConnection::deleteConnection( cmbConnections->currentText() );
    cmbConnections->removeItem( cmbConnections->currentIndex() );
    setConnectionListPosition();
    emit connectionsChanged();
  }
}

void QgsWMSSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WMS );
  dlg.exec();
}

void QgsWMSSourceSelect::btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
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
  item->setToolTip( 2, "<font color=black>" + names[1].simplified()  + "</font>" );
  item->setText( 3, names[2].simplified() );
  item->setToolTip( 3, "<font color=black>" + names[2].simplified()  + "</font>" );

  items[ id ] = item;

  return item;
}

void QgsWMSSourceSelect::clear()
{
  lstLayers->clear();
  lstTilesets->clearContents();

  mTreeInitialExpand.clear();
  mLayersFilterLineEdit->clearValue();

  mCRSs.clear();

  const auto constButtons = mImageFormatGroup->buttons();
  for ( QAbstractButton *b : constButtons )
  {
    b->setHidden( true );
  }

  mFeatureCount->setEnabled( false );

  mInterpretationCombo->setInterpretation( QString() );
}

bool QgsWMSSourceSelect::populateLayerList( const QgsWmsCapabilities &capabilities )
{
  const QVector<QgsWmsLayerProperty> layers = capabilities.supportedLayers();
  mLayerProperties = layers;

  QString defaultEncoding = QgsSettings().value( "qgis/WMSDefaultFormat", "" ).toString();

  bool first = true;
  QSet<QString> alreadyAddedLabels;
  const auto supportedImageEncodings = capabilities.supportedImageEncodings();
  for ( const QString &encoding : supportedImageEncodings )
  {
    int id = mMimeMap.value( encoding, -1 );
    if ( id < 0 )
    {
      QgsDebugMsg( QStringLiteral( "encoding %1 not supported." ).arg( encoding ) );
      continue;
    }
    // Different mime-types can map to the same label. Just add the first
    // match to avoid duplicates in the UI
    if ( alreadyAddedLabels.contains( mFormats[id].label ) )
    {
      continue;
    }
    alreadyAddedLabels.insert( mFormats[id].label );

    mImageFormatGroup->button( id )->setVisible( true );
    if ( first || encoding == defaultEncoding )
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

  for ( const QgsWmsLayerProperty &layer : layers )
  {
    QgsTreeWidgetItem *lItem = createItem( layer.orderId, QStringList() << layer.name << layer.title << layer.abstract, items, layerAndStyleCount, layerParents, layerParentNames );

    lItem->setData( 0, Qt::UserRole + 0, layer.name );
    lItem->setData( 0, Qt::UserRole + 1, "" );
    lItem->setData( 0, Qt::UserRole + 2, layer.crs );
    lItem->setData( 0, Qt::UserRole + 3, layer.title.isEmpty() ? layer.name : layer.title );

    // Also insert the styles
    // Layer Styles
    for ( const QgsWmsStyleProperty &property : layer.style )
    {
      QgsDebugMsgLevel( QStringLiteral( "got style name %1 and title '%2'." ).arg( property.name, property.title ), 2 );

      QgsTreeWidgetItem *lItem2 = new QgsTreeWidgetItem( lItem );
      lItem2->setText( 0, QString::number( ++layerAndStyleCount ) );
      lItem2->setText( 1, property.name.simplified() );
      lItem2->setText( 2, property.title.simplified() );
      lItem2->setText( 3, property.abstract.simplified() );

      lItem2->setData( 0, Qt::UserRole + 0, layer.name );
      lItem2->setData( 0, Qt::UserRole + 1, property.name );
      lItem2->setData( 0, Qt::UserRole + 3, property.title.isEmpty() ? property.name : property.title );
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
    for ( const QgsWmtsTileLayer &l : std::as_const( mTileLayers ) )
    {
      rows += l.styles.size() * l.setLinks.size() * l.formats.size();
    }

    lstTilesets->clearContents();
    lstTilesets->setRowCount( rows );
    lstTilesets->setSortingEnabled( false );

    int row = 0;
    for ( const QgsWmtsTileLayer &l : std::as_const( mTileLayers ) )
    {
      for ( const QgsWmtsStyle &style : l.styles )
      {
        for ( const QgsWmtsTileMatrixSetLink &setLink : l.setLinks )
        {
          for ( const QString &format : l.formats )
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
                item->setToolTip( tr( "Encoding %1 not supported." ).arg( format ) );
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

  lstLayers_itemSelectionChanged();

  return true;
}


void QgsWMSSourceSelect::btnConnect_clicked()
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
      capDownload.lastError()
    );
    return;
  }

  QgsWmsCapabilities caps { QgsProject::instance()->transformContext()  };
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

    collectDimensions( layers, uri );
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

    for ( const QgsWmtsTileLayer &l : std::as_const( mTileLayers ) )
    {
      if ( l.identifier == layers.join( QLatin1Char( ',' ) ) )
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
  QgsDebugMsgLevel( QStringLiteral( "crs=%2 " ).arg( crs ), 2 );

  if ( mFeatureCount->text().toInt() > 0 )
  {
    uri.setParam( QStringLiteral( "featureCount" ), mFeatureCount->text() );
  }

  if ( tabTilesets->isEnabled() && !mInterpretationCombo->interpretation().isEmpty() )
    uri.setParam( QStringLiteral( "interpretation" ), mInterpretationCombo->interpretation() );

  uri.setParam( QStringLiteral( "contextualWMSLegend" ), mContextualLegendCheckbox->isChecked() ? "1" : "0" );

  emit addRasterLayer( uri.encodedUri(),
                       leLayerName->text().isEmpty() ? titles.join( QLatin1Char( '/' ) ) : leLayerName->text(),
                       QStringLiteral( "wms" ) );
}

void QgsWMSSourceSelect::reset()
{
  lstLayers->clearSelection();
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

void QgsWMSSourceSelect::crsSelectorChanged( const QgsCoordinateReferenceSystem &crs )
{
  QStringList layers;
  const auto constSelectedItems = lstLayers->selectedItems();
  for ( QTreeWidgetItem *item : constSelectedItems )
  {
    QString layer = item->data( 0, Qt::UserRole + 0 ).toString();
    if ( !layer.isEmpty() )
      layers << layer;
  }

  mCRS = crs.authid();

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
    styles << QString();
    titles << titleName;

    if ( mCRSs.isEmpty() )
      mCRSs = qgis::listToSet( item->data( 0, Qt::UserRole + 2 ).toStringList() );
    else
      mCRSs.intersect( qgis::listToSet( item->data( 0, Qt::UserRole + 2 ).toStringList() ) );
  }
}

/**
 * retrieve selected layers
 */
void QgsWMSSourceSelect::lstLayers_itemSelectionChanged()
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
  const auto constSelectedItems = lstLayers->selectedItems();
  for ( QTreeWidgetItem *item : constSelectedItems )
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
      styles << QString();
      titles << titleName;
      if ( mCRSs.isEmpty() )
        mCRSs = qgis::listToSet( item->data( 0, Qt::UserRole + 2 ).toStringList() );
      else
        mCRSs.intersect( qgis::listToSet( item->data( 0, Qt::UserRole + 2 ).toStringList() ) );
    }
    else
    {
      // style: add named layer with selected style
      layers << layerName;
      styles << styleName;
      titles << titleName;
      if ( mCRSs.isEmpty() )
        mCRSs = qgis::listToSet( item->parent()->data( 0, Qt::UserRole + 2 ).toStringList() );
      else
        mCRSs.intersect( qgis::listToSet( item->parent()->data( 0, Qt::UserRole + 2 ).toStringList() ) );
    }
  }

  labelCoordRefSys->setText( tr( "Coordinate Reference System (%n available)", "crs count", mCRSs.count() ) );
  labelCoordRefSys->setDisabled( mCRSs.isEmpty() );
  mCrsSelector->setDisabled( mCRSs.isEmpty() );

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
      mCrsSelector->setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( mCRS ) );
    }

  }
  else if ( layers.isEmpty() || mCRSs.isEmpty() )
  {
    mCRS.clear();
    labelCoordRefSys->setText( tr( "Coordinate Reference System" ) );
    labelCoordRefSys->setDisabled( true );
  }

  updateLayerOrderTab( layers, styles, titles );
  updateButtons();
}

void QgsWMSSourceSelect::lstTilesets_itemClicked( QTableWidgetItem *item )
{
  Q_UNUSED( item )

  QTableWidgetItem *rowItem = lstTilesets->item( lstTilesets->currentRow(), 0 );
  bool wasSelected = mCurrentTileset == rowItem;

  lstTilesets->blockSignals( true );
  lstTilesets->clearSelection();
  if ( !wasSelected )
  {
    QgsDebugMsgLevel( QStringLiteral( "selecting current row %1" ).arg( lstTilesets->currentRow() ), 2 );
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
    labelCoordRefSys->setText( tr( "Coordinate Reference System (%n available)", "crs count", mCRSs.count() ) );
    labelCoordRefSys->setEnabled( !mCRSs.isEmpty() );
    mCrsSelector->setEnabled( !mCRSs.isEmpty() );

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
        mLastLayerName = titles.join( QLatin1Char( '/' ) );
        leLayerName->setText( mLastLayerName );
      }
    }
    else
    {
      mLastLayerName.clear();
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

void QgsWMSSourceSelect::collectDimensions( QStringList &layers, QgsDataSourceUri &uri )
{
  for ( const QgsWmsLayerProperty &layerProperty : std::as_const( mLayerProperties ) )
  {
    if ( layerProperty.name == layers.join( ',' ) )
    {
      // Check for layer dimensions
      for ( const QgsWmsDimensionProperty &dimension : std::as_const( layerProperty.dimensions ) )
      {
        // add temporal dimensions only
        if ( dimension.name == QLatin1String( "time" ) ||
             dimension.name == QLatin1String( "reference_time" ) )
        {
          QString name = dimension.name == QLatin1String( "time" ) ?
                         QStringLiteral( "timeDimensionExtent" ) : QStringLiteral( "referenceTimeDimensionExtent" );

          if ( !( uri.param( QLatin1String( "type" ) ) == QLatin1String( "wmst" ) ) )
            uri.setParam( QLatin1String( "type" ), QLatin1String( "wmst" ) );
          uri.setParam( name, dimension.extent );
        }
      }

      // WMS-T defaults settings
      if ( uri.param( QLatin1String( "type" ) ) == QLatin1String( "wmst" ) )
      {
        uri.setParam( QLatin1String( "temporalSource" ), QLatin1String( "provider" ) );
        uri.setParam( QLatin1String( "allowTemporalUpdates" ), QLatin1String( "true" ) );
      }

    }
  }
}

QString QgsWMSSourceSelect::selectedImageEncoding()
{
  // TODO: Match this hard coded list to the list of formats Qt reports it can actually handle.
  int id = mImageFormatGroup->checkedId();
  if ( id < 0 )
  {
    return QString();
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

void QgsWMSSourceSelect::cmbConnections_activated( int )
{
  // Remember which server was selected.
  QgsWMSConnection::setSelectedConnection( cmbConnections->currentText() );
}

void QgsWMSSourceSelect::filterLayers( const QString &searchText )
{
  std::function< void( QTreeWidgetItem *, bool ) > setChildrenVisible;
  setChildrenVisible = [&setChildrenVisible]( QTreeWidgetItem * item, bool visible )
  {
    for ( int i = 0; i < item->childCount(); ++i )
      setChildrenVisible( item->child( i ), visible );
    item->setHidden( !visible );
  };


  if ( searchText.isEmpty() )
  {
    // show everything and reset tree nesting
    setChildrenVisible( lstLayers->invisibleRootItem(), true );
    for ( QTreeWidgetItem *item : mTreeInitialExpand.keys() )
      if ( item )
        item->setExpanded( mTreeInitialExpand.value( item ) );
    mTreeInitialExpand.clear();
  }
  else
  {
    // hide all
    setChildrenVisible( lstLayers->invisibleRootItem(), false );
    // find and show matching items in name and title columns
    QSet<QTreeWidgetItem *> items = qgis::listToSet( lstLayers->findItems( searchText, Qt::MatchContains | Qt::MatchRecursive, 1 ) );
    items.unite( qgis::listToSet( lstLayers->findItems( searchText, Qt::MatchContains | Qt::MatchRecursive, 2 ) ) );

    // if nothing found, search in abstract too
    if ( items.isEmpty() )
    {
      items = qgis::listToSet( lstLayers->findItems( searchText, Qt::MatchContains | Qt::MatchRecursive, 3 ) );
    }

    mTreeInitialExpand.clear();
    for ( QTreeWidgetItem *item : std::as_const( items ) )
    {
      setChildrenVisible( item, true );

      QTreeWidgetItem *parent = item;
      while ( parent )
      {
        if ( mTreeInitialExpand.contains( parent ) )
          break;
        mTreeInitialExpand.insert( parent, parent->isExpanded() );
        parent->setExpanded( true );
        parent->setHidden( false );
        parent = parent->parent();
      }
    }
  }
}

void QgsWMSSourceSelect::filterTiles( const QString &searchText )
{
  QList<int> rowsShown;
  if ( !searchText.isEmpty() )
  {
    const QList<QTableWidgetItem *> items = lstTilesets->findItems( searchText, Qt::MatchContains );
    for ( const QTableWidgetItem *item : items )
    {
      rowsShown << item->row();
    }
  }
  for ( int r = 0; r < lstTilesets->rowCount(); r++ )
  {
    bool visible = rowsShown.isEmpty() || rowsShown.contains( r );
    lstTilesets->setRowHidden( r, !visible );
  }
}

QString QgsWMSSourceSelect::descriptionForAuthId( const QString &authId )
{
  if ( mCrsNames.contains( authId ) )
    return mCrsNames[ authId ];

  QgsCoordinateReferenceSystem qgisSrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( authId );
  mCrsNames.insert( authId, qgisSrs.userFriendlyIdentifier() );
  return qgisSrs.userFriendlyIdentifier();
}

void QgsWMSSourceSelect::mLayerUpButton_clicked()
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

void QgsWMSSourceSelect::mLayerDownButton_clicked()
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

QgsWmsInterpretationComboBox::QgsWmsInterpretationComboBox( QWidget *parent ): QComboBox( parent )
{
  addItem( tr( "Default" ), QString() );
  addItem( QgsWmsInterpretationConverterMapTilerTerrainRGB::displayName(), QgsWmsInterpretationConverterMapTilerTerrainRGB::interpretationKey() );
  addItem( QgsWmsInterpretationConverterTerrariumRGB::displayName(), QgsWmsInterpretationConverterTerrariumRGB::interpretationKey() );
}

void QgsWmsInterpretationComboBox::setInterpretation( const QString &interpretationKey )
{
  if ( ! interpretationKey.isEmpty() )
  {
    int index = findData( interpretationKey );
    if ( index == -1 )
      setCurrentIndex( 0 );
    else
      setCurrentIndex( index );
  }
}

QString QgsWmsInterpretationComboBox::interpretation() const
{
  return currentData().toString();
}
