/***************************************************************************
                         qgsnewspatialitelayerdialog.cpp
        Creates a new Spatialite layer. This dialog borrows heavily from
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
#include "qgsmaplayerregistry.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsgenericprojectionselector.h"
#include "qgsslconnect.h"
#include "qgscrscache.h"

#include "qgslogger.h"

#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QLibrary>

#include <spatialite.h>

QgsNewSpatialiteLayerDialog::QgsNewSpatialiteLayerDialog( QWidget *parent, Qt::WindowFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );

  QSettings settings;
  restoreGeometry( settings.value( "/Windows/NewSpatiaLiteLayer/geometry" ).toByteArray() );

  mAddAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionNewAttribute.svg" ) );
  mRemoveAttributeButton->setIcon( QgsApplication::getThemeIcon( "/mActionDeleteAttribute.svg" ) );
  mTypeBox->addItem( tr( "Text data" ), "text" );
  mTypeBox->addItem( tr( "Whole number" ), "integer" );
  mTypeBox->addItem( tr( "Decimal number" ), "real" );

  mPointRadioButton->setChecked( true );
  // Populate the database list from the stored connections
  settings.beginGroup( "/SpatiaLite/connections" );
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
  QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsByOgcWmsCrs( settings.value( "/Projections/layerDefaultCrs", GEO_EPSG_CRS_AUTHID ).toString() );
  srs.validate();
  mCrsId = srs.authid();
  leSRID->setText( srs.authid() + " - " + srs.description() );

  pbnFindSRID->setEnabled( mDatabaseComboBox->count() );

  connect( mNameEdit, SIGNAL( textChanged( QString ) ), this, SLOT( nameChanged( QString ) ) );
  connect( mAttributeView, SIGNAL( itemSelectionChanged() ), this, SLOT( selectionChanged() ) );
  connect( leLayerName, SIGNAL( textChanged( QString ) ), this, SLOT( checkOk() ) );
  connect( checkBoxPrimaryKey, SIGNAL( clicked() ), this, SLOT( checkOk() ) );

  mAddAttributeButton->setEnabled( false );
  mRemoveAttributeButton->setEnabled( false );

}

QgsNewSpatialiteLayerDialog::~QgsNewSpatialiteLayerDialog()
{
  QSettings settings;
  settings.setValue( "/Windows/NewSpatiaLiteLayer/geometry", saveGeometry() );
}

void QgsNewSpatialiteLayerDialog::on_mTypeBox_currentIndexChanged( int index )
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

void QgsNewSpatialiteLayerDialog::on_toolButtonNewDatabase_clicked()
{
  QString fileName = QFileDialog::getSaveFileName( this, tr( "New SpatiaLite Database File" ),
                     QDir::homePath(),
                     tr( "SpatiaLite" ) + " (*.sqlite *.db *.sqlite3 *.db3 *.s3db)" );

  if ( fileName.isEmpty() )
    return;

  if ( !fileName.endsWith( ".sqlite", Qt::CaseInsensitive ) && !fileName.endsWith( ".db", Qt::CaseInsensitive ) )
  {
    fileName += ".sqlite";
  }

  mDatabaseComboBox->insertItem( 0, fileName );
  mDatabaseComboBox->setCurrentIndex( 0 );
  createDb();
}

QString QgsNewSpatialiteLayerDialog::selectedType() const
{
  if ( mPointRadioButton->isChecked() )
  {
    return "POINT";
  }
  else if ( mLineRadioButton->isChecked() )
  {
    return "LINESTRING";
  }
  else if ( mPolygonRadioButton->isChecked() )
  {
    return "POLYGON";
  }
  else if ( mMultipointRadioButton->isChecked() )
  {
    return "MULTIPOINT";
  }
  else if ( mMultilineRadioButton->isChecked() )
  {
    return "MULTILINESTRING";
  }
  else if ( mMultipolygonRadioButton->isChecked() )
  {
    return "MULTIPOLYGON";
  }

  Q_ASSERT( !"no type selected" );
  return "";
}

void QgsNewSpatialiteLayerDialog::checkOk()
{
  bool created  = !leLayerName->text().isEmpty() &&
                  ( checkBoxPrimaryKey->isChecked() || mAttributeView->topLevelItemCount() > 0 ) &&
                  createDb();
  mOkButton->setEnabled( created );
}

void QgsNewSpatialiteLayerDialog::on_mAddAttributeButton_clicked()
{
  if ( !mNameEdit->text().isEmpty() )
  {
    QString myName = mNameEdit->text();
    //use userrole to avoid translated type string
    QString myType = mTypeBox->itemData( mTypeBox->currentIndex(), Qt::UserRole ).toString();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType ) );

    checkOk();

    mNameEdit->clear();
  }
}

void QgsNewSpatialiteLayerDialog::on_mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();

  checkOk();
}

void QgsNewSpatialiteLayerDialog::on_pbnFindSRID_clicked()
{
  // first get list of supported SRID from the selected Spatialite database
  // to build filter for projection selector
  sqlite3 *db = nullptr;
  bool status = true;
  int rc = sqlite3_open_v2( mDatabaseComboBox->currentText().toUtf8(), &db, SQLITE_OPEN_READONLY, nullptr );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, tr( "SpatiaLite Database" ), tr( "Unable to open the database" ) );
    return;
  }

  // load up the srid table
  const char *pzTail;
  sqlite3_stmt *ppStmt;
  QString sql = "select auth_name || ':' || auth_srid from spatial_ref_sys order by srid asc";

  QSet<QString> myCRSs;

  rc = sqlite3_prepare( db, sql.toUtf8(), sql.toUtf8().length(), &ppStmt, &pzTail );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    // get the first row of the result set
    while ( sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      myCRSs.insert( QString::fromUtf8(( const char * )sqlite3_column_text( ppStmt, 0 ) ) );
    }
  }
  else
  {
    // XXX query failed -- warn the user some how
    QMessageBox::warning( nullptr, tr( "Error" ), tr( "Failed to load SRIDS: %1" ).arg( sqlite3_errmsg( db ) ) );
    status = false;
  }
  // close the statement
  sqlite3_finalize( ppStmt );
  sqlite3_close( db );

  if ( !status )
  {
    return;
  }

  // prepare projection selector
  QgsGenericProjectionSelector *mySelector = new QgsGenericProjectionSelector( this );
  mySelector->setMessage();
  mySelector->setOgcWmsCrsFilter( myCRSs );
  mySelector->setSelectedAuthId( mCrsId );

  if ( mySelector->exec() )
  {
    QgsCoordinateReferenceSystem srs = QgsCRSCache::instance()->crsByOgcWmsCrs( mySelector->selectedAuthId() );
    QString crsId = srs.authid();
    if ( crsId != mCrsId )
    {
      mCrsId = crsId;
      leSRID->setText( srs.authid() + " - " + srs.description() );
    }
  }
  delete mySelector;
}

void QgsNewSpatialiteLayerDialog::nameChanged( const QString& name )
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

    QString spatialite_lib = QgsProviderRegistry::instance()->library( "spatialite" );
    QLibrary* myLib = new QLibrary( spatialite_lib );
    bool loaded = myLib->load();
    if ( loaded )
    {
      QgsDebugMsg( "spatialite provider loaded" );

      typedef bool ( *createDbProc )( const QString&, QString& );
      createDbProc createDbPtr = ( createDbProc ) cast_to_fptr( myLib->resolve( "createDb" ) );
      if ( createDbPtr )
      {
        res = createDbPtr( dbPath, errCause );
      }
      else
      {
        errCause = "Resolving createDb(...) failed";
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

  QSettings settings;
  if ( !settings.contains( key ) )
  {
    settings.setValue( "/SpatiaLite/connections/selected", fi.fileName() + tr( "@" ) + fi.canonicalFilePath() );
    settings.setValue( key, fi.canonicalFilePath() );

    QMessageBox::information( nullptr, tr( "SpatiaLite Database" ), tr( "Registered new database!" ) );
  }

  pbnFindSRID->setEnabled( true );

  return true;
}

void QgsNewSpatialiteLayerDialog::on_buttonBox_accepted()
{
  if ( apply() )
    accept();
}

void QgsNewSpatialiteLayerDialog::on_buttonBox_rejected()
{
  reject();
}

bool QgsNewSpatialiteLayerDialog::apply()
{
  // Build up the sql statement for creating the table
  QString sql = QString( "create table %1(" ).arg( quotedIdentifier( leLayerName->text() ) );
  QString delim = "";

  if ( checkBoxPrimaryKey->isChecked() )
  {
    sql += "pkuid integer primary key autoincrement";
    delim = ",";
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    sql += delim + QString( "%1 %2" ).arg( quotedIdentifier(( *it )->text( 0 ) ), ( *it )->text( 1 ) );
    delim = ",";
    ++it;
  }

  // complete the create table statement
  sql += ')';

  QgsDebugMsg( QString( "Creating table in database %1" ).arg( mDatabaseComboBox->currentText() ) );

  QgsDebugMsg( sql ); // OK

  QString sqlAddGeom = QString( "select AddGeometryColumn(%1,%2,%3,%4,2)" )
                       .arg( quotedValue( leLayerName->text() ),
                             quotedValue( leGeometryColumn->text() ) )
                       .arg( mCrsId.split( ':' ).value( 1, "0" ).toInt() )
                       .arg( quotedValue( selectedType() ) );
  QgsDebugMsg( sqlAddGeom ); // OK

  QString sqlCreateIndex = QString( "select CreateSpatialIndex(%1,%2)" )
                           .arg( quotedValue( leLayerName->text() ),
                                 quotedValue( leGeometryColumn->text() ) );
  QgsDebugMsg( sqlCreateIndex ); // OK

  sqlite3 *db;
  int rc = QgsSLConnect::sqlite3_open( mDatabaseComboBox->currentText().toUtf8(), &db );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this,
                          tr( "SpatiaLite Database" ),
                          tr( "Unable to open the database: %1" ).arg( mDatabaseComboBox->currentText() ) );
  }
  else
  {
    char * errmsg;
    rc = sqlite3_exec( db, sql.toUtf8(), nullptr, nullptr, &errmsg );
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
      rc = sqlite3_exec( db, sqlAddGeom.toUtf8(), nullptr, nullptr, &errmsg );
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
        rc = sqlite3_exec( db, sqlCreateIndex.toUtf8(), nullptr, nullptr, &errmsg );
        if ( rc != SQLITE_OK )
        {
          QMessageBox::warning( this,
                                tr( "Error Creating Spatial Index" ),
                                tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
          sqlite3_free( errmsg );
        }

        QgsVectorLayer *layer = new QgsVectorLayer( QString( "dbname='%1' table='%2'(%3) sql=" )
            .arg( mDatabaseComboBox->currentText(),
                  leLayerName->text(),
                  leGeometryColumn->text() ), leLayerName->text(), "spatialite" );
        if ( layer->isValid() )
        {
          // register this layer with the central layers registry
          QList<QgsMapLayer *> myList;
          myList << layer;
          //addMapLayers returns a list of all successfully added layers
          //so we compare that to our original list.
          if ( myList == QgsMapLayerRegistry::instance()->addMapLayers( myList ) )
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

    QgsSLConnect::sqlite3_close( db );
  }

  return false;
}

QString QgsNewSpatialiteLayerDialog::quotedIdentifier( QString id )
{
  id.replace( '\"', "\"\"" );
  return id.prepend( '\"' ).append( '\"' );
}

QString QgsNewSpatialiteLayerDialog::quotedValue( QString value )
{
  value.replace( '\'', "''" );
  return value.prepend( '\'' ).append( '\'' );
}


