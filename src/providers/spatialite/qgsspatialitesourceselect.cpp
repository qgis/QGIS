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
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"
#include "qgsproject.h"
#include "qgsgui.h"

#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QStringList>
#include <QPushButton>

#ifdef _MSC_VER
#define strcasecmp(a,b) stricmp(a,b)
#endif

QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  QgsGui::instance()->enableAutoGeometryRestore( this );

  connect( btnConnect, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::btnConnect_clicked );
  connect( btnNew, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::btnNew_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsSpatiaLiteSourceSelect::btnDelete_clicked );
  connect( mSearchGroupBox, &QGroupBox::toggled, this, &QgsSpatiaLiteSourceSelect::mSearchGroupBox_toggled );
  connect( mSearchTableEdit, &QLineEdit::textChanged, this, &QgsSpatiaLiteSourceSelect::mSearchTableEdit_textChanged );
  connect( mSearchColumnComboBox, &QComboBox::currentTextChanged, this, &QgsSpatiaLiteSourceSelect::mSearchColumnComboBox_currentIndexChanged );
  connect( mSearchModeComboBox, &QComboBox::currentTextChanged, this, &QgsSpatiaLiteSourceSelect::mSearchModeComboBox_currentIndexChanged );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsSpatiaLiteSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsSpatiaLiteSourceSelect::cmbConnections_activated );
  connect( mTablesTreeView, &QTreeView::clicked, this, &QgsSpatiaLiteSourceSelect::mTablesTreeView_clicked );
  connect( mTablesTreeView, &QTreeView::doubleClicked, this, &QgsSpatiaLiteSourceSelect::mTablesTreeView_doubleClicked );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsSpatiaLiteSourceSelect::showHelp );

  QgsSettings settings;
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), false ).toBool() );

  setWindowTitle( tr( "Add SpatiaLite Layer(s)" ) );
  btnEdit->hide();  // hide the edit button
  btnSave->hide();
  btnLoad->hide();

  mStatsButton = new QPushButton( tr( "&Update Statistics" ) );
  connect( mStatsButton, &QAbstractButton::clicked, this, &QgsSpatiaLiteSourceSelect::updateStatistics );
  mStatsButton->setEnabled( false );


  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsSpatiaLiteSourceSelect::buildQuery );
  mBuildQueryButton->setEnabled( false );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }

  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  buttonBox->addButton( mStatsButton, QDialogButtonBox::ActionRole );

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

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsSpatiaLiteSourceSelect::treeWidgetSelectionChanged );

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
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );
}


// Remember which database is selected
void QgsSpatiaLiteSourceSelect::cmbConnections_activated( int )
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
  int idx = subKey.indexOf( '@' );
  if ( idx > 0 )
    subKey.truncate( idx );

  QString msg = tr( "Are you sure you want to update the internal statistics for DB: %1?\n\n"
                    "This could take a long time (depending on the DB size), "
                    "but implies better performance thereafter." ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::question( this, tr( "Confirm Update Statistics" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result != QMessageBox::Yes )
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

void QgsSpatiaLiteSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  btnConnect_clicked();
}

void QgsSpatiaLiteSourceSelect::mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsSpatiaLiteSourceSelect::mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  setSql( index );
}

void QgsSpatiaLiteSourceSelect::mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : QString() );
}

void QgsSpatiaLiteSourceSelect::mSearchTableEdit_textChanged( const QString &text )
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

void QgsSpatiaLiteSourceSelect::mSearchColumnComboBox_currentIndexChanged( const QString &text )
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

void QgsSpatiaLiteSourceSelect::mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text )
  mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsSpatiaLiteSourceSelect::setLayerType( const QString &table, const QString &column, const QString &type )
{
  mTableModel.setGeometryTypesForTable( table, column, type );
  mTablesTreeView->sortByColumn( 0, Qt::AscendingOrder );
}


