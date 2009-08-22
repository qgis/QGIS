/***************************************************************************
                             qgsdbsourceselect.cpp
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
/* $Id$ */

#include "qgsdbsourceselect.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsnewconnection.h"
#include "qgspgquerybuilder.h"
#include "qgsdatasourceuri.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

#ifdef HAVE_PGCONFIG
#include <pg_config.h>
#endif

QgsDbSourceSelect::QgsDbSourceSelect( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mColumnTypeThread( NULL ), pd( 0 )
{
  setupUi( this );
  btnAdd->setEnabled( false );
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

  mTablesTreeView->setItemDelegateForColumn( QgsDbTableModel::dbtmPkCol, new QgsDbSourceSelectDelegate( this ) );

  QSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( "/qgis/addPostgisDC", false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );

  mSearchGroupBox->hide();

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );
}
/** Autoconnected SLOTS **/
// Slot for adding a new connection
void QgsDbSourceSelect::on_btnNew_clicked()
{
  addNewConnection();
}
// Slot for deleting an existing connection
void QgsDbSourceSelect::on_btnDelete_clicked()
{
  deleteConnection();
}
// Slot for performing action when the Add button is clicked
void QgsDbSourceSelect::on_btnAdd_clicked()
{
  addTables();
}

// Slot for editing a connection
void QgsDbSourceSelect::on_btnEdit_clicked()
{
  editConnection();
}

// Slot for showing help
void QgsDbSourceSelect::on_btnHelp_clicked()
{
  showHelp();
}
/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsDbSourceSelect::on_cmbConnections_activated( int )
{
  dbChanged();
}

void QgsDbSourceSelect::on_btnBuildQuery_clicked()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsDbSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  btnBuildQuery->setEnabled( index.parent().isValid() );
}

void QgsDbSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
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

void QgsDbSourceSelect::on_mSearchOptionsButton_clicked()
{
  if ( mSearchGroupBox->isVisible() )
  {
    mSearchGroupBox->hide();
  }
  else
  {
    mSearchGroupBox->show();
  }
}

void QgsDbSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void QgsDbSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
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

void QgsDbSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsDbSourceSelect::setLayerType( QString schema,
                                      QString table, QString column,
                                      QString type )
{
  mTableModel.setGeometryTypesForTable( schema, table, column, type );
  mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsDbTableModel::dbtmSchema, Qt::AscendingOrder );
}

QgsDbSourceSelect::~QgsDbSourceSelect()
{
  PQfinish( pd );
}

void QgsDbSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/PostgreSQL/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    cmbConnections->addItem( *it );
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();
}

void QgsDbSourceSelect::addNewConnection()
{
  QgsNewConnection *nc = new QgsNewConnection( this );

  if ( nc->exec() )
  {
    populateConnectionList();
  }
}

void QgsDbSourceSelect::editConnection()
{
  QgsNewConnection *nc = new QgsNewConnection( this, cmbConnections->currentText() );

  if ( nc->exec() )
  {
    nc->saveConnection();
  }
  populateConnectionList();
}

