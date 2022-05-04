/***************************************************************************
                             qgspgsourceselect.cpp
       Dialog to select PostgreSQL layer(s) and add it to the map canvas
                              -------------------
begin                : Sat Jun 22 2002
copyright            : (C) 2002 by Gary E.Sherman
email                : sherman at mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspgsourceselect.h"

#include "qgslogger.h"
#include "qgsdbfilterproxymodel.h"
#include "qgsapplication.h"
#include "qgspostgresprovider.h"
#include "qgspgnewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgscolumntypethread.h"
#include "qgssettings.h"
#include "qgsproxyprogresstask.h"
#include "qgsproject.h"
#include "qgsgui.h"
#include "qgsiconutils.h"
#include "qgspgtablemodel.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>
#include <QStyledItemDelegate>


//! Used to create an editor for when the user tries to change the contents of a cell
QWidget *QgsPgSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option )

  QString tableName = index.sibling( index.row(), QgsPgTableModel::DbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return nullptr;

  if ( index.column() == QgsPgTableModel::DbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsPgTableModel::DbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    static const QList<QgsWkbTypes::Type> types { QgsWkbTypes::Point,
        QgsWkbTypes::LineString,
        QgsWkbTypes::LineStringZ,
        QgsWkbTypes::LineStringM,
        QgsWkbTypes::LineStringZM,
        QgsWkbTypes::Polygon,
        QgsWkbTypes::PolygonZ,
        QgsWkbTypes::PolygonM,
        QgsWkbTypes::PolygonZM,
        QgsWkbTypes::MultiPoint,
        QgsWkbTypes::MultiPointZ,
        QgsWkbTypes::MultiPointM,
        QgsWkbTypes::MultiPointZM,
        QgsWkbTypes::MultiLineString,
        QgsWkbTypes::MultiLineStringZ,
        QgsWkbTypes::MultiLineStringM,
        QgsWkbTypes::MultiLineStringZM,
        QgsWkbTypes::MultiPolygon,
        QgsWkbTypes::MultiPolygonZ,
        QgsWkbTypes::MultiPolygonM,
        QgsWkbTypes::MultiPolygonZM,
        QgsWkbTypes::NoGeometry };
    for ( QgsWkbTypes::Type type : types )
    {
      cb->addItem( QgsIconUtils::iconForWkbType( type ), QgsPostgresConn::displayStringForWkbType( type ), type );
    }
    return cb;
  }

  if ( index.column() == QgsPgTableModel::DbtmPkCol )
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

  if ( index.column() == QgsPgTableModel::DbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    return le;
  }

  return nullptr;
}

void QgsPgSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast<QComboBox * >( editor );
  if ( cb )
  {
    if ( index.column() == QgsPgTableModel::DbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsPgTableModel::DbtmPkCol && !index.data( Qt::UserRole + 2 ).toStringList().isEmpty() )
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
    if ( index.column() == QgsPgTableModel::DbtmSrid && !ok )
      value.clear();

    le->setText( value );
  }
}

void QgsPgSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsPgTableModel::DbtmType )
    {
      QgsWkbTypes::Type type = static_cast< QgsWkbTypes::Type >( cb->currentData().toInt() );

      model->setData( index, QgsIconUtils::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QgsWkbTypes::Unknown ? QgsPostgresConn::displayStringForWkbType( type ) : tr( "Select…" ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsPgTableModel::DbtmPkCol )
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

    if ( index.column() == QgsPgTableModel::DbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter…" );
    }

    model->setData( index, value );
  }
}

QgsPgSourceSelect::QgsPgSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDbSourceSelect( parent, fl, theWidgetMode )
{
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsPgSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsPgSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsPgSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsPgSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsPgSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsPgSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsPgSourceSelect::btnLoad_clicked );
  connect( cmbConnections, &QComboBox::currentTextChanged, this, &QgsPgSourceSelect::cmbConnections_currentIndexChanged );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsPgSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }
  else
  {
    setWindowTitle( tr( "Add PostGIS Table(s)" ) );
  }

  populateConnectionList();


  mTableModel = new QgsPgTableModel( this );
  init( mTableModel, new QgsPgSourceSelectDelegate( this ) );

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsPgSourceSelect::treeWidgetSelectionChanged );

  mTablesTreeView->setSelectionMode( QAbstractItemView::ExtendedSelection );

  QgsSettings settings;
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/PgSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
  }
}

//! Autoconnected SLOTS
// Slot for adding a new connection
void QgsPgSourceSelect::btnNew_clicked()
{
  QgsPgNewConnection *nc = new QgsPgNewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsPgSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsPostgresProviderMetadata md = QgsPostgresProviderMetadata();
  md.deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsPgSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::PostGIS );
  dlg.exec();
}

void QgsPgSourceSelect::btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::PostGIS, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsPgSourceSelect::btnEdit_clicked()
{
  QgsPgNewConnection *nc = new QgsPgNewConnection( this, cmbConnections->currentText() );
  nc->setWindowTitle( tr( "Edit PostGIS Connection" ) );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

//! End Autoconnected SLOTS

// Remember which database is selected
void QgsPgSourceSelect::cmbConnections_currentIndexChanged( const QString &text )
{
  // Remember which database was selected.
  QgsPostgresConn::setSelectedConnection( text );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( QgsPostgresConn::allowGeometrylessTables( text ) );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsPgSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  btnConnect_clicked();
}

void QgsPgSourceSelect::setLayerType( const QgsPostgresLayerProperty &layerProperty )
{
  mTableModel->addTableEntry( layerProperty );
}

QgsPgSourceSelect::~QgsPgSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    finishList();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/PgSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel->columnCount(); i++ )
  {
    settings.setValue( QStringLiteral( "Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsPgSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsPostgresConn::connectionList() );
  cmbConnections->blockSignals( false );

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnSave->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );

  setConnectionListPosition();
}

// Slot for performing action when the Add button is clicked
void QgsPgSourceSelect::addButtonClicked()
{
  mSelectedTables.clear();

  QStringList dbTables;
  QList<QPair<QString, QString>> rasterTables;

  const auto constIndexes = mTablesTreeView->selectionModel()->selection().indexes();
  for ( const QModelIndex &idx : constIndexes )
  {
    if ( idx.column() != QgsPgTableModel::DbtmTable )
      continue;

    QString uri = mTableModel->layerURI( proxyModel()->mapToSource( idx ), connectionInfo( false ), mUseEstimatedMetadata );
    if ( uri.isNull() )
      continue;

    mSelectedTables << uri;
    if ( uri.startsWith( QLatin1String( "PG: " ) ) )
    {
      rasterTables.append( QPair<QString, QString>( idx.data().toString(), uri ) );
    }
    else
    {
      dbTables.append( uri );
    }
  }

  if ( mSelectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    if ( ! dbTables.isEmpty() )
    {
      emit addDatabaseLayers( dbTables, QStringLiteral( "postgres" ) );
    }
    if ( ! rasterTables.isEmpty() )
    {
      for ( const auto &u : std::as_const( rasterTables ) )
      {
        // Use "gdal" to proxy rasters to GDAL provider, or "postgresraster" for native PostGIS raster provider
        emit addRasterLayer( u.second, u.first, QLatin1String( "postgresraster" ) );
      }
    }

    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
    {
      accept();
    }

    // Clear selection after layers have been added
    mTablesTreeView->selectionModel()->clearSelection();

  }
}

void QgsPgSourceSelect::btnConnect_clicked()
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
  QgsDataSourceUri uri = QgsPostgresConn::connUri( cmbConnections->currentText() );

  QgsDebugMsg( "Connection info: " + uri.connectionInfo( false ) );

  mDataSrcUri = uri;
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QApplication::setOverrideCursor( Qt::BusyCursor );

  mColumnTypeThread = new QgsGeomColumnTypeThread( cmbConnections->currentText(), mUseEstimatedMetadata, cbxAllowGeometrylessTables->isChecked() );
  mColumnTypeTask = new QgsProxyProgressTask( tr( "Scanning tables for %1" ).arg( cmbConnections->currentText() ) );
  QgsApplication::taskManager()->addTask( mColumnTypeTask );

  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::setLayerType,
           this, &QgsPgSourceSelect::setLayerType );
  connect( mColumnTypeThread, &QThread::finished,
           this, &QgsPgSourceSelect::columnThreadFinished );
  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::progress,
           mColumnTypeTask, [ = ]( int i, int n )
  {
    mColumnTypeTask->setProxyProgress( 100.0 * static_cast< double >( i ) / n );
  } );
  connect( mColumnTypeThread, &QgsGeomColumnTypeThread::progressMessage,
           this, &QgsPgSourceSelect::progressMessage );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
}

void QgsPgSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  mTablesTreeView->sortByColumn( QgsPgTableModel::DbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsPgTableModel::DbtmSchema, Qt::AscendingOrder );
}

void QgsPgSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = nullptr;
  btnConnect->setText( tr( "Connect" ) );
  mColumnTypeTask->finalize( true );
  mColumnTypeTask = nullptr;

  finishList();
}

void QgsPgSourceSelect::reset()
{
  mTablesTreeView->clearSelection();
}

QStringList QgsPgSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsPgSourceSelect::connectionInfo( bool expandAuthCfg )
{
  return mDataSrcUri.connectionInfo( expandAuthCfg );
}

QgsDataSourceUri QgsPgSourceSelect::dataSourceUri()
{
  return mDataSrcUri;
}

void QgsPgSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsPgSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( QStringLiteral( "schema item found" ) );
    return;
  }

  QString tableName = mTableModel->itemFromIndex( index.sibling( index.row(), QgsPgTableModel::DbtmTable ) )->text();

  QString uri = mTableModel->layerURI( index, connectionInfo( false ), mUseEstimatedMetadata );
  if ( uri.isNull() )
  {
    QgsDebugMsg( QStringLiteral( "no uri" ) );
    return;
  }

  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, QStringLiteral( "postgres" ), options );
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

QString QgsPgSourceSelect::fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type )
{
  QString full_desc;
  if ( !schema.isEmpty() )
    full_desc = QgsPostgresConn::quotedIdentifier( schema ) + '.';
  full_desc += QgsPostgresConn::quotedIdentifier( table ) + " (" + column + ") " + type;
  return full_desc;
}

void QgsPgSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConn::selectedConnection();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsPgSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp )
}

void QgsPgSourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  Q_UNUSED( selected )
  emit enableButtons( !mTablesTreeView->selectionModel()->selection().isEmpty() );
}

void QgsPgSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-database-layer" ) );
}
