/***************************************************************************
                          qgscustomprojectiondialog.cpp

                             -------------------
    begin                : 2005
    copyright            : (C) 2005 by Tim Sutton
    email                : tim@linfiniti.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgscustomprojectiondialog.h"

//qgis includes
#include "qgis.h" //<--magick numbers
#include "qgisapp.h" //<--theme icons
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsprojectionselectiondialog.h"
#include "qgscrscache.h"
#include "qgssettings.h"

//qt includes
#include <QFileInfo>
#include <QMessageBox>
#include <QLocale>

//stdc++ includes
#include <fstream>
#include <sqlite3.h>

//proj4 includes
extern "C"
{
#include <proj_api.h>
}


QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  connect( pbnCalculate, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCalculate_clicked );
  connect( pbnAdd, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnAdd_clicked );
  connect( pbnRemove, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnRemove_clicked );
  connect( pbnCopyCRS, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCopyCRS_clicked );
  connect( leNameList, &QTreeWidget::currentItemChanged, this, &QgsCustomProjectionDialog::leNameList_currentItemChanged );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsCustomProjectionDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsCustomProjectionDialog::showHelp );

  QgsSettings settings;
  restoreGeometry( settings.value( QStringLiteral( "Windows/CustomProjection/geometry" ) ).toByteArray() );

  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  QFileInfo fileInfo;
  fileInfo.setFile( QgsApplication::qgisSettingsDirPath() );
  if ( !fileInfo.exists() )
  {
    QgsDebugMsg( "The qgis.db does not exist" );
  }

  populateList();
  if ( !mCustomCRSnames.empty() )
  {
    leName->setText( mCustomCRSnames[0] );
    teParameters->setPlainText( mCustomCRSparameters[0] );
    leNameList->setCurrentItem( leNameList->topLevelItem( 0 ) );
  }

  leNameList->hideColumn( QgisCrsIdColumn );

}

QgsCustomProjectionDialog::~QgsCustomProjectionDialog()
{
  QgsSettings settings;
  settings.setValue( QStringLiteral( "Windows/CustomProjection/geometry" ), saveGeometry() );
}


void QgsCustomProjectionDialog::populateList()
{
  //Setup connection to the existing custom CRS database:
  sqlite3 *database = nullptr;
  const char *tail = nullptr;
  sqlite3_stmt *preparedStatement = nullptr;
  //check the db is available
  int result = sqlite3_open_v2( QgsApplication::qgisUserDatabaseFilePath().toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( database ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  QString sql = QStringLiteral( "select srs_id,description,parameters from tbl_srs" );
  QgsDebugMsg( QString( "Query to populate existing list:%1" ).arg( sql ) );
  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &preparedStatement, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK )
  {
    QTreeWidgetItem *newItem = nullptr;
    QString id, name, parameters;
    QgsCoordinateReferenceSystem crs;
    while ( sqlite3_step( preparedStatement ) == SQLITE_ROW )
    {
      id = QString::fromUtf8( ( char * ) sqlite3_column_text( preparedStatement, 0 ) );
      name = QString::fromUtf8( ( char * ) sqlite3_column_text( preparedStatement, 1 ) );
      parameters = QString::fromUtf8( ( char * ) sqlite3_column_text( preparedStatement, 2 ) );

      crs.createFromProj4( parameters );
      mExistingCRSnames[id] = name;
      mExistingCRSparameters[id] = crs.toProj4();

      newItem = new QTreeWidgetItem( leNameList, QStringList() );
      newItem->setText( QgisCrsNameColumn, name );
      newItem->setText( QgisCrsIdColumn, id );
      newItem->setText( QgisCrsParametersColumn, crs.toProj4() );
    }
  }
  else
  {
    QgsDebugMsg( QString( "Populate list query failed: %1" ).arg( sql ) );
  }
  sqlite3_finalize( preparedStatement );
  sqlite3_close( database );

  leNameList->sortByColumn( QgisCrsNameColumn, Qt::AscendingOrder );

  QTreeWidgetItemIterator it( leNameList );
  while ( *it )
  {
    QString id = ( *it )->text( QgisCrsIdColumn );
    mCustomCRSids.push_back( id );
    mCustomCRSnames.push_back( mExistingCRSnames[id] );
    mCustomCRSparameters.push_back( mExistingCRSparameters[id] );
    it++;
  }
}

bool  QgsCustomProjectionDialog::deleteCrs( const QString &id )
{
  sqlite3 *database = nullptr;
  const char *tail = nullptr;
  sqlite3_stmt *preparedStatement = nullptr;

  QString sql = "delete from tbl_srs where srs_id=" + quotedValue( id );
  QgsDebugMsg( sql );
  //check the db is available
  int result = sqlite3_open( QgsApplication::qgisUserDatabaseFilePath().toUtf8(), &database );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( sqlite3_errmsg( database ), QgsApplication::qgisUserDatabaseFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &preparedStatement, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result != SQLITE_OK || sqlite3_step( preparedStatement ) != SQLITE_DONE )
  {
    QgsDebugMsg( QString( "failed to remove CRS from database in custom projection dialog: %1 [%2]" ).arg( sql, sqlite3_errmsg( database ) ) );
  }
  sqlite3_close( database );

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransformCache::instance()->invalidateCrs( QStringLiteral( "USER:%1" ).arg( id ) );

  return result == SQLITE_OK;
}

void  QgsCustomProjectionDialog::insertProjection( const QString &projectionAcronym )
{
  sqlite3 *database = nullptr;
  sqlite3_stmt *preparedStatement = nullptr;
  sqlite3 *srsDatabase = nullptr;
  QString sql;
  const char *tail = nullptr;
  //check the db is available
  int result = sqlite3_open( QgsApplication::qgisUserDatabaseFilePath().toUtf8(), &database );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( sqlite3_errmsg( database ), QgsApplication::qgisUserDatabaseFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  int srsResult = sqlite3_open( QgsApplication::srsDatabaseFilePath().toUtf8(), &srsDatabase );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database %1 [%2]" ).arg( QgsApplication::srsDatabaseFilePath(), sqlite3_errmsg( srsDatabase ) ) );
  }
  else
  {
    // Set up the query to retrieve the projection information needed to populate the PROJECTION list
    QString srsSql = "select acronym,name,notes,parameters from tbl_projection where acronym=" + quotedValue( projectionAcronym );

    const char *srsTail = nullptr;
    sqlite3_stmt *srsPreparedStatement = nullptr;
    srsResult = sqlite3_prepare( srsDatabase, srsSql.toUtf8(), srsSql.length(), &srsPreparedStatement, &srsTail );
    // XXX Need to free memory from the error msg if one is set
    if ( srsResult == SQLITE_OK )
    {
      if ( sqlite3_step( srsPreparedStatement ) == SQLITE_ROW )
      {
        QgsDebugMsg( "Trying to insert projection" );
        // We have the result from system srs.db. Now insert into user db.
        sql = "insert into tbl_projection(acronym,name,notes,parameters) values ("
              + quotedValue( QString::fromUtf8( ( char * )sqlite3_column_text( srsPreparedStatement, 0 ) ) )
              + ',' + quotedValue( QString::fromUtf8( ( char * )sqlite3_column_text( srsPreparedStatement, 1 ) ) )
              + ',' + quotedValue( QString::fromUtf8( ( char * )sqlite3_column_text( srsPreparedStatement, 2 ) ) )
              + ',' + quotedValue( QString::fromUtf8( ( char * )sqlite3_column_text( srsPreparedStatement, 3 ) ) )
              + ')'
              ;
        result = sqlite3_prepare( database, sql.toUtf8(), sql.length(), &preparedStatement, &tail );
        if ( result != SQLITE_OK || sqlite3_step( preparedStatement ) != SQLITE_DONE )
        {
          QgsDebugMsg( QString( "Update or insert failed in custom projection dialog: %1 [%2]" ).arg( sql, sqlite3_errmsg( database ) ) );
        }

        sqlite3_finalize( preparedStatement );
      }

      sqlite3_finalize( srsPreparedStatement );
    }
    else
    {
      QgsDebugMsg( QString( "prepare failed: %1 [%2]" ).arg( srsSql, sqlite3_errmsg( srsDatabase ) ) );
    }

    sqlite3_close( srsDatabase );
  }
  // close sqlite3 db
  sqlite3_close( database );
}

bool QgsCustomProjectionDialog::saveCrs( QgsCoordinateReferenceSystem parameters, const QString &name, QString id, bool newEntry )
{
  QString sql;
  int returnId;
  QString projectionAcronym = parameters.projectionAcronym();
  QString ellipsoidAcronym = parameters.ellipsoidAcronym();
  QgsDebugMsg( QString( "Saving a CRS:%1, %2, %3" ).arg( name, parameters.toProj4() ).arg( newEntry ) );
  if ( newEntry )
  {
    returnId = parameters.saveAsUserCrs( name );
    if ( returnId == -1 )
      return false;
    else
      id = QString::number( returnId );
  }
  else
  {
    sql = "update tbl_srs set description="
          + quotedValue( name )
          + ",projection_acronym=" + quotedValue( projectionAcronym )
          + ",ellipsoid_acronym=" + quotedValue( ellipsoidAcronym )
          + ",parameters=" + quotedValue( parameters.toProj4() )
          + ",is_geo=0" // <--shamelessly hard coded for now
          + " where srs_id=" + quotedValue( id )
          ;
    QgsDebugMsg( sql );
    sqlite3 *database = nullptr;
    const char *tail = nullptr;
    sqlite3_stmt *preparedStatement = nullptr;
    //check if the db is available
    int result = sqlite3_open( QgsApplication::qgisUserDatabaseFilePath().toUtf8(), &database );
    if ( result != SQLITE_OK )
    {
      QgsDebugMsg( QString( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( sqlite3_errmsg( database ), QgsApplication::qgisUserDatabaseFilePath() ) );
      // XXX This will likely never happen since on open, sqlite creates the
      //     database if it does not exist.
      Q_ASSERT( result == SQLITE_OK );
    }
    result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &preparedStatement, &tail );
    // XXX Need to free memory from the error msg if one is set
    if ( result != SQLITE_OK || sqlite3_step( preparedStatement ) != SQLITE_DONE )
    {
      QgsDebugMsg( QString( "failed to write to database in custom projection dialog: %1 [%2]" ).arg( sql, sqlite3_errmsg( database ) ) );
    }

    sqlite3_finalize( preparedStatement );
    // close sqlite3 db
    sqlite3_close( database );
    if ( result != SQLITE_OK )
      return false;
  }
  mExistingCRSparameters[id] = parameters.toProj4();
  mExistingCRSnames[id] = name;

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransformCache::instance()->invalidateCrs( QStringLiteral( "USER:%1" ).arg( id ) );

  // If we have a projection acronym not in the user db previously, add it.
  // This is a must, or else we can't select it from the vw_srs table.
  // Actually, add it always and let the SQL PRIMARY KEY remove duplicates.
  insertProjection( projectionAcronym );

  return true;
}


void QgsCustomProjectionDialog::pbnAdd_clicked()
{
  QString name = tr( "new CRS" );
  QString id;
  QgsCoordinateReferenceSystem parameters;

  QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );

  newItem->setText( QgisCrsNameColumn, name );
  newItem->setText( QgisCrsIdColumn, id );
  newItem->setText( QgisCrsParametersColumn, parameters.toProj4() );
  mCustomCRSnames.push_back( name );
  mCustomCRSids.push_back( id );
  mCustomCRSparameters.push_back( parameters.toProj4() );
  leNameList->setCurrentItem( newItem );
}

void QgsCustomProjectionDialog::pbnRemove_clicked()
{
  int i = leNameList->currentIndex().row();
  if ( i == -1 )
  {
    return;
  }
  QTreeWidgetItem *item = leNameList->takeTopLevelItem( i );
  delete item;
  if ( !mCustomCRSids[i].isEmpty() )
  {
    mDeletedCRSs.push_back( mCustomCRSids[i] );
  }
  mCustomCRSids.erase( mCustomCRSids.begin() + i );
  mCustomCRSnames.erase( mCustomCRSnames.begin() + i );
  mCustomCRSparameters.erase( mCustomCRSparameters.begin() + i );
}

void QgsCustomProjectionDialog::leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  //Store the modifications made to the current element before moving on
  int currentIndex, previousIndex;
  if ( previous )
  {
    previousIndex = leNameList->indexOfTopLevelItem( previous );
    mCustomCRSnames[previousIndex] = leName->text();
    mCustomCRSparameters[previousIndex] = teParameters->toPlainText();
    previous->setText( QgisCrsNameColumn, leName->text() );
    previous->setText( QgisCrsParametersColumn, teParameters->toPlainText() );
  }
  if ( current )
  {
    currentIndex = leNameList->indexOfTopLevelItem( current );
    leName->setText( mCustomCRSnames[currentIndex] );
    teParameters->setPlainText( current->text( QgisCrsParametersColumn ) );
  }
  else
  {
    //Can happen that current is null, for example if we just deleted the last element
    leName->clear();
    teParameters->clear();
    return;
  }
}

void QgsCustomProjectionDialog::pbnCopyCRS_clicked()
{
  QgsProjectionSelectionDialog *selector = new QgsProjectionSelectionDialog( this );
  if ( selector->exec() )
  {
    QgsCoordinateReferenceSystem srs = selector->crs();
    if ( leNameList->topLevelItemCount() == 0 )
    {
      pbnAdd_clicked();
    }
    teParameters->setPlainText( srs.toProj4() );
    mCustomCRSparameters[leNameList->currentIndex().row()] = srs.toProj4();
    leNameList->currentItem()->setText( QgisCrsParametersColumn, srs.toProj4() );

  }
  delete selector;
}

void QgsCustomProjectionDialog::buttonBox_accepted()
{
  //Update the current CRS:
  int i = leNameList->currentIndex().row();
  if ( i != -1 )
  {
    mCustomCRSnames[i] = leName->text();
    mCustomCRSparameters[i] = teParameters->toPlainText();
  }

  QgsDebugMsg( "We save the modified CRS." );

  //Check if all CRS are valid:
  QgsCoordinateReferenceSystem CRS;
  for ( int i = 0; i < mCustomCRSids.size(); ++i )
  {
    CRS.createFromProj4( mCustomCRSparameters[i] );
    if ( !CRS.isValid() )
    {
      QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                                tr( "The proj4 definition of '%1' is not valid." ).arg( mCustomCRSnames[i] ) );
      return;
    }
  }
  //Modify the CRS changed:
  bool save_success = true;
  for ( int i = 0; i < mCustomCRSids.size(); ++i )
  {
    CRS.createFromProj4( mCustomCRSparameters[i] );
    //Test if we just added this CRS (if it has no existing ID)
    if ( !mCustomCRSids[i].isEmpty() )
    {
      save_success &= saveCrs( CRS, mCustomCRSnames[i], QLatin1String( "" ), true );
    }
    else
    {
      if ( mExistingCRSnames[mCustomCRSids[i]] != mCustomCRSnames[i] || mExistingCRSparameters[mCustomCRSids[i]] != mCustomCRSparameters[i] )
      {
        save_success &= saveCrs( CRS, mCustomCRSnames[i], mCustomCRSids[i], false );
      }
    }
    if ( ! save_success )
    {
      QgsDebugMsg( QString( "Error when saving CRS '%1'" ).arg( mCustomCRSnames[i] ) );
    }
  }
  QgsDebugMsg( "We remove the deleted CRS." );
  for ( int i = 0; i < mDeletedCRSs.size(); ++i )
  {
    save_success &= deleteCrs( mDeletedCRSs[i] );
    if ( ! save_success )
    {
      QgsDebugMsg( QString( "Problem for layer '%1'" ).arg( mCustomCRSparameters[i] ) );
    }
  }
  if ( save_success )
  {
    accept();
  }
}

void QgsCustomProjectionDialog::pbnCalculate_clicked()
{
  // We must check the prj def is valid!
  projCtx pContext = pj_ctx_alloc();
  projPJ proj = pj_init_plus_ctx( pContext, teParameters->toPlainText().toLocal8Bit().data() );

  QgsDebugMsg( QString( "Proj: %1" ).arg( teParameters->toPlainText() ) );

  if ( !proj )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "This proj4 projection definition is not valid." ) );
    projectedX->clear();
    projectedY->clear();
    pj_free( proj );
    pj_ctx_free( pContext );
    return;

  }
  // Get the WGS84 coordinates
  bool okN, okE;
  double northing = northWGS84->text().toDouble( &okN ) * DEG_TO_RAD;
  double easthing = eastWGS84->text().toDouble( &okE )  * DEG_TO_RAD;

  if ( !okN || !okE )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "Northing and Easthing must be in decimal form." ) );
    projectedX->clear();
    projectedY->clear();
    pj_free( proj );
    pj_ctx_free( pContext );
    return;
  }

  projPJ wgs84Proj = pj_init_plus_ctx( pContext, GEOPROJ4.toLocal8Bit().data() ); //defined in qgis.h

  if ( !wgs84Proj )
  {
    QMessageBox::information( this, tr( "QGIS Custom Projection" ),
                              tr( "Internal Error (source projection invalid?)" ) );
    projectedX->clear();
    projectedY->clear();
    pj_free( wgs84Proj );
    pj_ctx_free( pContext );
    return;
  }

  double z = 0.0;

  int projResult = pj_transform( wgs84Proj, proj, 1, 0, &easthing, &northing, &z );
  if ( projResult != 0 )
  {
    projectedX->setText( tr( "Error" ) );
    projectedY->setText( tr( "Error" ) );
    QgsDebugMsg( pj_strerrno( projResult ) );
  }
  else
  {
    QString tmp;

    tmp = QLocale::system().toString( northing, 'f', 4 );
    projectedX->setText( tmp );
    tmp = QLocale::system().toString( easthing, 'f', 4 );
    projectedY->setText( tmp );
  }

  pj_free( proj );
  pj_free( wgs84Proj );
  pj_ctx_free( pContext );
}

QString QgsCustomProjectionDialog::quotedValue( QString value )
{
  value.replace( '\'', QLatin1String( "''" ) );
  return value.prepend( '\'' ).append( '\'' );
}

void QgsCustomProjectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}
