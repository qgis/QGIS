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
#include "qgsprojectionselectiontreewidget.h"

//standard includes
#include <sqlite3.h>

//qgis includes
#include "qgis.h" //magic numbers here
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgscoordinatereferencesystem.h"
#include "qgsmessagelog.h"
#include "qgssettings.h"
#include "qgsrectangle.h"
#include "qgsrubberband.h"
#include "qgsvectorlayer.h"
#include "qgsmaptoolpan.h"
#include "qgsvertexmarker.h"

//qt includes
#include <QFileInfo>
#include <QHeaderView>
#include <QResizeEvent>
#include <QMessageBox>

QgsProjectionSelectionTreeWidget::QgsProjectionSelectionTreeWidget( QWidget *parent )
  : QWidget( parent )
{
  setupUi( this );

  connect( lstCoordinateSystems, &QTreeWidget::itemDoubleClicked, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystems_itemDoubleClicked );
  connect( lstRecent, &QTreeWidget::itemDoubleClicked, this, &QgsProjectionSelectionTreeWidget::lstRecent_itemDoubleClicked );
  connect( lstCoordinateSystems, &QTreeWidget::currentItemChanged, this, &QgsProjectionSelectionTreeWidget::lstCoordinateSystems_currentItemChanged );
  connect( lstRecent, &QTreeWidget::currentItemChanged, this, &QgsProjectionSelectionTreeWidget::lstRecent_currentItemChanged );
  connect( cbxHideDeprecated, &QCheckBox::stateChanged, this, &QgsProjectionSelectionTreeWidget::cbxHideDeprecated_stateChanged );
  connect( leSearch, &QgsFilterLineEdit::textChanged, this, &QgsProjectionSelectionTreeWidget::leSearch_textChanged );

  mPreviewBand = new QgsRubberBand( mAreaCanvas, QgsWkbTypes::PolygonGeometry );
  mPreviewBand->setWidth( 4 );

  mPreviewBand2 = new QgsRubberBand( mAreaCanvas, QgsWkbTypes::PolygonGeometry );
  mPreviewBand2->setWidth( 4 );
  QColor rectColor = QColor( 185, 84, 210, 60 );
  mPreviewBand2->setColor( rectColor );

  mVertexMarker = new QgsVertexMarker( mAreaCanvas );
  mVertexMarker->setIconType( QgsVertexMarker::ICON_CROSS );
  mVertexMarker->setColor( QColor( 185, 84, 210 ) );
  mVertexMarker->setPenWidth( 3 );

  QgsCoordinateReferenceSystem srs( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  mAreaCanvas->setDestinationCrs( srs );

  QString layerPath = QgsApplication::pkgDataPath() + QStringLiteral( "/resources/data/world_map.shp" );
  mLayers << new QgsVectorLayer( layerPath );
  mAreaCanvas->setLayers( mLayers );
  mAreaCanvas->setMapTool( new QgsMapToolPan( mAreaCanvas ) );

  mAreaCanvas->setVisible( mShowMap );

  if ( QDialog *dlg = qobject_cast<QDialog *>( parent ) )
  {
    // mark selected projection for push to front if parent dialog is accepted
    connect( dlg, &QDialog::accepted, this, &QgsProjectionSelectionTreeWidget::pushProjectionToFront );
  }

  // Get the full path name to the sqlite3 spatial reference database.
  mSrsDatabaseFileName = QgsApplication::srsDatabaseFilePath();

  lstCoordinateSystems->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );
  lstCoordinateSystems->header()->resizeSection( QgisCrsIdColumn, 0 );
  lstCoordinateSystems->header()->setSectionResizeMode( QgisCrsIdColumn, QHeaderView::Fixed );

  // Hide (internal) ID column
  lstCoordinateSystems->setColumnHidden( QgisCrsIdColumn, true );

  lstRecent->header()->setSectionResizeMode( AuthidColumn, QHeaderView::Stretch );
  lstRecent->header()->resizeSection( QgisCrsIdColumn, 0 );
  lstRecent->header()->setSectionResizeMode( QgisCrsIdColumn, QHeaderView::Fixed );

  // Hide (internal) ID column
  lstRecent->setColumnHidden( QgisCrsIdColumn, true );

  mRecentProjections = QgsCoordinateReferenceSystem::recentProjections();

  mCheckBoxNoProjection->setHidden( true );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, this, &QgsProjectionSelectionTreeWidget::crsSelected );
  connect( mCheckBoxNoProjection, &QCheckBox::toggled, mFrameProjections, &QFrame::setDisabled );
}

