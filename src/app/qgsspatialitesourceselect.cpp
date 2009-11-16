/***************************************************************************
                             qgsspatialitesourceselect.cpp
       Dialog to select SpatiaLite layer(s) and add it to the map canvas
                              -------------------
begin                : Dec 2008
copyright            : (C) 2008 by Sandro Furieri
email                : a.furieri@lqt.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsspatialitesourceselect.h"

#include "qgisapp.h"
#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStringList>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect( QgisApp * app, Qt::WFlags fl ):
    QDialog( app, fl ), qgisApp( app )
{
  setupUi( this );
  mAddButton = new QPushButton( tr( "&Add" ) );
  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) );
  connect( buttonBox, SIGNAL( helpRequested() ), this, SLOT( helpClicked() ) );

  mAddButton->setEnabled( false );
  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setDynamicSortFilter( true );
  mProxyModel.setSourceModel( &mTableModel );
  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );

  mSearchGroupBox->hide();

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 1 );
}

/** Autoconnected SLOTS **/
// Slot for adding a new connection
void QgsSpatiaLiteSourceSelect::on_btnNew_clicked()
{
  addNewConnection();
}

// Slot for deleting an existing connection
void QgsSpatiaLiteSourceSelect::on_btnDelete_clicked()
{
  deleteConnection();
}

// Slot for performing action when the Add button is clicked
void QgsSpatiaLiteSourceSelect::addClicked()
{
  addTables();
}

// Slot for showing help
void QgsSpatiaLiteSourceSelect::helpClicked()
{
  QgsContextHelp::run( context_id );
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsSpatiaLiteSourceSelect::on_cmbConnections_activated( int )
{
  dbChanged();
}

void QgsSpatiaLiteSourceSelect::on_mSearchOptionsButton_clicked()
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

void QgsSpatiaLiteSourceSelect::on_mSearchTableEdit_textChanged( const QString & text )
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

void QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString & text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( 0 );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( 1 );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( 2 );
  }
}

void QgsSpatiaLiteSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsSpatiaLiteSourceSelect::setLayerType( QString table, QString column, QString type )
{
  mTableModel.setGeometryTypesForTable( table, column, type );
  mTablesTreeView->sortByColumn( 0, Qt::AscendingOrder );
}

sqlite3 *QgsSpatiaLiteSourceSelect::openSpatiaLiteDb( QString path )
{
  sqlite3 *handle = NULL;
  bool gcSpatiaLite = false;
  bool rsSpatiaLite = false;
  bool tableName = false;
  bool geomColumn = false;
  bool coordDims = false;
  bool gcSrid = false;
  bool type = false;
  bool spatialIndex = false;
  bool srsSrid = false;
  bool authName = false;
  bool authSrid = false;
  bool refSysName = false;
  bool proj4text = false;
  int ret;
  const char *name;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  QString errCause;

  // trying to open the SQLite DB
  mSqlitePath = path;

  ret = sqlite3_open_v2( path.toUtf8().constData(), &handle, SQLITE_OPEN_READWRITE, NULL );
  if ( ret )
  {
    // failure
    errCause = sqlite3_errmsg( handle );
    QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ),
                           tr( "Failure while connecting to: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
    mSqlitePath = "";
    return NULL;
  }
  // checking if table GEOMETRY_COLUMNS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(geometry_columns)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      name = results[( i * columns ) + 1];
      if ( strcasecmp( name, "f_table_name" ) == 0 )
        tableName = true;
      if ( strcasecmp( name, "f_geometry_column" ) == 0 )
        geomColumn = true;
      if ( strcasecmp( name, "coord_dimension" ) == 0 )
        coordDims = true;
      if ( strcasecmp( name, "srid" ) == 0 )
        gcSrid = true;
      if ( strcasecmp( name, "type" ) == 0 )
        type = true;
      if ( strcasecmp( name, "spatial_index_enabled" ) == 0 )
        spatialIndex = true;
    }
  }
  sqlite3_free_table( results );
  if ( tableName && geomColumn && type && coordDims && gcSrid && spatialIndex )
    gcSpatiaLite = true;

  // checking if table SPATIAL_REF_SYS exists and has the expected layout
  ret = sqlite3_get_table( handle, "PRAGMA table_info(spatial_ref_sys)", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      name = results[( i * columns ) + 1];
      if ( strcasecmp( name, "srid" ) == 0 )
        srsSrid = true;
      if ( strcasecmp( name, "auth_name" ) == 0 )
        authName = true;
      if ( strcasecmp( name, "auth_srid" ) == 0 )
        authSrid = true;
      if ( strcasecmp( name, "ref_sys_name" ) == 0 )
        refSysName = true;
      if ( strcasecmp( name, "proj4text" ) == 0 )
        proj4text = true;
    }
  }
  sqlite3_free_table( results );
  if ( srsSrid && authName && authSrid && refSysName && proj4text )
    rsSpatiaLite = true;

  // OK, this one seems to be a valid SpatiaLite DB
  if ( gcSpatiaLite && rsSpatiaLite )
    return handle;

  // this one cannot be a valid SpatiaLite DB - no Spatial MetaData where found
  closeSpatiaLiteDb( handle );
  errCause = tr( "seems to be a valid SQLite DB, but not a SpatiaLite's one ..." );
  QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ),
                         tr( "Failure while connecting to: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
  mSqlitePath = "";
  return NULL;

error:
  // unexpected IO error
  closeSpatiaLiteDb( handle );
  errCause = tr( "unknown error cause" );
  if ( errMsg != NULL )
  {
    errCause = errMsg;
    sqlite3_free( errMsg );
  }
  QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ),
                         tr( "Failure while connecting to: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
  mSqlitePath = "";
  return NULL;
}

