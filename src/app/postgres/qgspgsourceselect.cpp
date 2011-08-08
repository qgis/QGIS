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

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
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

QgsPgSourceSelect::QgsPgSourceSelect( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mColumnTypeThread( NULL ), pd( 0 )
{
  setupUi( this );

  mAddButton = new QPushButton( tr( "&Add" ) );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addTables() ) );

  mBuildQueryButton = new QPushButton( tr( "&Build query" ) );
  mBuildQueryButton->setToolTip( tr( "Build query" ) );
  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQuery() ) );
  mBuildQueryButton->setDisabled( true );

  mAddButton->setEnabled( false );
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
  nc->exec();
  delete nc;

  populateConnectionList();
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
  nc->exec();
  delete nc;

  populateConnectionList();
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsPgSourceSelect::on_cmbConnections_activated( int )
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/PostgreSQL/connections/selected", cmbConnections->currentText() );

  cbxAllowGeometrylessTables->blockSignals( true );
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
  PQfinish( pd );

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

  QString uri = m_connInfo;

  if ( !pkColumnName.isEmpty() )
  {
    uri += QString( " key=\"%1\"" ).arg( pkColumnName );
  }

  if ( mUseEstimatedMetadata )
  {
    uri += QString( " estimatedmetadata=true" );
  }

  uri += QString( " table=\"%1\".\"%2\"%3 sql=%4" )
         .arg( schemaName ).arg( tableName )
         .arg( geomColumnName.isEmpty() ? QString() : QString( " (%1)" ).arg( geomColumnName ) )
         .arg( sql );

  return uri;
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

    m_selectedTables << layerURI( mProxyModel.mapToSource( *selected_it ) );
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
  QSettings settings;

  QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();

  QString service                   = settings.value( key + "/service" ).toString();
  QString host                      = settings.value( key + "/host" ).toString();
  QString port                      = settings.value( key + "/port" ).toString();
  QString database                  = settings.value( key + "/database" ).toString();
  QString username                  = settings.value( key + "/username" ).toString();
  QString password                  = settings.value( key + "/password" ).toString();
  QgsDataSourceURI::SSLmode sslmode = ( QgsDataSourceURI::SSLmode ) settings.value( key + "/sslmode", QgsDataSourceURI::SSLprefer ).toInt();

  QgsDataSourceURI uri;
  if ( !service.isEmpty() )
  {
    uri.setConnection( service, database, username, password, sslmode );
  }
  else
  {
    uri.setConnection( host, port, database,  username, password, sslmode );
  }

  bool searchPublicOnly = settings.value( key + "/publicOnly" ).toBool();
  bool searchGeometryColumnsOnly = settings.value( key + "/geometryColumnsOnly" ).toBool();
  bool allowGeometrylessTables = cbxAllowGeometrylessTables->isChecked();
  mUseEstimatedMetadata = settings.value( key + "/estimatedMetadata" ).toBool();
  // Need to escape the password to allow for single quotes and backslashes

  QgsDebugMsg( "Connection info: " + uri.connectionInfo() );

  m_connInfo = uri.connectionInfo();

  // Tidy up an existing connection if one exists.
  if ( pd != 0 )
    PQfinish( pd );

  m_privConnInfo = m_connInfo;

  pd = PQconnectdb( m_privConnInfo.toLocal8Bit() );   // use what is set based on locale; after connecting, use Utf8
  // check the connection status
  if ( PQstatus( pd ) != CONNECTION_OK )
  {
    while ( PQstatus( pd ) != CONNECTION_OK )
    {
      bool ok = QgsCredentials::instance()->get( m_connInfo, username, password, QString::fromUtf8( PQerrorMessage( pd ) ) );
      if ( !ok )
        break;

      ::PQfinish( pd );

      QgsDataSourceURI uri( m_connInfo );
      if ( !username.isEmpty() )
        uri.setUsername( username );

      if ( !password.isEmpty() )
        uri.setPassword( password );

      m_privConnInfo = uri.connectionInfo();
      QgsDebugMsg( "connecting " + m_privConnInfo );
      pd = PQconnectdb( m_privConnInfo.toLocal8Bit() );
    }

    if ( PQstatus( pd ) == CONNECTION_OK )
    {
      QgsCredentials::instance()->put( m_connInfo, username, password );
    }
  }

  if ( PQstatus( pd ) == CONNECTION_OK )
  {
    //qDebug("Connection succeeded");
    // tell the DB that we want text encoded in UTF8
    PQsetClientEncoding( pd, QString( "UNICODE" ).toLocal8Bit() );

    PGresult *res = PQexec( pd, "SET application_name='Quantum GIS'" );

    if ( !res || PQresultStatus( res ) != PGRES_COMMAND_OK )
    {
      PQclear( res );
      res = PQexec( pd, "ROLLBACK" );
    }

    PQclear( res );

    // get the list of suitable tables and columns and populate the UI
    geomCol details;

    if ( getTableInfo( pd, searchGeometryColumnsOnly, searchPublicOnly, allowGeometrylessTables ) )
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
      mAddButton->setEnabled( true );
    // END CHANGES ECOS
  }
  else
  {
    QMessageBox::warning( this, tr( "Connection failed" ),
                          tr( "Connection to %1 on %2 failed. Either the database is down or your settings are incorrect.\n\n"
                              "Check your username and password and try again.\n\n"
                              "The database said:\n%3" )
                          .arg( settings.value( key + "/database" ).toString() )
                          .arg( !settings.value( key + "/service" ).toString().isEmpty()
                                ? settings.value( key + "/service" ).toString()
                                : settings.value( key + "/host" ).toString() )
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
    mColumnTypeThread->setConnInfo( m_privConnInfo, mUseEstimatedMetadata );
  }
  mColumnTypeThread->addGeometryColumn( schema, table, column );
}