QgsProjectionSelectionTreeWidget::~QgsProjectionSelectionTreeWidget()
{
  qDeleteAll( mLayers );
  delete mPreviewBand;
  delete mPreviewBand2;
  delete mVertexMarker;

  if ( !mPushProjectionToFront )
  {
    return;
  }

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
  QgsSettings settings;
  settings.setValue( QStringLiteral( "/UI/recentProjections" ), mRecentProjections );

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
  settings.setValue( QStringLiteral( "/UI/recentProjectionsProj4" ), projectionsProj4 );
  settings.setValue( QStringLiteral( "/UI/recentProjectionsAuthId" ), projectionsAuthId );
}

void QgsProjectionSelectionTreeWidget::resizeEvent( QResizeEvent *event )
{
  lstCoordinateSystems->header()->resizeSection( NameColumn, event->size().width() - 240 );
  lstCoordinateSystems->header()->resizeSection( AuthidColumn, 240 );
  lstCoordinateSystems->header()->resizeSection( QgisCrsIdColumn, 0 );

  lstRecent->header()->resizeSection( NameColumn, event->size().width() - 240 );
  lstRecent->header()->resizeSection( AuthidColumn, 240 );
  lstRecent->header()->resizeSection( QgisCrsIdColumn, 0 );
}

void QgsProjectionSelectionTreeWidget::showEvent( QShowEvent *event )
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

  emit initialized();

  // Pass up the inheritance hierarchy
  QWidget::showEvent( event );
}

QString QgsProjectionSelectionTreeWidget::ogcWmsCrsFilterAsSqlExpression( QSet<QString> *crsFilter )
{
  QString sqlExpression = QStringLiteral( "1" );           // it's "SQL" for "true"
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

  Q_FOREACH ( const QString &auth_id, *crsFilter )
  {
    QStringList parts = auth_id.split( ':' );

    if ( parts.size() < 2 )
      continue;

    authParts[ parts.at( 0 ).toUpper()].append( parts.at( 1 ).toUpper() );
  }

  if ( authParts.isEmpty() )
    return sqlExpression;

  if ( !authParts.isEmpty() )
  {
    QString prefix = QStringLiteral( " AND (" );
    for ( auto it = authParts.constBegin(); it != authParts.constEnd(); ++it )
    {
      sqlExpression += QStringLiteral( "%1(upper(auth_name)='%2' AND upper(auth_id) IN ('%3'))" )
                       .arg( prefix,
                             it.key(),
                             it.value().join( QStringLiteral( "','" ) ) );
      prefix = QStringLiteral( " OR " );
    }
    sqlExpression += ')';
  }

  QgsDebugMsgLevel( "exiting with '" + sqlExpression + "'.", 4 );

  return sqlExpression;
}

void QgsProjectionSelectionTreeWidget::applySelection( int column, QString value )
{
  if ( !mProjListDone || !mUserProjListDone )
  {
    // defer selection until loaded
    mSearchColumn = column;
    mSearchValue  = value;
    return;
  }

  if ( column == QgsProjectionSelectionTreeWidget::None )
  {
    // invoked deferred selection
    column = mSearchColumn;
    value  = mSearchValue;

    mSearchColumn = QgsProjectionSelectionTreeWidget::None;
    mSearchValue.clear();
  }

  if ( column == QgsProjectionSelectionTreeWidget::None )
    return;

  QList<QTreeWidgetItem *> nodes = lstCoordinateSystems->findItems( value, Qt::MatchExactly | Qt::MatchRecursive, column );
  if ( !nodes.isEmpty() )
  {
    QgsDebugMsgLevel( QString( "found %1,%2" ).arg( column ).arg( value ), 4 );
    lstCoordinateSystems->setCurrentItem( nodes.first() );
  }
  else
  {
    QgsDebugMsgLevel( QString( "nothing found for %1,%2" ).arg( column ).arg( value ), 4 );
    // deselect the selected item to avoid confusing the user
    lstCoordinateSystems->clearSelection();
    lstRecent->clearSelection();
    teProjection->clear();
    teSelected->clear();
  }
}

