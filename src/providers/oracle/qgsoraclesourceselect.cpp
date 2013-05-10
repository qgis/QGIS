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
#include "qgsoraclecolumntypethread.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsoracleprovider.h"
#include "qgsoraclenewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"

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
  if ( index.column() == QgsOracleTableModel::dbtmSql )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setText( index.data( Qt::DisplayRole ).toString() );
    return le;
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
    cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );
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
      QString tableName = index.sibling( index.row(), QgsOracleTableModel::dbtmTable ).data( Qt::DisplayRole ).toString();
      QString ownerName = index.sibling( index.row(), QgsOracleTableModel::dbtmOwner ).data( Qt::DisplayRole ).toString();

      values = mConn->pkCandidates( ownerName, tableName );
    }

    if ( values.size() == 0 )
      return 0;

    if ( values.size() > 0 )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->addItems( values );
      cb->setCurrentIndex( cb->findText( index.data( Qt::DisplayRole ).toString() ) );
      return cb;
    }
  }

  if ( index.column() == QgsOracleTableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    le->insert( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  return 0;
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
      model->setData( index, cb->currentText() );
      model->setData( index, cb->currentText(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
    model->setData( index, le->text() );
}

QgsOracleSourceSelect::QgsOracleSourceSelect( QWidget *parent, Qt::WFlags fl, bool managerMode, bool embeddedMode )
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

  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  mBuildQueryButton->setDisabled( true );

  if ( !mManagerMode )
  {
    buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
    connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addTables() ) );

    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQuery() ) );
  }

  populateConnectionList();

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
}

// Slot for performing action when the Add button is clicked
void QgsOracleSourceSelect::addTables()
{
  mSelectedTables.clear();

  foreach ( QModelIndex idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsOracleTableModel::dbtmTable )
      continue;

    QString uri = mTableModel.layerURI( mProxyModel.mapToSource( idx ), mConnInfo, mUseEstimatedMetadata );
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
    accept();
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

  // populate the table list
  QgsDataSourceURI uri = QgsOracleConn::connUri( cmbConnections->currentText() );

  QgsDebugMsg( "Connection info: " + uri.connectionInfo() );

  mConnInfo = uri.connectionInfo();
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QApplication::setOverrideCursor( Qt::BusyCursor );

  mIsConnected = true;
  mTablesTreeDelegate->setConn( QgsOracleConn::connectDb( uri.connectionInfo() ) );

  mColumnTypeThread = new QgsOracleColumnTypeThread( cmbConnections->currentText(), mUseEstimatedMetadata );

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

void QgsOracleSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = 0;
  btnConnect->setText( tr( "Connect" ) );

  finishList();
}

QStringList QgsOracleSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsOracleSourceSelect::connectionInfo()
{
  return mConnInfo;
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

  QString uri = mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata );
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
