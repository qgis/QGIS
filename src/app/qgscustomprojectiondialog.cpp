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
#include "qgssettings.h"
#include "qgssqliteutils.h"
#include "qgsgui.h"

//qt includes
#include <QFileInfo>
#include <QMessageBox>
#include <QLocale>

//stdc++ includes
#include <fstream>
#include <sqlite3.h>

//proj4 includes
#if PROJ_VERSION_MAJOR>=6
#include "qgsprojutils.h"
#include <proj.h>
#else
#include <proj_api.h>
#endif

#include "qgsogrutils.h"
#include <ogr_srs_api.h>

QgsCustomProjectionDialog::QgsCustomProjectionDialog( QWidget *parent, Qt::WindowFlags fl )
  : QDialog( parent, fl )
{
  setupUi( this );
  QgsGui::enableAutoGeometryRestore( this );

  connect( pbnCalculate, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCalculate_clicked );
  connect( pbnAdd, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnAdd_clicked );
  connect( pbnRemove, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnRemove_clicked );
  connect( pbnCopyCRS, &QPushButton::clicked, this, &QgsCustomProjectionDialog::pbnCopyCRS_clicked );
  connect( leNameList, &QTreeWidget::currentItemChanged, this, &QgsCustomProjectionDialog::leNameList_currentItemChanged );
  connect( buttonBox, &QDialogButtonBox::accepted, this, &QgsCustomProjectionDialog::buttonBox_accepted );
  connect( buttonBox, &QDialogButtonBox::helpRequested, this, &QgsCustomProjectionDialog::showHelp );
  connect( mButtonValidate, &QPushButton::clicked, this, &QgsCustomProjectionDialog::validateCurrent );

  leNameList->setSelectionMode( QAbstractItemView::ExtendedSelection );

  mFormatComboBox->addItem( tr( "WKT (Recommended)" ), static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) );
  mFormatComboBox->addItem( tr( "Proj String (Legacy — Not Recommended)" ), static_cast< int >( QgsCoordinateReferenceSystem::FormatProj ) );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );

  // user database is created at QGIS startup in QgisApp::createDB
  // we just check whether there is our database [MD]
  QFileInfo fileInfo;
  fileInfo.setFile( QgsApplication::qgisSettingsDirPath() );
  if ( !fileInfo.exists() )
  {
    QgsDebugMsg( QStringLiteral( "The qgis.db does not exist" ) );
  }

  populateList();
  if ( mDefinitions.empty() )
  {
    // create an empty definition which corresponds to the initial state of the dialog
    mDefinitions << Definition();
    QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );
    newItem->setText( QgisCrsNameColumn, QString() );
    newItem->setText( QgisCrsParametersColumn, QString() );
  }
  whileBlocking( leName )->setText( mDefinitions[0].name );
  whileBlocking( teParameters )->setPlainText( mDefinitions[0].wkt.isEmpty() ? mDefinitions[0].proj : mDefinitions[0].wkt );
  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mDefinitions[0].wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatProj : QgsCoordinateReferenceSystem::FormatWkt ) ) );
  leNameList->setCurrentItem( leNameList->topLevelItem( 0 ) );

  leNameList->hideColumn( QgisCrsIdColumn );

  connect( leName, &QLineEdit::textChanged, this, &QgsCustomProjectionDialog::updateListFromCurrentItem );
  connect( teParameters, &QPlainTextEdit::textChanged, this, &QgsCustomProjectionDialog::updateListFromCurrentItem );
  connect( mFormatComboBox, qgis::overload<int>::of( &QComboBox::currentIndexChanged ), this, &QgsCustomProjectionDialog::formatChanged );
}

