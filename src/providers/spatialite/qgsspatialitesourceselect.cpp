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
#include "qgsspatialiteconnection.h"

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgscontexthelp.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect( QWidget * parent, Qt::WindowFlags fl, bool embedded ):
    QDialog( parent, fl )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/SpatiaLiteSourceSelect/geometry" ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( "/Windows/SpatiaLiteSourceSelect/HoldDialogOpen", false ).toBool() );

  setWindowTitle( tr( "Add SpatiaLite Table(s)" ) );
  connectionsGroupBox->setTitle( tr( "Databases" ) );
  btnEdit->hide();  // hide the edit button
  btnSave->hide();
  btnLoad->hide();

  mStatsButton = new QPushButton( tr( "&Update statistics" ) );
  connect( mStatsButton, SIGNAL( clicked() ), this, SLOT( updateStatistics() ) );
  mStatsButton->setEnabled( false );

  mAddButton = new QPushButton( tr( "&Add" ) );
  connect( mAddButton, SIGNAL( clicked() ), this, SLOT( addClicked() ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  connect( mBuildQueryButton, SIGNAL( clicked() ), this, SLOT( buildQuery() ) );
  mBuildQueryButton->setEnabled( false );

  if ( embedded )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }
  else
  {
    buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    buttonBox->addButton( mStatsButton, QDialogButtonBox::ActionRole );
  }

  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setDynamicSortFilter( true );
  mProxyModel.setSourceModel( &mTableModel );
  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );

  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 1 );

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

QgsSpatiaLiteSourceSelect::~QgsSpatiaLiteSourceSelect()
{
  QSettings settings;
  settings.setValue( "/Windows/SpatiaLiteSourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/SpatiaLiteSourceSelect/HoldDialogOpen", mHoldDialogOpen->isChecked() );
}

// Slot for performing action when the Add button is clicked
void QgsSpatiaLiteSourceSelect::addClicked()
{
  addTables();
}

/** End Autoconnected SLOTS **/

// Remember which database is selected
void QgsSpatiaLiteSourceSelect::on_cmbConnections_activated( int )
{
  dbChanged();
}

void QgsSpatiaLiteSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsSpatiaLiteSourceSelect::updateStatistics()
{
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( "@" );
  if ( idx > 0 )
    subKey.truncate( idx );

  QString msg = tr( "Are you sure you want to update the internal statistics for DB: %1?\n\n"
                    "This could take a long time (depending on the DB size),\n"
                    "but implies better performance thereafter." ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::information( this, tr( "Confirm Update Statistics" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result != QMessageBox::Ok )
    return;

  // trying to connect to SpatiaLite DB
  QgsSpatiaLiteConnection conn( subKey );
  if ( conn.updateStatistics() )
  {
    QMessageBox::information( this, tr( "Update Statistics" ),
                              tr( "Internal statistics successfully updated for: %1" ).arg( subKey ) );
  }
  else
  {
    QMessageBox::critical( this, tr( "Update Statistics" ),
                           tr( "Error while updating internal statistics for: %1" ).arg( subKey ) );
  }
}

void QgsSpatiaLiteSourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  on_btnConnect_clicked();
}

void QgsSpatiaLiteSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsSpatiaLiteSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  setSql( index );
}

void QgsSpatiaLiteSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : "" );
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
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( 3 );
  }
}

void QgsSpatiaLiteSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsSpatiaLiteSourceSelect::setLayerType( QString table, QString column, QString type )
{
  mTableModel.setGeometryTypesForTable( table, column, type );
  mTablesTreeView->sortByColumn( 0, Qt::AscendingOrder );
}


void QgsSpatiaLiteSourceSelect::populateConnectionList()
{
  cmbConnections->clear();
  foreach ( QString name, QgsSpatiaLiteConnection::connectionList() )
  {
    // retrieving the SQLite DB name and full path
    QString text = name + tr( "@" ) + QgsSpatiaLiteConnection::connectionPath( name );
    cmbConnections->addItem( text );
  }
  setConnectionListPosition();

  btnConnect->setDisabled( cmbConnections->count() == 0 );
  btnDelete->setDisabled( cmbConnections->count() == 0 );

  cmbConnections->setDisabled( cmbConnections->count() == 0 );
}

