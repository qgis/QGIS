/***************************************************************************
                             qgsoraclesourceselect.cpp
       Dialog to select Oracle layer(s) and add it to the map canvas
                              -------------------
begin                : August 2012
copyright            : (C) 2012 by Juergen E. Fischer
email                : jef at norbit dot de
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsoraclesourceselect.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsoracleprovider.h"
#include "qgsoraclenewconnection.h"
#include "qgsoracletablecache.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgsoraclecolumntypethread.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

/** Used to create an editor for when the user tries to change the contents of a cell */
QWidget *QgsOracleSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );

  QString tableName = index.sibling( index.row(), QgsOracleTableModel::dbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return 0;

  if ( index.column() == QgsOracleTableModel::dbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsOracleTableModel::dbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    foreach ( QGis::WkbType type,
              QList<QGis::WkbType>()
              << QGis::WKBPoint
              << QGis::WKBLineString
              << QGis::WKBPolygon
              << QGis::WKBMultiPoint
              << QGis::WKBMultiLineString
              << QGis::WKBMultiPolygon
              << QGis::WKBNoGeometry )
    {
      cb->addItem( QgsOracleTableModel::iconForWkbType( type ), QgsOracleConn::displayStringForWkbType( type ), type );
    }
    return cb;
  }

  if ( index.column() == QgsOracleTableModel::dbtmPkCol )
  {
    bool isView = index.data( Qt::UserRole + 1 ).toBool();
    if ( !isView )
      return 0;

    QStringList values = index.data( Qt::UserRole + 2 ).toStringList();
    if ( values.size() == 0 )
    {
      QString ownerName = index.sibling( index.row(), QgsOracleTableModel::dbtmOwner ).data( Qt::DisplayRole ).toString();
      if ( conn() )
        values = conn()->pkCandidates( ownerName, tableName );
    }

    if ( values.size() == 0 )
      return 0;

    if ( values.size() > 0 )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->addItems( values );
      return cb;
    }
  }

  if ( index.column() == QgsOracleTableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    return le;
  }

  return 0;
}

void QgsOracleSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast<QComboBox* >( editor );
  if ( cb )
  {
    if ( index.column() == QgsOracleTableModel::dbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsOracleTableModel::dbtmPkCol && index.data( Qt::UserRole + 2 ).toBool() )
      cb->setCurrentIndex( cb->findText( value ) );
  }

  QLineEdit *le = qobject_cast<QLineEdit*>( editor );
  if ( le )
  {
    bool ok;
    value.toInt( &ok );
    if ( index.column() == QgsOracleTableModel::dbtmSrid && !ok )
      value = "";

    le->setText( value );
  }
}

void QgsOracleSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsOracleTableModel::dbtmType )
    {
      QGis::WkbType type = ( QGis::WkbType ) cb->itemData( cb->currentIndex() ).toInt();

      model->setData( index, QgsOracleTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QGis::WKBUnknown ? QgsOracleConn::displayStringForWkbType( type ) : tr( "Select..." ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsOracleTableModel::dbtmPkCol )
    {
      QString value( cb->currentText() );
      model->setData( index, value.isEmpty() ? tr( "Select..." ) : value );
      model->setData( index, !value.isEmpty(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    QString value( le->text() );

    if ( index.column() == QgsOracleTableModel::dbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter..." );
    }

    model->setData( index, value );
  }
}

QgsOracleSourceSelect::QgsOracleSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mColumnTypeThread( 0 )
    , mIsConnected( false )
{
  setupUi( this );

  if ( mEmbeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }
  else
  {
    setWindowTitle( tr( "Add Oracle Table(s)" ) );
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
  mBuildQueryButton->setDisabled( true );

  if ( !mManagerMode )
  {
    buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
    connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addTables() ) );

    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQuery() ) );
  }

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Owner" ) );
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Primary key column" ) );
  mSearchColumnComboBox->addItem( tr( "SRID" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setSourceModel( &mTableModel );

  mTablesTreeDelegate = new QgsOracleSourceSelectDelegate( this );

  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );
  mTablesTreeView->setItemDelegate( mTablesTreeDelegate );

  QSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( "/qgis/addOracleDC", false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( "/Windows/OracleSourceSelect/geometry" ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( "/Windows/OracleSourceSelect/HoldDialogOpen", false ).toBool() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QString( "/Windows/OracleSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
  }

  //hide the search options by default
  //they will be shown when the user ticks
  //the search options group box
  mSearchLabel->setVisible( false );
  mSearchColumnComboBox->setVisible( false );
  mSearchColumnsLabel->setVisible( false );
  mSearchModeComboBox->setVisible( false );
  mSearchModeLabel->setVisible( false );
  mSearchTableEdit->setVisible( false );

  populateConnectionList();
}
/** Autoconnected SLOTS **/
// Slot for adding a new connection
void QgsOracleSourceSelect::on_btnNew_clicked()
{
  QgsOracleNewConnection *nc = new QgsOracleNewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsOracleSourceSelect::on_btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  QgsOracleConn::deleteConnection( cmbConnections->currentText() );

  QgsOracleTableCache::removeFromCache( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsOracleSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Oracle );
  dlg.exec();
}

void QgsOracleSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::Oracle, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsOracleSourceSelect::on_btnEdit_clicked()
{
  QgsOracleNewConnection *nc = new QgsOracleNewConnection( this, cmbConnections->currentText() );
  if ( nc->exec() )
  {
    if ( nc->connName() != nc->originalConnName() )
      QgsOracleTableCache::renameConnectionInCache( nc->originalConnName(), nc->connName() );

    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsOracleSourceSelect::on_cmbConnections_currentIndexChanged( const QString & text )
{
  // Remember which database was selected.
  QgsOracleConn::setSelectedConnection( text );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( QgsOracleConn::allowGeometrylessTables( text ) );
  cbxAllowGeometrylessTables->blockSignals( false );

  // populate the table list
  mConnInfo = QgsOracleConn::connUri( cmbConnections->currentText() );

  QgsDebugMsg( "Connection info: " + mConnInfo.uri() );

  loadTableFromCache();
}

void QgsOracleSourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  if ( mIsConnected )
    on_btnConnect_clicked();
}

void QgsOracleSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsOracleSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsOracleSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QSettings settings;
  if ( settings.value( "/qgis/addOracleDC", false ).toBool() )
  {
    addTables();
  }
  else
  {
    setSql( index );
  }
}

void QgsOracleSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : "" );
}

void QgsOracleSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
{
  if ( mSearchModeComboBox->currentText() == tr( "Wildcard" ) )
  {
    mProxyModel._setFilterWildcard( text );
  }
  else if ( mSearchModeComboBox->currentText() == tr( "RegExp" ) )
  {
    mProxyModel._setFilterRegExp( text );
  }
}

void QgsOracleSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Owner" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmOwner );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::dbtmSql );
  }
}

void QgsOracleSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsOracleSourceSelect::setLayerType( QgsOracleLayerProperty layerProperty )
{
  QgsDebugMsg( "entering." );
  mTableModel.addTableEntry( layerProperty );
}

QgsOracleSourceSelect::~QgsOracleSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    finishList();
  }

  QSettings settings;
  settings.setValue( "/Windows/OracleSourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/OracleSourceSelect/HoldDialogOpen", mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/OracleSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsOracleSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsOracleConn::connectionList() );
  cmbConnections->blockSignals( false );

  setConnectionListPosition();

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnConnect->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  on_cmbConnections_currentIndexChanged( cmbConnections->currentText() );
}

// Slot for performing action when the Add button is clicked
void QgsOracleSourceSelect::addTables()
{
  mSelectedTables.clear();

  foreach ( QModelIndex idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsOracleTableModel::dbtmTable )
      continue;

    QString uri = mTableModel.layerURI( mProxyModel.mapToSource( idx ), mConnInfo );
    if ( uri.isNull() )
      continue;

    mSelectedTables << uri;
  }

  if ( mSelectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    emit addDatabaseLayers( mSelectedTables, "oracle" );
    if ( !mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }
}

void QgsOracleSourceSelect::on_btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( true );

  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    return;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsDataSourceURI uri = QgsOracleConn::connUri( cmbConnections->currentText() );

  mIsConnected = true;
  mTablesTreeDelegate->setConnectionInfo( uri.connectionInfo() );

  mColumnTypeThread = new QgsOracleColumnTypeThread( cmbConnections->currentText(),
      uri.useEstimatedMetadata(),
      cbxAllowGeometrylessTables->isChecked() );

  connect( mColumnTypeThread, SIGNAL( setLayerType( QgsOracleLayerProperty ) ),
           this, SLOT( setLayerType( QgsOracleLayerProperty ) ) );
  connect( mColumnTypeThread, SIGNAL( finished() ),
           this, SLOT( columnThreadFinished() ) );
  connect( mColumnTypeThread, SIGNAL( progress( int, int ) ),
           this, SIGNAL( progress( int, int ) ) );
  connect( mColumnTypeThread, SIGNAL( progressMessage( QString ) ),
           this, SIGNAL( progressMessage( QString ) ) );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
}

void QgsOracleSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  if ( cmbConnections->count() > 0 )
    mAddButton->setEnabled( true );

#if 0
  for ( int i = 0; i < QgsOracleTableModel::dbtmColumns; i++ )
    mTablesTreeView->resizeColumnToContents( i );
#endif

  mTablesTreeView->sortByColumn( QgsOracleTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsOracleTableModel::dbtmOwner, Qt::AscendingOrder );
}

static QgsOracleTableCache::CacheFlags _currentFlags( QString connName, bool useEstimatedMetadata, bool allowGeometrylessTables )
{
  QgsOracleTableCache::CacheFlags flags;
  if ( QgsOracleConn::geometryColumnsOnly( connName ) )
    flags |= QgsOracleTableCache::OnlyLookIntoMetadataTable;
  if ( QgsOracleConn::userTablesOnly( connName ) )
    flags |= QgsOracleTableCache::OnlyLookForUserTables;
  if ( QgsOracleConn::onlyExistingTypes( connName ) )
    flags |= QgsOracleTableCache::OnlyExistingGeometryTypes;
  if ( useEstimatedMetadata )
    flags |= QgsOracleTableCache::UseEstimatedTableMetadata;
  if ( allowGeometrylessTables )
    flags |= QgsOracleTableCache::AllowGeometrylessTables;
  return flags;
}

void QgsOracleSourceSelect::columnThreadFinished()
{
  if ( !mColumnTypeThread->isStopped() )
  {
    QString connName = mColumnTypeThread->connectionName();
    QgsOracleTableCache::CacheFlags flags = _currentFlags( connName, mColumnTypeThread->useEstimatedMetadata(), mColumnTypeThread->allowGeometrylessTables() );
    QgsOracleTableCache::saveToCache( connName, flags, mColumnTypeThread->layerProperties() );
  }

  delete mColumnTypeThread;
  mColumnTypeThread = 0;
  btnConnect->setText( tr( "Connect" ) );

  finishList();
}

QStringList QgsOracleSourceSelect::selectedTables()
{
  return mSelectedTables;
}

void QgsOracleSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "no owner item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsOracleTableModel::dbtmTable ) )->text();

  QString uri = mTableModel.layerURI( idx, mConnInfo );
  if ( uri.isNull() )
  {
    QgsDebugMsg( "no uri" );
    return;
  }

  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, "oracle" );
  if ( !vlayer->isValid() )
  {
    delete vlayer;
    return;
  }

  // create a query builder object
  QgsQueryBuilder *gb = new QgsQueryBuilder( vlayer, this );
  if ( gb->exec() )
  {
    mTableModel.setSql( mProxyModel.mapToSource( index ), gb->sql() );
  }

  delete gb;
  delete vlayer;
}

QString QgsOracleSourceSelect::fullDescription( QString owner, QString table, QString column, QString type )
{
  QString full_desc = "";
  if ( !owner.isEmpty() )
    full_desc = QgsOracleConn::quotedIdentifier( owner ) + ".";
  full_desc += QgsOracleConn::quotedIdentifier( table ) + " (" + column + ") " + type;
  return full_desc;
}

void QgsOracleSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsOracleConn::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsOracleSourceSelect::setSearchExpression( const QString& regexp )
{
  Q_UNUSED( regexp );
}

void QgsOracleSourceSelect::loadTableFromCache()
{
  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  QString connName = cmbConnections->currentText();
  QgsDataSourceURI uri = QgsOracleConn::connUri( connName );
  QVector<QgsOracleLayerProperty> layers;
  if ( !QgsOracleTableCache::loadFromCache( connName, _currentFlags( connName, uri.useEstimatedMetadata(), cbxAllowGeometrylessTables->isChecked() ), layers ) )
    return;

  foreach ( const QgsOracleLayerProperty& layerProperty, layers )
    mTableModel.addTableEntry( layerProperty );

  QApplication::setOverrideCursor( Qt::BusyCursor );


  mIsConnected = true;
  mTablesTreeDelegate->setConnectionInfo( uri.connectionInfo() );

  finishList();
}
