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

#include "qgslogger.h"
#include "qgsapplication.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"
#include "qgsproviderregistry.h"

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

#ifdef HAVE_GUI
QGISEXTERN QgsSpatiaLiteSourceSelect *selectWidget( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode widgetMode )
{
  return new QgsSpatiaLiteSourceSelect( parent, fl, widgetMode );
}
#endif
QgsSpatiaLiteSourceSelect::QgsSpatiaLiteSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode ):
  QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/SpatiaLiteSourceSelect/geometry" ) ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), false ).toBool() );

  setWindowTitle( tr( "Add SpatiaLite Layer(s)" ) );
  btnEdit->hide();  // hide the edit button
  // btnSave->hide();
  btnLoad->hide();
  btnSave->setText( tr( "Create empty Database" ) );
  cmbDbCreateOption = new QComboBox( connectionsGroupBox );
  cmbDbCreateOption->setObjectName( QStringLiteral( "cmbDbCreateOption" ) );
  horizontalLayout->addWidget( cmbDbCreateOption );
  cmbDbCreateOption->addItem( tr( "Spatialite (basic)" ), SpatialiteDbInfo::Spatialite40 );
  cmbDbCreateOption->addItem( tr( "Spatialite (with Styles, Raster and VectorCoverages Table's)" ), SpatialiteDbInfo::Spatialite45 );
  cmbDbCreateOption->addItem( "GeoPackage", SpatialiteDbInfo::SpatialiteGpkg );
  cmbDbCreateOption->addItem( "MBTiles", SpatialiteDbInfo::SpatialiteMBTiles );

  mStatsButton = new QPushButton( tr( "&Update Statistics" ) );
  connect( mStatsButton, &QAbstractButton::clicked, this, &QgsSpatiaLiteSourceSelect::updateStatistics );
  mStatsButton->setEnabled( false );

  mAddButton = new QPushButton( tr( "&Add" ) );
  connect( mAddButton, &QAbstractButton::clicked, this, &QgsSpatiaLiteSourceSelect::addClicked );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsSpatiaLiteSourceSelect::buildQuery );
  mBuildQueryButton->setEnabled( false );

  if ( mWidgetMode != QgsProviderRegistry::WidgetMode::None )
  {
    buttonBox->removeButton( buttonBox->button( QDialogButtonBox::Close ) );
    mHoldDialogOpen->hide();
  }

  buttonBox->addButton( mAddButton, QDialogButtonBox::ActionRole );
  buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
  buttonBox->addButton( mStatsButton, QDialogButtonBox::ActionRole );

  populateConnectionList();

  mSearchModeComboBox->addItem( tr( "Wildcard" ) );
  mSearchModeComboBox->addItem( tr( "RegExp" ) );

  mSearchColumnComboBox->addItem( tr( "All" ) );
  // This must be keep onSync with the QgsSpatiaLiteTableModel Header-Labels
  mSearchColumnComboBox->addItem( tr( "Table" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry-Type" ) );
  mSearchColumnComboBox->addItem( tr( "Geometry column" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
  mProxyModel.setDynamicSortFilter( true );
  mProxyModel.setSourceModel( &mTableModel );
  mTablesTreeView->setModel( &mProxyModel );
  mTablesTreeView->setSortingEnabled( true );
  mTablesTreeView->header()->setResizeMode( QHeaderView::ResizeToContents );

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
  cbxAllowGeometrylessTables->setVisible( false );
  connect( this, &QgsAbstractDataSourceWidget::addDatabaseLayers, this, &QgsSpatiaLiteSourceSelect::addDatabaseSource );
}

QgsSpatiaLiteSourceSelect::~QgsSpatiaLiteSourceSelect()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/SpatiaLiteSourceSelect/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/SpatiaLiteSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );
}

void QgsSpatiaLiteSourceSelect::on_btnSave_clicked()
{
  QString sDbCreateOption = cmbDbCreateOption->currentText();
  SpatialiteDbInfo::SpatialMetadata dbCreateOption = ( SpatialiteDbInfo::SpatialMetadata )cmbDbCreateOption->currentData().toInt();
  QString sFileName = QString( "empty_Spatialite40.db" );
  QString sDbType = "Spatialite";
  QString sFileTypes = QString( "%1 (*.sqlite *.db *.sqlite3 *.db3 *.s3db *.atlas *.mbtiles *.gpkg);;%2 (*)" ).arg( sDbType ).arg( tr( "All files" ) );
  switch ( dbCreateOption )
  {
    case SpatialiteDbInfo::Spatialite45:
      sFileName = QString( "empty_Spatialite45.db" );
      break;
    case SpatialiteDbInfo::SpatialiteGpkg:
      sFileName = QString( "empty_GeoPackage.gpkg" );
      sDbType = "GeoPackage";
      break;
    case SpatialiteDbInfo::SpatialiteMBTiles:
      sFileName = QString( "empty_MBTiles.mbtiles" );
      sDbType = "MBTiles";
      break;
    case SpatialiteDbInfo::Spatialite40:
    default:
      sFileName = QString( "empty_Spatialite40.db" );
      break;
  }

  QgsSettings settings;
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::currentPath() ).toString();
  lastUsedDir = QString( "%1/%2" ).arg( lastUsedDir ).arg( sFileName );
  QString sTitle = QString( tr( "Choose a Database to create [%1]" ) ).arg( sDbCreateOption );
  QString fileSelect = QFileDialog::getSaveFileName( this, sTitle, lastUsedDir, sFileTypes );

  if ( fileSelect.isEmpty() )
  {
    return;
  }
  if ( mTableModel.createDatabase( fileSelect, dbCreateOption ) )
  {
    mTablesTreeView->setColumnHidden( mTableModel.getColumnSortHidden(), true );
    mTablesTreeView->sortByColumn( mTableModel.getColumnSortHidden(), Qt::AscendingOrder );
    mTablesTreeView->expandToDepth( 1 );
  }
}