QStringList QgsPgSourceSelect::pkCandidates( PGconn *pg, QString schemaName, QString viewName )
{
  QStringList cols;
  cols << QString::null;

  QString sql = QString( "select attname from pg_attribute join pg_type on atttypid=pg_type.oid WHERE pg_type.typname IN ('int4','oid') AND attrelid=regclass('\"%1\".\"%2\"')" ).arg( schemaName ).arg( viewName );
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

bool QgsPgSourceSelect::getTableInfo( PGconn *pg, bool searchGeometryColumnsOnly, bool searchPublicOnly, bool allowGeometrylessTables )
{
  int nColumns = 0;
  int nGTables = 0;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  PGresult *result = 0;

  for ( int i = 0; i < 2; i++ )
  {
    QString gtableName, columnName;

    if ( i == 0 )
    {
      gtableName = "geometry_columns";
      columnName = "f_geometry_column";
    }
    else if ( i == 1 )
    {
      gtableName = "geography_columns";
      columnName = "f_geography_column";
    }

    // The following query returns only tables that exist and the user has SELECT privilege on.
    // Can't use regclass here because table must exist, else error occurs.
    QString sql = QString( "select "
                           "f_table_name,"
                           "f_table_schema,"
                           "%2,"
                           "upper(type),"
                           "pg_class.relkind"
                           " from "
                           "%1,"
                           "pg_class,"
                           "pg_namespace"
                           " where "
                           "relname=f_table_name"
                           " and f_table_schema=nspname"
                           " and pg_namespace.oid=pg_class.relnamespace"
                           " and has_schema_privilege(pg_namespace.nspname,'usage')"
                           " and has_table_privilege('\"'||pg_namespace.nspname||'\".\"'||pg_class.relname||'\"','select')" // user has select privilege
                           " order by "
                           "f_table_schema,f_table_name,%2" ).arg( gtableName ).arg( columnName );

    QgsDebugMsg( "sql: " + sql );

    result = PQexec( pg, sql.toUtf8() );
    if ( result )
    {
      if ( PQresultStatus( result ) != PGRES_TUPLES_OK )
      {
        PGresult *r = PQexec( pg, "COMMIT" );
        if ( r )
          PQclear( r );
      }
      else
      {
        nGTables++;

        if ( PQntuples( result ) > 0 )
        {

          for ( int idx = 0; idx < PQntuples( result ); idx++ )
          {
            QString tableName = QString::fromUtf8( PQgetvalue( result, idx, 0 ) );
            QString schemaName = QString::fromUtf8( PQgetvalue( result, idx, 1 ) );
            QString column = QString::fromUtf8( PQgetvalue( result, idx, 2 ) );
            QString type = QString::fromUtf8( PQgetvalue( result, idx, 3 ) );
            QString relkind = QString::fromUtf8( PQgetvalue( result, idx, 4 ) );

            QgsDebugMsg( QString( "%1 %2.%3.%4: %5 %6" )
                         .arg( gtableName )
                         .arg( schemaName ).arg( tableName ).arg( column )
                         .arg( type )
                         .arg( relkind ) );

            QString as = "";
            if ( type == "GEOMETRY" && !searchGeometryColumnsOnly )
            {
              addSearchGeometryColumn( schemaName, tableName,  column );
              as = type = "WAITING";
            }

            mTableModel.addTableEntry( type, schemaName, tableName, column, relkind == "v" ? pkCandidates( pg, schemaName, tableName ) : QStringList(), "" );
            nColumns++;
          }
        }
      }
    }

    PQclear( result );
    result = 0;
  }

  if ( nColumns == 0 )
  {
    QMessageBox::warning( this,
                          tr( "Accessible tables could not be determined" ),
                          tr( "Database connection was successful, but the accessible tables could not be determined." ) );
    nColumns = -1;
  }

  //search for geometry columns in tables that are not in the geometry_columns metatable
  QApplication::restoreOverrideCursor();

  if ( !searchGeometryColumnsOnly )
  {
    // Now have a look for geometry columns that aren't in the
    // geometry_columns table. This code is specific to postgresql,
    // but an equivalent query should be possible in other
    // databases.
    QString sql = "select "
                  "pg_class.relname"
                  ",pg_namespace.nspname"
                  ",pg_attribute.attname"
                  ",pg_class.relkind"
                  " from "
                  "pg_attribute"
                  ",pg_class"
                  ",pg_namespace"
                  " where "
                  "pg_namespace.oid=pg_class.relnamespace"
                  " and pg_attribute.attrelid = pg_class.oid"
                  " and ("
                  " exists (select * from pg_type WHERE pg_type.oid=pg_attribute.atttypid AND pg_type.typname IN ('geometry','geography'))"
                  " or pg_attribute.atttypid IN (select oid FROM pg_type a WHERE EXISTS (SELECT * FROM pg_type b WHERE a.typbasetype=b.oid AND b.typname IN ('geometry','geography')))"
                  ")"
                  " and has_schema_privilege( pg_namespace.nspname, 'usage' )"
                  " and has_table_privilege( '\"' || pg_namespace.nspname || '\".\"' || pg_class.relname || '\"', 'select' )";

    // user has select privilege
    if ( searchPublicOnly )
      sql += " and pg_namespace.nspname = 'public'";

    if ( nColumns > 0 )
    {
      sql += " and not exists (select * from geometry_columns WHERE pg_namespace.nspname=f_table_schema AND pg_class.relname=f_table_name)";

      if ( nGTables > 1 )
      {
        sql += " and not exists (select * from geography_columns WHERE pg_namespace.nspname=f_table_schema AND pg_class.relname=f_table_name)";
      }
    }
    else
    {
      nColumns = 0;
    }

    sql += " and pg_class.relkind in( 'v', 'r' )"; // only from views and relations (tables)

    QgsDebugMsg( "sql: " + sql );

    result = PQexec( pg, sql.toUtf8() );

    if ( PQresultStatus( result ) != PGRES_TUPLES_OK )
    {
      QMessageBox::warning( this,
                            tr( "Accessible tables could not be determined" ),
                            tr( "Database connection was successful, but the accessible tables could not be determined.\n\n"
                                "The error message from the database was:\n%1\n" )
                            .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
      if ( nColumns == 0 )
        nColumns = -1;
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

        QgsDebugMsg( QString( "%1.%2.%3: %4" ).arg( schema ).arg( table ).arg( column ).arg( relkind ) );

        addSearchGeometryColumn( schema, table, column );
        //details.push_back(geomPair(fullDescription(schema, table, column, "WAITING"), "WAITING"));
        mTableModel.addTableEntry( tr( "Waiting" ), schema, table, column, relkind == "v" ? pkCandidates( pg, schema, table ) : QStringList(), "" );
        nColumns++;
      }
    }

    PQclear( result );
    result = 0;
  }

  if ( allowGeometrylessTables )
  {
    QString sql = "select "
                  "pg_class.relname"
                  ",pg_namespace.nspname"
                  ",pg_class.relkind"
                  " from "
                  " pg_class"
                  ",pg_namespace"
                  " where "
                  "pg_namespace.oid=pg_class.relnamespace"
                  " and has_schema_privilege( pg_namespace.nspname, 'usage' )"
                  " and has_table_privilege( '\"' || pg_namespace.nspname || '\".\"' || pg_class.relname || '\"', 'select' )"
                  " and pg_class.relkind in( 'v', 'r' )";

    // user has select privilege
    if ( searchPublicOnly )
      sql += " and pg_namespace.nspname = 'public'";

    QgsDebugMsg( "sql: " + sql );

    result = PQexec( pg, sql.toUtf8() );

    if ( PQresultStatus( result ) != PGRES_TUPLES_OK )
    {
      QMessageBox::warning( this,
                            tr( "Accessible tables could not be determined" ),
                            tr( "Database connection was successful, but the accessible tables could not be determined.\n\n"
                                "The error message from the database was:\n%1\n" )
                            .arg( QString::fromUtf8( PQresultErrorMessage( result ) ) ) );
      if ( nColumns == 0 )
        nColumns = -1;
    }
    else if ( PQntuples( result ) > 0 )
    {
      for ( int i = 0; i < PQntuples( result ); i++ )
      {
        QString table  = QString::fromUtf8( PQgetvalue( result, i, 0 ) ); // relname
        QString schema = QString::fromUtf8( PQgetvalue( result, i, 1 ) ); // nspname
        QString relkind = QString::fromUtf8( PQgetvalue( result, i, 2 ) ); // relation kind

        QgsDebugMsg( QString( "%1.%2: %3" ).arg( schema ).arg( table ).arg( relkind ) );

        mTableModel.addTableEntry( tr( "No geometry" ), schema, table, QString::null, relkind == "v" ? pkCandidates( pg, schema, table ) : QStringList(), "" );
        nColumns++;
      }
    }

    PQclear( result );
    result = 0;
  }

  if ( nColumns == 0 )
  {
    QMessageBox::warning( this,
                          tr( "No accessible tables found" ),
                          tr( "Database connection was successful, but no accessible tables were found.\n\n"
                              "Please verify that you have SELECT privilege on a table carrying PostGIS\n"
                              "geometry." ) );
  }

  return nColumns > 0;
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
  QSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.value( "/PostgreSQL/connections/selected" ).toString();
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
