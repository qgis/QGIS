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
#include "qgspostgresprovider.h"
#include "qgspgnewconnection.h"
#include "qgsmanageconnectionsdialog.h"
#include "qgsquerybuilder.h"
#include "qgsdatasourceuri.h"
#include "qgsvectorlayer.h"
#include "qgscolumntypethread.h"

#include <QFileDialog>
#include <QInputDialog>
#include <QMessageBox>
#include <QSettings>
#include <QTextStream>
#include <QHeaderView>
#include <QStringList>

/** Used to create an editor for when the user tries to change the contents of a cell */
QWidget *QgsPgSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );
  if ( index.column() == QgsPgTableModel::dbtmSql )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setText( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  if ( index.column() == QgsPgTableModel::dbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    foreach( QGis::GeometryType type,
             QList<QGis::GeometryType>()
             << QGis::Point
             << QGis::Line
             << QGis::Polygon
             << QGis::NoGeometry )
    {
      cb->addItem( QgsPgTableModel::iconForGeomType( type ), QgsPostgresConn::displayStringForGeomType( type ), type );
    }
    cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );
    return cb;
  }

  if ( index.column() == QgsPgTableModel::dbtmPkCol )
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

  if ( index.column() == QgsPgTableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    le->insert( index.data( Qt::DisplayRole ).toString() );
    return le;
  }

  return 0;
}

void QgsPgSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsPgTableModel::dbtmType )
    {
      QGis::GeometryType type = ( QGis::GeometryType ) cb->itemData( cb->currentIndex() ).toInt();

      model->setData( index, QgsPgTableModel::iconForGeomType( type ), Qt::DecorationRole );
      model->setData( index, type != QGis::UnknownGeometry ? QgsPostgresConn::displayStringForGeomType( type ) : tr( "Select..." ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsPgTableModel::dbtmPkCol )
    {
      model->setData( index, cb->currentText() );
      model->setData( index, cb->currentText(), Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
    model->setData( index, le->text() );
}

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
  mSearchColumnComboBox->addItem( tr( "SRID" ) );
  mSearchColumnComboBox->addItem( tr( "Sql" ) );

  mProxyModel.setParent( this );
  mProxyModel.setFilterKeyColumn( -1 );
  mProxyModel.setFilterCaseSensitivity( Qt::CaseInsensitive );
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
  QString msg = tr( "Are you sure you want to remove the %1 connection and all associated settings?" )
                .arg( cmbConnections->currentText() );
  if ( QMessageBox::Ok != QMessageBox::information( this, tr( "Confirm Delete" ), msg, QMessageBox::Ok | QMessageBox::Cancel ) )
    return;

  QgsPgSourceSelect::deleteConnection( cmbConnections->currentText() );

  populateConnectionList();
  emit connectionsChanged();
}

void QgsPgSourceSelect::deleteConnection( QString name )
{
  QString key = "/Postgresql/connections/" + name;
  QSettings settings;
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
  QgsPostgresConn::setSelectedConnection( cmbConnections->currentText() );

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
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmSchema );
  }
  else if ( text == tr( "Table" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmTable );
  }
  else if ( text == tr( "Type" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmType );
  }
  else if ( text == tr( "Geometry column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmGeomCol );
  }
  else if ( text == tr( "Primary key column" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmPkCol );
  }
  else if ( text == tr( "SRID" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmSrid );
  }
  else if ( text == tr( "Sql" ) )
  {
    mProxyModel.setFilterKeyColumn( QgsPgTableModel::dbtmSql );
  }
}

void QgsPgSourceSelect::on_mSearchModeComboBox_currentIndexChanged( const QString & text )
{
  Q_UNUSED( text );
  on_mSearchTableEdit_textChanged( mSearchTableEdit->text() );
}

void QgsPgSourceSelect::setLayerType( QgsPostgresLayerProperty layerProperty )
{
  QgsDebugMsg( "entering." );
  mTableModel.setGeometryTypesForTable( layerProperty );
}

QgsPgSourceSelect::~QgsPgSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
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
  QStringList keys = QgsPostgresConn::connectionList();
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
void QgsPgSourceSelect::addTables()
{
  mSelectedTables.clear();

  foreach( QModelIndex idx, mTablesTreeView->selectionModel()->selection().indexes() )
  {
    if ( idx.column() != QgsPgTableModel::dbtmTable )
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
    emit addDatabaseLayers( mSelectedTables, "postgres" );
    accept();
  }
}

void QgsPgSourceSelect::on_btnConnect_clicked()
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
  QgsDataSourceURI uri = QgsPostgresConn::connUri( cmbConnections->currentText() );

  QgsDebugMsg( "Connection info: " + uri.connectionInfo() );

  mConnInfo = uri.connectionInfo();
  mUseEstimatedMetadata = uri.useEstimatedMetadata();

  QgsPostgresConn *conn = QgsPostgresConn::connectDb( uri.connectionInfo(), true );
  if ( conn )
  {
    QApplication::setOverrideCursor( Qt::WaitCursor );

    QSettings settings;
    QString key = "/PostgreSQL/connections/" + cmbConnections->currentText();

    bool searchPublicOnly = settings.value( key + "/publicOnly" ).toBool();
    bool searchGeometryColumnsOnly = settings.value( key + "/geometryColumnsOnly" ).toBool();
    bool allowGeometrylessTables = cbxAllowGeometrylessTables->isChecked();

    QVector<QgsPostgresLayerProperty> layers;
    if ( conn->supportedLayers( layers, searchGeometryColumnsOnly, searchPublicOnly, allowGeometrylessTables ) )
    {
      // Add the supported layers to the table
      foreach( QgsPostgresLayerProperty layer, layers )
      {
        QString type = layer.type;
        QString srid = layer.srid;
        if ( !searchGeometryColumnsOnly && !layer.geometryColName.isNull() )
        {
          if ( type == "GEOMETRY" || type.isNull() || srid.isEmpty() )
          {
            addSearchGeometryColumn( layer );
            type = "";
            srid = "";
          }
        }
        QgsDebugMsg( QString( "adding table %1.%2" ).arg( layer.schemaName ).arg( layer.tableName ) );

        layer.type = type;
        layer.srid = srid;
        mTableModel.addTableEntry( layer );
      }

      if ( mColumnTypeThread )
      {
        btnConnect->setText( tr( "Stop" ) );
        mColumnTypeThread->start();
      }
    }

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

    conn->disconnect();

    if ( !mColumnTypeThread )
    {
      finishList();
    }
  }
  else
  {
    // Let user know we couldn't initialise the Postgres/PostGIS provider
    QMessageBox::warning( this,
                          tr( "Postgres/PostGIS Provider" ),
                          tr( "Could not open the Postgres/PostGIS Provider" ) );
  }
}

