/***************************************************************************
 *   qgsprojectionselector.cpp                                             *
 *   Copyright (C) 2005 by Tim Sutton                                      *
 *   tim@linfiniti.com                                                     *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 ***************************************************************************/
#include <qgsprojectionselector.h>

//standard includes
#include <sqlite3.h>

//qgis includes
#include "qgis.h" //magic numbers here
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"

//qt includes
#include <QFileInfo>
#include <QHeaderView>
#include <QResizeEvent>
#include <QMessageBox>
#include <QSettings>

QgsProjectionSelector::QgsProjectionSelector( QWidget* parent, const char *name, Qt::WFlags fl )
    : QWidget( parent, fl )
    , mProjListDone( false )
    , mUserProjListDone( false )
    , mRecentProjListDone( false )
    , mSearchColumn( NONE )
{
  Q_UNUSED( name );
  setupUi( this );

  // Get the full path name to the sqlite3 spatial reference database.
  mSrsDatabaseFileName = QgsApplication::srsDbFilePath();

  lstCoordinateSystems->header()->setResizeMode( AUTHID_COLUMN, QHeaderView::Stretch );
  lstCoordinateSystems->header()->resizeSection( QGIS_CRS_ID_COLUMN, 0 );
  lstCoordinateSystems->header()->setResizeMode( QGIS_CRS_ID_COLUMN, QHeaderView::Fixed );

  // Hide (internal) ID column
  lstCoordinateSystems->setColumnHidden( QGIS_CRS_ID_COLUMN, true );

  lstRecent->header()->setResizeMode( AUTHID_COLUMN, QHeaderView::Stretch );
  lstRecent->header()->resizeSection( QGIS_CRS_ID_COLUMN, 0 );
  lstRecent->header()->setResizeMode( QGIS_CRS_ID_COLUMN, QHeaderView::Fixed );

  // Hide (internal) ID column
  lstRecent->setColumnHidden( QGIS_CRS_ID_COLUMN, true );

  // Read settings from persistent storage
  QSettings settings;
  mRecentProjections = settings.value( "/UI/recentProjections" ).toStringList();
  /*** The reading (above) of internal id from persistent storage should be removed sometime in the future */
  /*** This is kept now for backwards compatibility */

  QStringList projectionsProj4  = settings.value( "/UI/recentProjectionsProj4" ).toStringList();
  QStringList projectionsAuthId = settings.value( "/UI/recentProjectionsAuthId" ).toStringList();
  if ( projectionsAuthId.size() >= mRecentProjections.size() )
  {
    // We had saved state with AuthId and Proj4. Use that instead
    // to find out the crs id
    QgsDebugMsg( "Use popular projection list from AuthId/Proj4 saved state" );
    mRecentProjections.clear();
    for ( int i = 0; i <  projectionsAuthId.size(); i++ )
    {
      // Create a crs from the EPSG
      QgsCoordinateReferenceSystem crs;
      crs.createFromOgcWmsCrs( projectionsAuthId.at( i ) );
      if ( ! crs.isValid() )
      {
        // Couldn't create from EPSG, try the Proj4 string instead
        if ( ! crs.createFromProj4( projectionsProj4.at( i ) ) )
        {
          // No? Skip this entry
          continue;
        }
      }
      mRecentProjections << QString::number( crs.srsid() );
    }
  }
}