void QgsSpatiaLiteSourceSelect::closeSpatiaLiteDb( sqlite3 * handle )
{
  if ( handle )
    sqlite3_close( handle );
}

void QgsSpatiaLiteSourceSelect::populateConnectionList()
{
  QSettings settings;
  settings.beginGroup( "/SpatiaLite/connections" );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  cmbConnections->clear();
  while ( it != keys.end() )
  {
    // retrieving the SQLite DB name and full path
    QString text = *it + tr( "@" );
    text += settings.value( *it + "/sqlitepath", "###unknown###" ).toString();
    cmbConnections->addItem( text );
    ++it;
  }
  settings.endGroup();
  setConnectionListPosition();
}

void QgsSpatiaLiteSourceSelect::addNewConnection()
{
// Retrieve last used project dir from persistent settings
  sqlite3 *handle;
  QSettings settings;
  QString fullPath;
  QString lastUsedDir = settings.value( "/UI/lastSpatiaLiteDir", "." ).toString();

  QFileDialog *openFileDialog = new QFileDialog( this,
      tr( "Choose a SpatiaLite/SQLite DB to open" ),
      lastUsedDir, QObject::tr( "SQLite DB (*.sqlite);;All files (*.*)" ) );
  openFileDialog->setFileMode( QFileDialog::ExistingFile );

  if ( openFileDialog->exec() == QDialog::Accepted )
  {
    fullPath = openFileDialog->selectedFiles().first();
    QFileInfo myFI( fullPath );
    QString myPath = myFI.path();
    QString myName = myFI.fileName();

    handle = openSpatiaLiteDb( myFI.canonicalFilePath() );
    if ( handle )
    {
      // OK, this one is a valid SpatiaLite DB
      closeSpatiaLiteDb( handle );

      // Persist last used SpatiaLite dir
      settings.setValue( "/UI/lastSpatiaLiteDir", myPath );
      // inserting this SQLite DB path
      QString baseKey = "/SpatiaLite/connections/";
      settings.setValue( baseKey + "selected", myName );
      baseKey += myName;
      settings.setValue( baseKey + "/sqlitepath", fullPath );
    }
  }
  else
  {
    // if they didn't select anything, just return
    delete openFileDialog;
    return;
  }

  delete openFileDialog;
  populateConnectionList();
}

void QgsSpatiaLiteSourceSelect::deleteConnection()
{
  QSettings settings;
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( "@" );
  if ( idx > 0 )
    subKey.truncate( idx );

  QString key = "/SpatiaLite/connections/" + subKey;
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result == QMessageBox::Ok )
  {
    settings.remove( key + "/sqlitepath" );
    settings.remove( key );
    //if(!success){
    //  QMessageBox::information(this,"Unable to Remove","Unable to remove the connection " + cmbConnections->currentText());
    //}
    cmbConnections->removeItem( cmbConnections->currentIndex() ); // populateConnectionList();
    setConnectionListPosition();
  }
}

