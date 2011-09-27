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
#include "qgscontexthelp.h"
#include "qgspostgresconnection.h"
#include "qgspostgresprovider.h"
#include "qgspgnewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgscredentials.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

#ifdef HAVE_PGCONFIG
#include <pg_config.h>
#endif

// Note: Because the the geometry type select SQL is also in the qgspostgresprovider
// code this parameter is duplicated there.
static const int sGeomTypeSelectLimit = 100;

QgsPgSourceSelect::QgsPgSourceSelect( QWidget *parent, Qt::WFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mColumnTypeThread( NULL )
{
  setupUi( this );

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
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setDynamicSortFilter( true );
  mProxyModel.setSourceModel( &mTableModel );
  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );

  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );

  mTablesTreeView->setItemDelegate( new QgsPgSourceSelectDelegate( this ) );

  QSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( "/qgis/addPostgisDC", false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( "/Windows/PgSourceSelect/geometry" ).toByteArray() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QString( "/Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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
void QgsPgSourceSelect::on_btnNew_clicked()
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
void QgsPgSourceSelect::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/Postgresql/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  settings.remove( key + "/service" );
  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/database" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/sslmode" );
  settings.remove( key + "/publicOnly" );
  settings.remove( key + "/geometryColumnsOnly" );
  settings.remove( key + "/allowGeometrylessTables" );
  settings.remove( key + "/estimatedMetadata" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/save" );
  settings.remove( key );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsPgSourceSelect::on_btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::PostGIS );
  dlg.exec();
}

void QgsPgSourceSelect::on_btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load connections" ), ".",
                     tr( "XML files (*.xml *XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::PostGIS, fileName );
  dlg.exec();
  populateConnectionList();
}

// Slot for editing a connection
void QgsPgSourceSelect::on_btnEdit_clicked()
{
  QgsPgNewConnection *nc = new QgsPgNewConnection( this, cmbConnections->currentText() );
  if ( nc->exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
  delete nc;
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsPgSourceSelect::on_cmbConnections_activated( int )
{
  // Remember which database was selected.
  QgsPostgresConnection::setSelectedConnection( cmbConnections->currentText() );

  cbxAllowGeometrylessTables->blockSignals( true );
  QSettings settings;
  cbxAllowGeometrylessTables->setChecked( settings.value( "/PostgreSQL/connections/" + cmbConnections->currentText() + "/allowGeometrylessTables", false ).toBool() );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsPgSourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  on_btnConnect_clicked();
}

void QgsPgSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsPgSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsPgSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QSettings settings;
  if ( settings.value( "/qgis/addPostgisDC", false ).toBool() )
  {
    addTables();
  }
  else
  {
    setSql( index );
  }
}

void QgsPgSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void QgsPgSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmPkCol );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsDbTableModel::dbtmSql );
  }
}

void QgsPgSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsPgSourceSelect::setLayerType( QString schema,
                                      QString table, QString column,
                                      QString type )
{
  mTableModel.setGeometryTypesForTable( schema, table, column, type );
  mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmSchema, Qt::AscendingOrder );
}

