/***************************************************************************
  qgsdb2sourceselect.cpp
      dialog to select DB2 layer(s) and add to the map canvas
  --------------------------------------
  Date      : 2016-01-27
  Copyright : (C) 2016 by David Adler
                          Shirley Xiao, David Nguyen
  Email     : dadler at adtechgeospatial.com
              xshirley2012 at yahoo.com, davidng0123 at gmail.com
  Adapted from MSSQL provider by Tamas Szekeres
****************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 ***************************************************************************/

#include "qgsdb2sourceselect.h"
#include "qgsdb2dataitems.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsdb2provider.h"
#include "qgsdb2newconnection.h"
#include "qgsdb2geometrycolumns.h"

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
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlError>

/** Used to create an editor for when the user tries to change the contents of a cell */
QWidget *QgsDb2SourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  if ( index.column() == QgsDb2TableModel::dbtmSql )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setText( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  if ( index.column() == QgsDb2TableModel::dbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    Q_FOREACH ( QGis::WkbType type,
                QList<QGis::WkbType>()
                << QGis::WKBPoint
                << QGis::WKBLineString
                << QGis::WKBPolygon
                << QGis::WKBMultiPoint
                << QGis::WKBMultiLineString
                << QGis::WKBMultiPolygon
                << QGis::WKBNoGeometry )
    {
      cb->addItem( QgsDb2TableModel::iconForWkbType( type ), QgsDb2TableModel::displayStringForWkbType( type ), type );
    }
    cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );
    return cb;
  }

  if ( index.column() == QgsDb2TableModel::dbtmPkCol )
  {
    QStringList values = index.data( Qt::UserRole + 1 ).toStringList();

    if ( values.size() > 0 )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->addItems( values );
      cb->setCurrentIndex( cb->findText( index.data( Qt::DisplayRole ).toString() ) );
      return cb;
    }
  }

  if ( index.column() == QgsDb2TableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    le->insert( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  return 0;
}

void QgsDb2SourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsDb2TableModel::dbtmType )
    {
      QGis::WkbType type = ( QGis::WkbType ) cb->itemData( cb->currentIndex() ).toInt();

      model->setData( index, QgsDb2TableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QGis::WKBUnknown ? QgsDb2TableModel::displayStringForWkbType( type ) : tr( "Select..." ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsDb2TableModel::dbtmPkCol )
    {
      model->setData( index, cb->currentText() );
      model->setData( index, cb->currentText(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
    model->setData( index, le->text() );
}

QgsDb2SourceSelect::QgsDb2SourceSelect( QWidget *parent, Qt::WindowFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mColumnTypeThread( NULL )
    , mUseEstimatedMetadata( false )
{
  setupUi( this );

  setWindowTitle( tr( "Add Db2 Table(s)" ) );

  if ( mEmbeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
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

  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Schema" ) );
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

  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );
  mTablesTreeView->setItemDelegate( new QgsDb2SourceSelectDelegate( this ) );

  connect( mTablesTreeView->selectionModel(), SIGNAL( selectionChanged( const QItemSelection&, const QItemSelection& ) ), this, SLOT( treeWidgetSelectionChanged( const QItemSelection&, const QItemSelection& ) ) );

  QSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( "/qgis/addDb2DC", false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( "/Windows/Db2SourceSelect/geometry" ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( "/Windows/Db2SourceSelect/HoldDialogOpen", false ).toBool() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QString( "/Windows/Db2SourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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

  cbxAllowGeometrylessTables->setDisabled( true );
}
/** Autoconnected SLOTS **/
// Slot for adding a new connection
void QgsDb2SourceSelect::on_btnNew_clicked()
{
  QgsDb2NewConnection *nc = new QgsDb2NewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsDb2SourceSelect::on_btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  QgsDb2SourceSelect::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsDb2SourceSelect::deleteConnection( QString name )
{
  QString key = "/Db2/connections/" + name;
  QSettings settings;
  settings.remove( key + "/service" );
  settings.remove( key + "/driver" );
  settings.remove( key + "/port" );
  settings.remove( key + "/host" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/environment" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/useEstimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key );
}

void QgsDb2SourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::DB2 );
  dlg.exec();
}

void QgsDb2SourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::DB2, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsDb2SourceSelect::on_btnEdit_clicked()
{
  QgsDb2NewConnection *nc = new QgsDb2NewConnection( this, cmbConnections->currentText() );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsDb2SourceSelect::on_cmbConnections_activated( int )
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/Db2/connections/selected", cmbConnections->currentText() );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( settings.value( "/Db2/connections/" + cmbConnections->currentText() + "/allowGeometrylessTables", false ).toBool() );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsDb2SourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  on_btnConnect_clicked();
}

void QgsDb2SourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsDb2SourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsDb2SourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QSettings settings;
  if ( settings.value( "/qgis/addDb2DC", false ).toBool() )
  {
    addTables();
  }
  else
  {
    setSql( index );
  }
}

void QgsDb2SourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : "" );
}

void QgsDb2SourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void QgsDb2SourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDb2TableModel::dbtmSql );
  }
}

void QgsDb2SourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsDb2SourceSelect::setLayerType( QgsDb2LayerProperty layerProperty )
{
  mTableModel.setGeometryTypesForTable( layerProperty );
}

QgsDb2SourceSelect::~QgsDb2SourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
  }

  QSettings settings;
  settings.setValue( "/Windows/Db2SourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/Db2SourceSelect/HoldDialogOpen", mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/Db2SourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsDb2SourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/Db2/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }

  setConnectionListPosition();

  btnEdit->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );
  btnConnect->setDisabled( cmbConnections->count() == 0 );
  cmbConnections->setDisabled( cmbConnections->count() == 0 );
}

