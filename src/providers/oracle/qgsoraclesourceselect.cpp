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
#include "qgsoraclecolumntypetask.h"
#include "qgssettings.h"
#include "qgsproxyprogresstask.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgsoracletablemodel.h"
#include "qgsdbfilterproxymodel.h"


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
    for ( QgsWkbTypes::Type type :
          {
            QgsWkbTypes::Point,
            QgsWkbTypes::LineString,
            QgsWkbTypes::Polygon,
            QgsWkbTypes::MultiPoint,
            QgsWkbTypes::MultiLineString,
            QgsWkbTypes::MultiPolygon,
            QgsWkbTypes::NoGeometry
          } )
    {
      cb->addItem( QgsIconUtils::iconForWkbType( type ), QgsWkbTypes::translatedDisplayString( type ), type );
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
      if ( auto *lConn = conn() )
        values = lConn->pkCandidates( ownerName, tableName );
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
    ( void )value.toInt( &ok );
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

      model->setData( index, QgsIconUtils::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QgsWkbTypes::Unknown ? QgsWkbTypes::translatedDisplayString( type ) : tr( "Select…" ) );
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
  : QgsAbstractDbSourceSelect( parent, fl, theWidgetMode )
{
  QgsGui::instance()->enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsOracleSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsOracleSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsOracleSourceSelect::cmbConnections_currentIndexChanged );

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

  mTablesTreeDelegate = new QgsOracleSourceSelectDelegate( this );


  mTableModel = new QgsOracleTableModel( this );
  init( mTableModel, mTablesTreeDelegate );

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsOracleSourceSelect::treeWidgetSelectionChanged );

  mTablesTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  QgsSettings settings;
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "/Windows/OracleSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "/Windows/OracleSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
  }

  populateConnectionList();
}
//! Autoconnected SLOTS
// Slot for adding a new connection
void QgsOracleSourceSelect::btnNew_clicked()
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
void QgsOracleSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  QgsProviderMetadata *providerMetadata = QgsProviderRegistry::instance()->providerMetadata( QStringLiteral( "oracle" ) );
  providerMetadata->deleteConnection( cmbConnections->currentText() );

  QgsOracleTableCache::removeFromCache( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsOracleSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Oracle );
  dlg.exec();
}

void QgsOracleSourceSelect::btnLoad_clicked()
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
void QgsOracleSourceSelect::btnEdit_clicked()
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

//! End Autoconnected SLOTS

// Remember which database is selected
void QgsOracleSourceSelect::cmbConnections_currentIndexChanged( const QString &text )
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

void QgsOracleSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  if ( mIsConnected )
    btnConnect_clicked();
}

void QgsOracleSourceSelect::setLayerType( const QgsOracleLayerProperty &layerProperty )
{
  mTableModel->addTableEntry( layerProperty );
}

QgsOracleSourceSelect::~QgsOracleSourceSelect()
{
  if ( mColumnTypeTask )
  {
    mColumnTypeTask->cancel();
    finishList();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "/Windows/OracleSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
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

  cmbConnections_currentIndexChanged( cmbConnections->currentText() );
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

    QString uri = mTableModel->layerURI( proxyModel()->mapToSource( idx ), mConnInfo );
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

void QgsOracleSourceSelect::btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( true );

  if ( mColumnTypeTask )
  {
    mColumnTypeTask->cancel();
    return;
  }

  QModelIndex rootItemIndex = mTableModel->indexFromItem( mTableModel->invisibleRootItem() );
  mTableModel->removeRows( 0, mTableModel->rowCount( rootItemIndex ), rootItemIndex );

  QApplication::setOverrideCursor( Qt::BusyCursor );

  QgsDataSourceUri uri = QgsOracleConn::connUri( cmbConnections->currentText() );

  mIsConnected = true;
  mTablesTreeDelegate->setConnectionInfo( uri );

  mColumnTypeTask = new QgsOracleColumnTypeTask( cmbConnections->currentText(),
      QgsOracleConn::restrictToSchema( cmbConnections->currentText() ),
      uri.useEstimatedMetadata(),
      cbxAllowGeometrylessTables->isChecked() );

  connect( mColumnTypeTask, &QgsOracleColumnTypeTask::setLayerType,
           this, &QgsOracleSourceSelect::setLayerType );
  connect( mColumnTypeTask, &QgsTask::taskCompleted,
           this, &QgsOracleSourceSelect::columnTaskFinished );
  connect( mColumnTypeTask, &QgsTask::taskTerminated,
           this, &QgsOracleSourceSelect::columnTaskFinished );
  connect( mColumnTypeTask, &QgsOracleColumnTypeTask::progressMessage,
           this, &QgsAbstractDataSourceWidget::progressMessage );

  btnConnect->setText( tr( "Stop" ) );

  QgsApplication::taskManager()->addTask( mColumnTypeTask );
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

void QgsOracleSourceSelect::columnTaskFinished()
{
  if ( mColumnTypeTask->status() == QgsTask::Complete )
  {
    QString connName = mColumnTypeTask->connectionName();
    QgsOracleTableCache::CacheFlags flags = _currentFlags( connName, mColumnTypeTask->useEstimatedMetadata(), mColumnTypeTask->allowGeometrylessTables() );
    QgsOracleTableCache::saveToCache( connName, flags, mColumnTypeTask->layerProperties() );
  }

  // don't delete the task, taskManager takes ownership of it
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

  QModelIndex idx = proxyModel()->mapToSource( index );
  QString tableName = mTableModel->itemFromIndex( idx.sibling( idx.row(), QgsOracleTableModel::DbtmTable ) )->text();

  QString uri = mTableModel->layerURI( idx, mConnInfo );
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
    mTableModel->setSql( proxyModel()->mapToSource( index ), gb->sql() );
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
  QModelIndex rootItemIndex = mTableModel->indexFromItem( mTableModel->invisibleRootItem() );
  mTableModel->removeRows( 0, mTableModel->rowCount( rootItemIndex ), rootItemIndex );

  QString connName = cmbConnections->currentText();
  QgsDataSourceUri uri = QgsOracleConn::connUri( connName );
  QVector<QgsOracleLayerProperty> layers;
  if ( !QgsOracleTableCache::loadFromCache( connName, _currentFlags( connName, uri.useEstimatedMetadata(), cbxAllowGeometrylessTables->isChecked() ), layers ) )
    return;

  const auto constLayers = layers;
  for ( const QgsOracleLayerProperty &layerProperty : constLayers )
    mTableModel->addTableEntry( layerProperty );

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