QgsProjectionSelector::~QgsProjectionSelector()
{
  // Push current projection to front, only if set
  long crsId = selectedCrsId();
  if ( crsId == 0 )
    return;

  // Save persistent list of projects
  mRecentProjections.removeAll( QString::number( crsId ) );
  mRecentProjections.prepend( QString::number( crsId ) );
  // Prune size of list
  while ( mRecentProjections.size() > 8 )
  {
    mRecentProjections.removeLast();
  }

  // Save to file *** Should be removed sometims in the future ***
  QSettings settings;
  settings.setValue( "/UI/recentProjections", mRecentProjections );

  // Convert to EPSG and proj4, and save those values also

  QStringList projectionsProj4;
  QStringList projectionsAuthId;
  for ( int i = 0; i <  mRecentProjections.size(); i++ )
  {
    // Create a crs from the crsId
    QgsCoordinateReferenceSystem crs( mRecentProjections.at( i ).toLong(), QgsCoordinateReferenceSystem::InternalCrsId );
    if ( ! crs.isValid() )
    {
      // No? Skip this entry
      continue;
    }
    projectionsProj4 << crs.toProj4();
    projectionsAuthId << crs.authid();
  }
  settings.setValue( "/UI/recentProjectionsProj4", projectionsProj4 );
  settings.setValue( "/UI/recentProjectionsAuthId", projectionsAuthId );
}

void QgsProjectionSelector::resizeEvent( QResizeEvent * theEvent )
{
  lstCoordinateSystems->header()->resizeSection( NAME_COLUMN, theEvent->size().width() - 240 );
  lstCoordinateSystems->header()->resizeSection( AUTHID_COLUMN, 240 );
  lstCoordinateSystems->header()->resizeSection( QGIS_CRS_ID_COLUMN, 0 );

  lstRecent->header()->resizeSection( NAME_COLUMN, theEvent->size().width() - 240 );
  lstRecent->header()->resizeSection( AUTHID_COLUMN, 240 );
  lstRecent->header()->resizeSection( QGIS_CRS_ID_COLUMN, 0 );
}

void QgsProjectionSelector::showEvent( QShowEvent * theEvent )
{
  // ensure the projection list view is actually populated
  // before we show this widget
  loadCrsList( &mCrsFilter );
  loadUserCrsList( &mCrsFilter );

  if ( !mRecentProjListDone )
  {
    for ( int i = mRecentProjections.size() - 1; i >= 0; i-- )
      insertRecent( mRecentProjections.at( i ).toLong() );
    mRecentProjListDone = true;
  }

  // apply deferred selection
  applySelection();

  // Pass up the inheritance hierarchy
  QWidget::showEvent( theEvent );
}

QString QgsProjectionSelector::ogcWmsCrsFilterAsSqlExpression( QSet<QString> * crsFilter )
{
  QString sqlExpression = "1";             // it's "SQL" for "true"
  QMap<QString, QStringList> authParts;

  if ( !crsFilter )
    return sqlExpression;

  /*
     Ref: WMS 1.3.0, section 6.7.3 "Layer CRS":

     Every Layer CRS has an identifier that is a character string. Two types of
     Layer CRS identifiers are permitted: "label" and "URL" identifiers:

     Label: The identifier includes a namespace prefix, a colon, a numeric or
        string code, and in some instances a comma followed by additional
        parameters. This International Standard defines three namespaces:
        CRS, EpsgCrsId and AUTO2 [...]

     URL: The identifier is a fully-qualified Uniform Resource Locator that
        references a publicly-accessible file containing a definition of the CRS
        that is compliant with ISO 19111.
  */

  // iterate through all incoming CRSs

  foreach( QString auth_id, crsFilter->values() )
  {
    QStringList parts = auth_id.split( ":" );

    if ( parts.size() < 2 )
      continue;

    authParts[ parts.at( 0 ).toUpper()].append( parts.at( 1 ).toUpper() );
  }

  if ( authParts.isEmpty() )
    return sqlExpression;

  if ( authParts.size() > 0 )
  {
    QString prefix = " AND (";
    foreach( QString auth_name, authParts.keys() )
    {
      sqlExpression += QString( "%1(upper(auth_name)='%2' AND upper(auth_id) IN ('%3'))" )
                       .arg( prefix )
                       .arg( auth_name )
                       .arg( authParts[auth_name].join( "','" ) );
      prefix = " OR ";
    }
    sqlExpression += ")";
  }

  QgsDebugMsg( "exiting with '" + sqlExpression + "'." );

  return sqlExpression;
}

