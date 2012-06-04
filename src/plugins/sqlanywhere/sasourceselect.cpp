/***************************************************************************
  sasourceselect.cpp
  Dialogue box for defining vector layers from a SQL Anywhere database
  -------------------
    begin                : Dec 2010
    copyright            : (C) 2010 by iAnywhere Solutions, Inc.
    author               : David DeHaan
    email                : ddehaan at sybase dot com

  The author gratefully acknowledges that portions of this class were copied
  from QgsPgSourceSelect, and so the following copyright holds on the
  original content:
    qgpgsourceselect.cpp
    begin                : Sat Jun 22 2002
    copyright            : (C) 2002 by Gary E.Sherman
    email                : sherman at mrcc.com

 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 3 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "sasourceselect.h"
#include "sanewconnection.h"
#include "qgsquerybuilder.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgscredentials.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

static const int sGeomTypeSelectLimit = 100;

QString
quotedIdentifier( QString id )
{
  id.replace( "\"", "\"\"" );
  return id.prepend( "\"" ).append( "\"" );
} // quotedIdentifier()


SaSourceSelect::SaSourceSelect( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl ), mColumnTypeThread( NULL )
{
  setupUi( this );

  mAddButton = new QPushButton( tr( "&Add" ) );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addTables() ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Build Query" ) );
  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQuery() ) );
  mBuildQueryButton->setEnabled( false );

  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Schema" ) );
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Type" ) );
  mSearchColumnComboBox->addItem( tr( "SRID" ) );
  mSearchColumnComboBox->addItem( tr( "Line Interpretation" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setDynamicSortFilter( true );
  mProxyModel.setSourceModel( &mTableModel );

  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->setEditTriggers( QAbstractItemView::CurrentChanged );
  mTablesTreeView->setItemDelegate( new SaSourceSelectDelegate( this ) );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SaSourceSelect/geometry" ).toByteArray() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QString( "/Windows/SaSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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
/** Autoconnected SLOTS **/
// Slot for adding a new connection
void SaSourceSelect::on_btnNew_clicked()
{
  SaNewConnection *nc = new SaNewConnection( this );
  nc->exec();
  delete nc;

  populateConnectionList();
}
// Slot for deleting an existing connection
void SaSourceSelect::on_btnDelete_clicked()
{
  QSettings settings;
  QString key = "/SQLAnywhere/connections/" + cmbConnections->currentText();
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  settings.remove( key + "/host" );
  settings.remove( key + "/port" );
  settings.remove( key + "/server" );
  settings.remove( key + "/database" );
  settings.remove( key + "/parameters" );
  settings.remove( key + "/username" );
  settings.remove( key + "/password" );
  settings.remove( key + "/saveUsername" );
  settings.remove( key + "/savePassword" );
  settings.remove( key + "/simpleEncryption" );
  settings.remove( key + "/simpleMetadata" );
  settings.remove( key );

  populateConnectionList();
}

// Slot for editing a connection
void SaSourceSelect::on_btnEdit_clicked()
{
  SaNewConnection *nc = new SaNewConnection( this, cmbConnections->currentText() );
  nc->exec();
  delete nc;

  populateConnectionList();
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void SaSourceSelect::on_cmbConnections_activated( int )
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/SQLAnywhere/connections/selected", cmbConnections->currentText() );
}

void SaSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void SaSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void SaSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  setSql( index );
}

void SaSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void SaSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmType );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmSrid );
  }
  else if ( text == tr( "Line Interpretation" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmLineInterp );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( SaDbTableModel::dbtmSql );
  }
}

void SaSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void SaSourceSelect::setLayerType( QString schema,
                                   QString table, QString column, QString type,
                                   QString srid, QString interp )
{
  mTableModel.setGeometryTypesForTable( schema, table, column, type, srid, interp );
  mTablesTreeView->sortByColumn( SaDbTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( SaDbTableModel::dbtmSchema, Qt::AscendingOrder );
}

SaSourceSelect::~SaSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    delete mColumnTypeThread;
    mColumnTypeThread = NULL;
  }

  QSettings settings;
  settings.setValue( "/Windows/SaSourceSelect/geometry", saveGeometry() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/SaSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void SaSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/SQLAnywhere/connections" );
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

QString SaSourceSelect::layerURI( const QModelIndex &index )
{
  QString schemaName = mTableModel.itemFromIndex( index.sibling( index.row(), SaDbTableModel::dbtmSchema ) )->text();
  QString tableName = mTableModel.itemFromIndex( index.sibling( index.row(), SaDbTableModel::dbtmTable ) )->text();
  QString geomColumnName = mTableModel.itemFromIndex( index.sibling( index.row(), SaDbTableModel::dbtmGeomCol ) )->text();
  QString sql = mTableModel.itemFromIndex( index.sibling( index.row(), SaDbTableModel::dbtmSql ) )->text();

  QString uri = mConnInfo;
  uri += QString( " table=\"%1\".\"%2\" (%3) sql=%4" )
         .arg( schemaName ).arg( tableName )
         .arg( geomColumnName )
         .arg( sql );

  return uri;
}

// Slot for performing action when the Add button is clicked
void SaSourceSelect::addTables()
{
  mSelectedTables.clear();

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

    mSelectedTables << layerURI( mProxyModel.mapToSource( *selected_it ) );
  }

  if ( mSelectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a layer." ) );
  }
  else
  {
    accept();
  }
}

void SaSourceSelect::on_btnConnect_clicked()
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

  // load the SQL Anywhere interface
  if ( !SqlAnyConnection::initApi() )
  {
    QMessageBox::information( this,
                              tr( "Failed to load interface" ),
                              tr( SqlAnyConnection::failedInitMsg() ) );
    return;
  }

  // compute connection information
  QString key = "/SQLAnywhere/connections/" + cmbConnections->currentText();
  mEstimateMetadata = settings.value( key + "/estimateMetadata", false ).toBool();
  mOtherSchemas = settings.value( key + "/otherSchemas", false ).toBool();
  mConnInfo = SqlAnyConnection::makeUri( key
                                         , settings.value( key + "/host" ).toString()
                                         , settings.value( key + "/port" ).toString()
                                         , settings.value( key + "/server" ).toString()
                                         , settings.value( key + "/database" ).toString()
                                         , settings.value( key + "/parameters" ).toString()
                                         , settings.value( key + "/username" ).toString()
                                         , settings.value( key + "/password" ).toString()
                                         , settings.value( key + "/simpleEncryption", false ).toBool()
                                         , mEstimateMetadata );
  SaDebugMsg( "Connection info: " + mConnInfo );

  // establish read-only connection to the database
  char      errbuf[SACAPI_ERROR_SIZE];
  sacapi_i32     code;
  SqlAnyConnection  *conn = SqlAnyConnection::connect( mConnInfo, true, code, errbuf, sizeof( errbuf ) );

  if ( conn )
  {
    // get the list of suitable tables and columns and populate the UI
    geomCol details;

    if ( getTableInfo( conn->addRef(), mOtherSchemas ) )
    {
      // Start the thread that gets the geometry type for relations that
      // may take a long time to return
      if ( mColumnTypeThread != NULL )
      {
        connect( mColumnTypeThread, SIGNAL( setLayerType( QString, QString, QString, QString, QString, QString ) ),
                 this, SLOT( setLayerType( QString, QString, QString, QString, QString, QString ) ) );

        // Do it in a thread.
        mColumnTypeThread->start();
      }

    }
    else
    {
      SaDebugMsg( "Unable to get list of spatially enabled tables "
                  "from the database" );
    }
    if ( cmbConnections->count() > 0 )
      mAddButton->setEnabled( true );

    conn->release();

  }
  else
  {
    QMessageBox::warning( this, tr( "Connection failed" ),
                          tr( "Connection to database %1 failed. "
                              "Check settings and try again.\n\n"
                              "SQL Anywhere error code: %2\n"
                              "Description: %3" )
                          .arg( settings.value( key + "/database" ).toString() )
                          .arg( code )
                          .arg( errbuf ) );
  }

  mTablesTreeView->sortByColumn( SaDbTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( SaDbTableModel::dbtmSchema, Qt::AscendingOrder );

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

QStringList SaSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString SaSourceSelect::connectionInfo()
{
  return mConnInfo;
}

void SaSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    SaDebugMsg( "schema item found" );
    return;
  }

  QgsVectorLayer *vlayer = new QgsVectorLayer( layerURI( mProxyModel.mapToSource( index ) ), "querybuilder", "sqlanywhere" );

  if ( !vlayer->isValid() )
  {
    delete vlayer;
    return;
  }

  // create a query builder object
  QgsQueryBuilder *qb = new QgsQueryBuilder( vlayer, this );
  if ( qb->exec() )
  {
    mTableModel.setSql( mProxyModel.mapToSource( index ), qb->sql() );
  }

  delete qb;
  delete vlayer;
}

