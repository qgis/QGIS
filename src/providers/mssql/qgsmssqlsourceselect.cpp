/***************************************************************************
                             qgsmssqlsourceselect.cpp
       Dialog to select MSSQL layer(s) and add it to the map canvas
                              -------------------
    begin                : 2011-10-08
    copyright            : (C) 2011 by Tamas Szekeres
    email                : szekerest at gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsmssqlsourceselect.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqlnewconnection.h"
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
QWidget *QgsMssqlSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  if ( index.column() == QgsMssqlTableModel::dbtmSql )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setText( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  if ( index.column() == QgsMssqlTableModel::dbtmType && index.data( Qt::UserRole + 1 ).toBool() )
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
      cb->addItem( QgsMssqlTableModel::iconForWkbType( type ), QgsMssqlTableModel::displayStringForWkbType( type ), type );
    }
    cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );
    return cb;
  }

  if ( index.column() == QgsMssqlTableModel::dbtmPkCol )
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

  if ( index.column() == QgsMssqlTableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    le->insert( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  return 0;
}

void QgsMssqlSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsMssqlTableModel::dbtmType )
    {
      QGis::WkbType type = ( QGis::WkbType ) cb->itemData( cb->currentIndex() ).toInt();

      model->setData( index, QgsMssqlTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QGis::WKBUnknown ? QgsMssqlTableModel::displayStringForWkbType( type ) : tr( "Select..." ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsMssqlTableModel::dbtmPkCol )
    {
      model->setData( index, cb->currentText() );
      model->setData( index, cb->currentText(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
    model->setData( index, le->text() );
}

QgsMssqlSourceSelect::QgsMssqlSourceSelect( QWidget *parent, Qt::WFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mColumnTypeThread( NULL )
{
  setupUi( this );

  setWindowTitle( tr( "Add MSSQL Table(s)" ) );

  if ( mEmbeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
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
  mTablesTreeView->setItemDelegate( new QgsMssqlSourceSelectDelegate( this ) );

  QSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( "/qgis/addMSSQLDC", false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( "/Windows/MSSQLSourceSelect/geometry" ).toByteArray() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QString( "/Windows/MSSQLSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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
void QgsMssqlSourceSelect::on_btnNew_clicked()
{
  QgsMssqlNewConnection *nc = new QgsMssqlNewConnection( this );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}
// Slot for deleting an existing connection
void QgsMssqlSourceSelect::on_btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  QgsMssqlSourceSelect::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsMssqlSourceSelect::deleteConnection( QString name )
{
  QString key = "/MSSQL/connections/" + name;
  QSettings settings;
  settings.remove( key + "/service" );
  settings.remove( key + "/host" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/geometryColumns" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/useEstimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key );
}

void QgsMssqlSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::MSSQL );
  dlg.exec();
}

void QgsMssqlSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::MSSQL, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsMssqlSourceSelect::on_btnEdit_clicked()
{
  QgsMssqlNewConnection *nc = new QgsMssqlNewConnection( this, cmbConnections->currentText() );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsMssqlSourceSelect::on_cmbConnections_activated( int )
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/MSSQL/connections/selected", cmbConnections->currentText() );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( settings.value( "/MSSQL/connections/" + cmbConnections->currentText() + "/allowGeometrylessTables", false ).toBool() );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsMssqlSourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  on_btnConnect_clicked();
}

void QgsMssqlSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsMssqlSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsMssqlSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QSettings settings;
  if ( settings.value( "/qgis/addMSSQLDC", false ).toBool() )
  {
    addTables();
  }
  else
  {
    setSql( index );
  }
}

void QgsMssqlSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void QgsMssqlSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::dbtmSql );
  }
}

void QgsMssqlSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsMssqlSourceSelect::setLayerType( QgsMssqlLayerProperty layerProperty )
{
  QgsDebugMsg( "entering." );
  mTableModel.setGeometryTypesForTable( layerProperty );
}

QgsMssqlSourceSelect::~QgsMssqlSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
  }

  QSettings settings;
  settings.setValue( "/Windows/MSSQLSourceSelect/geometry", saveGeometry() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/MSSQLSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsMssqlSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/MSSQL/connections" );
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
void QgsMssqlSourceSelect::addTables()
{
  mSelectedTables.clear();

  foreach ( QModelIndex idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsMssqlTableModel::dbtmTable )
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
    emit addDatabaseLayers( mSelectedTables, "mssql" );
    accept();
  }
}

void QgsMssqlSourceSelect::on_btnConnect_clicked()
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
  QSettings settings;
  QString key = "/MSSQL/connections/" + cmbConnections->currentText();
  QString service = settings.value( key + "/service" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString database = settings.value( key + "/database" ).toString();
  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == "true" )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == "true" )
  {
    password = settings.value( key + "/password" ).toString();
  }

  bool useGeometryColumns = settings.value( key + "/geometryColumns", false ).toBool();

  bool allowGeometrylessTables = cbxAllowGeometrylessTables->isChecked();

  mConnInfo =  "dbname='" + database + "' host=" + host + " user='" + username + "' password='" + password + "'";
  if ( !service.isEmpty() )
    mConnInfo += " service='" + service + "'";

  QSqlDatabase db = QgsMssqlProvider::GetDatabase( service,
                    host, database, username, password );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    // Let user know we couldn't initialise the MSSQL provider
    QMessageBox::warning( this,
                          tr( "MSSQL Provider" ), db.lastError( ).text( ) );
    return;
  }

  QString connectionName;
  if ( service.isEmpty() )
  {
    if ( host.isEmpty() )
    {
      QMessageBox::warning( this,
                            tr( "MSSQL Provider" ), "QgsMssqlProvider host name not specified" );
      return;
    }

    if ( database.isEmpty() )
    {
      QMessageBox::warning( this,
                            tr( "MSSQL Provider" ), "QgsMssqlProvider database name not specified" );
      return;
    }
    connectionName = host + "." + database;
  }
  else
    connectionName = service;


  // Read supported layers from database
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // build sql statement
  QString query( "select " );
  if ( useGeometryColumns )
  {
    query += "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type from geometry_columns";
  }
  else
  {
    query += "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY' from sys.columns join sys.types on sys.columns.system_type_id = sys.types.system_type_id and sys.columns.user_type_id = sys.types.user_type_id join sys.objects on sys.objects.object_id = sys.columns.object_id join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and (sys.objects.type = 'U' or sys.objects.type = 'V')";
  }

  if ( allowGeometrylessTables )
  {
    query += " union all select sys.schemas.name, sys.objects.name, null, null, 'NONE' from sys.objects join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where not exists (select * from sys.columns sc1 join sys.types on sc1.system_type_id = sys.types.system_type_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and sys.objects.object_id = sc1.object_id) and (sys.objects.type = 'U' or sys.objects.type = 'V')";
  }

  // issue the sql query
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );
  q.exec( query );

  if ( q.isActive() )
  {
    while ( q.next() )
    {
      QgsMssqlLayerProperty layer;
      layer.schemaName = q.value( 0 ).toString();
      layer.tableName = q.value( 1 ).toString();
      layer.geometryColName = q.value( 2 ).toString();
      layer.srid = q.value( 3 ).toString();
      layer.type = q.value( 4 ).toString();
      layer.pkCols = QStringList(); //TODO

      QString type = layer.type;
      QString srid = layer.srid;

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == "GEOMETRY" || type.isNull() || srid.isEmpty() )
        {
          addSearchGeometryColumn( connectionName, layer );
          type = "";
          srid = "";
        }
      }

      layer.type = type;
      layer.srid = srid;
      mTableModel.addTableEntry( layer );
    }

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
  else
  {
    QApplication::restoreOverrideCursor();
    // Let user know we couldn't retieve tables from the MSSQL provider
    QMessageBox::warning( this,
                          tr( "MSSQL Provider" ), q.lastError( ).text( ) );
    return;
  }

  if ( !mColumnTypeThread )
  {
    finishList();
  }
}

void QgsMssqlSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  if ( cmbConnections->count() > 0 )
    mAddButton->setEnabled( true );

  mTablesTreeView->sortByColumn( QgsMssqlTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsMssqlTableModel::dbtmSchema, Qt::AscendingOrder );
}

void QgsMssqlSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = 0;
  btnConnect->setText( tr( "Connect" ) );

  finishList();
}

QStringList QgsMssqlSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsMssqlSourceSelect::connectionInfo()
{
  return mConnInfo;
}

void QgsMssqlSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "schema item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsMssqlTableModel::dbtmTable ) )->text();

  QgsVectorLayer *vlayer = new QgsVectorLayer( mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata ), tableName, "mssql" );

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

void QgsMssqlSourceSelect::addSearchGeometryColumn( QString connectionName, QgsMssqlLayerProperty layerProperty )
{
  // store the column details and do the query in a thread
  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsMssqlGeomColumnTypeThread( connectionName, mUseEstimatedMetadata );

    connect( mColumnTypeThread, SIGNAL( setLayerType( QgsMssqlLayerProperty ) ),
             this, SLOT( setLayerType( QgsMssqlLayerProperty ) ) );
    connect( this, SIGNAL( addGeometryColumn( QgsMssqlLayerProperty ) ),
             mColumnTypeThread, SLOT( addGeometryColumn( QgsMssqlLayerProperty ) ) );
    connect( mColumnTypeThread, SIGNAL( finished() ),
             this, SLOT( columnThreadFinished() ) );

  }

  emit addGeometryColumn( layerProperty );
}

QString QgsMssqlSourceSelect::fullDescription( QString schema, QString table, QString column, QString type )
{
  QString full_desc = "";
  if ( !schema.isEmpty() )
    full_desc = schema + ".";
  full_desc += table + " (" + column + ") " + type;
  return full_desc;
}

void QgsMssqlSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QSettings settings;
  QString toSelect = settings.value( "/MSSQL/connections/selected" ).toString();
  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsMssqlSourceSelect::setSearchExpression( const QString& regexp )
{
  Q_UNUSED( regexp );
}


QgsMssqlGeomColumnTypeThread::QgsMssqlGeomColumnTypeThread( QString connectionName, bool useEstimatedMetadata )
    : QThread()
    , mConnectionName( connectionName )
    , mUseEstimatedMetadata( useEstimatedMetadata )
{
  qRegisterMetaType<QgsMssqlLayerProperty>( "QgsMssqlLayerProperty" );
}

void QgsMssqlGeomColumnTypeThread::addGeometryColumn( QgsMssqlLayerProperty layerProperty )
{
  layerProperties << layerProperty;
}

void QgsMssqlGeomColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsMssqlGeomColumnTypeThread::run()
{
  mStopped = false;

  foreach ( QgsMssqlLayerProperty layerProperty, layerProperties )
  {
    if ( !mStopped )
    {
      QString table;
      if ( mUseEstimatedMetadata )
      {
        table = QString( "(SELECT TOP %1 [%2] FROM [%3].[%4] WHERE [%2] IS NOT NULL%5) AS t" )
                .arg( 100 )
                .arg( layerProperty.geometryColName )
                .arg( layerProperty.schemaName )
                .arg( layerProperty.tableName )
                .arg( layerProperty.sql.isEmpty() ? "" : QString( " AND (%1)" ).arg( layerProperty.sql ) );
      }
      else if ( !layerProperty.schemaName.isEmpty() )
      {
        table = QString( "[%1].[%2]%3" )
                .arg( layerProperty.schemaName )
                .arg( layerProperty.tableName )
                .arg( layerProperty.sql.isEmpty() ? "" : QString( " WHERE %1" ).arg( layerProperty.sql ) );
      }
      else
      {
        table = QString( "[%1]%2" )
                .arg( layerProperty.tableName )
                .arg( layerProperty.sql.isEmpty() ? "" : QString( " WHERE %1" ).arg( layerProperty.sql ) );
      }

      QString query = QString( "SELECT DISTINCT CASE WHEN [%1].STGeometryType() IN ('Point', 'MultiPoint') THEN 'POINT' WHEN [%1].STGeometryType() IN ('Linestring', 'MultiLinestring') THEN 'LINESTRING' WHEN [%1].STGeometryType() IN ('Polygon', 'MultiPolygon') THEN 'POLYGON' ELSE UPPER([%1].STGeometryType()) END, [%1].STSrid FROM %2" )
                      .arg( layerProperty.geometryColName )
                      .arg( table );


      // issue the sql query
      QSqlDatabase db = QSqlDatabase::database( mConnectionName );
      QSqlQuery q = QSqlQuery( db );
      q.setForwardOnly( true );
      if ( !q.exec( query ) )
      {
        QString msg = q.lastError().text();
        QgsDebugMsg( msg );
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