void QgsDbSourceSelect::deleteConnection()
{
  QSettings settings;
  QString key = "/Postgresql/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  QMessageBox::StandardButton result = QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key + "/host" );
    settings.remove( key + "/database" );
    settings.remove( key + "/username" );
    settings.remove( key + "/password" );
    settings.remove( key + "/port" );
    settings.remove( key + "/sslmode" );
    settings.remove( key + "/save" );
    settings.remove( key );
    //if(!success){
    //  QMessageBox::information(this,"Unable to Remove","Unable to remove the connection " + cmbConnections->currentText());
    //}
    cmbConnections->removeItem( cmbConnections->currentIndex() );  // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsDbSourceSelect::addTables()
{
  m_selectedTables.clear();

  typedef QMap<int, QVector<QString> > schemaInfo;
  QMap<QString, schemaInfo> dbInfo;

  QItemSelection selection = mTablesTreeView->selectionModel()->selection();
  QModelIndexList selectedIndices = selection.indexes();
  QStandardItem* currentItem = 0;

  QModelIndexList::const_iterator selected_it = selectedIndices.constBegin();
  for ( ; selected_it != selectedIndices.constEnd(); ++selected_it )
  {
    if ( !selected_it->parent().isValid() )
    {
      //top level items only contain the schema names
      continue;
    }
    currentItem = mTableModel.itemFromIndex( mProxyModel.mapToSource( *selected_it ) );
    if ( !currentItem )
    {
      continue;
    }

    QString currentSchemaName = currentItem->parent()->text();

    int currentRow = currentItem->row();
    int currentColumn = currentItem->column();

    if ( dbInfo[currentSchemaName][currentRow].size() == 0 )
    {
      dbInfo[currentSchemaName][currentRow].resize( QgsDbTableModel::dbtmColumns );
    }

    dbInfo[currentSchemaName][currentRow][currentColumn] = currentItem->text();
  }

  //now traverse all the schemas and table infos
  QMap<QString, schemaInfo>::const_iterator schema_it = dbInfo.constBegin();
  for ( ; schema_it != dbInfo.constEnd(); ++schema_it )
  {
    schemaInfo scheme = schema_it.value();
    schemaInfo::const_iterator entry_it = scheme.constBegin();
    for ( ; entry_it != scheme.constEnd(); ++entry_it )
    {
      QString schemaName = entry_it->at( QgsDbTableModel::dbtmSchema );
      QString tableName = entry_it->at( QgsDbTableModel::dbtmTable );
      QString geomColumnName = entry_it->at( QgsDbTableModel::dbtmGeomCol );
      QString pkColumnName = entry_it->at( QgsDbTableModel::dbtmPkCol );
      QString sql = entry_it->at( QgsDbTableModel::dbtmSql );

      if ( geomColumnName.contains( " AS " ) )
      {
        int a = geomColumnName.indexOf( " AS " );
        QString typeName = geomColumnName.mid( a + 4 ); //only the type name
        geomColumnName = geomColumnName.left( a ); //only the geom column name

        if ( !sql.isEmpty() )
        {
          sql += " AND ";
        }
        if ( typeName == "POINT" )
        {
          sql += QString( "GeometryType(\"%1\") IN ('POINT','MULTIPOINT')" ).arg( geomColumnName );
        }
        else if ( typeName == "LINESTRING" )
        {
          sql += QString( "GeometryType(\"%1\") IN ('LINESTRING','MULTILINESTRING')" ).arg( geomColumnName );
        }
        else if ( typeName == "POLYGON" )
        {
          sql += QString( "GeometryType(\"%1\") IN ('POLYGON','MULTIPOLYGON')" ).arg( geomColumnName );
        }
        else
        {
          continue;
        }
      }

      QString query;
      if ( !pkColumnName.isEmpty() )
      {
        query += QString( "key=\"%1\" " ).arg( pkColumnName );
      }

      query += QString( "table=\"%1\".\"%2\" (%3) sql=%4" )
               .arg( schemaName ).arg( tableName )
               .arg( geomColumnName )
               .arg( sql );

      m_selectedTables.push_back( query );
    }
  }

  if ( m_selectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    accept();
  }
}

void QgsDbSourceSelect::on_btnConnect_clicked()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread = 0;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  // populate the table list
  QSettings settings;

  QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();

  QString database = settings.value( key + "/database" ).toString();
  QString username = settings.value( key + "/username" ).toString();
  QString password = settings.value( key + "/password" ).toString();


  QgsDataSourceURI uri;
  uri.setConnection( settings.value( key + "/host" ).toString(),
                     settings.value( key + "/port" ).toString(),
                     database,
                     settings.value( key + "/username" ).toString(),
                     password,
                     ( QgsDataSourceURI::SSLmode ) settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() );



  bool searchPublicOnly = settings.value( key + "/publicOnly" ).toBool();
  bool searchGeometryColumnsOnly = settings.value( key + "/geometryColumnsOnly" ).toBool();

  // Need to escape the password to allow for single quotes and backslashes

  QgsDebugMsg( "Connection info: " + uri.connectionInfo() );

  m_connectionInfo = uri.connectionInfo();
  //qDebug(m_connectionInfo);
  // Tidy up an existing connection if one exists.
  if ( pd != 0 )
    PQfinish( pd );

  pd = PQconnectdb( m_connectionInfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8

#if defined(PG_VERSION_NUM) && PG_VERSION_NUM >= 80300
  // if the connection needs a password ask for one
  if ( PQstatus( pd ) == CONNECTION_BAD && PQconnectionNeedsPassword( pd ) )
#else
  // if the connection failed and we didn't have a password, ask for one and retry
  if ( PQstatus( pd ) == CONNECTION_BAD && password.isEmpty() )
#endif
  {
    // get password from user
    bool makeConnection = false;
    password = QInputDialog::getText( this, tr( "Password for " ) + username,
                                      tr( "Please enter your password:" ),
                                      QLineEdit::Password, QString::null, &makeConnection );
    // allow null password entry in case its valid for the database
    if ( makeConnection )
    {
      uri.setConnection( settings.value( key + "/host" ).toString(),
                         settings.value( key + "/port" ).toString(),
                         database,
                         settings.value( key + "/username" ).toString(),
                         password,
                         ( QgsDataSourceURI::SSLmode ) settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt() );

      m_connectionInfo = uri.connectionInfo();
      PQfinish( pd );
      pd = PQconnectdb( m_connectionInfo.toLocal8Bit() );  // use what is set based on locale; after connecting, use Utf8
    }
  }

  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    //qDebug("Connection succeeded");
    // tell the DB that we want text encoded in UTF8
    PQsetClientEncoding( pd, QString( "UNICODE" ).toLocal8Bit() );

    // get the list of suitable tables and columns and populate the UI
    geomCol details;

    if ( getTableInfo( pd, searchGeometryColumnsOnly, searchPublicOnly ) )
    {
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
    else
    {
      QgsDebugMsg( QString( "Unable to get list of spatially enabled tables from the database\n%1" ).arg( PQerrorMessage( pd ) ) );
    }
    // BEGIN CHANGES ECOS
    if ( cmbConnections->count() > 0 )
      btnAdd->setEnabled( true );
    // END CHANGES ECOS
  }
  else
  {
    QMessageBox::warning( this, tr( "Connection failed" ),
                          tr( "Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.\n\n"
                              "Check your username and password and try again.\n\n"
                              "The database said:\n%3" )
                          .arg( settings.value( key + "/database" ).toString() )
                          .arg( settings.value( key + "/host" ).toString() )
                          .arg( QString::fromUtf8( PQerrorMessage( pd ) ) ) );
  }

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
}

QStringList QgsDbSourceSelect::selectedTables()
{
  return m_selectedTables;
}

QString QgsDbSourceSelect::connectionInfo()
{
  return m_connectionInfo;
}

void QgsDbSourceSelect::setSql( const QModelIndex& index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "schema item found" );
    return;
  }

  if ( pd == 0 )
  {
    return;
  }

  //create "Schema"."Table" and find out existing sql string
  QModelIndex schemaSibling = index.sibling( index.row(), QgsDbTableModel::dbtmSchema );
  QModelIndex tableSibling = index.sibling( index.row(), QgsDbTableModel::dbtmTable );
  if ( !schemaSibling.isValid() || !tableSibling.isValid() )
  {
    return;
  }

  QString schemaName = mTableModel.itemFromIndex( mProxyModel.mapToSource( schemaSibling ) )->text();
  QString tableName = mTableModel.itemFromIndex( mProxyModel.mapToSource( tableSibling ) )->text();
  QString tableString = "\"" + schemaName + "\".\"" + tableName + "\"";
  QgsDebugMsg( tableString );

  QString currentSql;
  QModelIndex sqlSibling = index.sibling( index.row(), QgsDbTableModel::dbtmSql );
  if ( sqlSibling.isValid() )
  {
    currentSql = mTableModel.itemFromIndex( mProxyModel.mapToSource( sqlSibling ) )->text();
  }

  // create a query builder object
  QgsPgQueryBuilder * pgb = new QgsPgQueryBuilder( tableString, pd, this );
  // set the current sql in the query builder sql box
  pgb->setSql( currentSql );
  // set the PG connection object so it can be used to fetch the
  // fields for the table, get sample values, and test the query
  pgb->setConnection( pd );
  // show the dialog
  if ( pgb->exec() )
  {
    mTableModel.setSql( mProxyModel.mapToSource( index ), pgb->sql() );
  }
}

