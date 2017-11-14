/***************************************************************************
                         qgsnewspatialitelayerdialog.cpp
        Creates a new SpatiaLite layer. This dialog borrows heavily from
        qgsnewvectorlayerdialog.cpp
                             -------------------
    begin                : 2010-03-18
    copyright            : (C) 2010 by Gary Sherman
    email                : gsherman@mrcc.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/


#include "qgsnewspatialitelayerdialog.h"

#include "qgis.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgisapp.h" // <- for theme icons
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsprojectionselectiondialog.h"
#include "qgsspatialiteutils.h"
#include "qgslogger.h"
#include "qgssettings.h"

#include <QPushButton>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QLibrary>

#include <spatialite.h>

QgsNewSpatialiteLayerDialog::QgsNewSpatialiteLayerDialog( QWidget *parent, Qt::WindowFlags fl, const QgsCoordinateReferenceSystem &defaultCrs )
  : QDialog( parent, fl )
{
  setupUi( this );
  connect( mAddAttributeButton, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::mAddAttributeButton_clicked );
  connect( mRemoveAttributeButton, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::mRemoveAttributeButton_clicked );
  connect( mTypeBox, static_cast<void ( QComboBox::* )( int )>( &QComboBox::currentIndexChanged ), this, &QgsNewSpatialiteLayerDialog::mTypeBox_currentIndexChanged );
  connect( pbnFindSRID, &QPushButton::clicked, this, &QgsNewSpatialiteLayerDialog::pbnFindSRID_clicked );
  connect( toolButtonNewDatabase, &QToolButton::clicked, this, &QgsNewSpatialiteLayerDialog::toolButtonNewDatabase_clicked );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsNewSpatialiteLayerDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::rejected, this, &QgsNewSpatialiteLayerDialog::buttonBox_rejected );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/NewSpatiaLiteLayer/geometry" ) ).toByteArray() );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionNewAttribute.svg" ) ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( QStringLiteral( "/mActionDeleteAttribute.svg" ) ) );
  mTypeBox->addItem( tr( "Text data" ), "text" );
  mTypeBox->addItem( tr( "Whole number" ), "integer" );
  mTypeBox->addItem( tr( "Decimal number" ), "real" );

  mPointRadioButton->setChecked( true );
  // Populate the database list from the stored connections
  settings.beginGroup( QStringLiteral( "SpatiaLite/connections" ) );
  QStringList keys = settings.childGroups();
  QStringList::Iterator it = keys.begin();
  mDatabaseComboBox->clear();
  while ( it != keys.end() )
  {
    // retrieving the SQLite DB name and full path
    QString text = settings.value( *it + "/sqlitepath", "###unknown###" ).toString();
    mDatabaseComboBox->addItem( text );
    ++it;
  }
  settings.endGroup();

  mOkButton = buttonBox->button( QDialogButtonBox::Ok );
  mOkButton->setEnabled( false );

  // Set the SRID box to a default of WGS84
  mCrsId = defaultCrs.authid();
  leSRID->setText( defaultCrs.authid() + " - " + defaultCrs.description() );

  pbnFindSRID->setEnabled( mDatabaseComboBox->count() );

  connect( mNameEdit, &QLineEdit::textChanged, this, &QgsNewSpatialiteLayerDialog::nameChanged );
  connect( mAttributeView, &QTreeWidget::itemSelectionChanged, this, &QgsNewSpatialiteLayerDialog::selectionChanged );
  connect( leLayerName, &QLineEdit::textChanged, this, &QgsNewSpatialiteLayerDialog::checkOk );
  connect( checkBoxPrimaryKey, &QAbstractButton::clicked, this, &QgsNewSpatialiteLayerDialog::checkOk );

  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsNewSpatialiteLayerDialog::showHelp );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

}

QgsNewSpatialiteLayerDialog::~QgsNewSpatialiteLayerDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/NewSpatiaLiteLayer/geometry" ), saveGeometry() );
}

void QgsNewSpatialiteLayerDialog::mTypeBox_currentIndexChanged( int index )
{
  // This isn't used since widths are irrelevant in sqlite3
  switch ( index )
  {
    case 0: // Text data
    case 1: // Whole number
    case 2: // Decimal number
    default:
      break;
  }
}

void QgsNewSpatialiteLayerDialog::toolButtonNewDatabase_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "New SpatiaLite Database File" ),
                     QDir::homePath(),
                     tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.endsWith( QLatin1String( ".sqlite" ), Qt::CaseInsensitive ) && !fileName.endsWith( QLatin1String( ".db" ), Qt::CaseInsensitive ) )
  {
    fileName += QLatin1String( ".sqlite" );
  }

  mDatabaseComboBox->insertItem( 0, fileName );
  mDatabaseComboBox->setCurrentIndex( 0 );
  createDb();
}

QString QgsNewSpatialiteLayerDialog::selectedType() const
{
  if ( mPointRadioButton->isChecked() )
  {
    return QStringLiteral( "POINT" );
  }
  else if ( mLineRadioButton->isChecked() )
  {
    return QStringLiteral( "LINESTRING" );
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    return QStringLiteral( "POLYGON" );
  }
  else if ( mMultipointRadioButton->isChecked() )
  {
    return QStringLiteral( "MULTIPOINT" );
  }
  else if ( mMultilineRadioButton->isChecked() )
  {
    return QStringLiteral( "MULTILINESTRING" );
  }
  else if ( mMultipolygonRadioButton->isChecked() )
  {
    return QStringLiteral( "MULTIPOLYGON" );
  }

  Q_ASSERT( !"no type selected" );
  return QLatin1String( "" );
}

void QgsNewSpatialiteLayerDialog::checkOk()
{
  bool created  = !leLayerName->text().isEmpty() &&
                  ( checkBoxPrimaryKey->isChecked() || mAttributeView->topLevelItemCount() > 0 ) &&
                  createDb();
  mOkButton->setEnabled( created );
}

void QgsNewSpatialiteLayerDialog::mAddAttributeButton_clicked()
{
  if ( !mNameEdit->text().isEmpty() )
  {
    QString myName = mNameEdit->text();
    //use userrole to avoid translated type string
    QString myType = mTypeBox->currentData( Qt::UserRole ).toString();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType ) );

    checkOk();

    mNameEdit->clear();
  }
}

void QgsNewSpatialiteLayerDialog::mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewSpatialiteLayerDialog::pbnFindSRID_clicked()
{
  // first get list of supported SRID from the selected SpatiaLite database
  // to build filter for projection selector
  sqlite3_database_unique_ptr database;
  bool status = true;
  int rc = database.open_v2( mDatabaseComboBox->currentText(), SQLITE_OPEN_READONLY, nullptr );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, tr( "SpatiaLite Database" ), tr( "Unable to open the database" ) );
    return;
  }

  // load up the srid table
  sqlite3_statement_unique_ptr statement;
  QString sql = QStringLiteral( "select auth_name || ':' || auth_srid from spatial_ref_sys order by srid asc" );

  QSet<QString> myCRSs;

  statement = database.prepare( sql, rc );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    // get the first row of the result set
    while ( sqlite3_step( statement.get() ) == SQLITE_ROW )
    {
      myCRSs.insert( statement.columnAsText( 0 ) );
    }
  }
  else
  {
    // XXX query failed -- warn the user some how
    QMessageBox::warning( nullptr, tr( "Error" ), tr( "Failed to load SRIDS: %1" ).arg( database.errorMessage() ) );
    status = false;
  }

  if ( status )
  {
    // prepare projection selector
    QgsProjectionSelectionDialog *mySelector = new QgsProjectionSelectionDialog( this );
    mySelector->setMessage( QString() );
    mySelector->setOgcWmsCrsFilter( myCRSs );
    mySelector->setCrs( QgsCoordinateReferenceSystem::fromOgcWmsCrs( mCrsId ) );

    if ( mySelector->exec() )
    {
      QgsCoordinateReferenceSystem srs = mySelector->crs();
      QString crsId = srs.authid();
      if ( crsId != mCrsId )
      {
        mCrsId = crsId;
        leSRID->setText( srs.authid() + " - " + srs.description() );
      }
    }
    delete mySelector;
  }
}

void QgsNewSpatialiteLayerDialog::nameChanged( const QString &name )
{
  mAddAttributeButton->setDisabled( name.isEmpty() || ! mAttributeView->findItems( name, Qt::MatchExactly ).isEmpty() );
}

void QgsNewSpatialiteLayerDialog::selectionChanged()
{
  mRemoveAttributeButton->setDisabled( mAttributeView->selectedItems().isEmpty() );
}

bool QgsNewSpatialiteLayerDialog::createDb()
{
  QString dbPath = mDatabaseComboBox->currentText();
  if ( dbPath.isEmpty() )
    return false;

  QFile newDb( dbPath );
  if ( !newDb.exists() )
  {
    QString errCause;
    bool res = false;

    QString spatialite_lib = QgsProviderRegistry::instance()->library( QStringLiteral( "spatialite" ) );
    QLibrary *myLib = new QLibrary( spatialite_lib );
    bool loaded = myLib->load();
    if ( loaded )
    {
      QgsDebugMsg( "SpatiaLite provider loaded" );

      typedef bool ( *createDbProc )( const QString &, QString & );
      createDbProc createDbPtr = ( createDbProc ) cast_to_fptr( myLib->resolve( "createDb" ) );
      if ( createDbPtr )
      {
        res = createDbPtr( dbPath, errCause );
      }
      else
      {
        errCause = QStringLiteral( "Resolving createDb(...) failed" );
      }
    }
    delete myLib;

    if ( !res )
    {
      QMessageBox::warning( nullptr, tr( "SpatiaLite Database" ), errCause );
      pbnFindSRID->setEnabled( false );
    }
  }

  QFileInfo fi( newDb );
  if ( !fi.exists() )
  {
    pbnFindSRID->setEnabled( false );
    return false;
  }

  QString key = "/SpatiaLite/connections/" + fi.fileName() + "/sqlitepath";

  QgsSettings settings;
  if ( !settings.contains( key ) )
  {
    settings.setValue( QStringLiteral( "SpatiaLite/connections/selected" ), fi.fileName() + tr( "@" ) + fi.canonicalFilePath() );
    settings.setValue( key, fi.canonicalFilePath() );

    QMessageBox::information( nullptr, tr( "SpatiaLite Database" ), tr( "Registered new database!" ) );
  }

  pbnFindSRID->setEnabled( true );

  return true;
}

void QgsNewSpatialiteLayerDialog::buttonBox_accepted()
{
  if ( apply() )
    accept();
}

void QgsNewSpatialiteLayerDialog::buttonBox_rejected()
{
  reject();
}

bool QgsNewSpatialiteLayerDialog::apply()
{
  // Build up the sql statement for creating the table
  QString sql = QStringLiteral( "create table %1(" ).arg( quotedIdentifier( leLayerName->text() ) );
  QString delim;

  if ( checkBoxPrimaryKey->isChecked() )
  {
    sql += QLatin1String( "pkuid integer primary key autoincrement" );
    delim = QStringLiteral( "," );
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    sql += delim + QStringLiteral( "%1 %2" ).arg( quotedIdentifier( ( *it )->text( 0 ) ), ( *it )->text( 1 ) );
    delim = QStringLiteral( "," );
    ++it;
  }

  // complete the create table statement
  sql += ')';

  QgsDebugMsg( QString( "Creating table in database %1" ).arg( mDatabaseComboBox->currentText() ) );

  QgsDebugMsg( sql ); // OK

  QString sqlAddGeom = QStringLiteral( "select AddGeometryColumn(%1,%2,%3,%4,2)" )
                       .arg( quotedValue( leLayerName->text() ),
                             quotedValue( leGeometryColumn->text() ) )
                       .arg( mCrsId.split( ':' ).value( 1, QStringLiteral( "0" ) ).toInt() )
                       .arg( quotedValue( selectedType() ) );
  QgsDebugMsg( sqlAddGeom ); // OK

  QString sqlCreateIndex = QStringLiteral( "select CreateSpatialIndex(%1,%2)" )
                           .arg( quotedValue( leLayerName->text() ),
                                 quotedValue( leGeometryColumn->text() ) );
  QgsDebugMsg( sqlCreateIndex ); // OK

  spatialite_database_unique_ptr database;
  int rc = database.open( mDatabaseComboBox->currentText() );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this,
                          tr( "SpatiaLite Database" ),
                          tr( "Unable to open the database: %1" ).arg( mDatabaseComboBox->currentText() ) );
  }
  else
  {
    char *errmsg = nullptr;
    rc = sqlite3_exec( database.get(), sql.toUtf8(), nullptr, nullptr, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this,
                            tr( "Error Creating SpatiaLite Table" ),
                            tr( "Failed to create the SpatiaLite table %1. The database returned:\n%2" ).arg( leLayerName->text(), errmsg ) );
      sqlite3_free( errmsg );
    }
    else
    {
      // create the geometry column and the spatial index
      rc = sqlite3_exec( database.get(), sqlAddGeom.toUtf8(), nullptr, nullptr, &errmsg );
      if ( rc != SQLITE_OK )
      {
        QMessageBox::warning( this,
                              tr( "Error Creating Geometry Column" ),
                              tr( "Failed to create the geometry column. The database returned:\n%1" ).arg( errmsg ) );
        sqlite3_free( errmsg );
      }
      else
      {
        // create the spatial index
        rc = sqlite3_exec( database.get(), sqlCreateIndex.toUtf8(), nullptr, nullptr, &errmsg );
        if ( rc != SQLITE_OK )
        {
          QMessageBox::warning( this,
                                tr( "Error Creating Spatial Index" ),
                                tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
          sqlite3_free( errmsg );
        }

        QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "dbname='%1' table='%2'(%3) sql=" )
            .arg( mDatabaseComboBox->currentText(),
                  leLayerName->text(),
                  leGeometryColumn->text() ), leLayerName->text(), QStringLiteral( "spatialite" ) );
        if ( layer->isValid() )
        {
          // register this layer with the central layers registry
          QList<QgsMapLayer *> myList;
          myList << layer;
          //addMapLayers returns a list of all successfully added layers
          //so we compare that to our original list.
          if ( myList == QgsProject::instance()->addMapLayers( myList ) )
            return true;
        }
        else
        {
          QgsDebugMsg( leLayerName->text() + " is an invalid layer - not loaded" );
          QMessageBox::critical( this, tr( "Invalid Layer" ), tr( "%1 is an invalid layer and cannot be loaded." ).arg( leLayerName->text() ) );
          delete layer;
        }
      }
    }
  }

  return false;
}

QString QgsNewSpatialiteLayerDialog::quotedIdentifier( QString id )
{
  id.replace( '\"', QLatin1String( "\"\"" ) );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsNewSpatialiteLayerDialog::quotedValue( QString value )
{
  value.replace( '\'', QLatin1String( "''" ) );
  return value.prepend( '\'' ).append( '\'' );
}

void QgsNewSpatialiteLayerDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "managing_data_source/create_layers.html#creating-a-new-spatialite-layer" ) );
}