void QgsCustomProjectionDialog::populateList()
{
  //Setup connection to the existing custom CRS database:
  sqlite3_database_unique_ptr database;
  sqlite3_statement_unique_ptr preparedStatement;
  //check the db is available
  int result = database.open_v2( QgsApplication::qgisUserDatabaseFilePath(), SQLITE_OPEN_READONLY, nullptr );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database: %1" ).arg( database.errorMessage() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  QString sql = QStringLiteral( "select srs_id,description,parameters, wkt from tbl_srs" );
  QgsDebugMsgLevel( QStringLiteral( "Query to populate existing list:%1" ).arg( sql ), 4 );
  preparedStatement = database.prepare( sql, result );
  if ( result == SQLITE_OK )
  {
    QgsCoordinateReferenceSystem crs;
    while ( preparedStatement.step() == SQLITE_ROW )
    {
      const QString id = preparedStatement.columnAsText( 0 );
      const QString name = preparedStatement.columnAsText( 1 );
      const QString parameters = preparedStatement.columnAsText( 2 );
      const QString wkt = preparedStatement.columnAsText( 3 );

      if ( !wkt.isEmpty() )
        crs.createFromWkt( wkt );
      else
        crs.createFromProj( parameters );

      mExistingCRSnames[id] = name;
      const QString actualWkt = crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false );
      const QString actualWktFormatted = crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true );
      const QString actualProj = crs.toProj();
      mExistingCRSwkt[id] = wkt.isEmpty() ? QString() : actualWkt;
      mExistingCRSproj[id] = wkt.isEmpty() ? actualProj : QString();

      QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );
      newItem->setText( QgisCrsNameColumn, name );
      newItem->setText( QgisCrsIdColumn, id );
      newItem->setText( QgisCrsParametersColumn, wkt.isEmpty() ? actualProj : actualWkt );
      newItem->setData( 0, FormattedWktRole, actualWktFormatted );
    }
  }
  else
  {
    QgsDebugMsg( QStringLiteral( "Populate list query failed: %1" ).arg( sql ) );
  }
  preparedStatement.reset();

  leNameList->sortByColumn( QgisCrsNameColumn, Qt::AscendingOrder );

  QTreeWidgetItemIterator it( leNameList );
  while ( *it )
  {
    QString id = ( *it )->text( QgisCrsIdColumn );
    Definition def;
    def.id = id;
    def.name = mExistingCRSnames[id];
    def.wkt = mExistingCRSwkt[id];
    def.proj = mExistingCRSproj[id];
    mDefinitions.push_back( def );
    it++;
  }
}

bool  QgsCustomProjectionDialog::deleteCrs( const QString &id )
{
  sqlite3_database_unique_ptr database;

  QString sql = "delete from tbl_srs where srs_id=" + QgsSqliteUtils::quotedString( id );
  QgsDebugMsgLevel( sql, 4 );
  //check the db is available
  int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( database.errorMessage(),
                 QgsApplication::qgisUserDatabaseFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  {
    sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
    if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
    {
      QgsDebugMsg( QStringLiteral( "failed to remove CRS from database in custom projection dialog: %1 [%2]" ).arg( sql, database.errorMessage() ) );
    }
  }

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  return result == SQLITE_OK;
}

void  QgsCustomProjectionDialog::insertProjection( const QString &projectionAcronym )
{
  sqlite3_database_unique_ptr database;
  sqlite3_database_unique_ptr srsDatabase;
  QString sql;
  //check the db is available
  int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( database.errorMessage(),
                 QgsApplication::qgisUserDatabaseFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    Q_ASSERT( result == SQLITE_OK );
  }
  int srsResult = srsDatabase.open( QgsApplication::srsDatabaseFilePath() );
  if ( result != SQLITE_OK )
  {
    QgsDebugMsg( QStringLiteral( "Can't open database %1 [%2]" ).arg( QgsApplication::srsDatabaseFilePath(),
                 srsDatabase.errorMessage() ) );
  }
  else
  {
    // Set up the query to retrieve the projection information needed to populate the PROJECTION list
    QString srsSql = "select acronym,name,notes,parameters from tbl_projection where acronym=" + QgsSqliteUtils::quotedString( projectionAcronym );

    sqlite3_statement_unique_ptr srsPreparedStatement = srsDatabase.prepare( srsSql, srsResult );
    if ( srsResult == SQLITE_OK )
    {
      if ( srsPreparedStatement.step() == SQLITE_ROW )
      {
        QgsDebugMsgLevel( QStringLiteral( "Trying to insert projection" ), 4 );
        // We have the result from system srs.db. Now insert into user db.
        sql = "insert into tbl_projection(acronym,name,notes,parameters) values ("
              + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 0 ) )
              + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 1 ) )
              + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 2 ) )
              + ',' + QgsSqliteUtils::quotedString( srsPreparedStatement.columnAsText( 3 ) )
              + ')';
        sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
        if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
        {
          QgsDebugMsg( QStringLiteral( "Update or insert failed in custom projection dialog: %1 [%2]" ).arg( sql, database.errorMessage() ) );
        }
      }
    }
    else
    {
      QgsDebugMsg( QStringLiteral( "prepare failed: %1 [%2]" ).arg( srsSql, srsDatabase.errorMessage() ) );
    }
  }
}

