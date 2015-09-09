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
#include <QStyledItemDelegate>

/** Used to create an editor for when the user tries to change the contents of a cell */
QWidget *QgsPgSourceSelectDelegate::createEditor( QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index ) const
{
  Q_UNUSED( option );

  QString tableName = index.sibling( index.row(), QgsPgTableModel::dbtmTable ).data( Qt::DisplayRole ).toString();
  if ( tableName.isEmpty() )
    return 0;

  if ( index.column() == QgsPgTableModel::dbtmSql )
  {
    return new QLineEdit( parent );
  }

  if ( index.column() == QgsPgTableModel::dbtmType && index.data( Qt::UserRole + 1 ).toBool() )
  {
    QComboBox *cb = new QComboBox( parent );
    Q_FOREACH ( QGis::WkbType type,
                QList<QGis::WkbType>()
                << QGis::WKBPoint
                << QGis::WKBLineString
                << QGis::WKBPolygon
                << QGis::WKBMultiPoint
                << QGis::WKBMultiLineString
                << QGis::WKBMultiPolygon
                << QGis::WKBNoGeometry )
    {
      cb->addItem( QgsPgTableModel::iconForWkbType( type ), QgsPostgresConn::displayStringForWkbType( type ), type );
    }
    return cb;
  }

  if ( index.column() == QgsPgTableModel::dbtmPkCol )
  {
    QStringList values = index.data( Qt::UserRole + 1 ).toStringList();

    if ( values.size() > 0 )
    {
      QComboBox *cb = new QComboBox( parent );
      cb->setItemDelegate( new QStyledItemDelegate( parent ) );

      QStandardItemModel *model = new QStandardItemModel( values.size(), 1, cb );

      int row = 0;
      Q_FOREACH ( const QString& value, values )
      {
        QStandardItem *item = new QStandardItem( value );
        item->setFlags( Qt::ItemIsUserCheckable | Qt::ItemIsEnabled );
        item->setCheckable( true );
        item->setData( Qt::Unchecked, Qt::CheckStateRole );
        model->setItem( row++, 0, item );
      }

      cb->setModel( model );

      return cb;
    }
  }

  if ( index.column() == QgsPgTableModel::dbtmSrid )
  {
    QLineEdit *le = new QLineEdit( parent );
    le->setValidator( new QIntValidator( -1, 999999, parent ) );
    return le;
  }

  return 0;
}

void QgsPgSourceSelectDelegate::setEditorData( QWidget *editor, const QModelIndex &index ) const
{
  QString value( index.data( Qt::DisplayRole ).toString() );

  QComboBox *cb = qobject_cast<QComboBox* >( editor );
  if ( cb )
  {
    if ( index.column() == QgsPgTableModel::dbtmType )
      cb->setCurrentIndex( cb->findData( index.data( Qt::UserRole + 2 ).toInt() ) );

    if ( index.column() == QgsPgTableModel::dbtmPkCol && !index.data( Qt::UserRole + 2 ).toStringList().isEmpty() )
    {
      QStringList cols = index.data( Qt::UserRole + 2 ).toStringList();

      Q_FOREACH ( const QString& col, cols )
      {
        QStandardItemModel *cbm = qobject_cast<QStandardItemModel*>( cb->model() );
        for ( int idx = 0; idx < cbm->rowCount(); idx++ )
        {
          QStandardItem *item = cbm->item( idx, 0 );
          if ( item->text() != col )
            continue;

          item->setData( Qt::Checked, Qt::CheckStateRole );
          break;
        }
      }

    }
  }

  QLineEdit *le = qobject_cast<QLineEdit*>( editor );
  if ( le )
  {
    bool ok;
    value.toInt( &ok );
    if ( index.column() == QgsPgTableModel::dbtmSrid && !ok )
      value = "";

    le->setText( value );
  }
}

void QgsPgSourceSelectDelegate::setModelData( QWidget *editor, QAbstractItemModel *model, const QModelIndex &index ) const
{
  QComboBox *cb = qobject_cast<QComboBox *>( editor );
  if ( cb )
  {
    if ( index.column() == QgsPgTableModel::dbtmType )
    {
      QGis::WkbType type = ( QGis::WkbType ) cb->itemData( cb->currentIndex() ).toInt();

      model->setData( index, QgsPgTableModel::iconForWkbType( type ), Qt::DecorationRole );
      model->setData( index, type != QGis::WKBUnknown ? QgsPostgresConn::displayStringForWkbType( type ) : tr( "Select..." ) );
      model->setData( index, type, Qt::UserRole + 2 );
    }
    else if ( index.column() == QgsPgTableModel::dbtmPkCol )
    {
      QStandardItemModel *cbm = qobject_cast<QStandardItemModel*>( cb->model() );
      QStringList cols;
      for ( int idx = 0; idx < cbm->rowCount(); idx++ )
      {
        QStandardItem *item = cbm->item( idx, 0 );
        if ( item->data( Qt::CheckStateRole ) == Qt::Checked )
          cols << item->text();
      }

      model->setData( index, cols.isEmpty() ? tr( "Select..." ) : cols.join( ", " ) );
      model->setData( index, cols, Qt::UserRole + 2 );
    }
  }

  QLineEdit *le = qobject_cast<QLineEdit *>( editor );
  if ( le )
  {
    QString value( le->text() );

    if ( index.column() == QgsPgTableModel::dbtmSrid && value.isEmpty() )
    {
      value = tr( "Enter..." );
    }

    model->setData( index, value );
  }
}