void QgsProjectionSelector::setSelectedCrsName( QString theCRSName )
{
  applySelection( NAME_COLUMN, theCRSName );
}

void QgsProjectionSelector::setSelectedCrsId( long theCRSID )
{
  applySelection( QGIS_CRS_ID_COLUMN, QString::number( theCRSID ) );
}

void QgsProjectionSelector::setSelectedEpsg( long id )
{
  setSelectedAuthId( QString( "EPSG:%1" ).arg( id ) );
}

void QgsProjectionSelector::setSelectedAuthId( QString id )
{
  applySelection( AUTHID_COLUMN, id );
}

void QgsProjectionSelector::applySelection( int column, QString value )
{
  if ( !mProjListDone || !mUserProjListDone )
  {
    // defer selection until loaded
    mSearchColumn = column;
    mSearchValue  = value;
    return;
  }

  if ( column == NONE )
  {
    // invoked deferred selection
    column = mSearchColumn;
    value  = mSearchValue;
  }

  if ( column == NONE )
    return;

  QList<QTreeWidgetItem*> nodes = lstCoordinateSystems->findItems( value, Qt::MatchExactly | Qt::MatchRecursive, column );
  if ( nodes.count() > 0 )
  {
    QgsDebugMsg( QString( "found %1,%2" ).arg( column ).arg( value ) );
    lstCoordinateSystems->setCurrentItem( nodes.first() );
  }
  else
  {
    QgsDebugMsg( "nothing found" );
    // unselect the selected item to avoid confusing the user
    lstCoordinateSystems->clearSelection();
    lstRecent->clearSelection();
    teProjection->setText( "" );
  }
}

void QgsProjectionSelector::insertRecent( long theCrsId )
{
  if ( !mProjListDone || !mUserProjListDone )
    return;

  QList<QTreeWidgetItem*> nodes = lstCoordinateSystems->findItems( QString::number( theCrsId ), Qt::MatchExactly | Qt::MatchRecursive, QGIS_CRS_ID_COLUMN );
  if ( nodes.count() == 0 )
    return;

  lstRecent->insertTopLevelItem( 0, new QTreeWidgetItem( lstRecent, QStringList()
                                 << nodes.first()->text( NAME_COLUMN )
                                 << nodes.first()->text( AUTHID_COLUMN )
                                 << nodes.first()->text( QGIS_CRS_ID_COLUMN ) ) );
}

//note this line just returns the projection name!
QString QgsProjectionSelector::selectedName()
{
  // return the selected wkt name from the list view
  QTreeWidgetItem *lvi = lstCoordinateSystems->currentItem();
  return lvi ? lvi->text( NAME_COLUMN ) : QString::null;
}

// Returns the whole proj4 string for the selected projection node
QString QgsProjectionSelector::selectedProj4String()
{
  // Only return the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  //
  // Get the selected node
  QTreeWidgetItem *item = lstCoordinateSystems->currentItem();
  if ( !item || item->text( QGIS_CRS_ID_COLUMN ).isEmpty() )
    return "";

  QString srsId = item->text( QGIS_CRS_ID_COLUMN );

  QgsDebugMsg( "srsId = " + srsId );
  QgsDebugMsg( "USER_CRS_START_ID = " + QString::number( USER_CRS_START_ID ) );

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  QString databaseFileName;
  if ( srsId.toLong() >= USER_CRS_START_ID )
  {
    databaseFileName = QgsApplication::qgisUserDbFilePath();
    if ( !QFileInfo( databaseFileName ).exists() ) //its unlikely that this condition will ever be reached
      return QString( "" );
  }
  else //must be a system projection then
  {
    databaseFileName = mSrsDatabaseFileName;
  }

  QgsDebugMsg( "db = " + databaseFileName );

  sqlite3 *database;
  int rc = sqlite3_open( databaseFileName.toUtf8().data(), &database );
  if ( rc )
  {
    showDBMissingWarning( databaseFileName );
    return "";
  }

  // prepare the sql statement
  const char *tail;
  sqlite3_stmt *stmt;
  QString sql = QString( "select parameters from tbl_srs where srs_id=%1" ).arg( srsId );

  QgsDebugMsg( "Selection sql: " + sql );

  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  QString projString;
  if ( rc == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    projString = QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) );
  }

  // close the statement
  sqlite3_finalize( stmt );
  // close the database
  sqlite3_close( database );

  Q_ASSERT( !projString.isEmpty() );

  return projString;
}