bool QgsCustomProjectionDialog::saveCrs( QgsCoordinateReferenceSystem crs, const QString &name, const QString &existingId, bool newEntry, QgsCoordinateReferenceSystem::Format format )
{
  QString id = existingId;
  QString sql;
  long returnId = -1;
  QString projectionAcronym = crs.projectionAcronym();
  QString ellipsoidAcronym = crs.ellipsoidAcronym();
  if ( newEntry )
  {
    returnId = crs.saveAsUserCrs( name, format );
    if ( returnId == -1 )
      return false;
    else
      id = QString::number( returnId );
  }
  else
  {
    sql = "update tbl_srs set description="
          + QgsSqliteUtils::quotedString( name )
          + ",projection_acronym=" + ( !projectionAcronym.isEmpty() ? QgsSqliteUtils::quotedString( projectionAcronym ) : QStringLiteral( "''" ) )
          + ",ellipsoid_acronym=" + ( !ellipsoidAcronym.isEmpty() ? QgsSqliteUtils::quotedString( ellipsoidAcronym ) : QStringLiteral( "''" ) )
          + ",parameters=" + ( !crs.toProj().isEmpty() ? QgsSqliteUtils::quotedString( crs.toProj() ) : QStringLiteral( "''" ) )
          + ",is_geo=0" // <--shamelessly hard coded for now
          + ",wkt=" + ( format == QgsCoordinateReferenceSystem::FormatWkt ? QgsSqliteUtils::quotedString( crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) ) : QStringLiteral( "''" ) )
          + " where srs_id=" + QgsSqliteUtils::quotedString( id )
          ;
    QgsDebugMsgLevel( sql, 4 );
    sqlite3_database_unique_ptr database;
    //check if the db is available
    int result = database.open( QgsApplication::qgisUserDatabaseFilePath() );
    if ( result != SQLITE_OK )
    {
      QgsDebugMsg( QStringLiteral( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( database.errorMessage(),
                   QgsApplication::qgisUserDatabaseFilePath() ) );
      // XXX This will likely never happen since on open, sqlite creates the
      //     database if it does not exist.
      Q_ASSERT( result == SQLITE_OK );
    }
    sqlite3_statement_unique_ptr preparedStatement = database.prepare( sql, result );
    if ( result != SQLITE_OK || preparedStatement.step() != SQLITE_DONE )
    {
      QgsDebugMsg( QStringLiteral( "failed to write to database in custom projection dialog: %1 [%2]" ).arg( sql, database.errorMessage() ) );
    }

    preparedStatement.reset();
    if ( result != SQLITE_OK )
      return false;
  }
  mExistingCRSwkt[id] = format == QgsCoordinateReferenceSystem::FormatWkt ? crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) : QString();
  mExistingCRSproj[id] = format == QgsCoordinateReferenceSystem::FormatProj ? crs.toProj() : QString();
  mExistingCRSnames[id] = name;

  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateTransform::invalidateCache();

  // If we have a projection acronym not in the user db previously, add it.
  // This is a must, or else we can't select it from the vw_srs table.
  // Actually, add it always and let the SQL PRIMARY KEY remove duplicates.
  insertProjection( projectionAcronym );

  return true;
}