QgsPgSourceSelect::QgsPgSourceSelect( QWidget *parent, Qt::WindowFlags fl, bool managerMode, bool embeddedMode )
    : QDialog( parent, fl )
    , mManagerMode( managerMode )
    , mEmbeddedMode( embeddedMode )
    , mColumnTypeThread( 0 )
    , mUseEstimatedMetadata( false )
{
  setupUi( this );

  if ( mEmbeddedMode )
  {
    buttonBox->button( QDialogButtonBox::Close )->hide();
  }
  else
  {
    setWindowTitle( tr( "Add PostGIS Table(s)" ) );
  }

  mAddButton = new QPushButton( tr( "&Add" ) );
  mAddButton->setEnabled( false );

  mBuildQueryButton = new QPushButton( tr( "&Set Filter" ) );
  mBuildQueryButton->setToolTip( tr( "Set Filter" ) );
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
  mSearchColumnComboBox->addItem( tr( "Feature id" ) );
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
  mHoldDialogOpen->setChecked( settings.value( "/Windows/PgSourceSelect/HoldDialogOpen", false ).toBool() );

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

  QgsPostgresConn::deleteConnection( cmbConnections->currentText() );

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
void QgsPgSourceSelect::on_cmbConnections_currentIndexChanged( const QString & text )
{
  // Remember which database was selected.
  QgsPostgresConn::setSelectedConnection( text );

  cbxAllowGeometrylessTables->blockSignals( true );
  cbxAllowGeometrylessTables->setChecked( QgsPostgresConn::allowGeometrylessTables( text ) );
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

void QgsPgSourceSelect::on_mSearchGroupBox_toggled( bool checked )
{
  if ( mSearchTableEdit->text().isEmpty() )
    return;

  on_mSearchTableEdit_textChanged( checked ? mSearchTableEdit->text() : "" );
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
  else if ( text == tr( "Feature id" ) )
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

void QgsPgSourceSelect::setLayerType( const QgsPostgresLayerProperty& layerProperty )
{
  QgsDebugMsg( "entering." );
  mTableModel.addTableEntry( layerProperty );
}

QgsPgSourceSelect::~QgsPgSourceSelect()
{
  if ( mColumnTypeThread )
  {
    mColumnTypeThread->stop();
    mColumnTypeThread->wait();
    finishList();
  }

  QSettings settings;
  settings.setValue( "/Windows/PgSourceSelect/geometry", saveGeometry() );
  settings.setValue( "/Windows/PgSourceSelect/HoldDialogOpen", mHoldDialogOpen->isChecked() );

  for ( int i = 0; i < mTableModel.columnCount(); i++ )
  {
    settings.setValue( QString( "/Windows/PgSourceSelect/columnWidths/%1" ).arg( i ), mTablesTreeView->columnWidth( i ) );
  }
}

void QgsPgSourceSelect::populateConnectionList()
{
  cmbConnections->blockSignals( true );
  cmbConnections->clear();
  cmbConnections->addItems( QgsPostgresConn::connectionList() );
  cmbConnections->blockSignals( false );

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

  Q_FOREACH ( const QModelIndex& idx, mTablesTreeView->selectionModel()->selection().indexes() )
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
    if ( !mHoldDialogOpen->isChecked() )
    {
      accept();
    }
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

  QApplication::setOverrideCursor( Qt::BusyCursor );

  mColumnTypeThread = new QgsGeomColumnTypeThread( cmbConnections->currentText(), mUseEstimatedMetadata, cbxAllowGeometrylessTables->isChecked() );

  connect( mColumnTypeThread, SIGNAL( setLayerType( const QgsPostgresLayerProperty& ) ),
           this, SLOT( setLayerType( const QgsPostgresLayerProperty& ) ) );
  connect( mColumnTypeThread, SIGNAL( finished() ),
           this, SLOT( columnThreadFinished() ) );
  connect( mColumnTypeThread, SIGNAL( progress( int, int ) ),
           this, SIGNAL( progress( int, int ) ) );
  connect( mColumnTypeThread, SIGNAL( progressMessage( const QString& ) ),
           this, SIGNAL( progressMessage( const QString& ) ) );

  btnConnect->setText( tr( "Stop" ) );
  mColumnTypeThread->start();
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

  QString uri = mTableModel.layerURI( idx, mConnInfo, mUseEstimatedMetadata );
  if ( uri.isNull() )
  {
    QgsDebugMsg( "no uri" );
    return;
  }

  QgsVectorLayer *vlayer = new QgsVectorLayer( uri, tableName, "postgres" );
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

QString QgsPgSourceSelect::fullDescription( const QString& schema, const QString& table, const QString& column, const QString& type )
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