void QgsProjectionSelectionTreeWidget::insertRecent( long crsId )
{
  if ( !mProjListDone || !mUserProjListDone )
    return;

  QList<QTreeWidgetItem *> nodes = lstCoordinateSystems->findItems( QString::number( crsId ), Qt::MatchExactly | Qt::MatchRecursive, QgisCrsIdColumn );
  if ( nodes.isEmpty() )
    return;

  lstRecent->insertTopLevelItem( 0, new QTreeWidgetItem( lstRecent, QStringList()
                                 << nodes.first()->text( NameColumn )
                                 << nodes.first()->text( AuthidColumn )
                                 << nodes.first()->text( QgisCrsIdColumn ) ) );
}

//note this line just returns the projection name!
QString QgsProjectionSelectionTreeWidget::selectedName()
{
  // return the selected wkt name from the list view
  QTreeWidgetItem *lvi = lstCoordinateSystems->currentItem();
  return lvi ? lvi->text( NameColumn ) : QString();
}

void QgsProjectionSelectionTreeWidget::setCrs( const QgsCoordinateReferenceSystem &crs )
{
  if ( !crs.isValid() )
  {
    mCheckBoxNoProjection->setChecked( true );
  }
  else
  {
    mCheckBoxNoProjection->setChecked( false );
    applySelection( AuthidColumn, crs.authid() );
  }
}

void QgsProjectionSelectionTreeWidget::setPreviewRect( const QgsRectangle &rect )
{
  mPreviewRect = rect;
  mPreviewBand2->setToGeometry( QgsGeometry::fromRect( mPreviewRect ), nullptr );
  mPreviewBand2->show();
  mVertexMarker->setCenter( rect.center() );
  mVertexMarker->show();
}

QgsRectangle QgsProjectionSelectionTreeWidget::previewRect() const
{
  return mPreviewRect;
}

// Returns the whole proj4 string for the selected projection node
QString QgsProjectionSelectionTreeWidget::selectedProj4String()
{
  // Only return the projection if there is a node in the tree
  // selected that has an srid. This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  //
  // Get the selected node
  QTreeWidgetItem *item = lstCoordinateSystems->currentItem();
  if ( !item || item->text( QgisCrsIdColumn ).isEmpty() )
    return QLatin1String( "" );

  QString srsId = item->text( QgisCrsIdColumn );

  QgsDebugMsgLevel( "srsId = " + srsId, 4 );
  QgsDebugMsgLevel( "USER_CRS_START_ID = " + QString::number( USER_CRS_START_ID ), 4 );

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  QString databaseFileName;
  if ( srsId.toLong() >= USER_CRS_START_ID )
  {
    databaseFileName = QgsApplication::qgisUserDatabaseFilePath();
    if ( !QFileInfo::exists( databaseFileName ) ) //its unlikely that this condition will ever be reached
      return QString();
  }
  else //must be a system projection then
  {
    databaseFileName = mSrsDatabaseFileName;
  }

  QgsDebugMsgLevel( "db = " + databaseFileName, 4 );

  sqlite3 *database = nullptr;
  int rc = sqlite3_open_v2( databaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( rc )
  {
    showDBMissingWarning( databaseFileName );
    return QLatin1String( "" );
  }

  // prepare the sql statement
  const char *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;
  QString sql = QStringLiteral( "select parameters from tbl_srs where srs_id=%1" ).arg( srsId );

  QgsDebugMsgLevel( "Selection sql: " + sql, 4 );

  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  QString projString;
  if ( rc == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    projString = QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) );
  }

  // close the statement
  sqlite3_finalize( stmt );
  // close the database
  sqlite3_close( database );

  Q_ASSERT( !projString.isEmpty() );

  return projString;
}

