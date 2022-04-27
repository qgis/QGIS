/***************************************************************************
                              qgswfssourceselect.cpp
                              -------------------
  begin                : August 25, 2006
  copyright            : (C) 2016 by Marco Hugentobler
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

#include "qgswfsconstants.h"
#include "qgswfssourceselect.h"
#include "qgswfsconnection.h"
#include "qgswfscapabilities.h"
#include "qgswfsprovider.h"
#include "qgswfsdatasourceuri.h"
#include "qgswfsutils.h"
#include "qgswfsnewconnection.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsoapifprovider.h"
#include "qgssqlstatement.h"
#include "qgssettings.h"
#include "qgsgui.h"
#include "qgsquerybuilder.h"
#include "qgswfsguiutils.h"
#include "qgswfssubsetstringeditor.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>
#include <QRegularExpression>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_NAME,
  MODEL_IDX_ABSTRACT,
  MODEL_IDX_SQL
};

QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsWFSSourceSelect::cmbConnections_activated );
  connect( btnSave, &QPushButton::clicked, this, &QgsWFSSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsWFSSourceSelect::btnLoad_clicked );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsWFSSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }


  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  mBuildQueryButton->setDisabled( true );


  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::buildQueryButtonClicked );

  connect( buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject );
  connect( btnNew, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::addEntryToServerList );
  connect( btnEdit, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::modifyEntryOfServerList );
  connect( btnDelete, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::deleteEntryOfServerList );
  connect( btnConnect, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::connectToServer );
  connect( btnChangeSpatialRefSys, &QAbstractButton::clicked, this, &QgsWFSSourceSelect::changeCRS );
  connect( lineFilter, &QLineEdit::textChanged, this, &QgsWFSSourceSelect::filterChanged );
  populateConnectionList();
  mProjectionSelector = new QgsProjectionSelectionDialog( this );
  mProjectionSelector->setMessage( QString() );

  mItemDelegate = new QgsWFSItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  QgsSettings settings;
  QgsDebugMsgLevel( QStringLiteral( "restoring settings" ), 3 );
  cbxUseTitleLayerName->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/UseTitleLayerName" ), false ).toBool() );
  cbxFeatureCurrentViewExtent->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), true ).toBool() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/HoldDialogOpen" ), false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( tr( "Title" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_NAME, new QStandardItem( tr( "Name" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_ABSTRACT, new QStandardItem( tr( "Abstract" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_SQL, new QStandardItem( tr( "Sql" ) ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );

  connect( treeView, &QAbstractItemView::doubleClicked, this, &QgsWFSSourceSelect::treeWidgetItemDoubleClicked );
  connect( treeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsWFSSourceSelect::treeWidgetCurrentRowChanged );
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  QApplication::restoreOverrideCursor();

  QgsSettings settings;
  QgsDebugMsgLevel( QStringLiteral( "saving settings" ), 3 );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/UseTitleLayerName" ), cbxUseTitleLayerName->isChecked() );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), cbxFeatureCurrentViewExtent->isChecked() );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  delete mItemDelegate;
  delete mProjectionSelector;
  delete mModel;
  delete mModelProxy;
  delete mBuildQueryButton;
}

void QgsWFSSourceSelect::reset()
{
  treeView->clearSelection();
}

void QgsWFSSourceSelect::populateConnectionList()
{
  QStringList keys = QgsWfsConnection::connectionList();

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
  QString selectedConnection = QgsWfsConnection::selectedConnection();
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }

  changeConnection();
}

QString QgsWFSSourceSelect::getPreferredCrs( const QSet<QString> &crsSet ) const
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
  if ( crsSet.contains( geoEpsgCrsAuthId() ) )
  {
    return geoEpsgCrsAuthId();
  }

  //third: first entry in set
  return *( crsSet.constBegin() );
}

void QgsWFSSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsWFSSourceSelect::capabilitiesReplyFinished()
{
  QApplication::restoreOverrideCursor();
  btnConnect->setEnabled( true );

  if ( !mCapabilities )
    return;

  auto err = mCapabilities->errorCode();
  if ( err != QgsBaseNetworkRequest::NoError )
  {
    if ( mVersion == QgsWFSConstants::VERSION_AUTO )
    {
      startOapifLandingPageRequest();
    }
    else
    {
      QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( mCapabilities.get(), this );
      mCapabilities.reset();
      emit enableButtons( false );
    }
    return;
  }

  mCaps = mCapabilities->capabilities();

  mAvailableCRS.clear();
  for ( const QgsWfsCapabilities::FeatureType &featureType : std::as_const( mCaps.featureTypes ) )
  {
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem *titleItem = new QStandardItem( featureType.title );
    QStandardItem *nameItem = new QStandardItem( featureType.name );
    QStandardItem *abstractItem = new QStandardItem( featureType.abstract );
    abstractItem->setToolTip( "<font color=black>" + featureType.abstract  + "</font>" );
    abstractItem->setTextAlignment( Qt::AlignLeft | Qt::AlignTop );
    QStandardItem *filterItem = new QStandardItem();

    typedef QList< QStandardItem * > StandardItemList;
    mModel->appendRow( StandardItemList() << titleItem << nameItem << abstractItem << filterItem );

    // insert the available CRS into mAvailableCRS
    mAvailableCRS.insert( featureType.name, featureType.crslist );
  }

  resizeTreeViewAfterModelFill();
}


void QgsWFSSourceSelect::resizeTreeViewAfterModelFill()
{
  if ( mModel->rowCount() > 0 )
  {
    treeView->resizeColumnToContents( MODEL_IDX_TITLE );
    treeView->resizeColumnToContents( MODEL_IDX_NAME );
    treeView->resizeColumnToContents( MODEL_IDX_ABSTRACT );
    for ( int i = MODEL_IDX_TITLE; i < MODEL_IDX_ABSTRACT; i++ )
    {
      if ( treeView->columnWidth( i ) > 300 )
      {
        treeView->setColumnWidth( i, 300 );
      }
    }
    if ( treeView->columnWidth( MODEL_IDX_ABSTRACT ) > 150 )
    {
      treeView->setColumnWidth( MODEL_IDX_ABSTRACT, 150 );
    }
    btnChangeSpatialRefSys->setEnabled( true );
    treeView->selectionModel()->setCurrentIndex( mModelProxy->index( 0, 0 ), QItemSelectionModel::SelectCurrent | QItemSelectionModel::Rows );
    treeView->setFocus();
  }
  else
  {
    QMessageBox::information( nullptr, tr( "No Layers" ), tr( "capabilities document contained no layers." ) );
    emit enableButtons( false );
    mBuildQueryButton->setEnabled( false );
  }
}

void QgsWFSSourceSelect::startOapifLandingPageRequest()
{
  QgsWfsConnection connection( cmbConnections->currentText() );
  mOAPIFLandingPage.reset( new QgsOapifLandingPageRequest( connection.uri() ) );
  connect( mOAPIFLandingPage.get(), &QgsOapifLandingPageRequest::gotResponse, this, &QgsWFSSourceSelect::oapifLandingPageReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  mOAPIFLandingPage->request( synchronous, forceRefresh );
  QApplication::setOverrideCursor( Qt::WaitCursor );
  btnConnect->setEnabled( false );
}

void QgsWFSSourceSelect::oapifLandingPageReplyFinished()
{
  QApplication::restoreOverrideCursor();
  btnConnect->setEnabled( true );

  if ( !mOAPIFLandingPage )
    return;

  auto err = mOAPIFLandingPage->errorCode();
  if ( err != QgsBaseNetworkRequest::NoError )
  {
    if ( mVersion == QgsWFSConstants::VERSION_AUTO && mCapabilities )
    {
      QgsWfsGuiUtils::displayErrorMessageOnFailedCapabilities( mCapabilities.get(), this );
      mCapabilities.reset();
    }
    else
    {
      QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), mOAPIFLandingPage->errorMessage(), QMessageBox::Ok, this );
      box->setAttribute( Qt::WA_DeleteOnClose );
      box->setModal( true );
      box->open();
    }

    mOAPIFLandingPage.reset();
    emit enableButtons( false );
    return;
  }
  mCapabilities.reset();

  mAvailableCRS.clear();
  QString url( mOAPIFLandingPage->collectionsUrl() );

  // Add back any extra query parameters, see issue GH #46535
  const QgsWfsConnection connection( cmbConnections->currentText() );
  const QUrl connectionUrl( connection.uri().param( QStringLiteral( "url" ) ) );
  if ( ! connectionUrl.query().isEmpty() )
  {
    url.append( '?' );
    url.append( connectionUrl.query() );
  }

  mOAPIFLandingPage.reset();
  startOapifCollectionsRequest( url );
}

void QgsWFSSourceSelect::startOapifCollectionsRequest( const QString &url )
{
  QgsWfsConnection connection( cmbConnections->currentText() );
  mOAPIFCollections.reset( new QgsOapifCollectionsRequest( connection.uri(), url ) );
  connect( mOAPIFCollections.get(), &QgsOapifCollectionsRequest::gotResponse, this, &QgsWFSSourceSelect::oapifCollectionsReplyFinished );
  const bool synchronous = false;
  const bool forceRefresh = true;
  mOAPIFCollections->request( synchronous, forceRefresh );
  QApplication::setOverrideCursor( Qt::WaitCursor );
  btnConnect->setEnabled( false );
}

void QgsWFSSourceSelect::oapifCollectionsReplyFinished()
{
  QApplication::restoreOverrideCursor();
  btnConnect->setEnabled( true );
  if ( !mOAPIFCollections )
    return;

  auto err = mOAPIFCollections->errorCode();
  if ( err != QgsBaseNetworkRequest::NoError )
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Error" ), mOAPIFCollections->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->open();

    mOAPIFCollections.reset();
    emit enableButtons( false );
    return;
  }

  for ( const auto &collection : mOAPIFCollections->collections() )
  {
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem *titleItem = new QStandardItem( collection.mTitle );
    QStandardItem *nameItem = new QStandardItem( collection.mId );
    QStandardItem *abstractItem = new QStandardItem( collection.mDescription );
    abstractItem->setToolTip( "<font color=black>" + collection.mDescription  + "</font>" );
    abstractItem->setTextAlignment( Qt::AlignLeft | Qt::AlignTop );
    QStandardItem *filterItem = new QStandardItem();

    typedef QList< QStandardItem * > StandardItemList;
    mModel->appendRow( StandardItemList() << titleItem << nameItem << abstractItem << filterItem );
  }

  if ( !mOAPIFCollections->nextUrl().isEmpty() )
  {
    QString url( mOAPIFCollections->nextUrl() );
    mOAPIFCollections.reset();
    startOapifCollectionsRequest( url );
    return;
  }

  mVersion = QStringLiteral( "OGC_API_FEATURES" );

  resizeTreeViewAfterModelFill();
}

void QgsWFSSourceSelect::addEntryToServerList()
{
  auto nc = new QgsWFSNewConnection( this );
  nc->setAttribute( Qt::WA_DeleteOnClose );
  nc->setWindowTitle( tr( "Create a New WFS Connection" ) );

  // For testability, do not use exec()
  if ( !property( "hideDialogs" ).toBool() )
    nc->open();
  connect( nc, &QDialog::accepted, this, &QgsWFSSourceSelect::populateConnectionList );
  connect( nc, &QDialog::accepted, this, &QgsWFSSourceSelect::connectionsChanged );
}

void QgsWFSSourceSelect::modifyEntryOfServerList()
{
  auto nc = new QgsWFSNewConnection( this, cmbConnections->currentText() );
  nc->setAttribute( Qt::WA_DeleteOnClose );
  nc->setWindowTitle( tr( "Modify WFS Connection" ) );

  // For testability, do not use exec()
  if ( !property( "hideDialogs" ).toBool() )
    nc->open();
  connect( nc, &QDialog::accepted, this, &QgsWFSSourceSelect::populateConnectionList );
  connect( nc, &QDialog::accepted, this, &QgsWFSSourceSelect::connectionsChanged );
}

void QgsWFSSourceSelect::deleteEntryOfServerList()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result == QMessageBox::Yes )
  {
    QgsWfsConnection::deleteConnection( cmbConnections->currentText() );
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

  QgsWfsConnection connection( cmbConnections->currentText() );

  const QString uri = connection.uri().uri( false );
  mVersion = QgsWFSDataSourceURI( uri ).version();
  if ( mVersion == QLatin1String( "OGC_API_FEATURES" ) )
  {
    startOapifLandingPageRequest();
  }
  else
  {
    mCapabilities.reset( new QgsWfsCapabilities( uri ) );
    connect( mCapabilities.get(), &QgsWfsCapabilities::gotCapabilities, this, &QgsWFSSourceSelect::capabilitiesReplyFinished );
    const bool synchronous = false;
    const bool forceRefresh = true;
    if ( mVersion == QgsWFSConstants::VERSION_AUTO )
    {
      mCapabilities->setLogErrors( false ); // as this might be a OAPIF server
    }
    mCapabilities->requestCapabilities( synchronous, forceRefresh );
    QApplication::setOverrideCursor( Qt::WaitCursor );
  }
}


void QgsWFSSourceSelect::addButtonClicked()
{
  //get selected entry in treeview
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsWfsConnection connection( cmbConnections->currentText() );

  QString pCrsString( labelCoordRefSys->text() );

  //create layers that user selected from this WFS source
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
    QString typeName = mModel->item( row, MODEL_IDX_NAME )->text(); //WFS repository's name for layer
    QString titleName = mModel->item( row, MODEL_IDX_TITLE )->text(); //WFS type name title for layer name (if option is set)
    QString sql = mModel->item( row, MODEL_IDX_SQL )->text(); //optional SqL specified by user
    QString layerName = typeName;
    if ( cbxUseTitleLayerName->isChecked() && !titleName.isEmpty() )
    {
      layerName = titleName;
    }
    QgsDebugMsgLevel( "Layer " + typeName + " SQL is " + sql, 3 );

    mUri = QgsWFSDataSourceURI::build( connection.uri().uri( false ), typeName,
                                       pCrsString,
                                       isOapif() ? QString() : sql,
                                       isOapif() ? sql : QString(),
                                       cbxFeatureCurrentViewExtent->isChecked() );

    emit addVectorLayer( mUri, layerName, isOapif() ? QgsOapifProvider::OAPIF_PROVIDER_KEY : QgsWFSProvider::WFS_PROVIDER_KEY );
  }

  if ( ! mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
  {
    accept();
  }
}

void QgsWFSSourceSelect::buildQuery( const QModelIndex &index )
{
  if ( !index.isValid() )
  {
    return;
  }

  const QString typeName = index.sibling( index.row(), MODEL_IDX_NAME ).data().toString();

  //get available fields for wfs layer
  QgsWfsConnection connection( cmbConnections->currentText() );
  QgsWFSDataSourceURI uri( connection.uri().uri( false ) );
  uri.setTypeName( typeName );

  QModelIndex filterIndex = index.sibling( index.row(), MODEL_IDX_SQL );
  QString sql( filterIndex.data().toString() );

  if ( isOapif() )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );
    QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
    QgsVectorLayer vlayer( uri.uri(), typeName, QgsOapifProvider::OAPIF_PROVIDER_KEY, options );
    QApplication::restoreOverrideCursor();

    if ( !vlayer.isValid() )
    {
      return;
    }

    // create a query builder object
    QgsQueryBuilder gb( &vlayer, this );
    gb.setSql( sql );

    if ( gb.exec() )
    {
      auto provider = dynamic_cast<QgsOapifProvider *>( vlayer.dataProvider() );
      Q_ASSERT( provider );
      if ( provider->filterTranslatedState() == QgsOapifProvider::FilterTranslationState::FULLY_CLIENT )
      {
        QMessageBox::information( nullptr, tr( "Filter" ), tr( "Whole filter will be evaluated on client side." ) );
      }
      else if ( provider->filterTranslatedState() == QgsOapifProvider::FilterTranslationState::PARTIAL )
      {
        QMessageBox::information( nullptr, tr( "Filter" ),
                                  tr( "The following part of the filter will be evaluated on client side : %1" ).arg( provider->clientSideFilterExpression() ) );
      }
      mModelProxy->setData( filterIndex, QVariant( gb.sql() ) );
    }
    return;
  }

  QgsDataProvider::ProviderOptions providerOptions;
  QgsWFSProvider p( uri.uri(), providerOptions, mCaps );
  if ( !p.isValid() )
  {
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, tr( "Server exception" ), tr( "DescribeFeatureType failed" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( QStringLiteral( "WFSFeatureTypeErrorBox" ) );
    if ( !property( "hideDialogs" ).toBool() )
      box->open();

    return;
  }

  QString displayedTypeName( typeName );
  if ( !mCaps.setAmbiguousUnprefixedTypename.contains( QgsWFSUtils::removeNamespacePrefix( typeName ) ) )
    displayedTypeName = QgsWFSUtils::removeNamespacePrefix( typeName );
  QString allSql( "SELECT * FROM " + QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );
  if ( sql.isEmpty() )
  {
    sql = allSql;
  }

  auto d = QgsWfsSubsetStringEditor::create( nullptr, &p, this );

  d->setSubsetString( sql );

  mSQLIndex = index;
  mSQLComposerDialog = d;
  // For testability, do not use exec()
  if ( property( "hideDialogs" ).toBool() )
  {
    d->setAttribute( Qt::WA_DeleteOnClose );
    d->setModal( true );
    d->open();
    connect( d, &QDialog::accepted, this, &QgsWFSSourceSelect::updateSql );
  }
  else
  {
    // But we need to use exec() for real GUI, otherwise it does not look
    // right on Mac
    if ( d->exec() )
    {
      updateSql();
    }
    delete d;
  }
}

void QgsWFSSourceSelect::updateSql()
{
  QgsDebugMsgLevel( QStringLiteral( "updateSql called" ), 2 );
  Q_ASSERT( mSQLComposerDialog );

  const QString typeName = mSQLIndex.sibling( mSQLIndex.row(), MODEL_IDX_NAME ).data().toString();
  QModelIndex filterIndex = mSQLIndex.sibling( mSQLIndex.row(), MODEL_IDX_SQL );

  QString sql = mSQLComposerDialog->subsetString();
  mSQLComposerDialog = nullptr;

  QString displayedTypeName( typeName );
  if ( !mCaps.setAmbiguousUnprefixedTypename.contains( QgsWFSUtils::removeNamespacePrefix( typeName ) ) )
    displayedTypeName = QgsWFSUtils::removeNamespacePrefix( typeName );
  QString allSql( "SELECT * FROM " + QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );
  if ( sql == allSql )
    sql.clear();
  QgsDebugMsgLevel( "SQL text = " + sql, 2 );
  mModelProxy->setData( filterIndex, QVariant( sql ) );
}

void QgsWFSSourceSelect::changeCRS()
{
  if ( mProjectionSelector->exec() )
  {
    QString crsString = mProjectionSelector->crs().authid();
    labelCoordRefSys->setText( crsString );
  }
}

void QgsWFSSourceSelect::changeCRSFilter()
{
  QgsDebugMsgLevel( QStringLiteral( "changeCRSFilter called" ), 2 );
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    QString currentTypename = currentIndex.sibling( currentIndex.row(), MODEL_IDX_NAME ).data().toString();
    QgsDebugMsgLevel( QStringLiteral( "the current typename is: %1" ).arg( currentTypename ), 2 );

    QMap<QString, QStringList >::const_iterator crsIterator = mAvailableCRS.constFind( currentTypename );
    if ( crsIterator != mAvailableCRS.constEnd() )
    {
      QSet<QString> crsNames( qgis::listToSet( *crsIterator ) );

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

void QgsWFSSourceSelect::cmbConnections_activated( int index )
{
  Q_UNUSED( index )
  QgsWfsConnection::setSelectedConnection( cmbConnections->currentText() );
  changeConnection();
}

void QgsWFSSourceSelect::changeConnection()
{
  mCapabilities.reset();
  mOAPIFLandingPage.reset();
}

void QgsWFSSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WFS );
  dlg.exec();
}

void QgsWFSSourceSelect::btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::WFS, fileName );
  dlg.exec();
  populateConnectionList();
  emit connectionsChanged();
}

void QgsWFSSourceSelect::treeWidgetItemDoubleClicked( const QModelIndex &index )
{
  QgsDebugMsgLevel( QStringLiteral( "double-click called" ), 2 );
  buildQuery( index );
}

void QgsWFSSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  QgsDebugMsgLevel( QStringLiteral( "treeWidget_currentRowChanged called" ), 2 );
  changeCRSFilter();
  mBuildQueryButton->setEnabled( current.isValid() );
  emit enableButtons( current.isValid() );
}

void QgsWFSSourceSelect::buildQueryButtonClicked()
{
  QgsDebugMsgLevel( QStringLiteral( "mBuildQueryButton click called" ), 2 );
  buildQuery( treeView->selectionModel()->currentIndex() );
}

void QgsWFSSourceSelect::filterChanged( const QString &text )
{
  QgsDebugMsgLevel( "WFS FeatureType filter changed to :" + text, 2 );
  QRegularExpression regExp( text, QRegularExpression::CaseInsensitiveOption );
  mModelProxy->setFilterRegularExpression( regExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

QSize QgsWFSItemDelegate::sizeHint( const QStyleOptionViewItem &option, const QModelIndex &index ) const
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

void QgsWFSSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_ogc/ogc_client_support.html" ) );
}
