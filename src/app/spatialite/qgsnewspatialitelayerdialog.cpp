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

#include "qgsspatialitesridsdialog.h"
#include "qgsapplication.h"
#include "qgsproviderregistry.h"
#include "qgisapp.h" // <- for theme icons
#include <qgsvectorlayer.h>
#include <qgsmaplayerregistry.h>

#include "qgslogger.h"

#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>
#include <QLibrary>

#include <spatialite.h>

QgsNewSpatialiteLayerDialog::QgsNewSpatialiteLayerDialog( QWidget *parent, Qt::WFlags fl )
    : QDialog( parent, fl )
{
  setupUi( this );
  mAddAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionNewAttribute.png" ) );
  mRemoveAttributeButton->setIcon( QgisApp::getThemeIcon( "/mActionDeleteAttribute.png" ) );
  mTypeBox->addItem( tr( "Text data" ), "text" );
  mTypeBox->addItem( tr( "Whole number" ), "integer" );
  mTypeBox->addItem( tr( "Decimal number" ), "real" );

  mPointRadioButton->setChecked( true );
  // Populate the database list from the stored connections
  QSettings settings;
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

  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
  buttonBox->button( QDialogButtonBox::Apply )->setEnabled( false );

  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  buttonBox->button( QDialogButtonBox::Ok )->setDefault( true );

  // Set the SRID box to a default of WGS84
  leSRID->setText( "4326" );
  pbnFindSRID->setEnabled( mDatabaseComboBox->count() );
}

QgsNewSpatialiteLayerDialog::~QgsNewSpatialiteLayerDialog()
{
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
                     ".",
                     tr( "SpatiaLite (*.sqlite *.db )" ) );

  if ( fileName.isEmpty() )
    return;

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

  Q_ASSERT( "no type selected" == 0 );
  return "";
}

void QgsNewSpatialiteLayerDialog::on_leLayerName_textChanged( QString text )
{
  Q_UNUSED( text );
  bool created  = leLayerName->text().length() > 0 && mAttributeView->topLevelItemCount() > 0 && createDb();
  buttonBox->button( QDialogButtonBox::Ok )->setEnabled( created );
  buttonBox->button( QDialogButtonBox::Apply )->setEnabled( created );
}

void QgsNewSpatialiteLayerDialog::on_mAddAttributeButton_clicked()
{
  if ( mNameEdit->text().length() > 0 )
  {
    QString myName = mNameEdit->text();
    //use userrole to avoid translated type string
    QString myType = mTypeBox->itemData( mTypeBox->currentIndex(), Qt::UserRole ).toString();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType ) );
    if ( mAttributeView->topLevelItemCount() > 0  && leLayerName->text().length() > 0 )
    {
      bool created = createDb();
      buttonBox->button( QDialogButtonBox::Ok )->setEnabled( created );
      buttonBox->button( QDialogButtonBox::Apply )->setEnabled( created );
    }
    mNameEdit->clear();
  }
}

void QgsNewSpatialiteLayerDialog::on_mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();
  if ( mAttributeView->topLevelItemCount() == 0 )
  {
    buttonBox->button( QDialogButtonBox::Ok )->setEnabled( false );
    buttonBox->button( QDialogButtonBox::Apply )->setEnabled( false );
  }
}

void QgsNewSpatialiteLayerDialog::on_pbnFindSRID_clicked()
{
  // set the SRID from a list returned from the selected Spatialite database
  QgsSpatialiteSridsDialog *sridDlg = new QgsSpatialiteSridsDialog( this );
  if ( sridDlg->load( mDatabaseComboBox->currentText() ) )
  {
    if ( sridDlg->exec() )
    {
      // set the srid field to the selection
      leSRID->setText( sridDlg->selectedSrid() );
      sridDlg->accept();
    }
  }
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
      QMessageBox::warning( 0, tr( "SpatiaLite Database" ), errCause );
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

    QMessageBox::information( 0, tr( "SpatiaLite Database" ), tr( "Registered new database!" ) );
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
    sql += "pkuid integer primary key autoincrement,";
  }

  QTreeWidgetItemIterator it( mAttributeView );
  while ( *it )
  {
    sql += delim + QString( "%1 %2" ).arg( quotedIdentifier(( *it )->text( 0 ) ) ).arg(( *it )->text( 1 ) );

    delim = ",";

    ++it;
  }

  // complete the create table statement
  sql += ")";

  QgsDebugMsg( QString( "Creating table in database %1" ).arg( mDatabaseComboBox->currentText() ) );

  QgsDebugMsg( sql ); // OK

  QString sqlAddGeom = QString( "select AddGeometryColumn(%1,%2,%3,%4,2)" )
                       .arg( quotedValue( leLayerName->text() ) )
                       .arg( quotedValue( leGeometryColumn->text() ) )
                       .arg( leSRID->text().toInt() )
                       .arg( quotedValue( selectedType() ) );
  QgsDebugMsg( sqlAddGeom ); // OK

  QString sqlCreateIndex = QString( "select CreateSpatialIndex(%1,%2)" )
                           .arg( quotedValue( leLayerName->text() ) )
                           .arg( quotedValue( leGeometryColumn->text() ) );
  QgsDebugMsg( sqlCreateIndex ); // OK

  spatialite_init( 0 );

  sqlite3 *db;
  int rc = sqlite3_open( mDatabaseComboBox->currentText().toUtf8(), &db );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this,
                          tr( "SpatiaLite Database" ),
                          tr( "Unable to open the database: %1" ).arg( mDatabaseComboBox->currentText() ) );
  }
  else
  {
    char * errmsg;
    rc = sqlite3_exec( db, sql.toUtf8(), NULL, NULL, &errmsg );
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this,
                            tr( "Error Creating SpatiaLite Table" ),
                            tr( "Failed to create the SpatiaLite table %1. The database returned:\n%2" ).arg( leLayerName->text() ).arg( errmsg ) );
      sqlite3_free( errmsg );
    }
    else
    {
      // create the geometry column and the spatial index
      rc = sqlite3_exec( db, sqlAddGeom.toUtf8(), NULL, NULL, &errmsg );
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
        rc = sqlite3_exec( db, sqlCreateIndex.toUtf8(), NULL, NULL, &errmsg );
        if ( rc != SQLITE_OK )
        {
          QMessageBox::warning( this,
                                tr( "Error Creating Spatial Index" ),
                                tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
          sqlite3_free( errmsg );
        }

        QgsVectorLayer *layer = new QgsVectorLayer( QString( "dbname='%1' table='%2'(%3) sql=" )
            .arg( mDatabaseComboBox->currentText() )
            .arg( leLayerName->text() )
            .arg( leGeometryColumn->text() ), leLayerName->text(), "spatialite" );
        if ( layer->isValid() )
        {
          // register this layer with the central layers registry
          if ( QgsMapLayerRegistry::instance()->addMapLayer( layer ) )
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
  id.replace( "\"", "\"\"" );
  return id.prepend( "\"" ).append( "\"" );
}

QString QgsNewSpatialiteLayerDialog::quotedValue( QString value )
{
  value.replace( "'", "''" );
  return value.prepend( "'" ).append( "'" );
}