QString QgsProjectionSelectionTreeWidget::getSelectedExpression( const QString &expression ) const
{
  // Only return the attribute if there is a node in the tree
  // selected that has an srs_id.  This prevents error if the user
  // selects a top-level node rather than an actual coordinate
  // system
  //
  // Get the selected node and make sure it is a srs andx
  // not a top-level projection node
  QTreeWidgetItem *lvi = lstCoordinateSystems->currentItem();
  if ( !lvi || lvi->text( QgisCrsIdColumn ).isEmpty() )
    return QString();

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  QString databaseFileName;
  if ( lvi->text( QgisCrsIdColumn ).toLong() >= USER_CRS_START_ID )
  {
    databaseFileName = QgsApplication::qgisUserDatabaseFilePath();
    if ( !QFileInfo::exists( databaseFileName ) )
    {
      return QString();
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
  sqlite3 *database = nullptr;
  int rc = sqlite3_open_v2( databaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( rc )
  {
    QgsMessageLog::logMessage( tr( "Resource Location Error" ), tr( "Error reading database file from: \n %1\n"
                               "Because of this the projection selector will not work..." ).arg( databaseFileName ),
                               QgsMessageLog::CRITICAL );
    return QString();
  }

  // prepare the sql statement
  const char *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;
  QString sql = QStringLiteral( "select %1 from tbl_srs where srs_id=%2" )
                .arg( expression,
                      lvi->text( QgisCrsIdColumn ) );

  QgsDebugMsgLevel( QString( "Finding selected attribute using : %1" ).arg( sql ), 4 );
  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  QString attributeValue;
  if ( rc == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    // get the first row of the result set
    attributeValue = QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) );
  }

  // close the statement
  sqlite3_finalize( stmt );
  // close the database
  sqlite3_close( database );

  // return the srs
  return attributeValue;
}

QgsCoordinateReferenceSystem QgsProjectionSelectionTreeWidget::crs() const
{
  if ( mCheckBoxNoProjection->isChecked() )
    return QgsCoordinateReferenceSystem();

  int srid = getSelectedExpression( QStringLiteral( "srs_id" ) ).toLong();
  if ( srid >= USER_CRS_START_ID )
    return QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "USER:%1" ).arg( srid ) );
  else
    return QgsCoordinateReferenceSystem::fromOgcWmsCrs( getSelectedExpression( QStringLiteral( "upper(auth_name||':'||auth_id)" ) ) );
}

void QgsProjectionSelectionTreeWidget::setShowNoProjection( bool show )
{
  mCheckBoxNoProjection->setHidden( !show );
}

void QgsProjectionSelectionTreeWidget::setShowBoundsMap( bool show )
{
  mShowMap = show;
  mAreaCanvas->setVisible( show );

}

bool QgsProjectionSelectionTreeWidget::showNoProjection() const
{
  return !mCheckBoxNoProjection->isHidden();
}

bool QgsProjectionSelectionTreeWidget::showBoundsMap() const
{
  return mShowMap;
}

bool QgsProjectionSelectionTreeWidget::hasValidSelection() const
{
  QTreeWidgetItem *item = lstCoordinateSystems->currentItem();
  if ( mCheckBoxNoProjection->isChecked() )
    return true;
  else
    return item && !item->text( QgisCrsIdColumn ).isEmpty();
}

long QgsProjectionSelectionTreeWidget::selectedCrsId()
{
  QTreeWidgetItem *item = lstCoordinateSystems->currentItem();

  if ( item && !item->text( QgisCrsIdColumn ).isEmpty() )
    return lstCoordinateSystems->currentItem()->text( QgisCrsIdColumn ).toLong();
  else
    return 0;
}


void QgsProjectionSelectionTreeWidget::setOgcWmsCrsFilter( const QSet<QString> &crsFilter )
{
  mCrsFilter = crsFilter;
  mProjListDone = false;
  mUserProjListDone = false;
  lstCoordinateSystems->clear();
}

