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
  Q_UNUSED( option );

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
      cb->addItem( QgsPgTableModel::iconForWkbType( type ), QgsPostgresConn::displayStringForWkbType( type ), type );
    }
    return cb;
  }

  if ( index.column() == QgsPgTableModel::DbtmPkCol )
  {
    QStringList values = index.data( Qt::UserRole + 1 ).toStringList();

    if ( !values.isEmpty() )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->setItemDelegate( new QStyledItemDelegate( parent ) );

      QStandardItemModel *model = new QStandardItemModel( values.size(), 1, cb );

      int row = 0;
      Q_FOREACH ( const QString &value, values )
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

      Q_FOREACH ( const QString &col, cols )
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
    value.toInt( &ok );
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
      QgsWkbTypes::Type type = ( QgsWkbTypes::Type ) cb->currentData().toInt();

      model->setData( index, QgsPgTableModel::iconForWkbType( type ), Qt::DecorationRole );
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

      model->setData( index, cols.isEmpty() ? tr( "Select…" ) : cols.join( QStringLiteral( ", " ) ) );
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
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  connect( btnConnect, &QPushButton::clicked, this, &QgsPgSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsPgSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsPgSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsPgSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsPgSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsPgSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsPgSourceSelect::btnLoad_clicked );
  connect( mSearchGroupBox, &QGroupBox::toggled, this, &QgsPgSourceSelect::mSearchGroupBox_toggled );
  connect( mSearchTableEdit, &QLineEdit::textChanged, this, &QgsPgSourceSelect::mSearchTableEdit_textChanged );
  connect( mSearchColumnComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsPgSourceSelect::mSearchColumnComboBox_currentIndexChanged );
  connect( mSearchModeComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsPgSourceSelect::mSearchModeComboBox_currentIndexChanged );
  connect( cmbConnections, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsPgSourceSelect::cmbConnections_currentIndexChanged );
  connect( mTablesTreeView, &QTreeView::clicked, this, &QgsPgSourceSelect::mTablesTreeView_clicked );
  connect( mTablesTreeView, &QTreeView::doubleClicked, this, &QgsPgSourceSelect::mTablesTreeView_doubleClicked );
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

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
  mBuildQueryButton->setDisabled( true );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::Manager )
  {
    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsPgSourceSelect::buildQuery );
  }

  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Schema" ) );
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Comment" ) );
  mSearchColumnComboBox->addItem( tr( "Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Feature id" ) );
  mSearchColumnComboBox->addItem( tr( "SRID" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setSourceModel( &mTableModel );

  // Do not do dynamic sorting - otherwise whenever user selects geometry type / srid / pk columns,
  // that item suddenly jumps to the end of the list (because the item gets changed) which is very annoying.
  // The list gets sorted in finishList() method when the listing of tables and views has finished.
  mProxyModel.setDynamicSortFilter( false );

  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );
  mTablesTreeView->setItemDelegate( new QgsPgSourceSelectDelegate( this ) );

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsPgSourceSelect::treeWidgetSelectionChanged );

  QgsSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( QStringLiteral( "qgis/addPostgisDC" ), false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( QStringLiteral( "Windows/PgSourceSelect/geometry" ) ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/PgSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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
//! Autoconnected SLOTS *
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

  QgsPostgresConn::deleteConnection( cmbConnections->currentText() );

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
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

//! End Autoconnected SLOTS *

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

void QgsPgSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsPgSourceSelect::mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsPgSourceSelect::mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/addPostgisDC" ), false ).toBool() )
  {
    addButtonClicked();
  }
  else
  {
    setSql( index );
  }
}

void QgsPgSourceSelect::mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : QString() );
}

void QgsPgSourceSelect::mSearchTableEdit_textChanged( const QString &text )
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

void QgsPgSourceSelect::mSearchColumnComboBox_currentIndexChanged( const QString &text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmTable );
  }
  else if ( text == tr( "Comment" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmComment );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmGeomCol );
  }
  else if ( text == tr( "Feature id" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::DbtmSql );
  }
}

void QgsPgSourceSelect::mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text );
  mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsPgSourceSelect::setLayerType( const QgsPostgresLayerProperty &layerProperty )
{
  mTableModel.addTableEntry( layerProperty );
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
  settings.setValue( QStringLiteral( "Windows/PgSourceSelect/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/PgSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
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

  setConnectionListPosition();

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnConnect->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );
}

// Slot for performing action when the Add button is clicked
void QgsPgSourceSelect::addButtonClicked()
{
  mSelectedTables.clear();

  Q_FOREACH ( const QModelIndex &idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsPgTableModel::DbtmTable )
      continue;

    QString uri = mTableModel.layerURI( mProxyModel.mapToSource( idx ), connectionInfo( false ), mUseEstimatedMetadata );
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
    emit addDatabaseLayers( mSelectedTables, QStringLiteral( "postgres" ) );
    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
    {
      accept();
    }
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

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

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
    QgsDebugMsg( "schema item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsPgTableModel::DbtmTable ) )->text();

  QString uri = mTableModel.layerURI( idx, connectionInfo( false ), mUseEstimatedMetadata );
  if ( uri.isNull() )
  {
    QgsDebugMsg( "no uri" );
    return;
  }

  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, QStringLiteral( "postgres" ) );
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
  Q_UNUSED( regexp );
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
