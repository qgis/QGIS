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
#include "qgsmssqlgeomcolumntypethread.h"
#include "qgsmssqlprovider.h"
#include "qgsmssqlnewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgssettings.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlError>

//! Used to create an editor for when the user tries to change the contents of a cell
QWidget *QgsMssqlSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  if ( index.column() == QgsMssqlTableModel::DbtmSql )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setText( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  if ( index.column() == QgsMssqlTableModel::DbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    Q_FOREACH ( QgsWkbTypes::Type type,
                QList<QgsWkbTypes::Type>()
                << QgsWkbTypes::Point
                << QgsWkbTypes::LineString
                << QgsWkbTypes::Polygon
                << QgsWkbTypes::MultiPoint
                << QgsWkbTypes::MultiLineString
                << QgsWkbTypes::MultiPolygon
                << QgsWkbTypes::NoGeometry )
    {
      cb->addItem( QgsMssqlTableModel::iconForWkbType( type ), QgsWkbTypes::displayString( type ), type );
    }
    cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );
    return cb;
  }

  if ( index.column() == QgsMssqlTableModel::DbtmPkCol )
  {
    QStringList values = index.data( Qt::UserRole + 1 ).toStringList();

    if ( !values.isEmpty() )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->addItems( values );
      cb->setCurrentIndex( cb->findText( index.data( Qt::DisplayRole ).toString() ) );
      return cb;
    }
  }

  if ( index.column() == QgsMssqlTableModel::DbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    le->insert( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  return nullptr;
}

void QgsMssqlSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsMssqlTableModel::DbtmType )
    {
      QgsWkbTypes::Type type = static_cast< QgsWkbTypes::Type >( cb->currentData().toInt() );

      model->setData( index, QgsMssqlTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QgsWkbTypes::Unknown ? QgsWkbTypes::displayString( type ) : tr( "Select…" ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsMssqlTableModel::DbtmPkCol )
    {
      model->setData( index, cb->currentText() );
      model->setData( index, cb->currentText(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
    model->setData( index, le->text() );
}

QgsMssqlSourceSelect::QgsMssqlSourceSelect( QWidget *parent, Qt::WindowFlags fl, QgsProviderRegistry::WidgetMode theWidgetMode )
  : QgsAbstractDataSourceWidget( parent, fl, theWidgetMode )
{
  setupUi( this );
  connect( btnConnect, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnConnect_clicked );
  connect( cbxAllowGeometrylessTables, &QCheckBox::stateChanged, this, &QgsMssqlSourceSelect::cbxAllowGeometrylessTables_stateChanged );
  connect( btnNew, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnNew_clicked );
  connect( btnEdit, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnEdit_clicked );
  connect( btnDelete, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnDelete_clicked );
  connect( btnSave, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnSave_clicked );
  connect( btnLoad, &QPushButton::clicked, this, &QgsMssqlSourceSelect::btnLoad_clicked );
  connect( mSearchGroupBox, &QGroupBox::toggled, this, &QgsMssqlSourceSelect::mSearchGroupBox_toggled );
  connect( mSearchTableEdit, &QLineEdit::textChanged, this, &QgsMssqlSourceSelect::mSearchTableEdit_textChanged );
  connect( mSearchColumnComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsMssqlSourceSelect::mSearchColumnComboBox_currentIndexChanged );
  connect( mSearchModeComboBox, static_cast<void ( QComboBox::* )( const QString & )>( &QComboBox::currentIndexChanged ), this, &QgsMssqlSourceSelect::mSearchModeComboBox_currentIndexChanged );
  connect( cmbConnections, static_cast<void ( QComboBox::* )( int )>( &QComboBox::activated ), this, &QgsMssqlSourceSelect::cmbConnections_activated );
  connect( mTablesTreeView, &QTreeView::clicked, this, &QgsMssqlSourceSelect::mTablesTreeView_clicked );
  connect( mTablesTreeView, &QTreeView::doubleClicked, this, &QgsMssqlSourceSelect::mTablesTreeView_doubleClicked );
  setupButtons( buttonBox );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMssqlSourceSelect::showHelp );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::None )
  {
    mHoldDialogOpen->hide();
  }
  else
  {
    setWindowTitle( tr( "Add MSSQL Table(s)" ) );
  }

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
  mBuildQueryButton->setDisabled( true );

  if ( widgetMode() != QgsProviderRegistry::WidgetMode::Manager )
  {

    buttonBox->addButton( mBuildQueryButton, QDialogButtonBox::ActionRole );
    connect( mBuildQueryButton, &QAbstractButton::clicked, this, &QgsMssqlSourceSelect::buildQuery );
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

  connect( mTablesTreeView->selectionModel(), &QItemSelectionModel::selectionChanged, this, &QgsMssqlSourceSelect::treeWidgetSelectionChanged );

  QgsSettings settings;
  mTablesTreeView->setSelectionMode( settings.value( QStringLiteral( "qgis/addMSSQLDC" ), false ).toBool() ?
                                     QAbstractItemView::ExtendedSelection :
                                     QAbstractItemView::MultiSelection );


  //for Qt < 4.3.2, passing -1 to include all model columns
  //in search does not seem to work
  mSearchColumnComboBox->setCurrentIndex( 2 );

  restoreGeometry( settings.value( QStringLiteral( "Windows/MSSQLSourceSelect/geometry" ) ).toByteArray() );
  mHoldDialogOpen->setChecked( settings.value( QStringLiteral( "Windows/MSSQLSourceSelect/HoldDialogOpen" ), false ).toBool() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    mTablesTreeView->setColumnWidth( i, settings.value( QStringLiteral( "Windows/MSSQLSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) ).toInt() );
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

void QgsMssqlSourceSelect::btnNew_clicked()
{
  QgsMssqlNewConnection nc( this );
  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsMssqlSourceSelect::btnDelete_clicked()
{
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Yes != QMessageBox::question( this, tr( "Confirm Delete" ), msg, QMessageBox::Yes | QMessageBox::No ) )
    return;

  QgsMssqlSourceSelect::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsMssqlSourceSelect::deleteConnection( const QString &name )
{
  QString key = "/MSSQL/connections/" + name;
  QgsSettings settings;
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

void QgsMssqlSourceSelect::btnSave_clicked()
{
  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Export, QgsManageConnectionsDialog::MSSQL );
  dlg.exec();
}

void QgsMssqlSourceSelect::btnLoad_clicked()
{
  QString fileName = QFileDialog::getOpenFileName( this, tr( "Load Connections" ), QDir::homePath(),
                     tr( "XML files (*.xml *.XML)" ) );
  if ( fileName.isEmpty() )
  {
    return;
  }

  QgsManageConnectionsDialog dlg( this, QgsManageConnectionsDialog::Import, QgsManageConnectionsDialog::MSSQL, fileName );
  dlg.exec();
  populateConnectionList();
}

void QgsMssqlSourceSelect::btnEdit_clicked()
{
  QgsMssqlNewConnection nc( this, cmbConnections->currentText() );
  if ( nc.exec() )
  {
    populateConnectionList();
    emit connectionsChanged();
  }
}

void QgsMssqlSourceSelect::cmbConnections_activated( int )
{
  // Remember which database was selected.
  QgsSettings settings;
  settings.setValue( QStringLiteral( "MSSQL/connections/selected" ), cmbConnections->currentText() );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( settings.value( "/MSSQL/connections/" + cmbConnections->currentText() + "/allowGeometrylessTables", false ).toBool() );
  cbxAllowGeometrylessTables->blockSignals( false );
}

void QgsMssqlSourceSelect::cbxAllowGeometrylessTables_stateChanged( int )
{
  btnConnect_clicked();
}

void QgsMssqlSourceSelect::buildQuery()
{
  setSql( mTablesTreeView->currentIndex() );
}

void QgsMssqlSourceSelect::mTablesTreeView_clicked( const QModelIndex &index )
{
  mBuildQueryButton->setEnabled( index.parent().isValid() );
}

void QgsMssqlSourceSelect::mTablesTreeView_doubleClicked( const QModelIndex &index )
{
  QgsSettings settings;
  if ( settings.value( QStringLiteral( "qgis/addMSSQLDC" ), false ).toBool() )
  {
    addButtonClicked();
  }
  else
  {
    setSql( index );
  }
}

void QgsMssqlSourceSelect::mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : QString() );
}

void QgsMssqlSourceSelect::mSearchTableEdit_textChanged( const QString &text )
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

void QgsMssqlSourceSelect::mSearchColumnComboBox_currentIndexChanged( const QString &text )
{
  if ( text == tr( "All" ) )
  {
    mProxyModel.setFilterKeyColumn( -1 );
  }
  else if ( text == tr( "Schema" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsMssqlTableModel::DbtmSql );
  }
}

void QgsMssqlSourceSelect::mSearchModeComboBox_currentIndexChanged( const QString &text )
{
  Q_UNUSED( text );
  mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsMssqlSourceSelect::setLayerType( const QgsMssqlLayerProperty &layerProperty )
{
  mTableModel.setGeometryTypesForTable( layerProperty );
}

QgsMssqlSourceSelect::~QgsMssqlSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
  }

  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/MSSQLSourceSelect/geometry" ), saveGeometry() );
  settings.setValue( QStringLiteral( "Windows/MSSQLSourceSelect/HoldDialogOpen" ), mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QStringLiteral( "Windows/MSSQLSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsMssqlSourceSelect::populateConnectionList()
{
  QgsSettings settings;
  settings.beginGroup( QStringLiteral( "MSSQL/connections" ) );
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
void QgsMssqlSourceSelect::addButtonClicked()
{
  QgsDebugMsg( QStringLiteral( "mConnInfo:%1" ).arg( mConnInfo ) );
  mSelectedTables.clear();

  const QModelIndexList selection = mTablesTreeView->selectionModel()->selection().indexes();
  for ( const QModelIndex &idx : selection )
  {
    if ( idx.column() != QgsMssqlTableModel::DbtmTable )
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
    emit addDatabaseLayers( mSelectedTables, QStringLiteral( "mssql" ) );
    if ( !mHoldDialogOpen->isChecked() && widgetMode() == QgsProviderRegistry::WidgetMode::None )
    {
      accept();
    }
  }
}

void QgsMssqlSourceSelect::btnConnect_clicked()
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
  QgsSettings settings;
  QString key = "/MSSQL/connections/" + cmbConnections->currentText();
  QString service = settings.value( key + "/service" ).toString();
  QString host = settings.value( key + "/host" ).toString();
  QString database = settings.value( key + "/database" ).toString();
  QString username;
  QString password;
  if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
  {
    username = settings.value( key + "/username" ).toString();
  }

  if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
  {
    password = settings.value( key + "/password" ).toString();
  }

  bool useGeometryColumns = settings.value( key + "/geometryColumns", false ).toBool();

  bool allowGeometrylessTables = cbxAllowGeometrylessTables->isChecked();

  bool estimateMetadata = settings.value( key + "/estimatedMetadata", true ).toBool();

  mConnInfo = "dbname='" + database + '\'';
  if ( !host.isEmpty() )
    mConnInfo += " host='" + host + '\'';
  if ( !username.isEmpty() )
    mConnInfo += " user='" + username + '\'';
  if ( !password.isEmpty() )
    mConnInfo += " password='" + password + '\'';
  if ( !service.isEmpty() )
    mConnInfo += " service='" + service + '\'';

  QgsDebugMsg( QStringLiteral( "GetDatabase" ) );
  QSqlDatabase db = QgsMssqlProvider::GetDatabase( service, host, database, username, password );

  if ( !QgsMssqlProvider::OpenDatabase( db ) )
  {
    // Let user know we couldn't initialize the MSSQL provider
    QMessageBox::warning( this,
                          tr( "MSSQL Provider" ), db.lastError().text() );
    return;
  }

  QString connectionName = db.connectionName();

  // Test for geometry columns table first.  Don't use it if not found.
  QSqlQuery q = QSqlQuery( db );
  q.setForwardOnly( true );

  if ( useGeometryColumns )
  {
    QString testquery( QStringLiteral( "SELECT count(*) FROM INFORMATION_SCHEMA.TABLES WHERE TABLE_NAME = 'geometry_columns'" ) );
    if ( !q.exec( testquery ) || !q.first() || q.value( 0 ).toInt() == 0 )
    {
      QMessageBox::StandardButtons reply;
      reply = QMessageBox::question( this, QStringLiteral( "Scan full database?" ),
                                     QStringLiteral( "No geometry_columns table found. \nWould you like to search full database (might be slower)? " ),
                                     QMessageBox::Yes | QMessageBox::No
                                   );
      if ( reply == QMessageBox::Yes )
        useGeometryColumns = false;
      else
        return;
    }
  }

  // Read supported layers from database
  QApplication::setOverrideCursor( Qt::WaitCursor );

  // build sql statement
  QString query( QStringLiteral( "select " ) );
  if ( useGeometryColumns )
  {
    query += QLatin1String( "f_table_schema, f_table_name, f_geometry_column, srid, geometry_type from geometry_columns" );
  }
  else
  {
    query += QLatin1String( "sys.schemas.name, sys.objects.name, sys.columns.name, null, 'GEOMETRY' from sys.columns join sys.types on sys.columns.system_type_id = sys.types.system_type_id and sys.columns.user_type_id = sys.types.user_type_id join sys.objects on sys.objects.object_id = sys.columns.object_id join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and (sys.objects.type = 'U' or sys.objects.type = 'V')" );
  }

  if ( allowGeometrylessTables )
  {
    query += QLatin1String( " union all select sys.schemas.name, sys.objects.name, null, null, 'NONE' from sys.objects join sys.schemas on sys.objects.schema_id = sys.schemas.schema_id where not exists (select * from sys.columns sc1 join sys.types on sc1.system_type_id = sys.types.system_type_id where (sys.types.name = 'geometry' or sys.types.name = 'geography') and sys.objects.object_id = sc1.object_id) and (sys.objects.type = 'U' or sys.objects.type = 'V')" );
  }

  // issue the sql query
  q = QSqlQuery( db );
  q.setForwardOnly( true );
  ( void )q.exec( query );

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
      layer.isGeography = false;

      QString type = layer.type;
      QString srid = layer.srid;

      if ( !layer.geometryColName.isNull() )
      {
        if ( type == QLatin1String( "GEOMETRY" ) || type.isNull() || srid.isEmpty() )
        {
          addSearchGeometryColumn( connectionName, layer, estimateMetadata );
          type.clear();
          srid.clear();
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
                          tr( "MSSQL Provider" ), q.lastError().text() );
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

  mTablesTreeView->sortByColumn( QgsMssqlTableModel::DbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsMssqlTableModel::DbtmSchema, Qt::AscendingOrder );
}

void QgsMssqlSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = nullptr;
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

void QgsMssqlSourceSelect::refresh()
{
  populateConnectionList();
}

void QgsMssqlSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( QStringLiteral( "schema item found" ) );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsMssqlTableModel::DbtmTable ) )->text();

  std::unique_ptr< QgsVectorLayer > vlayer = qgis::make_unique< QgsVectorLayer>( mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata ), tableName, QStringLiteral( "mssql" ) );

  if ( !vlayer->isValid() )
  {
    return;
  }

  // create a query builder object
  QgsQueryBuilder gb( vlayer.get(), this );
  if ( gb.exec() )
  {
    mTableModel.setSql( mProxyModel.mapToSource( index ), gb.sql() );
  }
}

void QgsMssqlSourceSelect::addSearchGeometryColumn( const QString &connectionName, const QgsMssqlLayerProperty &layerProperty, bool estimateMetadata )
{
  // store the column details and do the query in a thread
  if ( !mColumnTypeThread )
  {
    mColumnTypeThread = new QgsMssqlGeomColumnTypeThread( connectionName, estimateMetadata );

    connect( mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::setLayerType,
             this, &QgsMssqlSourceSelect::setLayerType );
    connect( this, &QgsMssqlSourceSelect::addGeometryColumn,
             mColumnTypeThread, &QgsMssqlGeomColumnTypeThread::addGeometryColumn );
    connect( mColumnTypeThread, &QThread::finished,
             this, &QgsMssqlSourceSelect::columnThreadFinished );

  }

  emit addGeometryColumn( layerProperty );
}

QString QgsMssqlSourceSelect::fullDescription( const QString &schema, const QString &table, const QString &column, const QString &type )
{
  QString full_desc;
  if ( !schema.isEmpty() )
    full_desc = schema + '.';
  full_desc += table + " (" + column + ") " + type;
  return full_desc;
}

void QgsMssqlSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QgsSettings settings;
  QString toSelect = settings.value( QStringLiteral( "MSSQL/connections/selected" ) ).toString();
  cmbConnections->setCurrentIndex( cmbConnections->findText( toSelect ) );

  if ( cmbConnections->currentIndex() < 0 )
  {
    if ( toSelect.isNull() )
      cmbConnections->setCurrentIndex( 0 );
    else
      cmbConnections->setCurrentIndex( cmbConnections->count() - 1 );
  }
}

void QgsMssqlSourceSelect::setSearchExpression( const QString &regexp )
{
  Q_UNUSED( regexp );
}

void QgsMssqlSourceSelect::treeWidgetSelectionChanged( const QItemSelection &selected, const QItemSelection &deselected )
{
  Q_UNUSED( deselected )
  emit enableButtons( !selected.isEmpty() );
}

void QgsMssqlSourceSelect::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#loading-a-database-layer" ) );
}