void QgsCustomProjectionDialog::pbnAdd_clicked()
{
  QString name = tr( "new CRS" );

  QTreeWidgetItem *newItem = new QTreeWidgetItem( leNameList, QStringList() );

  newItem->setText( QgisCrsNameColumn, name );
  newItem->setText( QgisCrsIdColumn, QString() );
  newItem->setText( QgisCrsParametersColumn, QString() );
  newItem->setData( 0, FormattedWktRole, QString() );

  Definition def;
  def.name = name;
  mDefinitions.push_back( def );
  leNameList->setCurrentItem( newItem );
  leName->selectAll();
  leName->setFocus();

  mFormatComboBox->setCurrentIndex( mFormatComboBox->findData( QgsCoordinateReferenceSystem::FormatWkt ) );
}

void QgsCustomProjectionDialog::pbnRemove_clicked()
{
  const QModelIndexList selection = leNameList->selectionModel()->selectedRows();
  if ( selection.empty() )
    return;

  // make sure the user really wants to delete these definitions
  if ( QMessageBox::No == QMessageBox::question( this, tr( "Delete Projections" ),
       tr( "Are you sure you want to delete %n projections(s)?", "number of rows", selection.size() ),
       QMessageBox::Yes | QMessageBox::No ) )
    return;

  std::vector< int > selectedRows;
  selectedRows.reserve( selection.size() );
  for ( const QModelIndex &index : selection )
    selectedRows.emplace_back( index.row() );

  //sort rows in reverse order
  std::sort( selectedRows.begin(), selectedRows.end(), std::greater< int >() );
  for ( const int row : selectedRows )
  {
    if ( row < 0 )
    {
      // shouldn't happen?
      continue;
    }
    delete leNameList->takeTopLevelItem( row );
    if ( !mDefinitions[row].id.isEmpty() )
    {
      mDeletedCRSs.push_back( mDefinitions[row].id );
    }
    mDefinitions.erase( mDefinitions.begin() + row );
  }
}

void QgsCustomProjectionDialog::leNameList_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem *previous )
{
  //Store the modifications made to the current element before moving on
  int currentIndex, previousIndex;
  if ( previous )
  {
    previousIndex = leNameList->indexOfTopLevelItem( previous );

    mDefinitions[previousIndex].name = leName->text();
    switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
    {
      case QgsCoordinateReferenceSystem::FormatWkt:
        mDefinitions[previousIndex].wkt = teParameters->toPlainText();
        mDefinitions[previousIndex].proj.clear();
        break;

      case QgsCoordinateReferenceSystem::FormatProj:
        mDefinitions[previousIndex].proj = teParameters->toPlainText();
        mDefinitions[previousIndex].wkt.clear();
        break;
    }

    previous->setText( QgisCrsNameColumn, leName->text() );
    previous->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( teParameters->toPlainText() ) );
    previous->setData( 0, FormattedWktRole, teParameters->toPlainText() );
  }

  if ( current )
  {
    currentIndex = leNameList->indexOfTopLevelItem( current );
    whileBlocking( leName )->setText( mDefinitions[currentIndex].name );
    whileBlocking( teParameters )->setPlainText( !mDefinitions[currentIndex].wkt.isEmpty() ? current->data( 0, FormattedWktRole ).toString() : mDefinitions[currentIndex].proj );
    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( mDefinitions[currentIndex].wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatProj : QgsCoordinateReferenceSystem::FormatWkt ) ) );
  }
  else
  {
    //Can happen that current is null, for example if we just deleted the last element
    leName->clear();
    teParameters->clear();
    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );
    return;
  }
}

void QgsCustomProjectionDialog::pbnCopyCRS_clicked()
{
  std::unique_ptr< QgsProjectionSelectionDialog > selector = qgis::make_unique< QgsProjectionSelectionDialog >( this );
  if ( selector->exec() )
  {
    QgsCoordinateReferenceSystem srs = selector->crs();
    if ( leNameList->topLevelItemCount() == 0 )
    {
      pbnAdd_clicked();
    }

    whileBlocking( mFormatComboBox )->setCurrentIndex( mFormatComboBox->findData( static_cast< int >( QgsCoordinateReferenceSystem::FormatWkt ) ) );
    teParameters->setPlainText( srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
    mDefinitions[leNameList->currentIndex().row()].wkt = srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false );
    mDefinitions[leNameList->currentIndex().row()].proj.clear();

    leNameList->currentItem()->setText( QgisCrsParametersColumn, srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, false ) );
    leNameList->currentItem()->setData( 0, FormattedWktRole, srs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED, true ) );
  }
}

