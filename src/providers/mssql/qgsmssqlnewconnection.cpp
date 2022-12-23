/***************************************************************************
                    qgsmssqlnewconnection.cpp  -  description
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

#include <QInputDialog>
#include <QMessageBox>
#include <QSqlDatabase>
#include <QSqlError>
#include <QRegularExpression>
#include <QRegularExpressionValidator>
#include <QMenu>

#include "qgsmssqlnewconnection.h"
#include "qgsmssqlprovider.h"
#include "qgssettings.h"
#include "qgsmssqlconnection.h"
#include "qgsmssqldatabase.h"
#include "qgsgui.h"

QgsMssqlNewConnection::QgsMssqlNewConnection( QWidget *parent, const QString &connName, Qt::WindowFlags fl )
  : QDialog( parent, fl )
  , mOriginalConnName( connName )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( btnListDatabase, &QPushButton::clicked, this, &QgsMssqlNewConnection::btnListDatabase_clicked );
  connect( btnConnect, &QPushButton::clicked, this, &QgsMssqlNewConnection::btnConnect_clicked );
  connect( cb_trustedConnection, &QCheckBox::clicked, this, &QgsMssqlNewConnection::cb_trustedConnection_clicked );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsMssqlNewConnection::showHelp );

  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( true );
  connect( txtName, &QLineEdit::textChanged, this, &QgsMssqlNewConnection::updateOkButtonState );
  connect( txtService, &QLineEdit::textChanged, this, &QgsMssqlNewConnection::updateOkButtonState );
  connect( txtHost, &QLineEdit::textChanged, this, &QgsMssqlNewConnection::updateOkButtonState );
  connect( listDatabase, &QListWidget::currentItemChanged, this, &QgsMssqlNewConnection::updateOkButtonState );
  connect( listDatabase, &QListWidget::currentItemChanged, this, &QgsMssqlNewConnection::onCurrentDataBaseChange );
  connect( groupBoxGeometryColumns,  &QGroupBox::toggled, this, &QgsMssqlNewConnection::onCurrentDataBaseChange );
  connect( cb_allowGeometrylessTables,  &QCheckBox::clicked, this, &QgsMssqlNewConnection::onCurrentDataBaseChange );

  connect( checkBoxExtentFromGeometryColumns, &QCheckBox::toggled, this, &QgsMssqlNewConnection::onExtentFromGeometryToggled );
  connect( checkBoxPKFromGeometryColumns, &QCheckBox::toggled, this, &QgsMssqlNewConnection::onPrimaryKeyFromGeometryToggled );

  lblWarning->hide();

  if ( !connName.isEmpty() )
  {
    // populate the dialog with the information stored for the connection
    // populate the fields with the stored setting parameters
    const QgsSettings settings;

    const QString key = "/MSSQL/connections/" + connName;
    txtService->setText( settings.value( key + "/service" ).toString() );
    txtHost->setText( settings.value( key + "/host" ).toString() );
    listDatabase->addItem( settings.value( key + "/database" ).toString() );
    groupBoxSchemasFilter->setChecked( settings.value( key + "/schemasFiltering" ).toBool() );
    const QVariant schemasVariant = settings.value( key + "/excludedSchemas" );
    if ( schemasVariant.isValid() && schemasVariant.type() == QVariant::Map )
      mSchemaSettings = schemasVariant.toMap();

    listDatabase->setCurrentRow( 0 );
    groupBoxGeometryColumns->setChecked( QgsMssqlConnection::geometryColumnsOnly( connName ) );
    whileBlocking( checkBoxExtentFromGeometryColumns )->setChecked( QgsMssqlConnection::extentInGeometryColumns( connName ) );
    whileBlocking( checkBoxPKFromGeometryColumns )->setChecked( QgsMssqlConnection::primaryKeyInGeometryColumns( connName ) );
    cb_allowGeometrylessTables->setChecked( QgsMssqlConnection::allowGeometrylessTables( connName ) );
    cb_useEstimatedMetadata->setChecked( QgsMssqlConnection::useEstimatedMetadata( connName ) );
    mCheckNoInvalidGeometryHandling->setChecked( QgsMssqlConnection::isInvalidGeometryHandlingDisabled( connName ) );

    if ( settings.value( key + "/saveUsername" ).toString() == QLatin1String( "true" ) )
    {
      txtUsername->setText( settings.value( key + "/username" ).toString() );
      chkStoreUsername->setChecked( true );
      cb_trustedConnection->setChecked( false );
    }

    if ( settings.value( key + "/savePassword" ).toString() == QLatin1String( "true" ) )
    {
      txtPassword->setText( settings.value( key + "/password" ).toString() );
      chkStorePassword->setChecked( true );
    }

    txtName->setText( connName );
  }
  txtName->setValidator( new QRegularExpressionValidator( QRegularExpression( QStringLiteral( "[^\\/]+" ) ), txtName ) );
  cb_trustedConnection_clicked();

  schemaView->setModel( &mSchemaModel );
  schemaView->setContextMenuPolicy( Qt::CustomContextMenu );

  connect( schemaView, &QWidget::customContextMenuRequested, this, [this]( const QPoint & p )
  {
    QMenu menu;
    menu.addAction( tr( "Check All" ), this, [this]
    {
      mSchemaModel.checkAll();
    } );

    menu.addAction( tr( "Uncheck All" ), this, [this]
    {
      mSchemaModel.unCheckAll();
    } );

    menu.exec( this->schemaView->viewport()->mapToGlobal( p ) );
  }
         );
  onCurrentDataBaseChange();

  groupBoxSchemasFilter->setCollapsed( !groupBoxSchemasFilter->isChecked() );
}

//! Autoconnected SLOTS
void QgsMssqlNewConnection::accept()
{
  QgsSettings settings;
  QString baseKey = QStringLiteral( "/MSSQL/connections/" );
  settings.setValue( baseKey + "selected", txtName->text() );

  // warn if entry was renamed to an existing connection
  if ( ( mOriginalConnName.isNull() || mOriginalConnName.compare( txtName->text(), Qt::CaseInsensitive ) != 0 ) &&
       ( settings.contains( baseKey + txtName->text() + "/service" ) ||
         settings.contains( baseKey + txtName->text() + "/host" ) ) &&
       QMessageBox::question( this,
                              tr( "Save Connection" ),
                              tr( "Should the existing connection %1 be overwritten?" ).arg( txtName->text() ),
                              QMessageBox::Ok | QMessageBox::Cancel ) == QMessageBox::Cancel )
  {
    return;
  }

  // on rename delete the original entry first
  if ( !mOriginalConnName.isNull() && mOriginalConnName != txtName->text() )
  {
    settings.remove( baseKey + mOriginalConnName );
    settings.sync();
  }

  const QString connName = txtName->text();
  baseKey += connName;
  QString database;
  QListWidgetItem *item = listDatabase->currentItem();
  if ( item && item->text() != QLatin1String( "(from service)" ) )
  {
    database = item->text();
  }

  settings.setValue( baseKey + "/service", txtService->text() );
  settings.setValue( baseKey + "/host", txtHost->text() );
  settings.setValue( baseKey + "/database", database );
  settings.setValue( baseKey + "/username", chkStoreUsername->isChecked() ? txtUsername->text() : QString() );
  settings.setValue( baseKey + "/password", chkStorePassword->isChecked() ? txtPassword->text() : QString() );
  settings.setValue( baseKey + "/saveUsername", chkStoreUsername->isChecked() ? "true" : "false" );
  settings.setValue( baseKey + "/savePassword", chkStorePassword->isChecked() ? "true" : "false" );

  if ( groupBoxSchemasFilter->isChecked() )
  {
    if ( !mSchemaModel.dataBaseName().isEmpty() )
      mSchemaSettings.insert( mSchemaModel.dataBaseName(), mSchemaModel.uncheckedSchemas() );
    settings.setValue( baseKey + "/excludedSchemas", mSchemaSettings );
  }

  settings.setValue( baseKey + "/schemasFiltering", groupBoxSchemasFilter->isChecked() );

  QgsMssqlConnection::setGeometryColumnsOnly( connName, groupBoxGeometryColumns->isChecked() );
  QgsMssqlConnection::setExtentInGeometryColumns( connName, checkBoxExtentFromGeometryColumns->isChecked() && testExtentInGeometryColumns() );
  QgsMssqlConnection::setPrimaryKeyInGeometryColumns( connName, checkBoxPKFromGeometryColumns->isChecked() && testPrimaryKeyInGeometryColumns() );
  QgsMssqlConnection::setAllowGeometrylessTables( connName, cb_allowGeometrylessTables->isChecked() );
  QgsMssqlConnection::setUseEstimatedMetadata( connName, cb_useEstimatedMetadata->isChecked() );
  QgsMssqlConnection::setInvalidGeometryHandlingDisabled( connName, mCheckNoInvalidGeometryHandling->isChecked() );

  QDialog::accept();
}

void QgsMssqlNewConnection::btnConnect_clicked()
{
  testConnection();
}

void QgsMssqlNewConnection::btnListDatabase_clicked()
{
  listDatabases();
}

void QgsMssqlNewConnection::cb_trustedConnection_clicked()
{
  if ( cb_trustedConnection->checkState() == Qt::Checked )
  {
    txtUsername->setEnabled( false );
    txtUsername->clear();
    txtPassword->setEnabled( false );
    txtPassword->clear();
  }
  else
  {
    txtUsername->setEnabled( true );
    txtPassword->setEnabled( true );
  }
}

//! End  Autoconnected SLOTS

bool QgsMssqlNewConnection::testConnection( const QString &testDatabase )
{
  bar->pushMessage( tr( "Testing connection" ), tr( "……" ) );
  // Gross but needed to show the last message.
  qApp->processEvents();

  if ( txtService->text().isEmpty() && txtHost->text().isEmpty() )
  {
    bar->clearWidgets();
    bar->pushWarning( tr( "Connection Failed" ), tr( "Host name hasn't been specified." ) );
    return false;
  }

  std::shared_ptr<QgsMssqlDatabase> db = getDatabase( testDatabase );

  if ( !db->isValid() )
  {
    bar->clearWidgets();
    bar->pushWarning( tr( "Error opening connection" ), db->errorText() );
    return false;
  }
  else
  {
    bar->clearWidgets();
  }

  return true;
}

void QgsMssqlNewConnection::listDatabases()
{
  testConnection( QStringLiteral( "master" ) );
  QString currentDataBase;
  if ( listDatabase->currentItem() )
    currentDataBase = listDatabase->currentItem()->text();
  listDatabase->clear();
  const QString queryStr = QStringLiteral( "SELECT name FROM master..sysdatabases WHERE name NOT IN ('master', 'tempdb', 'model', 'msdb')" );

  std::shared_ptr<QgsMssqlDatabase> db = getDatabase( QStringLiteral( "master" ) );

  if ( db->isValid() )
  {
    QSqlQuery query = QSqlQuery( db->db() );
    query.setForwardOnly( true );
    ( void )query.exec( queryStr );

    if ( !txtService->text().isEmpty() )
    {
      listDatabase->addItem( QStringLiteral( "(from service)" ) );
    }

    if ( query.isActive() )
    {
      while ( query.next() )
      {
        const QString name = query.value( 0 ).toString();
        listDatabase->addItem( name );
      }
      listDatabase->setCurrentRow( 0 );
    }
  }

  for ( int i = 0; i < listDatabase->count(); ++i )
  {
    if ( listDatabase->item( i )->text() == currentDataBase )
    {
      listDatabase->setCurrentRow( i );
      break;
    }
  }
  onCurrentDataBaseChange();
}

void QgsMssqlNewConnection::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/opening_data.html#connecting-to-mssql-spatial" ) );
}

std::shared_ptr<QgsMssqlDatabase> QgsMssqlNewConnection::getDatabase( const QString &name ) const
{
  QString database;
  QListWidgetItem *item = listDatabase->currentItem();
  if ( !name.isEmpty() )
  {
    database = name;
  }
  else if ( item && item->text() != QLatin1String( "(from service)" ) )
  {
    database = item->text();
  }

  return QgsMssqlDatabase::connectDb( txtService->text().trimmed(),
                                      txtHost->text().trimmed(),
                                      database,
                                      txtUsername->text().trimmed(),
                                      txtPassword->text().trimmed() );
}


void QgsMssqlNewConnection::updateOkButtonState()
{
  QListWidgetItem *item = listDatabase->currentItem();
  const bool disabled = txtName->text().isEmpty() || ( txtService->text().isEmpty() && txtHost->text().isEmpty() ) || !item;
  buttonBox->button( QDialogButtonBox::Ok )->setDisabled( disabled );
}

void QgsMssqlNewConnection::onCurrentDataBaseChange()
{
  //First store the schema settings for the previous dataBase
  if ( !mSchemaModel.dataBaseName().isEmpty() )
    mSchemaSettings.insert( mSchemaModel.dataBaseName(), mSchemaModel.uncheckedSchemas() );

  QString databaseName;
  if ( listDatabase->currentItem() )
    databaseName = listDatabase->currentItem()->text();

  std::shared_ptr<QgsMssqlDatabase> db = getDatabase();

  QStringList schemasList = QgsMssqlConnection::schemas( db, nullptr );
  int i = 0;
  while ( i < schemasList.count() )
  {
    if ( QgsMssqlConnection::isSystemSchema( schemasList.at( i ) ) )
      schemasList.removeAt( i );
    else
      ++i;
  }

  mSchemaModel.setSettings( databaseName, schemasList, QgsMssqlConnection::excludedSchemasList( txtName->text(), databaseName ) );
}

void QgsMssqlNewConnection::onExtentFromGeometryToggled( bool checked )
{
  if ( !checked )
  {
    bar->clearWidgets();
    return;
  }

  if ( !testExtentInGeometryColumns() )
    bar->pushWarning( tr( "Use extent from geometry_columns table" ), tr( "Extent columns (qgis_xmin, qgis_ymin, qgis_xmax, qgis_ymax) not found." ) );
  else
    bar->pushInfo( tr( "Use extent from geometry_columns table" ), tr( "Extent columns found." ) );
}

void QgsMssqlNewConnection::onPrimaryKeyFromGeometryToggled( bool checked )
{
  if ( !checked )
  {
    bar->clearWidgets();
    return;
  }

  if ( !testPrimaryKeyInGeometryColumns() )
    bar->pushWarning( tr( "Use primary key(s) from geometry_columns table" ), tr( "Primary key column (qgs_pkey) not found." ) );
  else
    bar->pushInfo( tr( "Use primary key(s) from geometry_columns table" ), tr( "Primary key column found." ) );
}


bool QgsMssqlNewConnection::testExtentInGeometryColumns() const
{
  std::shared_ptr<QgsMssqlDatabase> db = getDatabase();
  if ( !db->isValid() )
    return false;

  const QString queryStr = QStringLiteral( "SELECT qgis_xmin,qgis_xmax,qgis_ymin,qgis_ymax FROM geometry_columns" );
  QSqlQuery query = QSqlQuery( db->db() );
  const bool test = query.exec( queryStr );

  return test;
}

bool QgsMssqlNewConnection::testPrimaryKeyInGeometryColumns() const
{
  std::shared_ptr<QgsMssqlDatabase> db = getDatabase();
  if ( !db->isValid() )
    return false;

  const QString queryStr = QStringLiteral( "SELECT qgis_pkey FROM geometry_columns" );
  QSqlQuery query = QSqlQuery( db->db() );
  const bool test = query.exec( queryStr );

  return test;
}

SchemaModel::SchemaModel( QObject *parent ): QAbstractListModel( parent )
{}

int SchemaModel::rowCount( const QModelIndex &parent ) const
{
  Q_UNUSED( parent )
  return mSchemas.count();
}

QVariant SchemaModel::data( const QModelIndex &index, int role ) const
{
  if ( !index.isValid() || index.row() >= mSchemas.count() )
    return QVariant();


  switch ( role )
  {
    case Qt::CheckStateRole:
      if ( mExcludedSchemas.contains( mSchemas.at( index.row() ) ) )
        return Qt::CheckState::Unchecked;
      else
        return Qt::CheckState::Checked;
      break;
    case Qt::DisplayRole:
      return mSchemas.at( index.row() );
      break;
    default:
      return QVariant();
  }

  return QVariant();
}

bool SchemaModel::setData( const QModelIndex &index, const QVariant &value, int role )
{
  if ( !index.isValid() || index.row() >= mSchemas.count() )
    return false;

  switch ( role )
  {
    case Qt::CheckStateRole:
      if ( value == Qt::Checked && mExcludedSchemas.contains( mSchemas.at( index.row() ) ) )
        mExcludedSchemas.removeOne( mSchemas.at( index.row() ) );
      else if ( value == Qt::Unchecked && !mExcludedSchemas.contains( mSchemas.at( index.row() ) ) )
        mExcludedSchemas.append( mSchemas.at( index.row() ) );
      return true;
      break;
    default:
      return false;
  }

  return false;
}

Qt::ItemFlags SchemaModel::flags( const QModelIndex &index ) const
{
  return QAbstractListModel::flags( index ) | Qt::ItemFlag::ItemIsUserCheckable;
}

QStringList SchemaModel::uncheckedSchemas() const
{
  return mExcludedSchemas;
}


QString SchemaModel::dataBaseName() const
{
  return mDataBaseName;
}

void SchemaModel::setDataBaseName( const QString &dataBaseName )
{
  mDataBaseName = dataBaseName;
}

void SchemaModel::setSettings( const QString &database, const QStringList &schemas, const QStringList &excludedSchemas )
{
  beginResetModel();
  mDataBaseName = database;
  mSchemas = schemas;
  mExcludedSchemas = excludedSchemas;
  endResetModel();
}

void SchemaModel::checkAll()
{
  mExcludedSchemas.clear();
  emit dataChanged( index( 0, 0, QModelIndex() ), index( mSchemas.count() - 1, 0, QModelIndex() ) );
}

void SchemaModel::unCheckAll()
{
  mExcludedSchemas = mSchemas;
  emit dataChanged( index( 0, 0, QModelIndex() ), index( mSchemas.count() - 1, 0, QModelIndex() ) );
}
