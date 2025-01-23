/***************************************************************************
                          qgsdamengsourceselect.cpp
    Dialog to select Dameng layer( s ) and add it to the map canvas
                             -------------------
    begin                : 2025/01/14
    copyright            : ( C ) 2025 by Haiyang Zhao
    email                : zhaohaiyang@dameng.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   ( at your option ) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsdamengsourceselect.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsdamengprovider.h"
#include "qgsdamengnewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgsdamengcolumntypethread.h"
#include "qgssettings.h"
#include "qgsproxyprogresstask.h"
#include "qgsproject.h"
#include "qgsgui.h"
#include "qgsiconutils.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>
#include <QStyledItemDelegate>


//! Used to create an editor for when the user tries to change the contents of a cell
QWidget *QgsDamengSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )

  QString tableName = index.sibling( index.row(), QgsDamengTableModel::DbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return nullptr;

  if ( index.column() == QgsDamengTableModel::DbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsDamengTableModel::DbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    static const QList<Qgis::WkbType> types { Qgis::WkbType::Point, Qgis::WkbType::LineString, Qgis::WkbType::LineStringZ, Qgis::WkbType::LineStringM, Qgis::WkbType::LineStringZM, Qgis::WkbType::Polygon, Qgis::WkbType::PolygonZ, Qgis::WkbType::PolygonM, Qgis::WkbType::PolygonZM, Qgis::WkbType::MultiPoint, Qgis::WkbType::MultiPointZ, Qgis::WkbType::MultiPointM, Qgis::WkbType::MultiPointZM, Qgis::WkbType::MultiLineString, Qgis::WkbType::MultiLineStringZ, Qgis::WkbType::MultiLineStringM, Qgis::WkbType::MultiLineStringZM, Qgis::WkbType::MultiPolygon, Qgis::WkbType::MultiPolygonZ, Qgis::WkbType::MultiPolygonM, Qgis::WkbType::MultiPolygonZM, Qgis::WkbType::NoGeometry };
    for ( Qgis::WkbType type : types )
    {
      cb->addItem( QgsIconUtils::iconForWkbType( type ), QgsDamengConn::displayStringForWkbType( type ), static_cast<quint32>( type ) );
    }
    return cb;
  }

  if ( index.column() == QgsDamengTableModel::DbtmPkCol )
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

  if ( index.column() == QgsDamengTableModel::DbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    return le;
  }

  return nullptr;
}

void QgsDamengSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast< QComboBox * >( editor );
  if ( cb )
  {
    if ( index.column() == QgsDamengTableModel::DbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsDamengTableModel::DbtmPkCol && !index.data( Qt::UserRole + 2 ).toStringList().isEmpty() )
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
    ( void ) value.toInt( &ok );
    if ( index.column() == QgsDamengTableModel::DbtmSrid && !ok )
      value.clear();

    le->setText( value );
  }
}

void QgsDamengSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsDamengTableModel::DbtmType )
    {
      Qgis::WkbType type = static_cast<Qgis::WkbType>( cb->currentData().toInt() );

      model->setData( index, QgsIconUtils::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != Qgis::WkbType::Unknown ? QgsDamengConn::displayStringForWkbType( type ) : tr( "Select…" ) );
      model->setData( index, static_cast<quint32>( type ), Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsDamengTableModel::DbtmPkCol )
    {
      QStandardItemModel *cbm = qobject_cast<QStandardItemModel *>( cb->model() );
      QStringList cols;
      for ( int idx = 0; idx < cbm->rowCount(); idx++ )
      {
        QStandardItem *item = cbm->item( idx, 0 );
        if ( item->data( Qt::CheckStateRole ) == Qt::Checked )
          cols << item->text();
      }

      model->setData( index, cols.isEmpty() ? tr( "Select…" ) : cols.join( QLatin1String( ", " ) ) );
      model->setData( index, cols, Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    QString value( le->text() );

    if ( index.column() == QgsDamengTableModel::DbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter…" );
    }

    model->setData( index, value );
  }
}

QgsDamengSourceSelect::QgsDamengSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDbSourceSelect( parent, fl, theWidgetMode )
{
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsDamengSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsDamengSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsDamengSourceSelect::cmbConnections_currentIndexChanged );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsDamengSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::Standalone )
  {
    mHoldDialogOpen->hide();
  }
  else
  {
    setWindowTitle( tr( "Add Dameng Table( s )" ) );
  }

  populateConnectionList();

  mTableModel = new QgsDamengTableModel( this );
  init( mTableModel, new QgsDamengSourceSelectDelegate( this ) );

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsDamengSourceSelect::treeWidgetSelectionChanged );

  mTablesTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  QgsSettings settings;
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/DmSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "Windows/DmSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
  }
}
//! Autoconnected SLOTS
// Slot for adding a new connection
void QgsDamengSourceSelect::btnNew_clicked()
{
  QgsDamengNewConnection *nc = new QgsDamengNewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsDamengSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                  .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsDamengProviderMetadata md = QgsDamengProviderMetadata();
  md.deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsDamengSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::Dameng );
  dlg.exec();
}

void QgsDamengSourceSelect::btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(), tr( "XML files (*.xml *.XML )" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::Dameng, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsDamengSourceSelect::btnEdit_clicked()
{
  QgsDamengNewConnection *nc = new QgsDamengNewConnection( this, cmbConnections->currentText() );
  nc->setWindowTitle( tr( "Edit Dameng Connection" ) );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

//! End Autoconnected SLOTS

// Remember which database is selected
void QgsDamengSourceSelect::cmbConnections_currentIndexChanged( const QString &text )
{
  // Remember which database was selected.
  QgsDamengConn::setSelectedConnection( text );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( QgsDamengConn::allowGeometrylessTables( text ) );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsDamengSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  btnConnect_clicked();
}

void QgsDamengSourceSelect::setLayerType( const QgsDamengLayerProperty &layerProperty )
{
  mTableModel->addTableEntry( layerProperty );
}

QgsDamengSourceSelect::~QgsDamengSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    finishList();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/DmSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    settings.setValue( QStringLiteral( "Windows/DmSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsDamengSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsDamengConn::connectionList() );
  cmbConnections->blockSignals( false );

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

// Slot for performing action when the Add button is clicked
void QgsDamengSourceSelect::addButtonClicked()
{
  mSelectedTables.clear();

  QStringList dbTables;

  const auto constIndexes = mTablesTreeView->selectionModel()->selection().indexes();
  for ( const QModelIndex &idx : constIndexes )
  {
    if ( idx.column() != QgsDamengTableModel::DbtmTable )
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
      emit addDatabaseLayers( dbTables, QStringLiteral( "dameng" ) );
    }

    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::Standalone )
    {
      accept();
    }

    // Clear selection after layers have been added
    mTablesTreeView->selectionModel()->clearSelection();
  }
}

void QgsDamengSourceSelect::btnConnect_clicked()
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
  QgsDataSourceUri uri = QgsDamengConn::connUri( cmbConnections->currentText() );

  QgsDebugMsgLevel( "Connection info: " + uri.connectionInfo( false ), 2 );

  mDataSrcUri = uri;
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QApplication::setOverrideCursor( Qt::BusyCursor );

  mColumnTypeThread = new QgsDamengGeomColumnTypeThread( cmbConnections->currentText(), mUseEstimatedMetadata, cbxAllowGeometrylessTables->isChecked() );
  mColumnTypeTask = new QgsProxyProgressTask( tr( "Scanning tables for %1" ).arg( cmbConnections->currentText() ) );
  QgsApplication::taskManager()->addTask( mColumnTypeTask );

  connect( mColumnTypeThread, &QgsDamengGeomColumnTypeThread::setLayerType, this, &QgsDamengSourceSelect::setLayerType );
  connect( mColumnTypeThread, &QThread::finished, this, &QgsDamengSourceSelect::columnThreadFinished );
  connect( mColumnTypeThread, &QgsDamengGeomColumnTypeThread::progress, mColumnTypeTask, [=]( int i, int n ) {
    mColumnTypeTask->setProxyProgress( 100.0 * static_cast<double>( i ) / n );
  } );
  connect( mColumnTypeThread, &QgsDamengGeomColumnTypeThread::progressMessage, this, &QgsDamengSourceSelect::progressMessage );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
}

void QgsDamengSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  mTablesTreeView->sortByColumn( QgsDamengTableModel::DbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsDamengTableModel::DbtmSchema, Qt::AscendingOrder );
}

void QgsDamengSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = nullptr;
  btnConnect->setText( tr( "Connect" ) );
  mColumnTypeTask->finalize( true );
  mColumnTypeTask = nullptr;

  finishList();
}

void QgsDamengSourceSelect::reset()
{
  mTablesTreeView->clearSelection();
}

QStringList QgsDamengSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsDamengSourceSelect::connectionInfo( bool expandAuthCfg )
{
  return mDataSrcUri.connectionInfo( expandAuthCfg );
}

QgsDataSourceUri QgsDamengSourceSelect::dataSourceUri()
{
  return mDataSrcUri;
}

void QgsDamengSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsDamengSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsgLevel( QStringLiteral( "schema item found" ), 2 );
    return;
  }

  QString tableName = mTableModel->itemFromIndex( index.sibling( index.row(), QgsDamengTableModel::DbtmTable ) )->text();

  QString uri = mTableModel->layerURI( index, connectionInfo( false ), mUseEstimatedMetadata );
  if ( uri.isNull() )
  {
    QgsDebugMsgLevel( QStringLiteral( "no uri" ), 2 );
    return;
  }

  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, QStringLiteral( "dameng" ), options );
  if ( !vlayer->isValid() )
  {
    delete vlayer;
    return;
  }

  // create a query builder object
  QgsQueryBuilder *gb = new QgsQueryBuilder( vlayer, this );
  if ( gb->exec() )
  {
    mTableModel->setSql( index, gb->sql() );
  }

  delete gb;
  delete vlayer;
}

QString QgsDamengSourceSelect::fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type )
{
  QString full_desc;
  if ( !schema.isEmpty() )
    full_desc = QgsDamengConn::quotedIdentifier( schema ) + '.';
  full_desc += QgsDamengConn::quotedIdentifier( table ) + " (" + column + ") " + type;
  return full_desc;
}

void QgsDamengSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsDamengConn::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsDamengSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp )
}

void QgsDamengSourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  Q_UNUSED( selected )
  emit enableButtons( !mTablesTreeView->selectionModel()->selection().isEmpty() );
}

void QgsDamengSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-database-layer" ) );
}