QString QgsProjectionSelector::getSelectedExpression( QString expression )
{
  // Only return the attribute if there is a node in the tree
  // selected that has an srs_id.  This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  //
  // Get the selected node and make sure it is a srs andx
  // not a top-level projection node
  QTreeWidgetItem *lvi = lstCoordinateSystems->currentItem();
  if ( !lvi || lvi->text( QGIS_CRS_ID_COLUMN ).isEmpty() )
    return 0;

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  QString databaseFileName;
  if ( lvi->text( QGIS_CRS_ID_COLUMN ).toLong() >= USER_CRS_START_ID )
  {
    databaseFileName = QgsApplication::qgisUserDbFilePath();
    if ( !QFileInfo( databaseFileName ).exists() )
    {
      return 0;
    }
  }
  else
  {
    databaseFileName = mSrsDatabaseFileName;
  }

  //
  // set up the database
  // XXX We could probabaly hold the database open for the life of this object,
  // assuming that it will never be used anywhere else. Given the low overhead,
  // opening it each time seems to be a reasonable approach at this time.
  sqlite3 *database;
  int rc = sqlite3_open( databaseFileName.toUtf8().data(), &database );
  if ( rc )
  {
    showDBMissingWarning( databaseFileName );
    return 0;
  }

  // prepare the sql statement
  const char *tail;
  sqlite3_stmt *stmt;
  QString sql = QString( "select %1 from tbl_srs where srs_id=%2" )
                .arg( expression )
                .arg( lvi->text( QGIS_CRS_ID_COLUMN ) );

  QgsDebugMsg( QString( "Finding selected attribute using : %1" ).arg( sql ) );
  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  QString attributeValue;
  if ( rc == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    // get the first row of the result set
    attributeValue = QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) );
  }

  // close the statement
  sqlite3_finalize( stmt );
  // close the database
  sqlite3_close( database );

  // return the srs
  return attributeValue;
}

long QgsProjectionSelector::selectedEpsg()
{
  if ( getSelectedExpression( "auth_name" ).compare( "EPSG", Qt::CaseInsensitive ) == 0 )
  {
    return getSelectedExpression( "auth_id" ).toLong();
  }
  else
  {
    QgsDebugMsg( "selected projection is NOT EPSG" );
    return 0;
  }
}

long QgsProjectionSelector::selectedPostgresSrId()
{
  return getSelectedExpression( "srid" ).toLong();
}


QString QgsProjectionSelector::selectedAuthId()
{
  int srid = getSelectedExpression( "srs_id" ).toLong();
  if ( srid >= USER_CRS_START_ID )
    return QString( "USER:%1" ).arg( srid );
  else
    return getSelectedExpression( "upper(auth_name||':'||auth_id)" );
}


long QgsProjectionSelector::selectedCrsId()
{
  QTreeWidgetItem* item = lstCoordinateSystems->currentItem();

  if ( item && !item->text( QGIS_CRS_ID_COLUMN ).isEmpty() )
    return lstCoordinateSystems->currentItem()->text( QGIS_CRS_ID_COLUMN ).toLong();
  else
    return 0;
}


void QgsProjectionSelector::setOgcWmsCrsFilter( QSet<QString> crsFilter )
{
  mCrsFilter = crsFilter;
  mProjListDone = false;
  mUserProjListDone = false;
  lstCoordinateSystems->clear();
}