void QgsCustomProjectionDialog::buttonBox_accepted()
{
  updateListFromCurrentItem();

  QgsDebugMsgLevel( QStringLiteral( "We save the modified CRS." ), 4 );

  //Check if all CRS are valid:
  QgsCoordinateReferenceSystem crs;
  for ( const Definition &def : qgis::as_const( mDefinitions ) )
  {
    if ( !def.wkt.isEmpty() )
      crs.createFromWkt( def.wkt );
    else
      crs.createFromProj( def.proj );

    if ( !crs.isValid() )
    {
      // auto select the invalid CRS row
      for ( int row = 0; row < leNameList->model()->rowCount(); ++row )
      {
        if ( leNameList->model()->data( leNameList->model()->index( row, QgisCrsNameColumn ) ).toString() == def.name )
        {
          leNameList->setCurrentItem( leNameList->invisibleRootItem()->child( row ) );
          break;
        }
      }

      QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                            tr( "The definition of '%1' is not valid." ).arg( def.name ) );
      return;
    }
    else if ( !crs.authid().isEmpty() && !crs.authid().startsWith( QLatin1String( "USER" ), Qt::CaseInsensitive ) )
    {
      // auto select the invalid CRS row
      for ( int row = 0; row < leNameList->model()->rowCount(); ++row )
      {
        if ( leNameList->model()->data( leNameList->model()->index( row, QgisCrsNameColumn ) ).toString() == def.name )
        {
          leNameList->setCurrentItem( leNameList->invisibleRootItem()->child( row ) );
          break;
        }
      }

      if ( def.wkt.isEmpty() )
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "Cannot save '%1' — this Proj string definition is equivalent to %2.\n\nTry changing the CRS definition to a WKT format instead." ).arg( def.name, crs.authid() ) );
      }
      else
      {
        const QStringList authparts = crs.authid().split( ':' );
        QString ref;
        if ( authparts.size() == 2 )
        {
          ref = QStringLiteral( "ID[\"%1\",%2]" ).arg( authparts.at( 0 ), authparts.at( 1 ) );
        }
        if ( !ref.isEmpty() && crs.toWkt( QgsCoordinateReferenceSystem::WKT_PREFERRED ).contains( ref ) )
        {
          QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                                tr( "Cannot save '%1' — the definition is equivalent to %2.\n\n(Try removing \"%3\" from the WKT definition.)" ).arg( def.name, crs.authid(), ref ) );
        }
        else
        {
          QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                                tr( "Cannot save '%1' — the definition is equivalent to %2." ).arg( def.name, crs.authid() ) );
        }
      }
      return;
    }
  }

  //Modify the CRS changed:
  bool saveSuccess = true;
  for ( const Definition &def : qgis::as_const( mDefinitions ) )
  {
    if ( !def.wkt.isEmpty() )
      crs.createFromWkt( def.wkt );
    else
      crs.createFromProj( def.proj );

    //Test if we just added this CRS (if it has no existing ID)
    if ( def.id.isEmpty() )
    {
      saveSuccess &= saveCrs( crs, def.name, QString(), true, !def.wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatWkt : QgsCoordinateReferenceSystem::FormatProj );
    }
    else
    {
      if ( mExistingCRSnames[def.id] != def.name
           || ( !def.wkt.isEmpty() && mExistingCRSwkt[def.id] != def.wkt )
           || ( !def.proj.isEmpty() && mExistingCRSproj[def.id] != def.proj )
         )
      {
        saveSuccess &= saveCrs( crs, def.name, def.id, false, !def.wkt.isEmpty() ? QgsCoordinateReferenceSystem::FormatWkt : QgsCoordinateReferenceSystem::FormatProj );
      }
    }
    if ( ! saveSuccess )
    {
      QgsDebugMsg( QStringLiteral( "Error when saving CRS '%1'" ).arg( def.name ) );
    }
  }
  QgsDebugMsgLevel( QStringLiteral( "We remove the deleted CRS." ), 4 );
  for ( int i = 0; i < mDeletedCRSs.size(); ++i )
  {
    saveSuccess &= deleteCrs( mDeletedCRSs[i] );
    if ( ! saveSuccess )
    {
      QgsDebugMsg( QStringLiteral( "Error deleting CRS for '%1'" ).arg( mDefinitions[i].name ) );
    }
  }
  if ( saveSuccess )
  {
    accept();
  }
}

