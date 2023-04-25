/***************************************************************************
   qgsredshiftsourceselect.cpp
   --------------------------------------
   Date      : 16.02.2021
   Copyright : (C) 2021 Amazon Inc. or its affiliates
   Author    : Marcel Bezdrighin
 ***************************************************************************/

/***************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/
#include "qgsredshiftsourceselect.h"

#include <QFileDialog>
#include <QHeaderView>
#include <QInputDialog>
#include <QMessageBox>
#include <QStringList>
#include <QStyledItemDelegate>
#include <QTextStream>

#include "qgsapplication.h"
#include "qgscolumntypethread.h"
#include "qgsdatasourceuri.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgslogger.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsproject.h"
#include "qgsproxyprogresstask.h"
#include "qgsquerybuilder.h"
#include "qgsredshiftnewconnection.h"
#include "qgsredshiftprovider.h"
#include "qgsredshifttablemodel.h"
#include "qgssettings.h"
#include "qgsvectorlayer.h"

/**
 * Used to create an editor for when the user tries to change the contents of a
 * cell
 */
QWidget *QgsRedshiftSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option,
    const QModelIndex &index ) const
{
  Q_UNUSED( option )

  QString tableName = index.sibling( index.row(), QgsRedshiftTableModel::DbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return nullptr;

  if ( index.column() == QgsRedshiftTableModel::DbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsRedshiftTableModel::DbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    static const QList<Qgis::WkbType> types { Qgis::WkbType::Point,
        Qgis::WkbType::LineString,
        Qgis::WkbType::LineStringZ,
        Qgis::WkbType::LineStringM,
        Qgis::WkbType::LineStringZM,
        Qgis::WkbType::Polygon,
        Qgis::WkbType::PolygonZ,
        Qgis::WkbType::PolygonM,
        Qgis::WkbType::PolygonZM,
        Qgis::WkbType::MultiPoint,
        Qgis::WkbType::MultiPointZ,
        Qgis::WkbType::MultiPointM,
        Qgis::WkbType::MultiPointZM,
        Qgis::WkbType::MultiLineString,
        Qgis::WkbType::MultiLineStringZ,
        Qgis::WkbType::MultiLineStringM,
        Qgis::WkbType::MultiLineStringZM,
        Qgis::WkbType::MultiPolygon,
        Qgis::WkbType::MultiPolygonZ,
        Qgis::WkbType::MultiPolygonM,
        Qgis::WkbType::MultiPolygonZM,
        Qgis::WkbType::NoGeometry };
    for ( Qgis::WkbType type : types )
    {
      cb->addItem( QgsRedshiftTableModel::iconForWkbType( type ), QgsRedshiftConn::displayStringForWkbType( type ),
                   static_cast< quint32>( type ) );
    }
    return cb;
  }

  if ( index.column() == QgsRedshiftTableModel::DbtmPkCol )
  {
    const QStringList values = index.data( Qt::UserRole + 1 ).toStringList();

    if ( !values.isEmpty() )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->setItemDelegate( new QStyledItemDelegate( parent ) );

      QStandardItemModel *model = new QStandardItemModel( values.size(), 1, cb );

      int row = 0;
      for ( const QString &value : values )
      {
        QStandardItem *item = new QStandardItem( value );
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setCheckable( true );
        item->setData( Qt::Unchecked, Qt::CheckStateRole );
        model->setItem( row++, 0, item );
      }

      cb->setModel( model );

      return cb;
    }
  }

  if ( index.column() == QgsRedshiftTableModel::DbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( 0, 999999, parent ) );
    return le;
  }

  return nullptr;
}

void QgsRedshiftSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsRedshiftTableModel::DbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsRedshiftTableModel::DbtmPkCol &&
         !index.data( Qt::UserRole + 2 ).toStringList().isEmpty() )
    {
      QStringList cols = index.data( Qt::UserRole + 2 ).toStringList();

      const auto constCols = cols;
      for ( const QString &col : constCols )
      {
        QStandardItemModel *cbm = qobject_cast<QStandardItemModel *>( cb->model() );
        for ( int idx = 0; idx < cbm->rowCount(); idx++ )
        {
          QStandardItem *item = cbm->item( idx, 0 );
          if ( item->text() != col )
            continue;

          item->setData( Qt::Checked, Qt::CheckStateRole );
          break;
        }
      }
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    bool ok;
    ( void )value.toInt( &ok );
    if ( index.column() == QgsRedshiftTableModel::DbtmSrid && !ok )
      value.clear();

    le->setText( value );
  }
}

void QgsRedshiftSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model,
    const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsRedshiftTableModel::DbtmType )
    {
      Qgis::WkbType type = static_cast<Qgis::WkbType>( cb->currentData().toInt() );

      model->setData( index, QgsRedshiftTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != Qgis::WkbType::Unknown ? QgsRedshiftConn::displayStringForWkbType( type )
                      : tr( "Select…" ) );
      model->setData( index, static_cast< quint32>( type ), Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsRedshiftTableModel::DbtmPkCol )
    {
      QStandardItemModel *cbm = qobject_cast<QStandardItemModel *>( cb->model() );
      QStringList cols;
      for ( int idx = 0; idx < cbm->rowCount(); idx++ )
      {
        QStandardItem *item = cbm->item( idx, 0 );
        if ( item->data( Qt::CheckStateRole ) == Qt::Checked )
          cols << item->text();
      }

      model->setData( index, cols.isEmpty() ? tr( "Select…" ) : cols.join( QStringLiteral( ", " ) ) );
      model->setData( index, cols, Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    QString value( le->text() );

    if ( index.column() == QgsRedshiftTableModel::DbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter…" );
    }

    model->setData( index, value );
  }
}

QgsRedshiftSourceSelect::QgsRedshiftSourceSelect( QWidget *parent, Qt::WindowFlags fl,
    QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDbSourceSelect( parent, fl, theWidgetMode )
{
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this,
           &QgsRedshiftSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsRedshiftSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this,
           &QgsRedshiftSourceSelect::cmbConnections_currentIndexChanged );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsRedshiftSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }
  else
  {
    setWindowTitle( tr( "Add Redshift Table(s)" ) );
  }

  populateConnectionList();

  mTableModel = new QgsRedshiftTableModel( this );
  init( mTableModel, new QgsRedshiftSourceSelectDelegate( this ) );

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this,
           &QgsRedshiftSourceSelect::treeWidgetSelectionChanged );

  mTablesTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  QgsSettings settings;
  mHoldDialogOpen->setChecked(
    settings.value( QStringLiteral( "Windows/RedshiftSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings
                                     .value( QStringLiteral( "Windows/RedshiftSourceSelect/columnWidths/%1" ).arg( i ),
                                         mTablesTreeView->columnWidth( i ) )
                                     .toInt() );
  }
}

//! Autoconnected SLOTS
// Slot for adding a new connection
void QgsRedshiftSourceSelect::btnNew_clicked()
{
  QgsRedshiftNewConnection *nc = new QgsRedshiftNewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsRedshiftSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all "
                    "associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsRedshiftProviderMetadata md = QgsRedshiftProviderMetadata();
  md.deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsRedshiftSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Redshift );
  dlg.exec();
}

void QgsRedshiftSourceSelect::btnLoad_clicked()
{
  QString fileName =
    QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::Redshift,
                                  fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsRedshiftSourceSelect::btnEdit_clicked()
{
  QgsRedshiftNewConnection *nc = new QgsRedshiftNewConnection( this, cmbConnections->currentText() );
  nc->setWindowTitle( tr( "Edit Redshift Connection" ) );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

//! End Autoconnected SLOTS

// Remember which database is selected
void QgsRedshiftSourceSelect::cmbConnections_currentIndexChanged( const QString &text )
{
  // Remember which database was selected.
  QgsRedshiftConn::setSelectedConnection( text );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( QgsRedshiftConn::allowGeometrylessTables( text ) );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsRedshiftSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  btnConnect_clicked();
}

void QgsRedshiftSourceSelect::setLayerType( const QgsRedshiftLayerProperty &layerProperty )
{
  mTableModel->addTableEntry( layerProperty );
}

QgsRedshiftSourceSelect::~QgsRedshiftSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    finishList();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/RedshiftSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    settings.setValue( QStringLiteral( "Windows/RedshiftSourceSelect/columnWidths/%1" ).arg( i ),
                       mTablesTreeView->columnWidth( i ) );
  }
}

void QgsRedshiftSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsRedshiftConn::connectionList() );
  cmbConnections->blockSignals( false );

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