void QgsProjectionSelectionTreeWidget::loadUserCrsList( QSet<QString> *crsFilter )
{
  if ( mUserProjListDone )
    return;

  QgsDebugMsgLevel( "Fetching user projection list...", 4 );

  // convert our Coordinate Reference System filter into the SQL expression
  QString sqlFilter = ogcWmsCrsFilterAsSqlExpression( crsFilter );

  // User defined coordinate system node
  // Make in an italic font to distinguish them from real projections
  mUserProjList = new QTreeWidgetItem( lstCoordinateSystems, QStringList( tr( "User Defined Coordinate Systems" ) ) );

  QFont fontTemp = mUserProjList->font( 0 );
  fontTemp.setItalic( true );
  fontTemp.setBold( true );
  mUserProjList->setFont( 0, fontTemp );
  mUserProjList->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/user.svg" ) ) );

  //determine where the user proj database lives for this user. If none is found an empty
  //now only will be shown
  QString databaseFileName = QgsApplication::qgisUserDatabaseFilePath();
  // first we look for ~/.qgis/qgis.db
  // if it doesn't exist we copy it in from the global resources dir

  //return straight away if the user has not created any custom projections
  if ( !QFileInfo::exists( databaseFileName ) )
  {
    QgsDebugMsg( "Users qgis.db not found...skipping" );
    mUserProjListDone = true;
    return;
  }

  sqlite3      *database = nullptr;
  const char   *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;
  //check the db is available
  int result = sqlite3_open_v2( databaseFileName.toUtf8().constData(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist. But we checked earlier for its existence
    //     and aborted in that case. This is because we may be running from read only
    //     media such as live cd and don't want to force trying to create a db.
    showDBMissingWarning( databaseFileName );
    return;
  }

  // Set up the query to retrieve the projection information needed to populate the list
  QString sql = QStringLiteral( "select description, srs_id from vw_srs where %1" ).arg( sqlFilter );

  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK )
  {
    QTreeWidgetItem *newItem = nullptr;
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      newItem = new QTreeWidgetItem( mUserProjList, QStringList( QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) ) ) );
      // EpsgCrsId for user projections is not always defined in some dbases.
      // It's also not written from customprojections dialog.
      // display the epsg (field 2) in the second column of the list view
      // newItem->setText( EPSG_COLUMN, QString::fromUtf8(( char * )sqlite3_column_text( stmt, 2 ) ) );
      // display the qgis srs_id (field 1) in the third column of the list view
      newItem->setText( QgisCrsIdColumn, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 1 ) ) );
      newItem->setText( AuthidColumn, QStringLiteral( "USER:%1" ).arg( QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 1 ) ).toInt() ) );
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  mUserProjListDone = true;
}

