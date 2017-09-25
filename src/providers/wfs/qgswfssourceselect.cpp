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
#include "qgsnewhttpconnection.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgssqlstatement.h"
#include "qgssettings.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QFileDialog>
#include <QPainter>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_NAME,
  MODEL_IDX_ABSTRACT,
  MODEL_IDX_SQL
};

QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
  , mCapabilities( nullptr )
  , mSQLComposerDialog( nullptr )
{
  setupUi( this );
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
  QgsDebugMsg( "restoring settings" );
  restoreGeometry( settings.value( QStringLiteral( "Windows/WFSSourceSelect/geometry" ) ).toByteArray() );
  cbxUseTitleLayerName->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/UseTitleLayerName" ), false ).toBool() );
  cbxFeatureCurrentViewExtent->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), true ).toBool() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/WFSSourceSelect/HoldDialogOpen" ), false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( QStringLiteral( "Title" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_NAME, new QStandardItem( QStringLiteral( "Name" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_ABSTRACT, new QStandardItem( QStringLiteral( "Abstract" ) ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_SQL, new QStandardItem( QStringLiteral( "Sql" ) ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );

  connect( treeView, &QAbstractItemView::doubleClicked, this, &QgsWFSSourceSelect::treeWidgetItemDoubleClicked );
  connect( treeView->selectionModel(), &QItemSelectionModel::currentRowChanged, this, &QgsWFSSourceSelect::treeWidgetCurrentRowChanged );
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  QgsSettings settings;
  QgsDebugMsg( "saving settings" );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/UseTitleLayerName" ), cbxUseTitleLayerName->isChecked() );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/FeatureCurrentViewExtent" ), cbxFeatureCurrentViewExtent->isChecked() );
  settings.setValue( QStringLiteral( "Windows/WFSSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  delete mItemDelegate;
  delete mProjectionSelector;
  delete mCapabilities;
  delete mModel;
  delete mModelProxy;
  delete mBuildQueryButton;
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

  QgsWfsConnection connection( cmbConnections->currentText() );
  delete mCapabilities;
  mCapabilities = new QgsWfsCapabilities( connection.uri().uri() );
  connect( mCapabilities, &QgsWfsCapabilities::gotCapabilities, this, &QgsWFSSourceSelect::capabilitiesReplyFinished );
}

QString QgsWFSSourceSelect::getPreferredCrs( const QSet<QString> &crsSet ) const
{
  if ( crsSet.size() < 1 )
  {
    return QLatin1String( "" );
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
  if ( crsSet.contains( GEO_EPSG_CRS_AUTHID ) )
  {
    return GEO_EPSG_CRS_AUTHID;
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
  btnConnect->setEnabled( true );

  if ( !mCapabilities )
    return;

  QgsWfsCapabilities::ErrorCode err = mCapabilities->errorCode();
  if ( err != QgsWfsCapabilities::NoError )
  {
    QString title;
    switch ( err )
    {
      case QgsWfsCapabilities::NetworkError:
        title = tr( "Network Error" );
        break;
      case QgsWfsCapabilities::XmlError:
        title = tr( "Capabilities document is not valid" );
        break;
      case QgsWfsCapabilities::ServerExceptionError:
        title = tr( "Server Exception" );
        break;
      default:
        title = tr( "Error" );
        break;
    }
    // handle errors
    QMessageBox *box = new QMessageBox( QMessageBox::Critical, title, mCapabilities->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( QStringLiteral( "WFSCapabilitiesErrorBox" ) );
    if ( !property( "hideDialogs" ).toBool() )
      box->open();

    emit enableButtons( false );
    return;
  }

  mCaps = mCapabilities->capabilities();

  mAvailableCRS.clear();
  Q_FOREACH ( const QgsWfsCapabilities::FeatureType &featureType, mCaps.featureTypes )
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

  if ( !mCaps.featureTypes.isEmpty() )
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

void QgsWFSSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS );
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
  QgsNewHttpConnection *nc = new QgsNewHttpConnection( this, QgsNewHttpConnection::ConnectionWfs, QgsWFSConstants::CONNECTIONS_WFS, cmbConnections->currentText() );
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
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
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
  if ( mCapabilities )
  {
    const bool synchronous = false;
    const bool forceRefresh = true;
    mCapabilities->requestCapabilities( synchronous, forceRefresh );
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
    QgsDebugMsg( "Layer " + typeName + " SQL is " + sql );

    mUri = QgsWFSDataSourceURI::build( connection.uri().uri(), typeName, pCrsString,
                                       sql, cbxFeatureCurrentViewExtent->isChecked() );

    emit addVectorLayer( mUri, layerName );
  }

  if ( ! mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
  {
    accept();
  }
}

QgsWFSValidatorCallback::QgsWFSValidatorCallback( QObject *parent,
    const QgsWFSDataSourceURI &uri,
    const QString &allSql,
    const QgsWfsCapabilities::Capabilities &caps )
  : QObject( parent )
  , mURI( uri )
  , mAllSql( allSql )
  , mCaps( caps )
{
}

bool QgsWFSValidatorCallback::isValid( const QString &sqlStr, QString &errorReason, QString &warningMsg )
{
  errorReason.clear();
  if ( sqlStr.isEmpty() || sqlStr == mAllSql )
    return true;

  QgsWFSDataSourceURI uri( mURI );
  uri.setSql( sqlStr );
  QgsWFSProvider p( uri.uri(), mCaps );
  if ( !p.isValid() )
  {
    errorReason = p.processSQLErrorMsg();
    return false;
  }
  warningMsg = p.processSQLWarningMsg();

  return true;
}

QgsWFSTableSelectedCallback::QgsWFSTableSelectedCallback( QgsSQLComposerDialog *dialog,
    const QgsWFSDataSourceURI &uri,
    const QgsWfsCapabilities::Capabilities &caps )
  : QObject( dialog )
  , mDialog( dialog )
  , mURI( uri )
  , mCaps( caps )
{
}

void QgsWFSTableSelectedCallback::tableSelected( const QString &name )
{
  QString typeName( QgsSQLStatement::stripQuotedIdentifier( name ) );
  QString prefixedTypename( mCaps.addPrefixIfNeeded( typeName ) );
  if ( prefixedTypename.isEmpty() )
    return;
  QgsWFSDataSourceURI uri( mURI );
  uri.setTypeName( prefixedTypename );
  QgsWFSProvider p( uri.uri(), mCaps );
  if ( !p.isValid() )
  {
    return;
  }

  QList< QgsSQLComposerDialog::PairNameType> fieldList;
  QString fieldNamePrefix( QgsSQLStatement::quotedIdentifierIfNeeded( typeName ) + "." );
  Q_FOREACH ( const QgsField &field, p.fields().toList() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !p.geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( p.geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, QStringLiteral( "geometry" ) );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", QLatin1String( "" ) );

  mDialog->addColumnNames( fieldList, name );
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
  QgsWFSDataSourceURI uri( connection.uri().uri() );
  uri.setTypeName( typeName );
  QgsWFSProvider p( uri.uri(), mCaps );
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

  QModelIndex filterIndex = index.sibling( index.row(), MODEL_IDX_SQL );
  QString sql( filterIndex.data().toString() );
  QString displayedTypeName( typeName );
  if ( !mCaps.setAmbiguousUnprefixedTypename.contains( QgsWFSUtils::removeNamespacePrefix( typeName ) ) )
    displayedTypeName = QgsWFSUtils::removeNamespacePrefix( typeName );
  QString allSql( "SELECT * FROM " + QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );
  if ( sql.isEmpty() )
  {
    sql = allSql;
  }

  QgsSQLComposerDialog *d = new QgsSQLComposerDialog( this );

  QgsWFSValidatorCallback *validatorCbk = new QgsWFSValidatorCallback( d, uri, allSql, mCaps );
  d->setSQLValidatorCallback( validatorCbk );

  QgsWFSTableSelectedCallback *tableSelectedCbk = new QgsWFSTableSelectedCallback( d, uri, mCaps );
  d->setTableSelectedCallback( tableSelectedCbk );

  const bool bSupportJoins = mCaps.featureTypes.size() > 1 && mCaps.supportsJoins;
  d->setSupportMultipleTables( bSupportJoins, QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  QMap< QString, QString > mapTypenameToTitle;
  Q_FOREACH ( const QgsWfsCapabilities::FeatureType f, mCaps.featureTypes )
    mapTypenameToTitle[f.name] = f.title;

  QList< QgsSQLComposerDialog::PairNameTitle > tablenames;
  tablenames << QgsSQLComposerDialog::PairNameTitle(
               QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ), mapTypenameToTitle[typeName] );
  if ( bSupportJoins )
  {
    for ( int i = 0; i < mModel->rowCount(); i++ )
    {
      const QString iterTypename = mModel->index( i, MODEL_IDX_NAME ).data().toString();
      if ( iterTypename != typeName )
      {
        QString displayedIterTypename( iterTypename );
        QString unprefixedIterTypename( QgsWFSUtils::removeNamespacePrefix( iterTypename ) );
        if ( !mCaps.setAmbiguousUnprefixedTypename.contains( unprefixedIterTypename ) )
          displayedIterTypename = unprefixedIterTypename;

        tablenames << QgsSQLComposerDialog::PairNameTitle(
                     QgsSQLStatement::quotedIdentifierIfNeeded( displayedIterTypename ), mapTypenameToTitle[iterTypename] );
      }
    }
  }
  d->addTableNames( tablenames );

  QList< QgsSQLComposerDialog::Function> functionList;
  Q_FOREACH ( const QgsWfsCapabilities::Function &f, mCaps.functionList )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    Q_FOREACH ( const QgsWfsCapabilities::Argument &arg, f.argumentList )
    {
      dialogF.argumentList << QgsSQLComposerDialog::Argument( arg.name, arg.type );
    }
    functionList << dialogF;
  }
  d->addFunctions( functionList );

  QList< QgsSQLComposerDialog::Function> spatialPredicateList;
  Q_FOREACH ( const QgsWfsCapabilities::Function &f, mCaps.spatialPredicatesList )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    Q_FOREACH ( const QgsWfsCapabilities::Argument &arg, f.argumentList )
    {
      dialogF.argumentList << QgsSQLComposerDialog::Argument( arg.name, arg.type );
    }
    spatialPredicateList << dialogF;
  }
  d->addSpatialPredicates( spatialPredicateList );

  QList< QgsSQLComposerDialog::PairNameType> fieldList;
  QString fieldNamePrefix;
  if ( bSupportJoins )
  {
    fieldNamePrefix = QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) + ".";
  }
  Q_FOREACH ( const QgsField &field, p.fields().toList() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !p.geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( p.geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, QStringLiteral( "geometry" ) );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", QLatin1String( "" ) );

  d->addColumnNames( fieldList, QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  d->setSql( sql );

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
  QgsDebugMsg( "updateSql called" );
  Q_ASSERT( mSQLComposerDialog );

  const QString typeName = mSQLIndex.sibling( mSQLIndex.row(), MODEL_IDX_NAME ).data().toString();
  QModelIndex filterIndex = mSQLIndex.sibling( mSQLIndex.row(), MODEL_IDX_SQL );

  QString sql = mSQLComposerDialog->sql();
  mSQLComposerDialog = nullptr;

  QString displayedTypeName( typeName );
  if ( !mCaps.setAmbiguousUnprefixedTypename.contains( QgsWFSUtils::removeNamespacePrefix( typeName ) ) )
    displayedTypeName = QgsWFSUtils::removeNamespacePrefix( typeName );
  QString allSql( "SELECT * FROM " + QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );
  if ( sql == allSql )
    sql.clear();
  QgsDebugMsg( "SQL text = " + sql );
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
  QgsDebugMsg( "changeCRSFilter called" );
  //evaluate currently selected typename and set the CRS filter in mProjectionSelector
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( currentIndex.isValid() )
  {
    QString currentTypename = currentIndex.sibling( currentIndex.row(), MODEL_IDX_NAME ).data().toString();
    QgsDebugMsg( QString( "the current typename is: %1" ).arg( currentTypename ) );

    QMap<QString, QStringList >::const_iterator crsIterator = mAvailableCRS.constFind( currentTypename );
    if ( crsIterator != mAvailableCRS.constEnd() )
    {
      QSet<QString> crsNames( crsIterator->toSet() );

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

void QgsWFSSourceSelect::on_cmbConnections_activated( int index )
{
  Q_UNUSED( index );
  QgsWfsConnection::setSelectedConnection( cmbConnections->currentText() );

  QgsWfsConnection connection( cmbConnections->currentText() );

  delete mCapabilities;
  mCapabilities = new QgsWfsCapabilities( connection.uri().uri() );
  connect( mCapabilities, &QgsWfsCapabilities::gotCapabilities, this, &QgsWFSSourceSelect::capabilitiesReplyFinished );
}

void QgsWFSSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::WFS );
  dlg.exec();
}

void QgsWFSSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), QDir::homePath(),
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

void QgsWFSSourceSelect::treeWidgetItemDoubleClicked( const QModelIndex &index )
{
  QgsDebugMsg( "double-click called" );
  buildQuery( index );
}

void QgsWFSSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex &current, const QModelIndex &previous )
{
  Q_UNUSED( previous )
  QgsDebugMsg( "treeWidget_currentRowChanged called" );
  changeCRSFilter();
  mBuildQueryButton->setEnabled( current.isValid() );
  emit enableButtons( current.isValid() );
}

void QgsWFSSourceSelect::buildQueryButtonClicked()
{
  QgsDebugMsg( "mBuildQueryButton click called" );
  buildQuery( treeView->selectionModel()->currentIndex() );
}

void QgsWFSSourceSelect::filterChanged( const QString &text )
{
  QgsDebugMsg( "WFS FeatureType filter changed to :" + text );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
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