// Slot for performing action when the Add button is clicked
void QgsDb2SourceSelect::addTables()
{
  QgsDebugMsg( QString( "mConnInfo:%1" ).arg( mConnInfo ) );
  mSelectedTables.clear();

  Q_FOREACH ( const QModelIndex& idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsDb2TableModel::dbtmTable )
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
    emit addDatabaseLayers( mSelectedTables, "DB2" );
    if ( !mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }
}

void QgsDb2SourceSelect::on_btnConnect_clicked()
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

  QString errorMsg;
  bool success = QgsDb2ConnectionItem::ConnInfoFromSettings( cmbConnections->currentText(), mConnInfo, errorMsg );
  if ( !success )
  {
    QgsDebugMsg( "settings error: " + errorMsg );
    QMessageBox::warning( this,
                          tr( "DB2 Provider" ), errorMsg );
    return;
  }

  QSqlDatabase db = QgsDb2Provider::getDatabase( mConnInfo, errorMsg );

  if ( !errorMsg.isEmpty() )
  {
    // Let user know we couldn't initialize the DB2 provider
    QMessageBox::warning( this,
                          tr( "DB2 Provider" ), errorMsg );
    return;
  }

  QgsDb2GeometryColumns db2GC = QgsDb2GeometryColumns( db );
  int sqlcode = db2GC.open();
  if ( 0 != sqlcode )
  {
    QMessageBox::warning( this, tr( "DB2GSE.ST_GEOMETRY_COLUMNS Not Found" ),
                          tr( "DB2GSE.ST_GEOMETRY_COLUMNS not found. The DB2 Spatial Extender is not enabled or set up." ) );
    return;
  }

  QApplication::setOverrideCursor( Qt::WaitCursor );

  if ( db2GC.isActive() )
  {
    // Read supported layers from database
    QgsDb2LayerProperty layer;

    while ( db2GC.populateLayerProperty( layer ) )
    {
      QgsDebugMsg( "layer type: " + layer.type );
      mTableModel.addTableEntry( layer );

      if ( mColumnTypeThread )
      {
        btnConnect->setText( tr( "Stop" ) );
        mColumnTypeThread->start();
      }

      //if we have only one schema item, expand it by default
      int numTopLevelItems = mTableModel.invisibleRootItem()->rowCount();
      if ( numTopLevelItems < 2 || mTableModel.tableCount() < 20 )
      {
        //expand all the toplevel items
        for ( int i = 0; i < numTopLevelItems; ++i )
        {
          mTablesTreeView->expand( mProxyModel.mapFromSource(
                                     mTableModel.indexFromItem( mTableModel.invisibleRootItem()->child( i ) ) ) );
        }
      }
    }
  }
  else
  {
    QApplication::restoreOverrideCursor();
    // Let user know we couldn't retrieve tables from the Db2 provider
    QMessageBox::warning( this,
                          tr( "DB2 Provider" ), db.lastError().text() );
    return;
  }

  if ( !mColumnTypeThread )
  {
    finishList();
  }
}

void QgsDb2SourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  mTablesTreeView->sortByColumn( QgsDb2TableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsDb2TableModel::dbtmSchema, Qt::AscendingOrder );
}

void QgsDb2SourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = 0;
  btnConnect->setText( tr( "Connect" ) );

  finishList();
}