QgsPgSourceSelect::~QgsPgSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    delete mColumnTypeThread;
    mColumnTypeThread = NULL;
  }

  QSettings settings;
  settings.setValue( "/Windows/PgSourceSelect/geometry", saveGeometry() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsPgSourceSelect::populateConnectionList()
{
  QStringList keys = QgsPostgresConnection::connectionList();
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

QString QgsPgSourceSelect::layerURI( const QModelIndex &index )
{
  QString schemaName = mTableModel.itemFromIndex( index.sibling( index.row(), QgsDbTableModel::dbtmSchema ) )->text();
  QString tableName = mTableModel.itemFromIndex( index.sibling( index.row(), QgsDbTableModel::dbtmTable ) )->text();
  QString geomColumnName = mTableModel.itemFromIndex( index.sibling( index.row(), QgsDbTableModel::dbtmGeomCol ) )->text();
  QString pkColumnName = mTableModel.itemFromIndex( index.sibling( index.row(), QgsDbTableModel::dbtmPkCol ) )->text();
  QString sql = mTableModel.itemFromIndex( index.sibling( index.row(), QgsDbTableModel::dbtmSql ) )->text();

  if ( geomColumnName.contains( " AS " ) )
  {
    int a = geomColumnName.indexOf( " AS " );
    QString typeName = geomColumnName.mid( a + 4 ); //only the type name
    geomColumnName = geomColumnName.left( a ); //only the geom column name
    QString geomFilter;

    if ( typeName == "POINT" )
    {
      geomFilter = QString( "upper(geometrytype(\"%1\")) IN ('POINT','MULTIPOINT')" ).arg( geomColumnName );
    }
    else if ( typeName == "LINESTRING" )
    {
      geomFilter = QString( "upper(geometrytype(\"%1\")) IN ('LINESTRING','MULTILINESTRING')" ).arg( geomColumnName );
    }
    else if ( typeName == "POLYGON" )
    {
      geomFilter = QString( "upper(geometrytype(\"%1\")) IN ('POLYGON','MULTIPOLYGON')" ).arg( geomColumnName );
    }

    if ( !geomFilter.isEmpty() && !sql.contains( geomFilter ) )
    {
      if ( !sql.isEmpty() )
      {
        sql += " AND ";
      }

      sql += geomFilter;
    }
  }

  QgsDataSourceURI uri( m_connInfo );
  uri.setDataSource( schemaName, tableName, geomColumnName, sql, pkColumnName );
  uri.setUseEstimatedMetadata( mUseEstimatedMetadata );

  return uri.uri();
}

// Slot for performing action when the Add button is clicked
void QgsPgSourceSelect::addTables()
{
  m_selectedTables.clear();

  QItemSelection selection = mTablesTreeView->selectionModel()->selection();
  QModelIndexList selectedIndices = selection.indexes();
  QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
  for ( ; selected_it != selectedIndices.constEnd(); ++selected_it )
  {
    if ( !selected_it->parent().isValid() || selected_it->column() > 0 )
    {
      //top level items only contain the schema names
      continue;
    }

    QModelIndex index = mProxyModel.mapToSource( *selected_it );
    m_selectedTables << layerURI( index );
  }

  if ( m_selectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    emit addDatabaseLayers( m_selectedTables, "postgres" );
    accept();
  }
}

void QgsPgSourceSelect::on_btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( true );

  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread = 0;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  // populate the table list
  QgsPostgresConnection connection( cmbConnections->currentText() );
  QgsDataSourceURI uri( connection.connectionInfo() );

  QgsDebugMsg( "Connection info: " + uri.connectionInfo() );

  m_connInfo = uri.connectionInfo();
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QgsPostgresProvider *pgProvider = connection.provider();
  if ( pgProvider )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QSettings settings;
    QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();

    bool searchPublicOnly = settings.value( key + "/publicOnly" ).toBool();
    bool searchGeometryColumnsOnly = settings.value( key + "/geometryColumnsOnly" ).toBool();
    bool allowGeometrylessTables = cbxAllowGeometrylessTables->isChecked();

    QVector<QgsPostgresLayerProperty> layers;
    if ( pgProvider->supportedLayers( layers, searchGeometryColumnsOnly, searchPublicOnly, allowGeometrylessTables ) )
    {
      // Add the supported layers to the table
      foreach ( QgsPostgresLayerProperty layer, layers)
      {
        QString type = layer.type;
        if ( !searchGeometryColumnsOnly && layer.geometryColName != QString::null )
        {
          if ( type == "GEOMETRY" || type == QString::null )
          {
            addSearchGeometryColumn( layer.schemaName, layer.tableName, layer.geometryColName );
            type = tr( "Waiting" );
          }
        }
        QgsDebugMsg( QString( "adding table %1.%2" ).arg(layer.schemaName).arg(layer.tableName) );
        mTableModel.addTableEntry( type, layer.schemaName, layer.tableName, layer.geometryColName, layer.pkCols, layer.sql );
      }

      // Start the thread that gets the geometry type for relations that
      // may take a long time to return
      if ( mColumnTypeThread != NULL )
      {
        connect( mColumnTypeThread, SIGNAL( setLayerType( QString, QString, QString, QString ) ),
                 this, SLOT( setLayerType( QString, QString, QString, QString ) ) );

        // Do it in a thread.
        mColumnTypeThread->start();
      }
    }

    // BEGIN CHANGES ECOS
    if ( cmbConnections->count() > 0 )
      mAddButton->setEnabled( true );
    // END CHANGES ECOS

    mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmTable, Qt::AscendingOrder );
    mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmSchema, Qt::AscendingOrder );

    //if we have only one schema item, expand it by default
    int numTopLevelItems = mTableModel.invisibleRootItem()->rowCount();
    if ( numTopLevelItems < 2 || mTableModel.tableCount() < 20 )
    {
      //expand all the toplevel items
      for ( int i = 0; i < numTopLevelItems; ++i )
      {
        mTablesTreeView->expand( mProxyModel.mapFromSource( mTableModel.indexFromItem( mTableModel.invisibleRootItem()->child( i ) ) ) );
      }
    }

    delete pgProvider;
    QApplication::restoreOverrideCursor();
  }
  else
  {
    // Let user know we couldn't initialise the Postgres/PostGIS provider
    QMessageBox::warning( this,
                          tr( "Postgres/PostGIS Provider" ),
                          tr( "Could not open the Postgres/PostGIS Provider" ) );
  }
}