void QgsPgSourceSelect::finishList()
{
  QApplication::restoreOverrideCursor();

  if ( cmbConnections->count() > 0 )
    mAddButton->setEnabled( true );

#if 0
  for ( int i = 0; i < QgsPgTableModel::dbtmColumns; i++ )
    mTablesTreeView->resizeColumnToContents( i );
#endif

  mTablesTreeView->sortByColumn( QgsPgTableModel::dbtmTable, Qt::AscendingOrder );
  mTablesTreeView->sortByColumn( QgsPgTableModel::dbtmSchema, Qt::AscendingOrder );

}

void QgsPgSourceSelect::columnThreadFinished()
{
  delete mColumnTypeThread;
  mColumnTypeThread = 0;
  btnConnect->setText( tr( "Connect" ) );

  finishList();
}

QStringList QgsPgSourceSelect::selectedTables()
{
  return mSelectedTables;
}

QString QgsPgSourceSelect::connectionInfo()
{
  return mConnInfo;
}

void QgsPgSourceSelect::setSql( const QModelIndex &index )
{
  if ( !index.parent().isValid() )
  {
    QgsDebugMsg( "schema item found" );
    return;
  }

  QModelIndex idx = mProxyModel.mapToSource( index );
  QString tableName = mTableModel.itemFromIndex( idx.sibling( idx.row(), QgsPgTableModel::dbtmTable ) )->text();

  QgsVectorLayer *vlayer = new QgsVectorLayer( mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata ), tableName, "postgres" );

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

void QgsPgSourceSelect::addSearchGeometryColumn( QgsPostgresLayerProperty layerProperty )
{
  // store the column details and do the query in a thread
  if ( !mColumnTypeThread )
  {
    QgsPostgresConn *conn = QgsPostgresConn::connectDb( mConnInfo, true /* readonly */ );
    if ( conn )
    {

      mColumnTypeThread = new QgsGeomColumnTypeThread( conn, mUseEstimatedMetadata );

      connect( mColumnTypeThread, SIGNAL( setLayerType( QgsPostgresLayerProperty ) ),
               this, SLOT( setLayerType( QgsPostgresLayerProperty ) ) );
      connect( this, SIGNAL( addGeometryColumn( QgsPostgresLayerProperty ) ),
               mColumnTypeThread, SLOT( addGeometryColumn( QgsPostgresLayerProperty ) ) );
      connect( mColumnTypeThread, SIGNAL( finished() ),
               this, SLOT( columnThreadFinished() ) );
    }
  }

  emit addGeometryColumn( layerProperty );
}

QString QgsPgSourceSelect::fullDescription( QString schema, QString table, QString column, QString type )
{
  QString full_desc = "";
  if ( !schema.isEmpty() )
    full_desc = QgsPostgresConn::quotedIdentifier( schema ) + ".";
  full_desc += QgsPostgresConn::quotedIdentifier( table ) + " (" + column + ") " + type;
  return full_desc;
}

void QgsPgSourceSelect::setConnectionListPosition()
{
  // If possible, set the item currently displayed database
  QString toSelect = QgsPostgresConn::selectedConnection();
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