void QgsCustomProjectionDialog::updateListFromCurrentItem()
{
  QTreeWidgetItem *item = leNameList->currentItem();
  if ( !item )
    return;

  int currentIndex = leNameList->indexOfTopLevelItem( item );
  if ( currentIndex < 0 )
    return;

  mDefinitions[currentIndex].name = leName->text();
  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatWkt:
      mDefinitions[currentIndex].wkt = teParameters->toPlainText();
      mDefinitions[currentIndex].proj.clear();
      break;

    case QgsCoordinateReferenceSystem::FormatProj:
      mDefinitions[currentIndex].proj = teParameters->toPlainText();
      mDefinitions[currentIndex].wkt.clear();
      break;
  }

  item->setText( QgisCrsNameColumn, leName->text() );
  item->setText( QgisCrsParametersColumn, multiLineWktToSingleLine( teParameters->toPlainText() ) );
  item->setData( 0, FormattedWktRole, teParameters->toPlainText() );
}

#if PROJ_VERSION_MAJOR>=6
static void proj_collecting_logger( void *user_data, int /*level*/, const char *message )
{
  QStringList *dest = reinterpret_cast< QStringList * >( user_data );
  QString messageString( message );
  messageString.replace( QStringLiteral( "internal_proj_create: " ), QString() );
  dest->append( messageString );
}

#endif

void QgsCustomProjectionDialog::validateCurrent()
{
  const QString projDef = teParameters->toPlainText();

#if PROJ_VERSION_MAJOR>=6
  PJ_CONTEXT *context = proj_context_create();

  QStringList projErrors;
  proj_log_func( context, &projErrors, proj_collecting_logger );
  QgsProjUtils::proj_pj_unique_ptr crs;

  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatWkt:
    {
      PROJ_STRING_LIST warnings = nullptr;
      PROJ_STRING_LIST grammerErrors = nullptr;
      crs.reset( proj_create_from_wkt( context, projDef.toLatin1().constData(), nullptr, &warnings, &grammerErrors ) );
      QStringList warningStrings;
      QStringList grammerStrings;
      for ( auto iter = warnings; iter && *iter; ++iter )
        warningStrings << QString( *iter );
      for ( auto iter = grammerErrors; iter && *iter; ++iter )
        grammerStrings << QString( *iter );
      proj_string_list_destroy( warnings );
      proj_string_list_destroy( grammerErrors );

      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This WKT projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This WKT projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + warningStrings.join( '\n' ) + grammerStrings.join( '\n' ) );
      }
      break;
    }

    case QgsCoordinateReferenceSystem::FormatProj:
    {
      const QString projCrsString = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
      crs.reset( proj_create( context, projCrsString.toLatin1().constData() ) );
      if ( crs )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This proj projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This proj projection definition is not valid:" ) + QStringLiteral( "\n\n" ) + projErrors.join( '\n' ) );
      }
      break;
    }
  }

  // reset logger to terminal output
  proj_log_func( context, nullptr, nullptr );
  proj_context_destroy( context );
  context = nullptr;