// Slot for performing action when the Add button is clicked
void QgsSpatiaLiteSourceSelect::addClicked()
{
  addTables();
}

//! End Autoconnected SLOTS *

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
  if ( !mTableModel.isSpatialite() )
  {
    return;
  }
  QString subKey = mTableModel.getDbName( false ); // without path
  if ( collectSelectedTables() > 0 )
  {
    if ( m_selectedLayers.size() == 1 )
      subKey = QString( "%1 of %2" ).arg( m_selectedLayers.at( 0 ) ).arg( subKey );
    else
      subKey = QString( "%1 selected layers of %2" ).arg( m_selectedLayers.size() ).arg( subKey );
  }

  QString msg = tr( "Are you sure you want to update the internal statistics for DB: %1?\n\n"
                    "This could take a long time (depending on the DB size),\n"
                    "but implies better performance thereafter." ).arg( subKey );
  QMessageBox::StandardButton result =
    QMessageBox::information( this, tr( "Confirm Update Statistics" ), msg, QMessageBox::Ok | QMessageBox::Cancel );
  if ( result != QMessageBox::Ok )
    return;

  if ( mTableModel.UpdateLayerStatistics( m_selectedLayers ) )
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
#if 0
void QgsSpatiaLiteSourceSelect::on_cbxAllowGeometrylessTables_stateChanged( int )
{
  if ( cbxAllowGeometrylessTables->isEnabled() )
  {
    on_btnConnect_clicked()
  }
}
#endif

void QgsSpatiaLiteSourceSelect::on_mTablesTreeView_clicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel.mapToSource( index );
  bool bValidSelection = false;
  if ( !mTableModel.getLayerName( idx ).isEmpty() )
  {
    bValidSelection = index.parent().isValid();
  }
  // Only known and valid layers will be enabled [directories, Non-Spatial will not]
  mBuildQueryButton->setEnabled( bValidSelection );
  if ( mTableModel.isSpatialite() )
  {
    mStatsButton->setEnabled( bValidSelection );
  }
}

void QgsSpatiaLiteSourceSelect::on_mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel.mapToSource( index );
  if ( !mTableModel.getLayerName( idx ).isEmpty() )
  {
    // Only known and valid layers will be enabled [directories, Non-Spatial will not]
    setSql( index );
  }
}

void QgsSpatiaLiteSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : QLatin1String( "" ) );
}

void QgsSpatiaLiteSourceSelect::on_mSearchTableEdit_textChanged( const QString &text )
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

void QgsSpatiaLiteSourceSelect::on_mSearchColumnComboBox_currentIndexChanged( const QString &text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( mTableModel.getTableNameIndex() );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( mTableModel.getGeometryNameIndex() );
  }
  else if ( text == tr( "Geometry-Type" ) )
  {
    mProxyModel.setFilterKeyColumn( mTableModel.getGeometryTypeIndex() );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( mTableModel.getSqlQueryIndex() );
  }
}

void QgsSpatiaLiteSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
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

void QgsSpatiaLiteSourceSelect::on_btnNew_clicked()
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
  QString lastUsedDir = settings.value( QStringLiteral( "UI/lastSpatiaLiteDir" ), QDir::currentPath() ).toString();
  // RasterLite1 : sometimes used the '.atlas' extension for [Vector and Rasters]
  QString fileSelect = QFileDialog::getOpenFileName( parent,
                       tr( "Choose a SpatiaLite/SQLite DB to open" ),
                       lastUsedDir, tr( "SpatiaLite DB" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db *.atlas *.mbtiles *.gpkg);;" + tr( "All files" ) + " (*)" );

  if ( fileSelect.isEmpty() )
    return false;

  QFileInfo fileInfo( fileSelect );
  QString sPath = fileInfo.path();
  QString sName = fileInfo.fileName();
  QString savedName = fileInfo.fileName();
  QString baseKey = QStringLiteral( "/SpatiaLite/connections/" );
  // if there is already a connection with this name, ask for a new name
  while ( ! settings.value( baseKey + savedName + "/sqlitepath", "" ).toString().isEmpty() )
  {
    bool ok;
    savedName = QInputDialog::getText( nullptr, tr( "Cannot add connection '%1'" ).arg( sName ),
                                       tr( "A connection with the same name already exists,\nplease provide a new name:" ), QLineEdit::Normal,
                                       QLatin1String( "" ), &ok );
    if ( !ok || savedName.isEmpty() )
    {
      return false;
    }
  }

  // Persist last used SpatiaLite dir
  settings.setValue( QStringLiteral( "UI/lastSpatiaLiteDir" ), sPath );
  // inserting this SQLite DB path
  settings.setValue( baseKey + "selected", savedName );
  settings.setValue( baseKey + savedName + "/sqlitepath", fileInfo.canonicalFilePath() );
  return true;
}
QString QgsSpatiaLiteSourceSelect::layerUriSql( const QModelIndex &index )
{
  QString tableName = mTableModel.getTableName( index );
  QString geomColumnName = mTableModel.getGeometryName( index );
  QString sql = mTableModel.getSqlQuery( index );

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
  return sql;
}

// Slot for deleting an existing connection
void QgsSpatiaLiteSourceSelect::on_btnDelete_clicked()
{
  QString subKey = cmbConnections->currentText();
  int idx = subKey.indexOf( '@' );
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
int QgsSpatiaLiteSourceSelect::collectSelectedTables()
{
  m_selectedLayers.clear();
  m_selectedLayersSql.clear();
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
    if ( ( !currentItem ) || ( !currentItem->isSelectable() ) )
    {
      continue;
    }

    QString currentSchemaName = currentItem->parent()->text();

    int currentRow = currentItem->row();
    if ( !dbInfo[currentSchemaName].contains( currentRow ) )
    {
      dbInfo[currentSchemaName][currentRow] = true;
      m_selectedLayers.append( mTableModel.getLayerName( mProxyModel.mapToSource( *selected_it ) ) );
      m_selectedLayersSql  << layerUriSql( mProxyModel.mapToSource( *selected_it ) );
    }
  }
  return m_selectedLayers.size();
}
void QgsSpatiaLiteSourceSelect::addTables()
{
  collectSelectedTables();
  if ( m_selectedLayers.empty() )
  {
    QMessageBox::information( this, tr( "Select Table" ), tr( "You must select a table in order to add a Layer." ) );
  }
  else
  {
    addDbMapLayers( );
    if ( mWidgetMode == QgsProviderRegistry::WidgetMode::None && ! mHoldDialogOpen->isChecked() )
    {
      accept();
    }
  }
}

