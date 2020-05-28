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
#include "qgsoracleprovider.h"
#include "qgsoraclenewconnection.h"
#include "qgsoracletablecache.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgsoraclecolumntypethread.h"
#include "qgssettings.h"
#include "qgsproxyprogresstask.h"
#include "qgsgui.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

//! Used to create an editor for when the user tries to change the contents of a cell
QWidget *QgsOracleSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )

  QString tableName = index.sibling( index.row(), QgsOracleTableModel::DbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return nullptr;

  if ( index.column() == QgsOracleTableModel::DbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsOracleTableModel::DbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    Q_FOREACH ( QgsWkbTypes::Type type,
                QList<QgsWkbTypes::Type>()
                << QgsWkbTypes::Point
                << QgsWkbTypes::LineString
                << QgsWkbTypes::Polygon
                << QgsWkbTypes::MultiPoint
                << QgsWkbTypes::MultiLineString
                << QgsWkbTypes::MultiPolygon
                << QgsWkbTypes::NoGeometry )
    {
      cb->addItem( QgsOracleTableModel::iconForWkbType( type ), QgsOracleConn::displayStringForWkbType( type ), type );
    }
    return cb;
  }

  if ( index.column() == QgsOracleTableModel::DbtmPkCol )
  {
    bool isView = index.data( Qt::UserRole + 1 ).toBool();
    if ( !isView )
      return nullptr;

    QStringList values = index.data( Qt::UserRole + 2 ).toStringList();
    if ( values.size() == 0 )
    {
      QString ownerName = index.sibling( index.row(), QgsOracleTableModel::DbtmOwner ).data( Qt::DisplayRole ).toString();
      if ( conn() )
        values = conn()->pkCandidates( ownerName, tableName );
    }

    if ( values.size() == 0 )
      return nullptr;

    if ( values.size() > 0 )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->addItems( values );
      return cb;
    }
  }

  if ( index.column() == QgsOracleTableModel::DbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    return le;
  }

  return nullptr;
}

void QgsOracleSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast<QComboBox * >( editor );
  if ( cb )
  {
    if ( index.column() == QgsOracleTableModel::DbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsOracleTableModel::DbtmPkCol && index.data( Qt::UserRole + 2 ).toBool() )
      cb->setCurrentIndex( cb->findText( value ) );
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    bool ok;
    value.toInt( &ok );
    if ( index.column() == QgsOracleTableModel::DbtmSrid && !ok )
      value.clear();

    le->setText( value );
  }
}

void QgsOracleSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsOracleTableModel::DbtmType )
    {
      QgsWkbTypes::Type type = static_cast< QgsWkbTypes::Type >( cb->currentData().toInt() );

      model->setData( index, QgsOracleTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QgsWkbTypes::Unknown ? QgsOracleConn::displayStringForWkbType( type ) : tr( "Select…" ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsOracleTableModel::DbtmPkCol )
    {
      QString value( cb->currentText() );
      model->setData( index, value.isEmpty() ? tr( "Select…" ) : value );
      model->setData( index, !value.isEmpty(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    QString value( le->text() );

    if ( index.column() == QgsOracleTableModel::DbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter…" );
    }

    model->setData( index, value );
  }
}

QgsOracleSourceSelect::QgsOracleSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  QgsGui::instance()->enableAutoGeometryRestore( this );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsOracleSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }
  else
  {
    setWindowTitle( tr( "Add Oracle Table(s)" ) );
  }

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
  mBuildQueryButton->setDisabled( true );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::Manager )
  {
    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsOracleSourceSelect::buildQuery );
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

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsOracleSourceSelect::treeWidgetSelectionChanged );

  mTablesTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  QgsSettings settings;
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "/Windows/OracleSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "/Windows/OracleSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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
//! Autoconnected SLOTS *
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
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QStringLiteral( "." ),
                     tr( "XML files (*.xml *.XML)" ) );
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

//! End Autoconnected SLOTS *

// Remember which database is selected
void QgsOracleSourceSelect::on_cmbConnections_currentIndexChanged( const QString &text )
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

void QgsOracleSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex & )
{
  addButtonClicked();
}

void QgsOracleSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : QString() );
}

void QgsOracleSourceSelect::on_mSearchTableEdit_textChanged( const QString &text )
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

void QgsOracleSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString &text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Owner" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmOwner );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsOracleTableModel::DbtmSql );
  }
}

void QgsOracleSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsOracleSourceSelect::setLayerType( const QgsOracleLayerProperty &layerProperty )
{
  mTableModel.addTableEntry( layerProperty );
}

QgsOracleSourceSelect::~QgsOracleSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    finishList();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Windows/OracleSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QStringLiteral( "Windows/OracleSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
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
void QgsOracleSourceSelect::addButtonClicked()
{
  mSelectedTables.clear();

  const auto constIndexes = mTablesTreeView->selectionModel()->selection().indexes();
  for ( QModelIndex idx : constIndexes )
  {
    if ( idx.column() != QgsOracleTableModel::DbtmTable )
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
    emit addDatabaseLayers( mSelectedTables, QStringLiteral( "oracle" ) );
    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
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

  QgsDataSourceUri uri = QgsOracleConn::connUri( cmbConnections->currentText() );

  mIsConnected = true;
  mTablesTreeDelegate->setConnectionInfo( uri );

  mColumnTypeThread = new QgsOracleColumnTypeThread( cmbConnections->currentText(),
      QgsOracleConn::restrictToSchema( cmbConnections->currentText() ),
      uri.useEstimatedMetadata(),
      cbxAllowGeometrylessTables->isChecked() );
  mColumnTypeTask = new QgsProxyProgressTask( tr( "Scanning tables for %1" ).arg( cmbConnections->currentText() ) );
  QgsApplication::taskManager()->addTask( mColumnTypeTask );

  connect( mColumnTypeThread, &QgsOracleColumnTypeThread::setLayerType,
           this, &QgsOracleSourceSelect::setLayerType );
  connect( mColumnTypeThread, &QThread::finished,
           this, &QgsOracleSourceSelect::columnThreadFinished );
  connect( mColumnTypeThread, &QgsOracleColumnTypeThread::progress,
           mColumnTypeTask, [ = ]( int i, int n )
  {
    mColumnTypeTask->setProxyProgress( 100.0 * static_cast< double >( i ) / n );
  } );
  connect( mColumnTypeThread, &QgsOracleColumnTypeThread::progressMessage,
           this, &QgsAbstractDataSourceWidget::progressMessage );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
}

void QgsOracleSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  mTablesTreeView->sortByColumn( QgsOracleTableModel::DbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsOracleTableModel::DbtmOwner, Qt::AscendingOrder );
}

static QgsOracleTableCache::CacheFlags _currentFlags( const QString &connName, bool useEstimatedMetadata, bool allowGeometrylessTables )
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
  mColumnTypeThread = nullptr;

  mColumnTypeTask->finalize( true );
  mColumnTypeTask = nullptr;

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
    QgsDebugMsg( QStringLiteral( "no owner item found" ) );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsOracleTableModel::DbtmTable ) )->text();

  QString uri = mTableModel.layerURI( idx, mConnInfo );
  if ( uri.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "no uri" ) );
    return;
  }

  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, QStringLiteral( "oracle" ) );
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

QString QgsOracleSourceSelect::fullDescription( const QString &owner, const QString &table, const QString &column, const QString &type )
{
  QString fullDesc;
  if ( !owner.isEmpty() )
    fullDesc = QgsOracleConn::quotedIdentifier( owner ) + '.';
  fullDesc += QgsOracleConn::quotedIdentifier( table ) + QStringLiteral( " (" ) + column + QStringLiteral( ") " ) + type;
  return fullDesc;
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

void QgsOracleSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp )
}

void QgsOracleSourceSelect::loadTableFromCache()
{
  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  QString connName = cmbConnections->currentText();
  QgsDataSourceUri uri = QgsOracleConn::connUri( connName );
  QVector<QgsOracleLayerProperty> layers;
  if ( !QgsOracleTableCache::loadFromCache( connName, _currentFlags( connName, uri.useEstimatedMetadata(), cbxAllowGeometrylessTables->isChecked() ), layers ) )
    return;

  const auto constLayers = layers;
  for ( const QgsOracleLayerProperty &layerProperty : constLayers )
    mTableModel.addTableEntry( layerProperty );

  QApplication::setOverrideCursor( Qt::BusyCursor );


  mIsConnected = true;
  mTablesTreeDelegate->setConnectionInfo( uri );

  finishList();
}

void QgsOracleSourceSelect::treeWidgetSelectionChanged( const QItemSelection &, const QItemSelection & )
{
  emit enableButtons( !mTablesTreeView->selectionModel()->selection().isEmpty() );
}

void QgsOracleSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-database-layer" ) );
}