#else
  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatWkt:
    {
      QByteArray ba = projDef.toLatin1();
      const char *pWkt = ba.data();
      OGRSpatialReferenceH crs = OSRNewSpatialReference( nullptr );

      OGRErr myInputResult = OSRImportFromWkt( crs, const_cast< char ** >( & pWkt ) );
      if ( myInputResult == OGRERR_NONE )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This WKT projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This WKT projection definition is not valid." ) );
      }

      OSRDestroySpatialReference( crs );
      break;
    }
    case QgsCoordinateReferenceSystem::FormatProj:
    {
      projCtx pContext = pj_ctx_alloc();
      projPJ proj = pj_init_plus_ctx( pContext, projDef.toLocal8Bit().data() );

      if ( proj )
      {
        QMessageBox::information( this, tr( "Custom Coordinate Reference System" ),
                                  tr( "This proj projection definition is valid." ) );
      }
      else
      {
        QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                              tr( "This proj projection definition is not valid" ) );
      }
      pj_free( proj );
      pj_ctx_free( pContext );
      break;
    }
  }

#endif
}

void QgsCustomProjectionDialog::formatChanged()
{
  QgsCoordinateReferenceSystem crs;
  QString newFormatString;
  switch ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) )
  {
    case QgsCoordinateReferenceSystem::FormatProj:
    {
      crs.createFromWkt( multiLineWktToSingleLine( teParameters->toPlainText() ) );
      if ( crs.isValid() )
        newFormatString = crs.toProj();
      break;
    }

    case QgsCoordinateReferenceSystem::FormatWkt:
    {
#if PROJ_VERSION_MAJOR>=6
      {
        PJ_CONTEXT *pjContext = QgsProjContext::get();
        QString proj = teParameters->toPlainText();
        proj.replace( QStringLiteral( "+type=crs" ), QString() );
        proj += QStringLiteral( " +type=crs" );
        QgsProjUtils::proj_pj_unique_ptr crs( proj_create( QgsProjContext::get(), proj.toLatin1().constData() ) );
        if ( crs )
        {
          const QByteArray multiLineOption = QStringLiteral( "MULTILINE=YES" ).toLocal8Bit();
          const char *const options[] = {multiLineOption.constData(), nullptr};
          newFormatString = QString( proj_as_wkt( pjContext, crs.get(), PJ_WKT2_2019, options ) );
        }
      }
#else
      crs.createFromProj( teParameters->toPlainText() );
      if ( crs.isValid() )
        newFormatString = crs.toWkt( QgsCoordinateReferenceSystem::WKT2_2018, false );
#endif
      break;
    }
  }
  if ( !newFormatString.isEmpty() )
    teParameters->setPlainText( newFormatString );
}

void QgsCustomProjectionDialog::pbnCalculate_clicked()
{
  // We must check the prj def is valid!
#if PROJ_VERSION_MAJOR>=6
  PJ_CONTEXT *pContext = QgsProjContext::get();
  QString projDef = teParameters->toPlainText();
  QgsDebugMsgLevel( QStringLiteral( "Proj: %1" ).arg( projDef ), 3 );
#else
  if ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) == QgsCoordinateReferenceSystem::FormatWkt )
  {
    // it's not trivial to implement, and we've gotta draw the line somewhere...
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "Testing WKT based CRS definitions requires Proj version 6 or later." ) );
    return;
  }

  projCtx pContext = pj_ctx_alloc();
  projPJ proj = pj_init_plus_ctx( pContext, teParameters->toPlainText().toLocal8Bit().data() );
  QgsDebugMsgLevel( QStringLiteral( "Proj: %1" ).arg( teParameters->toPlainText() ), 3 );

  if ( !proj )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "This proj projection definition is not valid." ) );
    projectedX->clear();
    projectedY->clear();
    pj_free( proj );
    pj_ctx_free( pContext );
    return;

  }
#endif
  // Get the WGS84 coordinates
  bool okN, okE;
  double latitude = northWGS84->text().toDouble( &okN );
  double longitude = eastWGS84->text().toDouble( &okE );

#if PROJ_VERSION_MAJOR<6
  latitude *= DEG_TO_RAD;
  longitude *= DEG_TO_RAD;
#endif

  if ( !okN || !okE )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "Northing and Easting must be in decimal form." ) );
    projectedX->clear();
    projectedY->clear();
