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
#include "qgsgenericprojectionselector.h"
#include "qgscontexthelp.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatetransform.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgssqlstatement.h"
#include "qgssqlcomposerdialog.h"
#include "qgscrscache.h"

#include <QDomDocument>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QFileDialog>
#include <QPainter>

enum
{
  MODEL_IDX_TITLE,
  MODEL_IDX_NAME,
  MODEL_IDX_ABSTRACT,
  MODEL_IDX_SQL
};

QgsWFSSourceSelect::QgsWFSSourceSelect( QWidget* parent, Qt::WindowFlags fl, bool embeddedMode )
    : QDialog( parent, fl )
    , mCapabilities( nullptr )
    , mSQLComposerDialog( nullptr )
{
  setupUi( this );

  if ( embeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  mBuildQueryButton->setDisabled( true );


  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addLayer() ) );

  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQueryButtonClicked() ) );

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

  mItemDelegate = new QgsWFSItemDelegate( treeView );
  treeView->setItemDelegate( mItemDelegate );

  QSettings settings;
  QgsDebugMsg( "restoring settings" );
  restoreGeometry( settings.value( "/Windows/WFSSourceSelect/geometry" ).toByteArray() );
  cbxUseTitleLayerName->setChecked( settings.value( "/Windows/WFSSourceSelect/UseTitleLayerName", false ).toBool() );
  cbxFeatureCurrentViewExtent->setChecked( settings.value( "/Windows/WFSSourceSelect/FeatureCurrentViewExtent", true ).toBool() );
  mHoldDialogOpen->setChecked( settings.value( "/Windows/WFSSourceSelect/HoldDialogOpen", false ).toBool() );

  mModel = new QStandardItemModel();
  mModel->setHorizontalHeaderItem( MODEL_IDX_TITLE, new QStandardItem( "Title" ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_NAME, new QStandardItem( "Name" ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_ABSTRACT, new QStandardItem( "Abstract" ) );
  mModel->setHorizontalHeaderItem( MODEL_IDX_SQL, new QStandardItem( "Sql" ) );

  mModelProxy = new QSortFilterProxyModel( this );
  mModelProxy->setSourceModel( mModel );
  mModelProxy->setSortCaseSensitivity( Qt::CaseInsensitive );
  treeView->setModel( mModelProxy );

  connect( treeView, SIGNAL( doubleClicked( const QModelIndex& ) ), this, SLOT( treeWidgetItemDoubleClicked( const QModelIndex& ) ) );
  connect( treeView->selectionModel(), SIGNAL( currentRowChanged( QModelIndex, QModelIndex ) ), this, SLOT( treeWidgetCurrentRowChanged( const QModelIndex&, const QModelIndex& ) ) );
}

QgsWFSSourceSelect::~QgsWFSSourceSelect()
{
  QSettings settings;
  QgsDebugMsg( "saving settings" );
  settings.setValue( "/Windows/WFSSourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/WFSSourceSelect/UseTitleLayerName", cbxUseTitleLayerName->isChecked() );
  settings.setValue( "/Windows/WFSSourceSelect/FeatureCurrentViewExtent", cbxFeatureCurrentViewExtent->isChecked() );
  settings.setValue( "/Windows/WFSSourceSelect/HoldDialogOpen", mHoldDialogOpen->isChecked() );

  delete mItemDelegate;
  delete mProjectionSelector;
  delete mCapabilities;
  delete mModel;
  delete mModelProxy;
  delete mAddButton;
  delete mBuildQueryButton;
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
  QString selectedConnection = QgsWFSConnection::selectedConnection();
  int index = cmbConnections->findText( selectedConnection );
  if ( index != -1 )
  {
    cmbConnections->setCurrentIndex( index );
  }

  QgsWFSConnection connection( cmbConnections->currentText() );
  delete mCapabilities;
  mCapabilities = new QgsWFSCapabilities( connection.uri().uri() );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
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

void QgsWFSSourceSelect::capabilitiesReplyFinished()
{
  btnConnect->setEnabled( true );

  if ( !mCapabilities )
    return;
  QgsWFSCapabilities::ErrorCode err = mCapabilities->errorCode();
  if ( err != QgsWFSCapabilities::NoError )
  {
    QString title;
    switch ( err )
    {
      case QgsWFSCapabilities::NetworkError:
        title = tr( "Network Error" );
        break;
      case QgsWFSCapabilities::XmlError:
        title = tr( "Capabilities document is not valid" );
        break;
      case QgsWFSCapabilities::ServerExceptionError:
        title = tr( "Server Exception" );
        break;
      default:
        tr( "Error" );
        break;
    }
    // handle errors
    QMessageBox* box = new QMessageBox( QMessageBox::Critical, title, mCapabilities->errorMessage(), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( "WFSCapabilitiesErrorBox" );
    if ( !property( "hideDialogs" ).toBool() )
      box->open();

    mAddButton->setEnabled( false );
    return;
  }

  mCaps = mCapabilities->capabilities();

  mAvailableCRS.clear();
  Q_FOREACH ( const QgsWFSCapabilities::FeatureType& featureType, mCaps.featureTypes )
  {
    // insert the typenames, titles and abstracts into the tree view
    QStandardItem* titleItem = new QStandardItem( featureType.title );
    QStandardItem* nameItem = new QStandardItem( featureType.name );
    QStandardItem* abstractItem = new QStandardItem( featureType.abstract );
    abstractItem->setToolTip( "<font color=black>" + featureType.abstract  + "</font>" );
    abstractItem->setTextAlignment( Qt::AlignLeft | Qt::AlignTop );
    QStandardItem* filterItem = new QStandardItem();

    typedef QList< QStandardItem* > StandardItemList;
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
    mAddButton->setEnabled( false );
    mBuildQueryButton->setEnabled( false );
  }
}

void QgsWFSSourceSelect::addEntryToServerList()
{
  QgsNewHttpConnection* nc = new QgsNewHttpConnection( this, QgsWFSConstants::CONNECTIONS_WFS );
  nc->setAttribute( Qt::WA_DeleteOnClose );
  nc->setWindowTitle( tr( "Create a new WFS connection" ) );

  // For testability, do not use exec()
  if ( !property( "hideDialogs" ).toBool() )
    nc->open();
  connect( nc, SIGNAL( accepted() ), this, SLOT( populateConnectionList() ) );
  connect( nc, SIGNAL( accepted() ), this, SIGNAL( connectionsChanged() ) );
}

void QgsWFSSourceSelect::modifyEntryOfServerList()
{
  QgsNewHttpConnection* nc = new QgsNewHttpConnection( this, QgsWFSConstants::CONNECTIONS_WFS, cmbConnections->currentText() );
  nc->setAttribute( Qt::WA_DeleteOnClose );
  nc->setWindowTitle( tr( "Modify WFS connection" ) );

  // For testability, do not use exec()
  if ( !property( "hideDialogs" ).toBool() )
    nc->open();
  connect( nc, SIGNAL( accepted() ), this, SLOT( populateConnectionList() ) );
  connect( nc, SIGNAL( accepted() ), this, SIGNAL( connectionsChanged() ) );
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
    mCapabilities->requestCapabilities( false );
  }
}


void QgsWFSSourceSelect::addLayer()
{
  //get selected entry in treeview
  QModelIndex currentIndex = treeView->selectionModel()->currentIndex();
  if ( !currentIndex.isValid() )
  {
    return;
  }

  QgsWFSConnection connection( cmbConnections->currentText() );

  QString pCrsString( labelCoordRefSys->text() );

  //create layers that user selected from this WFS source
  QModelIndexList list = treeView->selectionModel()->selectedRows();
  for ( int i = 0; i < list.size(); i++ )
  { //add a wfs layer to the map
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

    emit addWfsLayer( mUri, layerName );
  }

  if ( !mHoldDialogOpen->isChecked() )
  {
    accept();
  }
}

class QgsWFSValidatorCallback: public QObject, public QgsSQLComposerDialog::SQLValidatorCallback
{
  public:
    QgsWFSValidatorCallback( QObject* parent,
                             const QgsWFSDataSourceURI& uri, const QString& allSql,
                             const QgsWFSCapabilities::Capabilities& caps );
    bool isValid( const QString& sql, QString& errorReason, QString& warningMsg ) override;
  private:
    QgsWFSDataSourceURI mURI;
    QString mAllSql;
    const QgsWFSCapabilities::Capabilities& mCaps;
};

QgsWFSValidatorCallback::QgsWFSValidatorCallback( QObject* parent,
    const QgsWFSDataSourceURI& uri,
    const QString& allSql,
    const QgsWFSCapabilities::Capabilities& caps )
    : QObject( parent )
    , mURI( uri )
    , mAllSql( allSql )
    , mCaps( caps )
{
}

bool QgsWFSValidatorCallback::isValid( const QString& sqlStr, QString& errorReason, QString& warningMsg )
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

class QgsWFSTableSelectedCallback: public QObject, public QgsSQLComposerDialog::TableSelectedCallback
{
  public:
    QgsWFSTableSelectedCallback( QgsSQLComposerDialog* dialog,
                                 const QgsWFSDataSourceURI& uri,
                                 const QgsWFSCapabilities::Capabilities& caps );
    void tableSelected( const QString& name ) override;

  private:
    QgsSQLComposerDialog* mDialog;
    QgsWFSDataSourceURI mURI;
    const QgsWFSCapabilities::Capabilities& mCaps;
};

QgsWFSTableSelectedCallback::QgsWFSTableSelectedCallback( QgsSQLComposerDialog* dialog,
    const QgsWFSDataSourceURI& uri,
    const QgsWFSCapabilities::Capabilities& caps )
    : QObject( dialog )
    , mDialog( dialog )
    , mURI( uri )
    , mCaps( caps )
{
}

void QgsWFSTableSelectedCallback::tableSelected( const QString& name )
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
  Q_FOREACH ( const QgsField& field, p.fields().toList() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !p.geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( p.geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, "geometry" );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", "" );

  mDialog->addColumnNames( fieldList, name );
}

void QgsWFSSourceSelect::buildQuery( const QModelIndex& index )
{
  if ( !index.isValid() )
  {
    return;
  }

  const QString typeName = index.sibling( index.row(), MODEL_IDX_NAME ).data().toString();

  //get available fields for wfs layer
  QgsWFSConnection connection( cmbConnections->currentText() );
  QgsWFSDataSourceURI uri( connection.uri().uri() );
  uri.setTypeName( typeName );
  QgsWFSProvider p( uri.uri(), mCaps );
  if ( !p.isValid() )
  {
    QMessageBox* box = new QMessageBox( QMessageBox::Critical, tr( "Server exception" ), tr( "DescribeFeatureType failed" ), QMessageBox::Ok, this );
    box->setAttribute( Qt::WA_DeleteOnClose );
    box->setModal( true );
    box->setObjectName( "WFSFeatureTypeErrorBox" );
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

  QgsSQLComposerDialog* d = new QgsSQLComposerDialog( this );

  QgsWFSValidatorCallback* validatorCbk = new QgsWFSValidatorCallback( d, uri, allSql, mCaps );
  d->setSQLValidatorCallback( validatorCbk );

  QgsWFSTableSelectedCallback* tableSelectedCbk = new QgsWFSTableSelectedCallback( d, uri, mCaps );
  d->setTableSelectedCallback( tableSelectedCbk );

  const bool bSupportJoins = mCaps.featureTypes.size() > 1 && mCaps.supportsJoins;
  d->setSupportMultipleTables( bSupportJoins, QgsSQLStatement::quotedIdentifierIfNeeded( displayedTypeName ) );

  QMap< QString, QString > mapTypenameToTitle;
  Q_FOREACH ( const QgsWFSCapabilities::FeatureType f, mCaps.featureTypes )
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
  Q_FOREACH ( const QgsWFSCapabilities::Function& f, mCaps.functionList )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    Q_FOREACH ( const QgsWFSCapabilities::Argument& arg, f.argumentList )
    {
      dialogF.argumentList << QgsSQLComposerDialog::Argument( arg.name, arg.type );
    }
    functionList << dialogF;
  }
  d->addFunctions( functionList );

  QList< QgsSQLComposerDialog::Function> spatialPredicateList;
  Q_FOREACH ( const QgsWFSCapabilities::Function& f, mCaps.spatialPredicatesList )
  {
    QgsSQLComposerDialog::Function dialogF;
    dialogF.name = f.name;
    dialogF.returnType = f.returnType;
    dialogF.minArgs = f.minArgs;
    dialogF.maxArgs = f.maxArgs;
    Q_FOREACH ( const QgsWFSCapabilities::Argument& arg, f.argumentList )
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
  Q_FOREACH ( const QgsField& field, p.fields().toList() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( field.name() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, field.typeName() );
  }
  if ( !p.geometryAttribute().isEmpty() )
  {
    QString fieldName( fieldNamePrefix + QgsSQLStatement::quotedIdentifierIfNeeded( p.geometryAttribute() ) );
    fieldList << QgsSQLComposerDialog::PairNameType( fieldName, "geometry" );
  }
  fieldList << QgsSQLComposerDialog::PairNameType( fieldNamePrefix + "*", "" );

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
    connect( d, SIGNAL( accepted() ), this, SLOT( updateSql() ) );
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
    QString crsString = mProjectionSelector->selectedAuthId();
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

    QMap<QString, QStringList >::const_iterator crsIterator = mAvailableCRS.find( currentTypename );
    if ( crsIterator != mAvailableCRS.end() )
    {
      QSet<QString> crsNames( crsIterator->toSet() );

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

void QgsWFSSourceSelect::on_cmbConnections_activated( int index )
{
  Q_UNUSED( index );
  QgsWFSConnection::setSelectedConnection( cmbConnections->currentText() );

  QgsWFSConnection connection( cmbConnections->currentText() );

  delete mCapabilities;
  mCapabilities = new QgsWFSCapabilities( connection.uri().uri() );
  connect( mCapabilities, SIGNAL( gotCapabilities() ), this, SLOT( capabilitiesReplyFinished() ) );
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

void QgsWFSSourceSelect::treeWidgetItemDoubleClicked( const QModelIndex& index )
{
  QgsDebugMsg( "double click called" );
  buildQuery( index );
}

void QgsWFSSourceSelect::treeWidgetCurrentRowChanged( const QModelIndex & current, const QModelIndex & previous )
{
  Q_UNUSED( previous )
  QgsDebugMsg( "treeWidget_currentRowChanged called" );
  changeCRSFilter();
  mBuildQueryButton->setEnabled( current.isValid() );
  mAddButton->setEnabled( current.isValid() );
}

void QgsWFSSourceSelect::buildQueryButtonClicked()
{
  QgsDebugMsg( "mBuildQueryButton click called" );
  buildQuery( treeView->selectionModel()->currentIndex() );
}

void QgsWFSSourceSelect::filterChanged( const QString& text )
{
  QgsDebugMsg( "WFS FeatureType filter changed to :" + text );
  QRegExp::PatternSyntax mySyntax = QRegExp::PatternSyntax( QRegExp::RegExp );
  Qt::CaseSensitivity myCaseSensitivity = Qt::CaseInsensitive;
  QRegExp myRegExp( text, myCaseSensitivity, mySyntax );
  mModelProxy->setFilterRegExp( myRegExp );
  mModelProxy->sort( mModelProxy->sortColumn(), mModelProxy->sortOrder() );
}

QSize QgsWFSItemDelegate::sizeHint( const QStyleOptionViewItem & option, const QModelIndex & index ) const
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