QStringList QgsPgSourceSelect::selectedTables()
{
  return m_selectedTables;
}

QString QgsPgSourceSelect::connectionInfo()
{
  return m_connInfo;
}

void QgsPgSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "schema item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsDbTableModel::dbtmTable ) )->text();

  QgsVectorLayer *vlayer = new QgsVectorLayer( layerURI( idx ), tableName, "postgres" );

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

void QgsPgSourceSelect::addSearchGeometryColumn( const QString &schema, const QString &table, const QString &column )
{
  // store the column details and do the query in a thread
  if ( mColumnTypeThread == NULL )
  {
    mColumnTypeThread = new QgsGeomColumnTypeThread();
    mColumnTypeThread->setConnInfo( m_connInfo, mUseEstimatedMetadata );
  }
  mColumnTypeThread->addGeometryColumn( schema, table, column );
}

QString QgsPgSourceSelect::fullDescription( QString schema, QString table,
    QString column, QString type )
{
  QString full_desc = "";
  if ( schema.length() > 0 )
    full_desc = '"' + schema + "\".\"";
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void QgsPgSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConnection::selectedConnection();
  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsPgSourceSelect::setSearchExpression( const QString& regexp )
{
  Q_UNUSED( regexp );
}

void QgsGeomColumnTypeThread::setConnInfo( QString conninfo, bool useEstimatedMetadata )
{
  mConnInfo = conninfo;
  mUseEstimatedMetadata = useEstimatedMetadata;
}

void QgsGeomColumnTypeThread::addGeometryColumn( QString schema, QString table, QString column )
{
  schemas.push_back( schema );
  tables.push_back( table );
  columns.push_back( column );
}

void QgsGeomColumnTypeThread::stop()
{
  mStopped = true;
}

void QgsGeomColumnTypeThread::getLayerTypes()
{
  mStopped = false;

  PGconn *pd = PQconnectdb( mConnInfo.toLocal8Bit() );
  // check the connection status
  if ( PQstatus( pd ) != CONNECTION_OK )
  {
    PQfinish( pd );

    QgsDataSourceURI uri( mConnInfo );
    QString username = uri.username();
    QString password = uri.password();

    // use cached credentials
    bool ok = QgsCredentials::instance()->get( mConnInfo, username, password, QString::fromUtf8( PQerrorMessage( pd ) ) );
    if ( !ok )
      return;

    if ( !username.isEmpty() )
      uri.setUsername( username );

    if ( !password.isEmpty() )
      uri.setPassword( password );

    pd = PQconnectdb( uri.connectionInfo().toLocal8Bit() );
    if ( PQstatus( pd ) == CONNECTION_OK )
      QgsCredentials::instance()->put( mConnInfo, username, password );

  }

  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    PQsetClientEncoding( pd, QString( "UNICODE" ).toLocal8Bit() );

    PGresult *res = PQexec( pd, "SET application_name='Quantum GIS'" );
    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK )
    {
      PQclear( res );
      res = PQexec( pd, "ROLLBACK" );
    }
    PQclear( res );

    for ( uint i = 0; i < schemas.size() && !mStopped; i++ )
    {
      QString query = QString( "select distinct "
                               "case"
                               " when upper(geometrytype(%1)) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                               " when upper(geometrytype(%1)) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                               " when upper(geometrytype(%1)) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                               " end "
                               "from " ).arg( "\"" + columns[i] + "\"" );
      if ( mUseEstimatedMetadata )
      {
        query += QString( "(select %1 from %2 where %1 is not null limit %3) as t" )
                 .arg( "\"" + columns[i] + "\"" )
                 .arg( "\"" + schemas[i] + "\".\"" + tables[i] + "\"" )
                 .arg( sGeomTypeSelectLimit );
      }
      else
      {
        query += "\"" + schemas[i] + "\".\"" + tables[i] + "\"";
      }

      QgsDebugMsg( "sql: " + query );

      PGresult *gresult = PQexec( pd, query.toUtf8() );
      QString type;
      if ( PQresultStatus( gresult ) == PGRES_TUPLES_OK )
      {
        QStringList types;

        for ( int j = 0; j < PQntuples( gresult ); j++ )
        {
          QString type = QString::fromUtf8( PQgetvalue( gresult, j, 0 ) );
          if ( type != "" )
            types += type;
        }

        type = types.join( "," );
      }
      PQclear( gresult );

      // Now tell the layer list dialog box...
      emit setLayerType( schemas[i], tables[i], columns[i], type );
    }
  }

  PQfinish( pd );
}