void QgsProjectionSelectionTreeWidget::loadCrsList( QSet<QString> *crsFilter )
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
  mGeoList->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/mIconProjectionEnabled.svg" ) ) );

  // Projected coordinate system node
  mProjList = new QTreeWidgetItem( lstCoordinateSystems, QStringList( tr( "Projected Coordinate Systems" ) ) );

  fontTemp = mProjList->font( 0 );
  fontTemp.setItalic( true );
  fontTemp.setBold( true );
  mProjList->setFont( 0, fontTemp );
  mProjList->setIcon( 0, QgsApplication::getThemeIcon( QStringLiteral( "/transformed.svg" ) ) );

  //bail out in case the projections db does not exist
  //this is necessary in case the pc is running linux with a
  //read only filesystem because otherwise sqlite will try
  //to create the db file on the fly

  if ( !QFileInfo::exists( mSrsDatabaseFileName ) )
  {
    mProjListDone = true;
    return;
  }

  // open the database containing the spatial reference data
  sqlite3 *database = nullptr;
  int rc = sqlite3_open_v2( mSrsDatabaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( rc )
  {
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    showDBMissingWarning( mSrsDatabaseFileName );
    return;
  }

  const char *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;
  // Set up the query to retrieve the projection information needed to populate the list
  //note I am giving the full field names for clarity here and in case someone
  //changes the underlying view TS
  QString sql = QStringLiteral( "select description, srs_id, upper(auth_name||':'||auth_id), is_geo, name, parameters, deprecated from vw_srs where %1 order by name,description" )
                .arg( sqlFilter );

  rc = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( rc == SQLITE_OK )
  {
    QTreeWidgetItem *newItem = nullptr;
    // Cache some stuff to speed up creating of the list of projected
    // spatial reference systems
    QString previousSrsType( QLatin1String( "" ) );
    QTreeWidgetItem *previousSrsTypeNode = nullptr;

    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      // check to see if the srs is geographic
      int isGeo = sqlite3_column_int( stmt, 3 );
      if ( isGeo )
      {
        // this is a geographic coordinate system
        // Add it to the tree (field 0)
        newItem = new QTreeWidgetItem( mGeoList, QStringList( QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) ) ) );

        // display the authority name (field 2) in the second column of the list view
        newItem->setText( AuthidColumn, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 2 ) ) );

        // display the qgis srs_id (field 1) in the third column of the list view
        newItem->setText( QgisCrsIdColumn, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 1 ) ) );
      }
      else
      {
        // This is a projected srs
        QTreeWidgetItem *node = nullptr;
        QString srsType = QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 4 ) );
        // Find the node for this type and add the projection to it
        // If the node doesn't exist, create it
        if ( srsType == previousSrsType )
        {
          node = previousSrsTypeNode;
        }
        else
        {
          // Different from last one, need to search
          QList<QTreeWidgetItem *> nodes = lstCoordinateSystems->findItems( srsType, Qt::MatchExactly | Qt::MatchRecursive, NameColumn );
          if ( nodes.isEmpty() )
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
        newItem = new QTreeWidgetItem( node, QStringList( QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) ) ) );
        // display the authority id (field 2) in the second column of the list view
        newItem->setText( AuthidColumn, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 2 ) ) );
        // display the qgis srs_id (field 1) in the third column of the list view
        newItem->setText( QgisCrsIdColumn, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 1 ) ) );
        // expand also parent node
        newItem->parent()->setExpanded( true );
      }

      // display the qgis deprecated in the user data of the item
      newItem->setData( 0, Qt::UserRole, QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 6 ) ) );
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
void QgsProjectionSelectionTreeWidget::lstCoordinateSystems_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  QgsDebugMsgLevel( "Entered.", 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( "no current item", 4 );
    return;
  }

  lstCoordinateSystems->scrollToItem( current );

  // If the item has children, it's not an end node in the tree, and
  // hence is just a grouping thingy, not an actual CRS.
  if ( current->childCount() == 0 )
  {
    // Found a real CRS
    emit crsSelected();

    teSelected->setText( selectedName() );
    updateBoundsPreview();

    QList<QTreeWidgetItem *> nodes = lstRecent->findItems( current->text( QgisCrsIdColumn ), Qt::MatchExactly, QgisCrsIdColumn );
    if ( !nodes.isEmpty() )
    {
      QgsDebugMsgLevel( QString( "found srs %1 in recent" ).arg( current->text( QgisCrsIdColumn ) ), 4 );
      lstRecent->setCurrentItem( nodes.first() );
    }
    else
    {
      QgsDebugMsgLevel( QString( "srs %1 not recent" ).arg( current->text( QgisCrsIdColumn ) ), 4 );
      lstRecent->clearSelection();
      lstCoordinateSystems->setFocus( Qt::OtherFocusReason );
    }
  }
  else
  {
    // Not an CRS - remove the highlight so the user doesn't get too confused
    current->setSelected( false );
    teProjection->clear();
    teSelected->clear();
    lstRecent->clearSelection();
  }
}

void QgsProjectionSelectionTreeWidget::lstCoordinateSystems_itemDoubleClicked( QTreeWidgetItem *current, int column )
{
  Q_UNUSED( column );

  QgsDebugMsgLevel( "Entered.", 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( "no current item", 4 );
    return;
  }

  // If the item has children, it's not an end node in the tree, and
  // hence is just a grouping thingy, not an actual CRS.
  if ( current->childCount() == 0 )
    emit projectionDoubleClicked();
}