void QgsDbSourceSelect::addSearchGeometryColumn( const QString &schema, const QString &table, const QString &column )
{
  // store the column details and do the query in a thread
  if ( mColumnTypeThread == NULL )
  {
    mColumnTypeThread = new QgsGeomColumnTypeThread();
    mColumnTypeThread->setConnInfo( m_connectionInfo );
  }
  mColumnTypeThread->addGeometryColumn( schema, table, column );
}

QStringList QgsDbSourceSelect::pkCandidates( PGconn *pg, QString schemaName, QString tableName )
{
  QStringList cols;
  cols << QString::null;

  QString sql = QString( "select attname from pg_attribute join pg_type on atttypid=pg_type.oid WHERE pg_type.typname='int4' AND attrelid=regclass('\"%1\".\"%2\"')" ).arg( schemaName ).arg( tableName );
  QgsDebugMsg( sql );
  PGresult *colRes = PQexec( pg, sql.toUtf8() );

  if ( PQresultStatus( colRes ) == PGRES_TUPLES_OK )
  {
    for ( int i = 0; i < PQntuples( colRes ); i++ )
    {
      QgsDebugMsg( PQgetvalue( colRes, i, 0 ) );
      cols << QString::fromUtf8( PQgetvalue( colRes, i, 0 ) );
    }
  }
  else
  {
    QgsDebugMsg( QString( "SQL:%1\nresult:%2\nerror:%3\n" ).arg( sql ).arg( PQresultStatus( colRes ) ).arg( PQerrorMessage( pg ) ) );
  }

  PQclear( colRes );

  return cols;
}