void QgsSpatiaLiteSourceSelect::populateConnectionList()
{
  cmbConnections->clear();
  Q_FOREACH ( const QString &name, QgsSpatiaLiteConnection::connectionList() )
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

void QgsSpatiaLiteSourceSelect::btnNew_clicked()
{
  if ( ! newConnection( this ) )
    return;
  populateConnectionList();
  emit connectionsChanged();
}

bool QgsSpatiaLiteSourceSelect::newConnection( QWidget *parent )
{
  // Retrieve last used project dir from persistent settings
  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::homePath() ).toString();

  QString myFile = QFileDialog::getOpenFileName( parent,
                   tr( "Choose a SpatiaLite/SQLite DB to open" ),
                   lastUsedDir, tr( "SpatiaLite DB" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db);;" + tr( "All files" ) + " (*)" );

  if ( myFile.isEmpty() )
    return false;

  QFileInfo myFI( myFile );
  QString myPath = myFI.path();
  QString myName = myFI.fileName();
  QString savedName = myFI.fileName();
  QString baseKey = QStringLiteral( "/SpatiaLite/connections/" );

  // TODO: keep the test
  //handle = openSpatiaLiteDb( myFI.canonicalFilePath() );
  //if ( !handle )
  //  return false;
  // OK, this one is a valid SpatiaLite DB
  //closeSpatiaLiteDb( handle );

  // if there is already a connection with this name, ask for a new name
  while ( ! settings.value( baseKey + savedName + "/sqlitepath", "" ).toString().isEmpty() )
  {
    bool ok;
    savedName = QInputDialog::getText( nullptr, tr( "Add Connection" ),
                                       tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,
                                       QString(), &ok );
    if ( !ok || savedName.isEmpty() )
    {
      return false;
    }
  }

  // Persist last used SpatiaLite dir
  settings.setValue( QStringLiteral( "UI/lastSpatiaLiteDir" ), myPath );
  // inserting this SQLite DB path
  settings.setValue( baseKey + "selected", savedName );
  settings.setValue( baseKey + savedName + "/sqlitepath", myFI.canonicalFilePath() );
  return true;
}

QString QgsSpatiaLiteSourceSelect::layerURI( const QModelIndex &index )
{
  QString tableName = mTableModel.itemFromIndex( index.sibling( index.row(), 0 ) )->text();
  QString geomColumnName = mTableModel.itemFromIndex( index.sibling( index.row(), 2 ) )->text();
  QString sql = mTableModel.itemFromIndex( index.sibling( index.row(), 3 ) )->text();

  if ( geomColumnName.contains( QLatin1String( " AS " ) ) )
  {
    int a = geomColumnName.indexOf( QLatin1String( " AS " ) );
    QString typeName = geomColumnName.mid( a + 4 ); //only the type name
    geomColumnName = geomColumnName.left( a ); //only the geom column name
    QString geomFilter;

    if ( typeName == QLatin1String( "POINT" ) )
    {
      geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('POINT','MULTIPOINT')" ).arg( geomColumnName );
    }
    else if ( typeName == QLatin1String( "LINESTRING" ) )
    {
      geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('LINESTRING','MULTILINESTRING')" ).arg( geomColumnName );
    }
    else if ( typeName == QLatin1String( "POLYGON" ) )
    {
      geomFilter = QStringLiteral( "geometrytype(\"%1\") IN ('POLYGON','MULTIPOLYGON')" ).arg( geomColumnName );
    }

    if ( !geomFilter.isEmpty() && !sql.contains( geomFilter ) )
    {
      if ( !sql.isEmpty() )
      {
        sql += QLatin1String( " AND " );
      }

      sql += geomFilter;
    }
  }

  QgsDataSourceUri uri( connectionInfo() );
  uri.setDataSource( QString(), tableName, geomColumnName, sql, QString() );
  return uri.uri();
}

// Slot for deleting an existing connection
void QgsSpatiaLiteSourceSelect::btnDelete_clicked()
{
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( '@' );
  if ( idx > 0 )
    subKey.truncate( idx );

  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No );
  if ( result != QMessageBox::Yes )
    return;

  QgsSpatiaLiteConnection::deleteConnection( subKey );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsSpatiaLiteSourceSelect::addButtonClicked()
{
  m_selectedTables.clear();

  typedef QMap < int, bool >schemaInfo;
  QMap < QString, schemaInfo > dbInfo;

  QItemSelection selection = mTablesTreeView->selectionModel()->selection();
  QModelIndexList selectedIndices = selection.indexes();
  QStandardItem *currentItem = nullptr;

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
    emit addDatabaseLayers( m_selectedTables, QStringLiteral( "spatialite" ) );
    if ( widgetMode() == QgsProviderRegistry::WidgetMode::None && ! mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }
}

void QgsSpatiaLiteSourceSelect::btnConnect_clicked()
{
  cbxAllowGeometrylessTables->setEnabled( false );

  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( '@' );
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
                               tr( "Failure while connecting to: %1\n\n%2" ).arg( mSqlitePath, errCause ) );
        break;
      case QgsSpatiaLiteConnection::FailedToGetTables:
        QMessageBox::critical( this, tr( "SpatiaLite getTableInfo Error" ),
                               tr( "Failure exploring tables from: %1\n\n%2" ).arg( mSqlitePath, errCause ) );
        break;
      case QgsSpatiaLiteConnection::FailedToCheckMetadata:
        QMessageBox::critical( this, tr( "SpatiaLite metadata check failed" ),
                               tr( "Failure getting table metadata. Is %1 really a SpatiaLite database?\n\n%2" ).arg( mSqlitePath, errCause ) );
        break;
      default:
        QMessageBox::critical( this, tr( "SpatiaLite Error" ),
                               tr( "Unexpected error when working with %1\n\n%2" ).arg( mSqlitePath, errCause ) );
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
  const auto constTables = tables;
  for ( const QgsSpatiaLiteConnection::TableEntry &table : constTables )
  {
    mTableModel.addTableEntry( table.type, table.tableName, table.column, QString() );
  }

  if ( cmbConnections->count() > 0 )
  {
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
  return QStringLiteral( "dbname='%1'" ).arg( QString( mSqlitePath ).replace( '\'', QLatin1String( "\\'" ) ) );
}

void QgsSpatiaLiteSourceSelect::setSql( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), 0 ) )->text();

  const QgsVectorLayer::LayerOptions options { QgsProject::instance()->transformContext() };
  QgsVectorLayer *vlayer = new QgsVectorLayer( layerURI( idx ), tableName, QStringLiteral( "spatialite" ), options );

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

QString QgsSpatiaLiteSourceSelect::fullDescription( const QString &table, const QString &column, const QString &type )
{
  QString full_desc;
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void QgsSpatiaLiteSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QgsSettings settings;
  settings.setValue( QStringLiteral( "SpatiaLite/connections/selected" ), cmbConnections->currentText() );
}

void QgsSpatiaLiteSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsSpatiaLiteSourceSelect::setConnectionListPosition()
{
  QgsSettings settings;
  // If possible, set the item currently displayed database
  QString toSelect = settings.value( QStringLiteral( "SpatiaLite/connections/selected" ) ).toString();

  toSelect += '@' + settings.value( "/SpatiaLite/connections/" + toSelect + "/sqlitepath" ).toString();

  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsSpatiaLiteSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp )
}

void QgsSpatiaLiteSourceSelect::treeWidgetSelectionChanged( const QItemSelection &, const QItemSelection & )
{
  emit enableButtons( !mTablesTreeView->selectionModel()->selection().isEmpty() );
}

void QgsSpatiaLiteSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#spatialite-layers" ) );
}