void QgsProjectionSelectionTreeWidget::lstRecent_currentItemChanged( QTreeWidgetItem *current, QTreeWidgetItem * )
{
  QgsDebugMsgLevel( "Entered.", 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( "no current item", 4 );
    return;
  }

  lstRecent->scrollToItem( current );

  QList<QTreeWidgetItem *> nodes = lstCoordinateSystems->findItems( current->text( QgisCrsIdColumn ), Qt::MatchExactly | Qt::MatchRecursive, QgisCrsIdColumn );
  if ( !nodes.isEmpty() )
    lstCoordinateSystems->setCurrentItem( nodes.first() );
}

void QgsProjectionSelectionTreeWidget::lstRecent_itemDoubleClicked( QTreeWidgetItem *current, int column )
{
  Q_UNUSED( column );

  QgsDebugMsgLevel( "Entered.", 4 );

  if ( !current )
  {
    QgsDebugMsgLevel( "no current item", 4 );
    return;
  }

  QList<QTreeWidgetItem *> nodes = lstCoordinateSystems->findItems( current->text( QgisCrsIdColumn ), Qt::MatchExactly | Qt::MatchRecursive, QgisCrsIdColumn );
  if ( !nodes.isEmpty() )
    emit projectionDoubleClicked();
}

void QgsProjectionSelectionTreeWidget::hideDeprecated( QTreeWidgetItem *item )
{
  if ( item->data( 0, Qt::UserRole ).toBool() )
  {
    item->setHidden( cbxHideDeprecated->isChecked() );
    if ( item->isSelected() && item->isHidden() )
    {
      item->setSelected( false );
      teProjection->clear();
      teSelected->clear();
    }
  }

  for ( int i = 0; i < item->childCount(); i++ )
    hideDeprecated( item->child( i ) );
}

void QgsProjectionSelectionTreeWidget::cbxHideDeprecated_stateChanged()
{
  for ( int i = 0; i < lstCoordinateSystems->topLevelItemCount(); i++ )
    hideDeprecated( lstCoordinateSystems->topLevelItem( i ) );
}