void SaSourceSelect::addSearchGeometryColumn( const QString &schema, const QString &table, const QString &column, const QString &geomtype, const QString &sridstr, const QString &lineinterp )
{
  // store the column details and do the query in a thread
  if ( mColumnTypeThread == NULL )
  {
    mColumnTypeThread = new SaGeomColTypeThread();
    mColumnTypeThread->setConnInfo( mConnInfo, mEstimateMetadata, mOtherSchemas );
  }
  mColumnTypeThread->addGeometryColumn( schema, table, column, geomtype, sridstr, lineinterp );
}

// Accepts ownership of given connection pointer.
bool SaSourceSelect::getTableInfo( SqlAnyConnection *conn, bool searchOtherSchemas )
{
  QString     sql;
  SqlAnyStatement *stmt;
  int      n = 0;

  QApplication::setOverrideCursor( Qt::WaitCursor );

  sql = "SELECT g.table_schema, g.table_name, g.column_name, "
        "COALESCE( UCASE(g.geometry_type_name), 'ST_GEOMETRY' ), "
        "g.srs_id, "
        "IF s.round_earth = 'Y' THEN 'ROUND EARTH' ELSE 'PLANAR' ENDIF "
        "FROM SYS.ST_GEOMETRY_COLUMNS g "
        "LEFT OUTER JOIN SYS.ST_SPATIAL_REFERENCE_SYSTEMS s "
        "ON g.srs_id = s.srs_id ";
  if ( !searchOtherSchemas )
  {
    sql += QString( "WHERE g.table_schema=USER " );
  }

  stmt = conn->execute_direct( sql );
  if ( stmt->isValid() )
  {
    for ( ; stmt->fetchNext() ; n++ )
    {
      QString schema;
      QString tabname;
      QString colname;
      int     srid = -1;
      QString sridstr;
      QString lineinterp;
      QString geomtype;
      bool    waiting = false;

      stmt->getString( 0, schema );
      stmt->getString( 1, tabname );
      stmt->getString( 2, colname );
      stmt->getString( 3, geomtype );
      stmt->getInt( 4, srid );
      stmt->getString( 5, lineinterp );

      if ( srid == -1 )  // null srid and lineinterp
      {
        sridstr = lineinterp = "WAITING";
        waiting = true;
      }
      else
      {
        sridstr = QString::number( srid );
      }
      if ( geomtype == "ST_GEOMETRY" )
      {
        geomtype = "WAITING";
        waiting = true;
      }

      if ( waiting )
      {
        addSearchGeometryColumn( schema, tabname, colname, geomtype, sridstr, lineinterp );
      }

      mTableModel.addTableEntry( geomtype, schema, tabname, sridstr, lineinterp, colname, "" );
    }

  }
  else
  {
    SaDebugMsg( QString( "SQL Anywhere error %1: %2" )
                .arg( stmt->errCode() )
                .arg( stmt->errMsg() ) );
  }
  delete stmt;
  conn->release();

  QApplication::restoreOverrideCursor();

  if ( n == 0 )
  {
    QMessageBox::warning( this,
                          tr( "No accessible tables found" ),
                          tr( "Database connection was successful, but no tables "
                              "containing geometry columns were %1." )
                          .arg( searchOtherSchemas ?
                                tr( "found" ) : tr( "found in your schema" ) )
                        );
  }

  return n > 0;
}