void QgsSpatiaLiteSourceSelect::on_btnNew_clicked()
{
  if ( ! newConnection( this ) )
    return;
  populateConnectionList();
  emit connectionsChanged();
}

bool QgsSpatiaLiteSourceSelect::newConnection( QWidget* parent )
{
  // Retrieve last used project dir from persistent settings
  QSettings settings;
  QString lastUsedDir = settings.value( "/UI/lastSpatiaLiteDir", "." ).toString();

  QString myFile = QFileDialog::getOpenFileName( parent,
                   tr( "Choose a SpatiaLite/SQLite DB to open" ),
                   lastUsedDir, tr( "SpatiaLite DB" ) + " (*.sqlite *.db);;" + tr( "All files" ) + " (*)" );

  if ( myFile.isEmpty() )
    return false;

  QFileInfo myFI( myFile );
  QString myPath = myFI.path();
  QString myName = myFI.fileName();
  QString baseKey = "/SpatiaLite/connections/";

  // TODO: keep the test
  //handle = openSpatiaLiteDb( myFI.canonicalFilePath() );
  //if ( !handle )
  //  return false;
  // OK, this one is a valid SpatiaLite DB
  //closeSpatiaLiteDb( handle );

  // if there is already a connection with this name, warn user (#9404) and do nothing
  // ideally, user should be able to change item name so that several sqlite files with same name can co-exist
  if ( ! settings.value( baseKey + myName + "/sqlitepath", "" ).toString().isEmpty() )
  {
    QMessageBox::critical( parent, tr( "Error" ), tr( "Cannot add connection '%1' : a connection with the same name already exists." ).arg( myName ) );
    return false;
  }

  // Persist last used SpatiaLite dir
  settings.setValue( "/UI/lastSpatiaLiteDir", myPath );
  // inserting this SQLite DB path
  settings.setValue( baseKey + "selected", myName );
  settings.setValue( baseKey + myName + "/sqlitepath", myFI.canonicalFilePath() );
  return true;
}