void QgsProjectionSelector::loadUserCrsList( QSet<QString> *crsFilter )
{
  if ( mUserProjListDone )
    return;

  QgsDebugMsg( "Fetching user projection list..." );

  // convert our Coordinate Reference System filter into the SQL expression
  QString sqlFilter = ogcWmsCrsFilterAsSqlExpression( crsFilter );

  // User defined coordinate system node
  // Make in an italic font to distinguish them from real projections
  mUserProjList = new QTreeWidgetItem( lstCoordinateSystems, QStringList( tr( "User Defined Coordinate Systems" ) ) );

  QFont fontTemp = mUserProjList->font( 0 );
  fontTemp.setItalic( true );
  fontTemp.setBold( true );
  mUserProjList->setFont( 0, fontTemp );
  mUserProjList->setIcon( 0, QIcon( QgsApplication::activeThemePath() + "user.png" ) );

  //determine where the user proj database lives for this user. If none is found an empty
  //now only will be shown
  QString databaseFileName = QgsApplication::qgisUserDbFilePath();
  // first we look for ~/.qgis/qgis.db
  // if it doesnt exist we copy it in from the global resources dir

  //return straight away if the user has not created any custom projections
  if ( !QFileInfo( databaseFileName ).exists( ) )
  {
    QgsDebugMsg( "Users qgis.db not found...skipping" );
    mUserProjListDone = true;
    return;
  }

  sqlite3      *database;
  const char   *tail;
  sqlite3_stmt *stmt;
  //check the db is available
  int result = sqlite3_open( databaseFileName.toUtf8().constData(), &database );
  if ( result )
  {
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist. But we checked earlier for its existance
    //     and aborted in that case. This is because we may be runnig from read only
    //     media such as live cd and don't want to force trying to create a db.
    showDBMissingWarning( databaseFileName );
    return;
  }

  // Set up the query to retrieve the projection information needed to populate the list
  QString sql = QString( "select description, srs_id from vw_srs where %1" ).arg( sqlFilter );

  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK )
  {
    QTreeWidgetItem *newItem;
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      newItem = new QTreeWidgetItem( mUserProjList, QStringList( QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) ) ) );
      // EpsgCrsId for user projections is not always defined in some dbases.
      // It's also not written from customprojections dialog.
      // display the epsg (field 2) in the second column of the list view
      // newItem->setText( EPSG_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 2 ) ) );
      // display the qgis srs_id (field 1) in the third column of the list view
      newItem->setText( QGIS_CRS_ID_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 1 ) ) );
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  mUserProjListDone = true;
}