void QgsSpatiaLiteSourceSelect::addTables()
{
  m_selectedTables.clear();

  typedef QMap < int, QVector < QString > >schemaInfo;
  QMap < QString, schemaInfo > dbInfo;

  QItemSelection selection = mTablesTreeView->selectionModel()->selection();
  QModelIndexList selectedIndices = selection.indexes();
  QStandardItem *currentItem = 0;

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
      dbInfo[currentSchemaName][currentRow].resize( 5 );
    }

    dbInfo[currentSchemaName][currentRow][currentColumn] = currentItem->text();
  }

  //now traverse all the schemas and table infos
  QString tableName, geomColumnName;
  QString query;

  QMap < QString, schemaInfo >::const_iterator schema_it = dbInfo.constBegin();
  for ( ; schema_it != dbInfo.constEnd(); ++schema_it )
  {
    schemaInfo scheme = schema_it.value();
    schemaInfo::const_iterator entry_it = scheme.constBegin();
    for ( ; entry_it != scheme.constEnd(); ++entry_it )
    {
      tableName = entry_it->at( 0 );
      geomColumnName = entry_it->at( 2 );

      query = "\"" + tableName + "\" (" + geomColumnName;
      m_selectedTables.push_back( query );
    }
  }

  if ( m_selectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a Layer." ) );
  }
  else
  {
    accept();
  }
}

void QgsSpatiaLiteSourceSelect::on_btnConnect_clicked()
{
  sqlite3 *handle;

  QSettings settings;
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( "@" );
  if ( idx > 0 )
    subKey.truncate( idx );

  QFileInfo fi( settings.value( QString( "/SpatiaLite/connections/%1/sqlitepath" ).arg( subKey ) ).toString() );

  if ( !fi.exists() )
    // db doesn't exists
    return;

  // trying to connect to SpatiaLite DB
  handle = openSpatiaLiteDb( fi.canonicalFilePath() );
  if ( handle == NULL )
  {
    // unexpected error; invalid SpatiaLite DB
    return;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  // populate the table list
  // get the list of suitable tables and columns and populate the UI
  geomCol details;

  if ( !getTableInfo( handle ) )
  {
    QgsDebugMsg( QString( "Unable to get list of spatially enabled tables from the database\n%1" ).arg( sqlite3_errmsg( handle ) ) );
  }
  closeSpatiaLiteDb( handle );

  // BEGIN CHANGES ECOS
  if ( cmbConnections->count() > 0 )
    mAddButton->setEnabled( true );
  // END CHANGES ECOS

  mTablesTreeView->sortByColumn( 0, Qt::AscendingOrder );
  mTablesTreeView->header()->resizeSection( 1, 140 );
  mTablesTreeView->resizeColumnToContents( 0 );

  //expand all the toplevel items
  int numTopLevelItems = mTableModel.invisibleRootItem()->rowCount();
  for ( int i = 0; i < numTopLevelItems; ++i )
  {
    mTablesTreeView->expand( mProxyModel.mapFromSource( mTableModel.indexFromItem( mTableModel.invisibleRootItem()->child( i ) ) ) );
  }
}

QStringList QgsSpatiaLiteSourceSelect::selectedTables()
{
  return m_selectedTables;
}

QString QgsSpatiaLiteSourceSelect::connectionInfo()
{
  return mSqlitePath;
}

bool QgsSpatiaLiteSourceSelect::getTableInfo( sqlite3 * handle )
{
  int ret;
  int i;
  char **results;
  int rows;
  int columns;
  char *errMsg = NULL;
  bool ok = false;
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // setting the SQLite DB name
  QFileInfo myFI( mSqlitePath );
  QString myName = myFI.fileName();
  mTableModel.setSqliteDb( myName );

  // the following query return the tables containing a Geometry column
  ret = sqlite3_get_table( handle,
                           "SELECT f_table_name, f_geometry_column, type FROM geometry_columns", &results, &rows, &columns, &errMsg );
  if ( ret != SQLITE_OK )
    goto error;
  if ( rows < 1 )
    ;
  else
  {
    for ( i = 1; i <= rows; i++ )
    {
      QString tableName = results[( i * columns ) + 0];
      QString column = results[( i * columns ) + 1];
      QString type = results[( i * columns ) + 2];

      mTableModel.addTableEntry( type, tableName, column );
    }
    ok = true;
  }
  sqlite3_free_table( results );

  QApplication::restoreOverrideCursor();
  return ok;

error:
  // unexpected IO error
  QString errCause = tr( "unknown error cause" );
  if ( errMsg != NULL )
  {
    errCause = errMsg;
    sqlite3_free( errMsg );
  }
  QMessageBox::critical( this, tr( "SpatiaLite getTableInfo Error" ),
                         tr( "Failure exploring tables from: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
  return false;
}

QString QgsSpatiaLiteSourceSelect::fullDescription( QString table, QString column, QString type )
{
  QString full_desc = "";
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void QgsSpatiaLiteSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QSettings settings;
  settings.setValue( "/SpatiaLite/connections/selected", cmbConnections->currentText() );
}

void QgsSpatiaLiteSourceSelect::setConnectionListPosition()
{
  QSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.value( "/SpatiaLite/connections/selected" ).toString();
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

void QgsSpatiaLiteSourceSelect::setSearchExpression( const QString & regexp )
{
}