QString QgsSpatiaLiteSourceSelect::layerURI( const QModelIndex &index )
{
  QString tableName = mTableModel.itemFromIndex( index.sibling( index.row(), 0 ) )->text();
  QString geomColumnName = mTableModel.itemFromIndex( index.sibling( index.row(), 2 ) )->text();
  QString sql = mTableModel.itemFromIndex( index.sibling( index.row(), 3 ) )->text();

  if ( geomColumnName.contains( " AS " ) )
  {
    int a = geomColumnName.indexOf( " AS " );
    QString typeName = geomColumnName.mid( a + 4 ); //only the type name
    geomColumnName = geomColumnName.left( a ); //only the geom column name
    QString geomFilter;

    if ( typeName == "POINT" )
    {
      geomFilter = QString( "geometrytype(\"%1\") IN ('POINT','MULTIPOINT')" ).arg( geomColumnName );
    }
    else if ( typeName == "LINESTRING" )
    {
      geomFilter = QString( "geometrytype(\"%1\") IN ('LINESTRING','MULTILINESTRING')" ).arg( geomColumnName );
    }
    else if ( typeName == "POLYGON" )
    {
      geomFilter = QString( "geometrytype(\"%1\") IN ('POLYGON','MULTIPOLYGON')" ).arg( geomColumnName );
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

  QgsDataSourceURI uri( connectionInfo() );
  uri.setDataSource( "", tableName, geomColumnName, sql, "" );
  return uri.uri();
}

// Slot for deleting an existing connection
void QgsSpatiaLiteSourceSelect::on_btnDelete_clicked()
{
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( "@" );
  if ( idx > 0 )
    subKey.truncate( idx );

  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result != QMessageBox::Ok )
    return;

  QgsSpatiaLiteConnection::deleteConnection( subKey );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsSpatiaLiteSourceSelect::addTables()
{
  m_selectedTables.clear();

  typedef QMap < int, bool >schemaInfo;
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
    if ( !dbInfo[currentSchemaName].contains( currentRow ) )
    {
      dbInfo[currentSchemaName][currentRow] = true;
      m_selectedTables << layerURI( mProxyModel.mapToSource( *selected_it ) );
    }
  }

  if ( m_selectedTables.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a Layer." ) );
  }
  else
  {
    emit addDatabaseLayers( m_selectedTables, "spatialite" );
    if ( !mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }
}

void QgsSpatiaLiteSourceSelect::on_btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( false );

  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( "@" );
  if ( idx > 0 )
    subKey.truncate( idx );

  // trying to connect to SpatiaLite DB
  QgsSpatiaLiteConnection conn( subKey );
  mSqlitePath = conn.path();

  QApplication::setOverrideCursor( Qt::WaitCursor );

  QgsSpatiaLiteConnection::Error err;
  err = conn.fetchTables( cbxAllowGeometrylessTables->isChecked() );

  QApplication::restoreOverrideCursor();

  if ( err != QgsSpatiaLiteConnection::NoError )
  {
    QString errCause = conn.errorMessage();
    switch ( err )
    {
      case QgsSpatiaLiteConnection::NotExists:
        QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ),
                               tr( "Database does not exist: %1" ).arg( mSqlitePath ) );
        break;
      case QgsSpatiaLiteConnection::FailedToOpen:
        QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ),
                               tr( "Failure while connecting to: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
        break;
      case QgsSpatiaLiteConnection::FailedToGetTables:
        QMessageBox::critical( this, tr( "SpatiaLite getTableInfo Error" ),
                               tr( "Failure exploring tables from: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
        break;
      default:
        QMessageBox::critical( this, tr( "SpatiaLite Error" ),
                               tr( "Unexpected error when working with: %1\n\n%2" ).arg( mSqlitePath ).arg( errCause ) );
    }
    mSqlitePath = QString();
    return;
  }

  QModelIndex rootItemIndex = mTableModel.indexFromItem( mTableModel.invisibleRootItem() );
  mTableModel.removeRows( 0, mTableModel.rowCount( rootItemIndex ), rootItemIndex );

  // populate the table list
  // get the list of suitable tables and columns and populate the UI

  mTableModel.setSqliteDb( subKey );

  QList<QgsSpatiaLiteConnection::TableEntry> tables = conn.tables();
  foreach ( const QgsSpatiaLiteConnection::TableEntry& table, tables )
  {
    mTableModel.addTableEntry( table.type, table.tableName, table.column, "" );
  }

  if ( cmbConnections->count() > 0 )
  {
    mAddButton->setEnabled( true );
    mStatsButton->setEnabled( true );
  }

  mTablesTreeView->sortByColumn( 0, Qt::AscendingOrder );

  //expand all the toplevel items
  int numTopLevelItems = mTableModel.invisibleRootItem()->rowCount();
  for ( int i = 0; i < numTopLevelItems; ++i )
  {
    mTablesTreeView->expand( mProxyModel.mapFromSource( mTableModel.indexFromItem( mTableModel.invisibleRootItem()->child( i ) ) ) );
  }
  mTablesTreeView->resizeColumnToContents( 0 );
  mTablesTreeView->resizeColumnToContents( 1 );

  cbxAllowGeometrylessTables->setEnabled( true );
}

QStringList QgsSpatiaLiteSourceSelect::selectedTables()
{
  return m_selectedTables;
}

QString QgsSpatiaLiteSourceSelect::connectionInfo()
{
  return QString( "dbname='%1'" ).arg( QString( mSqlitePath ).replace( "'", "\\'" ) );
}

void QgsSpatiaLiteSourceSelect::setSql( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), 0 ) )->text();

  QgsVectorLayer *vlayer = new QgsVectorLayer( layerURI( idx ), tableName, "spatialite" );

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

  toSelect += "@" + settings.value( "/SpatiaLite/connections/" + toSelect + "/sqlitepath" ).toString();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsSpatiaLiteSourceSelect::setSearchExpression( const QString & regexp )
{
  Q_UNUSED( regexp );
}