QString SaSourceSelect::fullDescription( QString schema, QString table,
    QString column, QString type )
{
  QString full_desc = "";
  if ( schema.length() > 0 )
    full_desc = '"' + schema + "\".\"";
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void SaSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.value( "/SQLAnywhere/connections/selected" ).toString();
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

void SaSourceSelect::setSearchExpression( const QString& regexp )
{
  Q_UNUSED( regexp );
}

void SaGeomColTypeThread::setConnInfo( QString conninfo, bool estMeta, bool otherSchemas )
{
  mConnInfo = conninfo;
  mEstimateMetadata = estMeta;
  mOtherSchemas = otherSchemas;
}

void SaGeomColTypeThread::addGeometryColumn( QString schema, QString table, QString column, QString geomtype, QString sridstr, QString lineinterp )
{
  schemas.push_back( schema );
  tables.push_back( table );
  columns.push_back( column );
  geomtypes.push_back( geomtype );
  sridstrs.push_back( sridstr );
  lineinterps.push_back( lineinterp );
}

void SaGeomColTypeThread::stop()
{
  mStopped = true;
}

void SaGeomColTypeThread::getLayerTypes()
{
  mStopped = false;

  // establish read-only connection to the database
  char  errbuf[SACAPI_ERROR_SIZE];
  sacapi_i32  code;
  SqlAnyConnection *conn = SqlAnyConnection::connect( mConnInfo, true, code, errbuf, sizeof( errbuf ) );

  if ( conn )
  {
    for ( uint i = 0; i < schemas.size() && !mStopped; i++ )
    {
      QString geomtype = geomtypes[i];
      QString sridstr = sridstrs[i];
      QString lineinterp = lineinterps[i];

      QString sql;
      QString quotedTableName;
      QString fromStr;
      SqlAnyStatement *stmt;

      quotedTableName = QString( "%1.%2" )
                        .arg( quotedIdentifier( schemas[i] ) )
                        .arg( quotedIdentifier( tables[i] ) );
      if ( mEstimateMetadata )
      {
        fromStr = QString( "(SELECT TOP %1 %2 FROM %3 WHERE %2 IS NOT NULL ) AS sampleGeoms " )
                  .arg( sGeomTypeSelectLimit )
                  .arg( quotedIdentifier( columns[i] ) )
                  .arg( quotedTableName );
      }
      else
      {
        fromStr = quotedTableName;
      }

      // retrieve distinct geometry types
      if ( geomtype == "WAITING" )
      {
        QStringList types;

        sql = QString(
                "SELECT DISTINCT "
                "CASE "
                "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_POINT','ST_MULTIPOINT') THEN 'ST_POINT' "
                "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_LINESTRING','ST_MULTILINESTRING') THEN 'ST_LINESTRING' "
                "WHEN UCASE(%1.ST_GeometryType()) IN ('ST_POLYGON','ST_MULTIPOLYGON') THEN 'ST_POLYGON' "
                "ELSE 'ST_GEOMETRY' "
                "END "
                "FROM %2 " )
              .arg( quotedIdentifier( columns[i] ) )
              .arg( fromStr );

        stmt = conn->execute_direct( sql );
        if ( stmt->isValid() )
        {
          while ( stmt->fetchNext() )
          {
            QString type;
            stmt->getString( 0, type );
            types += type;
          }
        }
        delete stmt;

        if ( types.isEmpty() )
        {
          geomtype = "ST_GEOMETRY";
        }
        else
        {
          geomtype = types.join( "," );
        }
      }

      // retrieve distinct srids
      if ( sridstr == "WAITING" )
      {
        QStringList srids;
        QStringList interps;

        sql = QString(
                "SELECT srid, "
                "IF round_earth = 'Y' THEN 'ROUND EARTH' ELSE 'PLANAR' ENDIF "
                "FROM ( "
                "SELECT DISTINCT %1.ST_SRID() AS srid FROM %2 "
                ") AS sridlist, SYS.ST_SPATIAL_REFERENCE_SYSTEMS "
                "WHERE srid = srs_id " )
              .arg( quotedIdentifier( columns[i] ) )
              .arg( fromStr );

        stmt = conn->execute_direct( sql );
        if ( stmt->isValid() )
        {
          while ( stmt->fetchNext() )
          {
            int srid;
            QString interp;
            stmt->getInt( 0, srid );
            stmt->getString( 1, interp );
            srids += QString::number( srid );
            if ( !interps.contains( interp ) )
            {
              interps += interp;
            }
          }
        }
        delete stmt;

        if ( srids.isEmpty() )
        {
          sridstr = "UNKNOWN";
          lineinterp = "UNKNOWN";
        }
        else
        {
          sridstr = srids.join( "," );
          lineinterp = interps.join( "," );
        }
      }

      // Now tell the layer list dialog box...
      emit setLayerType( schemas[i], tables[i], columns[i], geomtype, sridstr, lineinterp );
    }

    conn->release();
  }
}