void QgsProjectionSelector::loadCrsList( QSet<QString> *crsFilter )
{
  if ( mProjListDone )
    return;

  // convert our Coordinate Reference System filter into the SQL expression
  QString sqlFilter = ogcWmsCrsFilterAsSqlExpression( crsFilter );

  // Create the top-level nodes for the list view of projections
  // Make in an italic font to distinguish them from real projections
  //
  // Geographic coordinate system node
  mGeoList = new QTreeWidgetItem( lstCoordinateSystems, QStringList( tr( "Geographic Coordinate Systems" ) ) );

  QFont fontTemp = mGeoList->font( 0 );
  fontTemp.setItalic( true );
  fontTemp.setBold( true );
  mGeoList->setFont( 0, fontTemp );
  mGeoList->setIcon( 0, QIcon( QgsApplication::activeThemePath() + "geographic.png" ) );

  // Projected coordinate system node
  mProjList = new QTreeWidgetItem( lstCoordinateSystems, QStringList( tr( "Projected Coordinate Systems" ) ) );

  fontTemp = mProjList->font( 0 );
  fontTemp.setItalic( true );
  fontTemp.setBold( true );
  mProjList->setFont( 0, fontTemp );
  mProjList->setIcon( 0, QIcon( QgsApplication::activeThemePath() + "transformed.png" ) );

  //bail out in case the projections db does not exist
  //this is necessary in case the pc is running linux with a
  //read only filesystem because otherwise sqlite will try
  //to create the db file on the fly

  if ( !QFileInfo( mSrsDatabaseFileName ).exists() )
  {
    mProjListDone = true;
    return;
  }

  // open the database containing the spatial reference data
  sqlite3 *database;
  int rc = sqlite3_open( mSrsDatabaseFileName.toUtf8().data(), &database );
  if ( rc )
  {
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    showDBMissingWarning( mSrsDatabaseFileName );
    return;
  }
  // prepare the sql statement
  const char *tail;
  sqlite3_stmt *stmt;
  // get total count of records in the projection table
  QString sql = "select count(*) from tbl_srs";

  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  Q_ASSERT( rc == SQLITE_OK );
  sqlite3_step( stmt );
  sqlite3_finalize( stmt );

  // Set up the query to retrieve the projection information needed to populate the list
  //note I am giving the full field names for clarity here and in case someone
  //changes the underlying view TS
  sql = QString( "select description, srs_id, upper(auth_name||':'||auth_id), is_geo, name, parameters, deprecated from vw_srs where %1 order by name,description" )
        .arg( sqlFilter );

  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    QTreeWidgetItem *newItem;
    // Cache some stuff to speed up creating of the list of projected
    // spatial reference systems
    QString previousSrsType( "" );
    QTreeWidgetItem* previousSrsTypeNode = 0;

    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      // check to see if the srs is geographic
      int isGeo = sqlite3_column_int( stmt, 3 );
      if ( isGeo )
      {
        // this is a geographic coordinate system
        // Add it to the tree (field 0)
        newItem = new QTreeWidgetItem( mGeoList, QStringList( QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) ) ) );

        // display the authority name (field 2) in the second column of the list view
        newItem->setText( AUTHID_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 2 ) ) );

        // display the qgis srs_id (field 1) in the third column of the list view
        newItem->setText( QGIS_CRS_ID_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 1 ) ) );
      }
      else
      {
        // This is a projected srs
        QTreeWidgetItem *node;
        QString srsType = QString::fromUtf8(( char* )sqlite3_column_text( stmt, 4 ) );
        // Find the node for this type and add the projection to it
        // If the node doesn't exist, create it
        if ( srsType == previousSrsType )
        {
          node = previousSrsTypeNode;
        }
        else
        { // Different from last one, need to search
          QList<QTreeWidgetItem*> nodes = lstCoordinateSystems->findItems( srsType, Qt::MatchExactly | Qt::MatchRecursive, NAME_COLUMN );
          if ( nodes.count() == 0 )
          {
            // the node doesn't exist -- create it
            // Make in an italic font to distinguish them from real projections
            node = new QTreeWidgetItem( mProjList, QStringList( srsType ) );
            QFont fontTemp = node->font( 0 );
            fontTemp.setItalic( true );
            node->setFont( 0, fontTemp );
          }
          else
          {
            node = nodes.first();
          }
          // Update the cache.
          previousSrsType = srsType;
          previousSrsTypeNode = node;
        }
        // add the item, setting the projection name in the first column of the list view
        newItem = new QTreeWidgetItem( node, QStringList( QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) ) ) );
        // display the authority id (field 2) in the second column of the list view
        newItem->setText( AUTHID_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 2 ) ) );
        // display the qgis srs_id (field 1) in the third column of the list view
        newItem->setText( QGIS_CRS_ID_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 1 ) ) );
        // expand also parent node
        newItem->parent()->setExpanded( true );
      }

      // display the qgis deprecated in the user data of the item
      newItem->setData( 0, Qt::UserRole, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 6 ) ) );
      newItem->setHidden( cbxHideDeprecated->isChecked() );
    }
    mProjList->setExpanded( true );
  }

  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  // close the database
  sqlite3_close( database );

  mProjListDone = true;
}