QStringList QgsDb2SourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsDb2SourceSelect::connectionInfo()
{
  return mConnInfo;
}

void QgsDb2SourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "schema item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsDb2TableModel::dbtmTable ) )->text();

  QgsVectorLayer *vlayer = new QgsVectorLayer( mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata ), tableName, "DB2" );

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

void QgsDb2SourceSelect::addSearchGeometryColumn( QString connectionName, QgsDb2LayerProperty layerProperty, bool estimateMetadata )
{
  // store the column details and do the query in a thread
  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsDb2GeomColumnTypeThread( connectionName, estimateMetadata );

    connect( mColumnTypeThread, SIGNAL( setLayerType( QgsDb2LayerProperty ) ),
             this, SLOT( setLayerType( QgsDb2LayerProperty ) ) );
    connect( this, SIGNAL( addGeometryColumn( QgsDb2LayerProperty ) ),
             mColumnTypeThread, SLOT( addGeometryColumn( QgsDb2LayerProperty ) ) );
    connect( mColumnTypeThread, SIGNAL( finished() ),
             this, SLOT( columnThreadFinished() ) );

  }

  emit addGeometryColumn( layerProperty );
}

QString QgsDb2SourceSelect::fullDescription( QString schema, QString table, QString column, QString type )
{
  QString full_desc = "";
  if ( !schema.isEmpty() )
    full_desc = schema + ".";
  full_desc += table + " (" + column + ") " + type;
  return full_desc;
}

void QgsDb2SourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QSettings settings;
  QString toSelect = settings.value( "/Db2/connections/selected" ).toString();
  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsDb2SourceSelect::setSearchExpression( const QString& regexp )
{
  Q_UNUSED( regexp );
}

void QgsDb2SourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  mAddButton->setEnabled( !selected.isEmpty() );
}


QgsDb2GeomColumnTypeThread::QgsDb2GeomColumnTypeThread( QString connectionName, bool useEstimatedMetadata )
    : QThread()
    , mConnectionName( connectionName )
    , mUseEstimatedMetadata( useEstimatedMetadata )
    , mStopped( false )
{
  qRegisterMetaType<QgsDb2LayerProperty>( "QgsDb2LayerProperty" );
}

void QgsDb2GeomColumnTypeThread::addGeometryColumn( QgsDb2LayerProperty layerProperty )
{
  layerProperties << layerProperty;
}

void QgsDb2GeomColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsDb2GeomColumnTypeThread::run()
{
  mStopped = false;

  for ( QList<QgsDb2LayerProperty>::iterator it = layerProperties.begin(),
        end = layerProperties.end();
        it != end; ++it )
  {
    QgsDb2LayerProperty &layerProperty = *it;

    if ( !mStopped )
    {
      QString table;
      table = QString( "%1[%2]" )
              .arg( layerProperty.schemaName.isEmpty() ? "" : QString( "[%1]." ).arg( layerProperty.schemaName ),
                    layerProperty.tableName );

      QString query = QString( "SELECT %3"
                               " UPPER([%1].STGeometryType()),"
                               " [%1].STSrid"
                               " FROM %2"
                               " WHERE [%1] IS NOT NULL %4"
                               " GROUP BY [%1].STGeometryType(), [%1].STSrid" )
                      .arg( layerProperty.geometryColName,
                            table,
                            mUseEstimatedMetadata ? "TOP 1" : "",
                            layerProperty.sql.isEmpty() ? "" : QString( " AND %1" ).arg( layerProperty.sql ) );

      // issue the sql query
      QSqlDatabase db = QSqlDatabase::database( mConnectionName );
      if ( !QgsDb2Provider::openDatabase( db ) )
      {
        QgsDebugMsg( db.lastError().text() );
        continue;
      }

      QSqlQuery q = QSqlQuery( db );
      q.setForwardOnly( true );
      if ( !q.exec( query ) )
      {
        QgsDebugMsg( q.lastError().text() );
      }

      QString type;
      QString srid;

      if ( q.isActive() )
      {
        QStringList types;
        QStringList srids;

        while ( q.next() )
        {
          QString type = q.value( 0 ).toString().toUpper();
          QString srid = q.value( 1 ).toString();

          if ( type.isEmpty() )
            continue;

          types << type;
          srids << srid;
        }

        type = types.join( "," );
        srid = srids.join( "," );
      }

      layerProperty.type = type;
      layerProperty.srid = srid;
    }
    else
    {
      layerProperty.type = "";
      layerProperty.srid = "";
    }

    // Now tell the layer list dialog box...
    emit setLayerType( layerProperty );
  }
}