#if PROJ_VERSION_MAJOR<6
    pj_free( proj );
    pj_ctx_free( pContext );
#endif
    return;
  }

#if PROJ_VERSION_MAJOR < 6
  projPJ wgs84Proj = pj_init_plus_ctx( pContext, geoProj4().data() ); //defined in qgis.h

  if ( !wgs84Proj )
  {
    QMessageBox::critical( this, tr( "Custom Coordinate Reference System" ),
                           tr( "Internal Error (source projection invalid?)" ) );
    projectedX->clear();
    projectedY->clear();
    pj_free( wgs84Proj );
    pj_ctx_free( pContext );
    return;
  }
#endif

#if PROJ_VERSION_MAJOR>=6
  if ( static_cast< QgsCoordinateReferenceSystem::Format >( mFormatComboBox->currentData().toInt() ) == QgsCoordinateReferenceSystem::FormatProj )
    projDef = projDef + ( projDef.contains( QStringLiteral( "+type=crs" ) ) ? QString() : QStringLiteral( " +type=crs" ) );
  QgsProjUtils::proj_pj_unique_ptr res( proj_create_crs_to_crs( pContext, "EPSG:4326", projDef.toUtf8(), nullptr ) );
  if ( !res )
  {
    QMessageBox::warning( this, tr( "Custom Coordinate Reference System" ),
                          tr( "This CRS projection definition is not valid." ) );
    projectedX->clear();
    projectedY->clear();
    return;
  }

  // careful -- proj 6 respects CRS axis, so we've got latitude/longitude flowing in, and ....?? coming out?
  proj_trans_generic( res.get(), PJ_FWD,
                      &latitude, sizeof( double ), 1,
                      &longitude, sizeof( double ), 1,
                      nullptr, sizeof( double ), 0,
                      nullptr, sizeof( double ), 0 );
  int projResult = proj_errno( res.get() );
#else
  double z = 0.0;
  int projResult = pj_transform( wgs84Proj, proj, 1, 0, &longitude, &latitude, &z );
#endif
  if ( projResult != 0 )
  {
    projectedX->setText( tr( "Error" ) );
    projectedY->setText( tr( "Error" ) );
#if PROJ_VERSION_MAJOR>=6
    QgsDebugMsg( proj_errno_string( projResult ) );
#else
    QgsDebugMsg( pj_strerrno( projResult ) );
#endif
  }
  else
  {
    QString tmp;

    int precision = 4;
    bool isLatLong = false;

#if PROJ_VERSION_MAJOR>= 6
    isLatLong = QgsProjUtils::usesAngularUnit( projDef );
#else
    isLatLong = pj_is_latlong( proj );
    if ( isLatLong )
    {
      latitude *= RAD_TO_DEG;
      longitude *= RAD_TO_DEG;
    }
#endif
    if ( isLatLong )
    {
      precision = 7;
    }

#if PROJ_VERSION_MAJOR>= 6
    tmp = QLocale().toString( longitude, 'f', precision );
    projectedX->setText( tmp );
    tmp = QLocale().toString( latitude, 'f', precision );
    projectedY->setText( tmp );
#else
    tmp = QLocale().toString( latitude, 'f', precision );
    projectedX->setText( tmp );
    tmp = QLocale().toString( longitude, 'f', precision );
    projectedY->setText( tmp );
#endif
  }

#if PROJ_VERSION_MAJOR<6
  pj_free( proj );
  pj_free( wgs84Proj );
  pj_ctx_free( pContext );
#endif
}

void QgsCustomProjectionDialog::showHelp()
{
  QgsHelp::openHelp( QStringLiteral( "working_with_projections/working_with_projections.html" ) );
}

QString QgsCustomProjectionDialog::multiLineWktToSingleLine( const QString &wkt )
{
  QString res = wkt;
  QRegularExpression re( QStringLiteral( "\\s*\\n\\s*" ) );
  re.setPatternOptions( QRegularExpression::MultilineOption );
  res.replace( re, QString() );
  return res;
}