bool QgsDbSourceSelect::getTableInfo( PGconn *pg, bool searchGeometryColumnsOnly, bool searchPublicOnly )
{
  int n = 0;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // The following query returns only tables that exist and the user has SELECT privilege on.
  // Can't use regclass here because table must exist, else error occurs.
  QString sql = "select "
                "f_table_name,"
                "f_table_schema,"
                "f_geometry_column,"
                "type"
                " from "
                "geometry_columns,"
                "pg_class,"
                "pg_namespace"
                " where "
                "relname=f_table_name"
                " and f_table_schema=nspname"
                " and pg_namespace.oid=pg_class.relnamespace"
                " and has_schema_privilege(pg_namespace.nspname,'usage')"
                " and has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select')" // user has select privilege
                " order by "
                "f_table_schema,f_table_name,f_geometry_column";

  PGresult *result = PQexec( pg, sql.toUtf8() );
  if ( result )
  {
    if ( PQresultStatus( result ) != PGRES_TUPLES_OK )
    {
      QMessageBox::warning( this,
                            tr( "Accessible tables could not be determined" ),
                            tr( "Database connection was successful, but the accessible tables could not be determined.\n\n"
                                "The error message from the database was:\n%1\n" )
                            .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
      n = -1;
    }
    else if ( PQntuples( result ) > 0 )
    {
      for ( int idx = 0; idx < PQntuples( result ); idx++ )
      {
        QString tableName = QString::fromUtf8( PQgetvalue( result, idx, 0 ) );
        QString schemaName = QString::fromUtf8( PQgetvalue( result, idx, 1 ) );
        QString column = QString::fromUtf8( PQgetvalue( result, idx, 2 ) );
        QString type = QString::fromUtf8( PQgetvalue( result, idx, 3 ) );

        QString as = "";
        if ( type == "GEOMETRY" && !searchGeometryColumnsOnly )
        {
          addSearchGeometryColumn( schemaName, tableName,  column );
          as = type = "WAITING";
        }

        mTableModel.addTableEntry( type, schemaName, tableName, column, pkCandidates( pg, schemaName, tableName ), "" );
        n++;
      }
    }
  }

  PQclear( result );

  //search for geometry columns in tables that are not in the geometry_columns metatable
  QApplication::restoreOverrideCursor();

  if ( !searchGeometryColumnsOnly )
  {
    // Now have a look for geometry columns that aren't in the
    // geometry_columns table. This code is specific to postgresql,
    // but an equivalent query should be possible in other
    // databases.
    sql = "select "
          "pg_class.relname,"
          "pg_namespace.nspname,"
          "pg_attribute.attname,"
          "pg_class.relkind"
          " from "
          "pg_attribute,"
          "pg_class,"
          "pg_namespace"
          " where "
          "pg_namespace.oid = pg_class.relnamespace"
          " and pg_attribute.attrelid = pg_class.oid "
          " and ("
          "pg_attribute.atttypid = regtype('geometry')"
          " or pg_attribute.atttypid IN (select oid FROM pg_type WHERE typbasetype=regtype('geometry'))"
          ")"
          " and has_schema_privilege(pg_namespace.nspname,'usage')"
          " and has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select')";
    // user has select privilege
    if ( searchPublicOnly )
      sql += " and pg_namespace.nspname = 'public'";

    if ( n > 0 )
    {
      sql += " and not exists (select * from geometry_columns WHERE pg_namespace.nspname=f_table_schema AND pg_class.relname=f_table_name)";
    }
    else
    {
      n = 0;
    }

    sql += " and pg_class.relkind in ('v', 'r')"; // only from views and relations (tables)

    result = PQexec( pg, sql.toUtf8() );

    if ( PQresultStatus( result ) != PGRES_TUPLES_OK )
    {
      QMessageBox::warning( this,
                            tr( "Accessible tables could not be determined" ),
                            tr( "Database connection was successful, but the accessible tables could not be determined.\n\n"
                                "The error message from the database was:\n%1\n" )
                            .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
      if ( n == 0 )
        n = -1;
    }
    else if ( PQntuples( result ) > 0 )
    {
      for ( int i = 0; i < PQntuples( result ); i++ )
      {
        // Have the column name, schema name and the table name. The concept of a
        // catalog doesn't exist in postgresql so we ignore that, but we
        // do need to get the geometry type.

        // Make the assumption that the geometry type for the first
        // row is the same as for all other rows.

        QString table  = QString::fromUtf8( PQgetvalue( result, i, 0 ) ); // relname
        QString schema = QString::fromUtf8( PQgetvalue( result, i, 1 ) ); // nspname
        QString column = QString::fromUtf8( PQgetvalue( result, i, 2 ) ); // attname
        QString relkind = QString::fromUtf8( PQgetvalue( result, i, 3 ) ); // relation kind

        addSearchGeometryColumn( schema, table, column );
        //details.push_back(geomPair(fullDescription(schema, table, column, "WAITING"), "WAITING"));
        mTableModel.addTableEntry( "Waiting", schema, table, column, pkCandidates( pg, schema, table ), "" );
        n++;
      }
    }

    PQclear( result );
  }

  if ( n == 0 )
  {
    QMessageBox::warning( this,
                          tr( "No accessible tables found" ),
                          tr( "Database connection was successful, but no accessible tables were found.\n\n"
                              "Please verify that you have SELECT privilege on a table carrying PostGIS\n"
                              "geometry." ) );
  }

  return n > 0;
}

void QgsDbSourceSelect::showHelp()
{
  QgsContextHelp::run( context_id );
}

QString QgsDbSourceSelect::fullDescription( QString schema, QString table,
    QString column, QString type )
{
  QString full_desc = "";
  if ( schema.length() > 0 )
    full_desc = '"' + schema + "\".\"";
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void QgsDbSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/PostgreSQL/connections/selected",
                     cmbConnections->currentText() );
}

void QgsDbSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.value( "/PostgreSQL/connections/selected" ).toString();
  // Does toSelect exist in cmbConnections?
  bool set = false;
  for ( int i = 0; i < cmbConnections->count(); ++i )
    if ( cmbConnections->itemText( i ) == toSelect )
    {
      cmbConnections->setCurrentIndex( i );
      set = true;
      break;
    }
  // If we couldn't find the stored item, but there are some,
  // default to the last item (this makes some sense when deleting
  // items as it allows the user to repeatidly click on delete to
  // remove a whole lot of items).
  if ( !set && cmbConnections->count() > 0 )
  {
    // If toSelect is null, then the selected connection wasn't found
    // by QSettings, which probably means that this is the first time
    // the user has used qgis with database connections, so default to
    // the first in the list of connetions. Otherwise default to the last.
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsDbSourceSelect::setSearchExpression( const QString& regexp )
{
}

void QgsGeomColumnTypeThread::setConnInfo( QString s )
{
  mConnInfo = s;
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
  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    PQsetClientEncoding( pd, QString( "UNICODE" ).toLocal8Bit() );

    for ( uint i = 0; i < schemas.size(); i++ )
    {
      QString query = QString( "select distinct "
                               "case"
                               " when geometrytype(%1) IN ('POINT','MULTIPOINT') THEN 'POINT'"
                               " when geometrytype(%1) IN ('LINESTRING','MULTILINESTRING') THEN 'LINESTRING'"
                               " when geometrytype(%1) IN ('POLYGON','MULTIPOLYGON') THEN 'POLYGON'"
                               " end "
                               "from "
                               "\"%2\".\"%3\"" ).arg( "\"" + columns[i] + "\"" ).arg( schemas[i] ).arg( tables[i] );
      PGresult* gresult = PQexec( pd, query.toUtf8() );
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

      if ( mStopped )
        break;

      // Now tell the layer list dialog box...
      emit setLayerType( schemas[i], tables[i], columns[i], type );
    }
  }

  PQfinish( pd );
}