// New coordinate system selected from the list
void QgsProjectionSelector::on_lstCoordinateSystems_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  QgsDebugMsg( "Entered." );
  lstCoordinateSystems->scrollToItem( current );

  // If the item has children, it's not an end node in the tree, and
  // hence is just a grouping thingy, not an actual CRS.
  if ( current && current->childCount() == 0 )
  {
    // Found a real CRS
    emit sridSelected( QString::number( selectedCrsId() ) );

    teProjection->setText( selectedProj4String() );

    QList<QTreeWidgetItem*> nodes = lstRecent->findItems( current->text( QGIS_CRS_ID_COLUMN ), Qt::MatchExactly, QGIS_CRS_ID_COLUMN );
    if ( nodes.count() > 0 )
    {
      QgsDebugMsg( QString( "found srs %1 in recent" ).arg( current->text( QGIS_CRS_ID_COLUMN ) ) );
      lstRecent->setCurrentItem( nodes.first() );
    }
    else
    {
      QgsDebugMsg( QString( "srs %1 not recent" ).arg( current->text( QGIS_CRS_ID_COLUMN ) ) );
      lstRecent->clearSelection();
    }
  }
  else
  {
    // Not an CRS - remove the highlight so the user doesn't get too confused
    if ( current )
      current->setSelected( false );
    teProjection->setText( "" );
    lstRecent->clearSelection();
  }
}

void QgsProjectionSelector::on_lstRecent_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  QgsDebugMsg( "Entered." );
  lstRecent->scrollToItem( current );

  QList<QTreeWidgetItem*> nodes = lstCoordinateSystems->findItems( current->text( QGIS_CRS_ID_COLUMN ), Qt::MatchExactly | Qt::MatchRecursive, QGIS_CRS_ID_COLUMN );
  if ( nodes.count() > 0 )
    lstCoordinateSystems->setCurrentItem( nodes.first() );
}

void QgsProjectionSelector::hideDeprecated( QTreeWidgetItem *item )
{
  if ( item->data( 0, Qt::UserRole ).toBool() )
  {
    item->setHidden( cbxHideDeprecated->isChecked() );
    if ( item->isSelected() && item->isHidden() )
    {
      item->setSelected( false );
      teProjection->setText( "" );
    }
  }

  for ( int i = 0; i < item->childCount(); i++ )
    hideDeprecated( item->child( i ) );
}

void QgsProjectionSelector::on_cbxHideDeprecated_stateChanged()
{
  for ( int i = 0; i < lstCoordinateSystems->topLevelItemCount(); i++ )
    hideDeprecated( lstCoordinateSystems->topLevelItem( i ) );
}

void QgsProjectionSelector::on_leSearch_textChanged( const QString & theFilterTxt )
{
  // filter recent crs's
  QTreeWidgetItemIterator itr( lstRecent );
  while ( *itr )
  {
    if (( *itr )->childCount() == 0 ) // it's an end node aka a projection
    {
      if (( *itr )->text( NAME_COLUMN ).contains( theFilterTxt, Qt::CaseInsensitive )
          || ( *itr )->text( AUTHID_COLUMN ).contains( theFilterTxt, Qt::CaseInsensitive )
         )
      {
        ( *itr )->setHidden( false );
        QTreeWidgetItem * parent = ( *itr )->parent();
        while ( parent )
        {
          parent->setExpanded( true );
          parent->setHidden( false );
          parent = parent->parent();
        }
      }
      else
      {
        ( *itr )->setHidden( true );
      }
    }
    else
    {
      ( *itr )->setHidden( true );
    }
    ++itr;
  }

  // filter crs's
  QTreeWidgetItemIterator it( lstCoordinateSystems );
  while ( *it )
  {
    if (( *it )->childCount() == 0 ) // it's an end node aka a projection
    {
      if (( *it )->text( NAME_COLUMN ).contains( theFilterTxt, Qt::CaseInsensitive )
          || ( *it )->text( AUTHID_COLUMN ).contains( theFilterTxt, Qt::CaseInsensitive )
         )
      {
        ( *it )->setHidden( false );
        QTreeWidgetItem * parent = ( *it )->parent();
        while ( parent )
        {
          parent->setExpanded( true );
          parent->setHidden( false );
          parent = parent->parent();
        }
      }
      else
      {
        ( *it )->setHidden( true );
      }
    }
    else
    {
      ( *it )->setHidden( true );
    }
    ++it;
  }
}