// Slot for performing action when the Add button is clicked
void QgsRedshiftSourceSelect::addButtonClicked()
{
  mSelectedTables.clear();

  QStringList dbTables;

  const auto constIndexes = mTablesTreeView->selectionModel()->selection().indexes();
  for ( const QModelIndex &idx : constIndexes )
  {
    if ( idx.column() != QgsRedshiftTableModel::DbtmTable )
      continue;

    QString uri = mTableModel->layerURI( proxyModel()->mapToSource( idx ), connectionInfo( false ), mUseEstimatedMetadata );
    if ( uri.isNull() )
      continue;

    mSelectedTables << uri;

    dbTables.append( uri );
  }

  if ( mSelectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    if ( !dbTables.isEmpty() )
    {
      emit addDatabaseLayers( dbTables, QStringLiteral( "redshift" ) );
    }

    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
    {
      accept();
    }

    // Clear selection after layers have been added
    mTablesTreeView->selectionModel()->clearSelection();
  }
}

void QgsRedshiftSourceSelect::btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( true );

  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    return;
  }

  QModelIndex rootItemIndex = mTableModel->indexFromItem( mTableModel->invisibleRootItem() );
  mTableModel->removeRows( 0, mTableModel->rowCount( rootItemIndex ), rootItemIndex );
  mTableModel->setConnectionName( cmbConnections->currentText() );

  // populate the table list
  QgsDataSourceUri uri = QgsRedshiftConn::connUri( cmbConnections->currentText() );


  mDataSrcUri = uri;
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QApplication::setOverrideCursor( Qt::BusyCursor );

  mColumnTypeThread = new QgsGeomColumnTypeThread( cmbConnections->currentText(), mUseEstimatedMetadata,
      cbxAllowGeometrylessTables->isChecked() );
  mColumnTypeTask = new QgsProxyProgressTask( tr( "Scanning tables for %1" ).arg( cmbConnections->currentText() ) );
  QgsApplication::taskManager()->addTask( mColumnTypeTask );

  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::setLayerType, this, &QgsRedshiftSourceSelect::setLayerType );
  connect( mColumnTypeThread, &QThread::finished, this, &QgsRedshiftSourceSelect::columnThreadFinished );
  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::progress, mColumnTypeTask,
  [ = ]( int i, int n ) { mColumnTypeTask->setProxyProgress( 100.0 * static_cast<double>( i ) / n ); } );
  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::progressMessage, this,
           &QgsRedshiftSourceSelect::progressMessage );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
}

void QgsRedshiftSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  mTablesTreeView->sortByColumn( QgsRedshiftTableModel::DbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsRedshiftTableModel::DbtmSchema, Qt::AscendingOrder );
}

void QgsRedshiftSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = nullptr;
  btnConnect->setText( tr( "Connect" ) );
  mColumnTypeTask->finalize( true );
  mColumnTypeTask = nullptr;

  finishList();
}

void QgsRedshiftSourceSelect::reset()
{
  mTablesTreeView->clearSelection();
}

QStringList QgsRedshiftSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsRedshiftSourceSelect::connectionInfo( bool expandAuthCfg )
{
  return mDataSrcUri.connectionInfo( expandAuthCfg );
}

QgsDataSourceUri QgsRedshiftSourceSelect::dataSourceUri()
{
  return mDataSrcUri;
}

void QgsRedshiftSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsRedshiftSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( QStringLiteral( "schema item found" ) );
    return;
  }

  QModelIndex idx = proxyModel()->mapToSource( index );
  QString tableName = mTableModel->itemFromIndex( idx.sibling( idx.row(), QgsRedshiftTableModel::DbtmTable ) )->text();

  QString uri = mTableModel->layerURI( idx, connectionInfo( false ), mUseEstimatedMetadata );
  if ( uri.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "no uri" ) );
    return;
  }

  const QgsVectorLayer::LayerOptions options{QgsProject::instance()->transformContext()};
  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, QStringLiteral( "redshift" ), options );
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

QString QgsRedshiftSourceSelect::fullDescription( const QString &schema, const QString &table, const QString &column,
    const QString &type )
{
  QString full_desc;
  if ( !schema.isEmpty() )
    full_desc = QgsRedshiftConn::quotedIdentifier( schema ) + '.';
  full_desc += QgsRedshiftConn::quotedIdentifier( table ) + " (" + column + ") " + type;
  return full_desc;
}

void QgsRedshiftSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsRedshiftConn::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsRedshiftSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp )
}

void QgsRedshiftSourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected,
    const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  Q_UNUSED( selected )
  emit enableButtons( !mTablesTreeView->selectionModel()->selection().isEmpty() );
}

void QgsRedshiftSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-database-layer" ) );
}