void QgsSpatiaLiteSourceSelect::on_btnConnect_clicked()
{
  // cbxAllowGeometrylessTables->setEnabled( false );
  // trying to connect to SpatiaLite DB
  QgsSpatiaLiteConnection connectionInfo( cmbConnections->currentText() );
  mSqlitePath = connectionInfo.dbPath();

  addDatabaseSource( QStringList( mSqlitePath ) );

}

void QgsSpatiaLiteSourceSelect::addDatabaseSource( QStringList const &paths, QString const &providerKey )
{
  Q_UNUSED( providerKey );
  QApplication::setOverrideCursor( Qt::WaitCursor );
  QString fileName = paths.at( 0 );
  bool bLoadLayers = true;
  bool bShared = true;
  SpatialiteDbInfo *spatialiteDbInfo = QgsSpatiaLiteUtils::CreateSpatialiteConnection( fileName, bLoadLayers, bShared );

  QApplication::restoreOverrideCursor();

  if ( !spatialiteDbInfo )
  {
    if ( !QFile::exists( mSqlitePath ) )
    {
      QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( "Database does not exist: %1" ).arg( mSqlitePath ) );
    }
    else
    {
      QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( " File is not a Sqlite3 Container: %1" ).arg( mSqlitePath ) );
    }
    delete spatialiteDbInfo;
    spatialiteDbInfo = nullptr;
    return;
  }
  else
  {
    if ( !spatialiteDbInfo->isDbValid() )
    {
      QMessageBox::critical( this, tr( "SpatiaLite DB Open Error" ), tr( "The read Sqlite3 Container is not supported by QgsSpatiaLiteProvider,QgsOgrProvider or QgsGdalProvider: %1" ).arg( mSqlitePath ) );
      return;
    }
    if ( fileName != mSqlitePath )
    {
      mSqlitePath = fileName;
    }
    // populate the table list
    // get the list of suitable tables and columns and populate the UI
    setSpatialiteDbInfo( spatialiteDbInfo );
  }
}
void QgsSpatiaLiteSourceSelect::setSpatialiteDbInfo( SpatialiteDbInfo *spatialiteDbInfo, bool setConnectionInfo )
{
  mTableModel.setSqliteDb( spatialiteDbInfo, true );
  mTablesTreeView->setColumnHidden( mTableModel.getColumnSortHidden(), true );
  mTablesTreeView->sortByColumn( mTableModel.getColumnSortHidden(), Qt::AscendingOrder );
  mTablesTreeView->expandToDepth( 1 );
  if ( setConnectionInfo )
  {
    mSqlitePath = spatialiteDbInfo->getDatabaseFileName();
    // TODO: add to comboBox, if not there already
  }
}

void QgsSpatiaLiteSourceSelect::setSql( const QModelIndex &index )
{
  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.getTableName( idx );
  QString sLayerNameUris = mTableModel.getLayerNameUris( idx );
  if ( sLayerNameUris.isEmpty() )
  {
    return;
  }
  QgsVectorLayer *vlayer = new QgsVectorLayer( sLayerNameUris, tableName, QStringLiteral( "spatialite" ) );

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
  QString full_desc = QLatin1String( "" );
  full_desc += table + "\" (" + column + ") " + type;
  return full_desc;
}

void QgsSpatiaLiteSourceSelect::dbChanged()
{
  // Remember which database was selected.
  QgsSettings settings;
  settings.setValue( QStringLiteral( "SpatiaLite/connections/selected" ), cmbConnections->currentText() );
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
  Q_UNUSED( regexp );
}

void QgsSpatiaLiteSourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( selected )
  Q_UNUSED( deselected )
  bool bValidSelection = false;
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
    if ( ( !currentItem ) || ( !currentItem->isSelectable() ) )
    {
      continue;
    }
    bValidSelection = true;
    break;
  }
  mAddButton->setEnabled( bValidSelection );
}