void QgsProjectionSelectionTreeWidget::leSearch_textChanged( const QString &filterTxt )
{
  QString filterTxtCopy = filterTxt;
  filterTxtCopy.replace( QRegExp( "\\s+" ), QStringLiteral( ".*" ) );
  QRegExp re( filterTxtCopy, Qt::CaseInsensitive );

  // filter recent crs's
  QTreeWidgetItemIterator itr( lstRecent );
  while ( *itr )
  {
    if ( ( *itr )->childCount() == 0 ) // it's an end node aka a projection
    {
      if ( ( *itr )->text( NameColumn ).contains( re )
           || ( *itr )->text( AuthidColumn ).contains( re )
         )
      {
        ( *itr )->setHidden( false );
        QTreeWidgetItem *parent = ( *itr )->parent();
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
    if ( ( *it )->childCount() == 0 ) // it's an end node aka a projection
    {
      if ( ( *it )->text( NameColumn ).contains( re )
           || ( *it )->text( AuthidColumn ).contains( re )
         )
      {
        ( *it )->setHidden( false );
        QTreeWidgetItem *parent = ( *it )->parent();
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


void QgsProjectionSelectionTreeWidget::pushProjectionToFront()
{
  // set flag to push selected projection to front in destructor
  mPushProjectionToFront = true;
}


long QgsProjectionSelectionTreeWidget::getLargestCrsIdMatch( const QString &sql )
{
  long srsId = 0;

  //
  // Now perform the actual search
  //

  sqlite3      *database = nullptr;
  const char   *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;
  int           result;

  // first we search the users db as any srsid there will be definition be greater than in sys db

  //check the db is available
  QString databaseFileName = QgsApplication::qgisUserDatabaseFilePath();
  if ( QFileInfo::exists( databaseFileName ) ) //only bother trying to open if the file exists
  {
    result = sqlite3_open_v2( databaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
    if ( result )
    {
      // XXX This will likely never happen since on open, sqlite creates the
      //     database if it does not exist. But we checked earlier for its existence
      //     and aborted in that case. This is because we may be running from read only
      //     media such as live cd and don't want to force trying to create a db.
      showDBMissingWarning( databaseFileName );
      return 0;
    }

    result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
    // XXX Need to free memory from the error msg if one is set
    if ( result == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
    {
      QString srsIdString = QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) );
      srsId = srsIdString.toLong();
      // close the sqlite3 statement
      sqlite3_finalize( stmt );
      sqlite3_close( database );
      return srsId;
    }
  }
  else
  {
    //only bother looking in srs.db if it wasn't found above
    result = sqlite3_open_v2( mSrsDatabaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
    if ( result )
    {
      QgsDebugMsg( QString( "Can't open * user * database: %1" ).arg( sqlite3_errmsg( database ) ) );
      //no need for assert because user db may not have been created yet
      return 0;
    }
  }

  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );
  // XXX Need to free memory from the error msg if one is set
  if ( result == SQLITE_OK && sqlite3_step( stmt ) == SQLITE_ROW )
  {
    QString srsIdString = QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) );
    srsId = srsIdString.toLong();
  }

  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  return srsId;
}

void QgsProjectionSelectionTreeWidget::updateBoundsPreview()
{
  QTreeWidgetItem *lvi = lstCoordinateSystems->currentItem();
  if ( !lvi || lvi->text( QgisCrsIdColumn ).isEmpty() )
    return;

  QgsCoordinateReferenceSystem currentCrs = crs();
  if ( !currentCrs.isValid() )
    return;

  QgsRectangle rect = currentCrs.bounds();
  if ( !rect.isEmpty() )
  {
    mPreviewBand->setToGeometry( QgsGeometry::fromRect( rect ), nullptr );
    mPreviewBand->setColor( QColor( 255, 0, 0, 65 ) );
    mAreaCanvas->setExtent( rect );
    mPreviewBand->show();
    mAreaCanvas->zoomOut();
    QString extentString = tr( "Extent: %1" ).arg( rect.toString( 2 ) );
    QString proj4String = tr( "Proj4: %1" ).arg( selectedProj4String() );
    teProjection->setText( extentString + "\n" + proj4String );
  }
  else
  {
    mPreviewBand->hide();
    mAreaCanvas->zoomToFullExtent();
    QString extentString = tr( "Extent: Extent not known" );
    QString proj4String = tr( "Proj4: %1" ).arg( selectedProj4String() );
    teProjection->setText( extentString + "\n" + proj4String );
  }
}

QStringList QgsProjectionSelectionTreeWidget::authorities()
{
  sqlite3      *database = nullptr;
  const char   *tail = nullptr;
  sqlite3_stmt *stmt = nullptr;

  int result = sqlite3_open_v2( mSrsDatabaseFileName.toUtf8().data(), &database, SQLITE_OPEN_READONLY, nullptr );
  if ( result )
  {
    QgsDebugMsg( QString( "Can't open * user * database: %1" ).arg( sqlite3_errmsg( database ) ) );
    //no need for assert because user db may not have been created yet
    return QStringList();
  }

  QString sql = QStringLiteral( "select distinct auth_name from tbl_srs" );
  result = sqlite3_prepare( database, sql.toUtf8(), sql.toUtf8().length(), &stmt, &tail );

  QStringList authorities;
  if ( result == SQLITE_OK )
  {
    while ( sqlite3_step( stmt ) == SQLITE_ROW )
    {
      authorities << QString::fromUtf8( ( char * )sqlite3_column_text( stmt, 0 ) );
    }

  }

  // close the sqlite3 statement
  sqlite3_finalize( stmt );
  sqlite3_close( database );

  return authorities;
}

QString QgsProjectionSelectionTreeWidget::sqlSafeString( const QString &theSQL ) const
{
  QString retval = theSQL;
  retval.replace( '\\', QLatin1String( "\\\\" ) );
  retval.replace( '\"', QLatin1String( "\\\"" ) );
  retval.replace( '\'', QLatin1String( "\\'" ) );
  retval.replace( '%', QLatin1String( "\\%" ) );
  return retval;
}

void QgsProjectionSelectionTreeWidget::showDBMissingWarning( const QString &fileName )
{

  QMessageBox::critical( this, tr( "Resource Location Error" ),
                         tr( "Error reading database file from: \n %1\n"
                             "Because of this the projection selector will not work..." )
                         .arg( fileName ) );
}
