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
/* $Id$ */

/*
extern "C"
{
#include <spatialite/sqlite3.h>
#include <spatialite/gaiageo.h>
#include <spatialite.h>
}
*/
#include "qgsnewspatialitelayerdialog.h"
#include "qgsspatialitesridsdialog.h"
#include "qgsapplication.h"
#include "qgisapp.h" // <- for theme icons
#include "qgslogger.h"
//#include <sqlite3.h>
#include <QPushButton>
#include <QSettings>
#include <QLineEdit>
#include <QMessageBox>
#include <QFileDialog>



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
    //QString text = *it + tr( "@" );
    QString text = settings.value( *it + "/sqlitepath", "###unknown###" ).toString();
    mDatabaseComboBox->addItem( text );
    ++it;
  }
  settings.endGroup();
  mApplyButton = buttonBox->button( QDialogButtonBox::Apply );
  mApplyButton->setEnabled( false );
  mApplyButton->setDefault( true );
  connect( buttonBox->button( QDialogButtonBox::Apply ), SIGNAL( clicked() ), this, SLOT( apply() ) );

  // Set the SRID box to a default of WGS84
  leSRID->setText( "4326" );

  // flag to indicate if we need to create a new db before adding a layer
  needNewDb = false;
}

QgsNewSpatialiteLayerDialog::~QgsNewSpatialiteLayerDialog()
{
}

void QgsNewSpatialiteLayerDialog::createNewDb()
{
//  QMessageBox::information( 0, tr( "Spatialite Layer" ), tr( "Create new db clicked" ) );
  QString fileName = QFileDialog::getSaveFileName(this, tr("New Spatialite Database File"),
      ".",
      tr("Spatialite (*.sqlite *.db )"));

  if ( !fileName.isEmpty() )
  {
    mDatabaseComboBox->insertItem( 0, fileName );
    mDatabaseComboBox->setCurrentIndex(0);
    needNewDb = true;
  }  
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
}

QString QgsNewSpatialiteLayerDialog::selectedCrsId() const
{
  return leSRID->text();
}

void QgsNewSpatialiteLayerDialog::apply()
{
  // Check to see if the db exists and if not create it
  createDb();

  // Init spatialite
  spatialite_init( 0 );
  QString newLayerName = leLayerName->text();
  int rc = sqlite3_open_v2( mDatabaseComboBox->currentText().toUtf8(), &db, SQLITE_OPEN_READWRITE, NULL );
  if ( rc != SQLITE_OK )
  {
    QMessageBox::warning( this, "Spatialite Database", tr( "Unable to open the database"));
  }
  else
  {
    char * errmsg;
    QString sql = QString("CREATE TABLE %1 (id integer primary key autoincrement, name text)").arg( newLayerName );
    rc = sqlite3_exec( db, sql.toUtf8(), NULL, NULL, &errmsg);
    if ( rc != SQLITE_OK )
    {
      QMessageBox::warning( this, tr( "Error Creating Table" ),
          tr( "Failed to create the Spatialite table. The database returned:\n%1" ).arg( errmsg ) );
      sqlite3_free( errmsg );
    }
    else
    {
      // get the geometry type 
      QString geomType = selectedType();
      // create the geometry column and the spatial index
      sql = QString("select AddGeometryColumn('%1', 'geom', %2, '%3', 2)").arg( newLayerName ).arg( leSRID->text() ).arg( geomType );
      rc = sqlite3_exec( db, sql.toUtf8(), NULL, NULL, &errmsg);
      if ( rc != SQLITE_OK )
      {
        QMessageBox::warning( this, tr( "Error Creating Geometry Column" ),
            tr( "Failed to create the geometry column. The database returned:\n%1" ).arg( errmsg ) );
        sqlite3_free( errmsg );
      }
      else
      {
        // create the spatial index
        sql = QString("select CreateSpatialIndex('%1', 'geom')").arg( newLayerName );
        rc = sqlite3_exec( db, sql.toUtf8(), NULL, NULL, &errmsg);
        if ( rc != SQLITE_OK )
        {
          QMessageBox::warning( this, tr( "Error Creating Spatial Index" ),
              tr( "Failed to create the spatial index. The database returned:\n%1" ).arg( errmsg ) );
          sqlite3_free( errmsg );
        }
        else
        {
          QMessageBox::information( 0, tr( "Spatialite Layer" ), tr( "Layer created" ) );
        }
      }
    }
  }
}
void QgsNewSpatialiteLayerDialog::on_leLayerName_textChanged( QString text )
{
  if ( leLayerName->text().length() > 0 && mAttributeView->topLevelItemCount() > 0)
  {
    mApplyButton->setEnabled( true );
  }
}