long QgsProjectionSelector::getLargestCRSIDMatch( QString theSql )
{
  long srsId = 0;

  //
  // Now perform the actual search
  //

  sqlite3      *database;
  const char   *tail;
  sqlite3_stmt *stmt;
  int           result;

  // first we search the users db as any srsid there will be definition be greater than in sys db

  //check the db is available
  QString databaseFileName = QgsApplication::qgisUserDbFilePath();
  if ( QFileInfo( databaseFileName ).exists() ) //only bother trying to open if the file exists
  {
    result = sqlite3_open( databaseFileName.toUtf8().data(), &database );
    if ( result )
    {
      // XXX This will likely never happen since on open, sqlite creates the
      //     database if it does not exist. But we checked earlier for its existance
      //     and aborted in that case. This is because we may be runnig from read only
      //     media such as live cd and don't want to force trying to create a db.
      showDBMissingWarning( databaseFileName );
      return 0;
    }

    result = sqlite3_prepare( database, theSql.toUtf8(), theSql.toUtf8().length(), &stmt, &tail );
    // XXX Need to free memory from the error msg if one is set
    if ( result == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
    {
      QString srsIdString = QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) );
      srsId = srsIdString.toLong();
      // close the sqlite3 statement
      sqlite3_finalize( stmt );
      sqlite3_close( database );
      return srsId;
    }
  }
  else
  {
    //only bother looking in srs.db if it wasnt found above
    result = sqlite3_open( mSrsDatabaseFileName.toUtf8().data(), &database );
    if ( result )
    {
      QgsDebugMsg( QString( "Can't open * user * database: %1" ).arg( sqlite3_errmsg( database ) ) );
      //no need for assert because user db may not have been created yet
      return 0;
    }
  }

  result = sqlite3_prepare( database, theSql.toUtf8(), theSql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    QString srsIdString = QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) );
    srsId = srsIdString.toLong();
  }

  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  return srsId;
}

QStringList QgsProjectionSelector::authorities()
{
  sqlite3      *database;
  const char   *tail;
  sqlite3_stmt *stmt;

  int result = sqlite3_open( mSrsDatabaseFileName.toUtf8().data(), &database );
  if ( result )
  {
    QgsDebugMsg( QString( "Can't open * user * database: %1" ).arg( sqlite3_errmsg( database ) ) );
    //no need for assert because user db may not have been created yet
    return QStringList();
  }

  QString theSql = "select distinct auth_name from tbl_srs";
  result = sqlite3_prepare( database, theSql.toUtf8(), theSql.toUtf8().length(), &stmt, &tail );

  QStringList authorities;
  if ( result == SQLITE_OK )
  {
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      authorities << QString::fromUtf8(( char * )sqlite3_column_text( stmt, 0 ) );
    }

  }

  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  return authorities;
}

/*!linfinity qtcreator qgis
* \brief Make the string safe for use in SQL statements.
*  This involves escaping single quotes, double quotes, backslashes,
*  and optionally, percentage symbols.  Percentage symbols are used
*  as wildcards sometimes and so when using the string as part of the
*  LIKE phrase of a select statement, should be escaped.
* \arg const QString in The input string to make safe.
* \return The string made safe for SQL statements.
*/
const QString QgsProjectionSelector::sqlSafeString( const QString theSQL )
{
  QString retval = theSQL;
  retval.replace( "\\", "\\\\" );
  retval.replace( '\"', "\\\"" );
  retval.replace( "\'", "\\'" );
  retval.replace( "%", "\\%" );
  return retval;
}

void QgsProjectionSelector::showDBMissingWarning( const QString theFileName )
{

  QMessageBox::critical( this, tr( "Resource Location Error" ),
                         tr( "Error reading database file from: \n %1\n"
                             "Because of this the projection selector will not work..." )
                         .arg( theFileName ) );
}