void QgsNewSpatialiteLayerDialog::on_mAddAttributeButton_clicked()
{
  if ( mNameEdit->text().length() > 0 )
  {
    QString myName = mNameEdit->text();
    //use userrole to avoid translated type string
    QString myType = mTypeBox->itemData( mTypeBox->currentIndex(), Qt::UserRole ).toString();
    mAttributeView->addTopLevelItem( new QTreeWidgetItem( QStringList() << myName << myType  ) );
    if ( mAttributeView->topLevelItemCount() > 0  && leLayerName->text().length() > 0 )
    {
      mApplyButton->setEnabled( true );
    }
    mNameEdit->clear();
  }
}

void QgsNewSpatialiteLayerDialog::on_mRemoveAttributeButton_clicked()
{
  delete mAttributeView->currentItem();
  if ( mAttributeView->topLevelItemCount() == 0 )
  {
    mApplyButton->setEnabled( false );
  }
}

void QgsNewSpatialiteLayerDialog::on_pbnFindSRID_clicked()
{
  /* hold on to this for a bit
     int rc = sqlite3_open( mDatabaseComboBox->currentText().toUtf8().data(), &db );
     if ( rc != SQLITE_OK )
     {
     QMessageBox::warning( this, "Spatialite Database", tr( "Unable to open the database: %1").arg(mDatabaseComboBox->currentText() ) );
     }
     */
  // set the SRID from a list returned from the selected Spatialite database
  QgsSpatialiteSridsDialog *sridDlg = new QgsSpatialiteSridsDialog(this);
  if ( sridDlg->load( mDatabaseComboBox->currentText() ) )
  {
    if ( sridDlg->exec() )
    {
      // set the srid field to the selection
      sridDlg->selectedSrid();
      leSRID->setText( sridDlg->selectedSrid() );
      sridDlg->accept();
    }
  }
}

// Create a QList of QStringList objects that define the layer attributes. 
// Each QStringList contains the field name and its type.
QList<QStringList> * QgsNewSpatialiteLayerDialog::attributes() const
{
  QTreeWidgetItemIterator it( mAttributeView );
  QList<QStringList> *list = new QList<QStringList>;
  while ( *it )
  {
    QTreeWidgetItem *item = *it;
    QStringList items;
    items << item->text( 0 );
    items << item->text( 1 );
    list->append(items);
    
    //QString type = QString( "%1;%2;%3" ).arg( item->text( 1 ) ).arg( item->text( 2 ) ).arg( item->text( 3 ) );
    //at.push_back( std::make_pair( item->text( 0 ), type ) );
    //QgsDebugMsg( QString( "appending %1//%2" ).arg( item->text( 0 ) ).arg( type ) );
    ++it;
  }
  return list;
}

QString QgsNewSpatialiteLayerDialog::databaseName() const
{
  return mDatabaseComboBox->currentText();
}

QString QgsNewSpatialiteLayerDialog::layerName() const
{
  return leLayerName->text();
}
bool QgsNewSpatialiteLayerDialog::createDb()
{
  QFile newDb( mDatabaseComboBox->currentText() );
  if ( !newDb.exists() )
  {
    qWarning( "creating a new db" );
    // copy the spatilite template to the user specified path
    QString spatialiteTemplate = QgsApplication::qgisSpatialiteDbTemplatePath();
    QFile spatialiteTemplateDb( spatialiteTemplate );

    QFileInfo fullPath = QFileInfo( mDatabaseComboBox->currentText() );
    QDir path = fullPath.dir();
    qWarning("making this dir: %s", path.absolutePath().toUtf8().data());


    // Must be sure there is destination directory ~/.qgis
    QDir().mkpath( path.absolutePath( ) );

    qWarning("Copying %s ", spatialiteTemplate.toUtf8().data());
    qWarning("to %s", newDb.fileName().toUtf8().data());

    //now copy the template db file into the chosen location
    bool isDbFileCopied = spatialiteTemplateDb.copy( newDb.fileName() );

    if ( !isDbFileCopied )
    {
      // QMessageBox::warning( this, "Spatialite Database", tr( "Unable to copy the template database to your new location" ));
      return false;
    }
  }
    return true;
}
        
