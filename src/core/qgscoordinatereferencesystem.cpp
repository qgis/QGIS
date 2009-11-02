/***************************************************************************
                          qgscoordinatereferencesystem.cpp

                             -------------------
    begin                : 2007
    copyright            : (C) 2007 by Gary E. Sherman
    email                : sherman@mrcc.com
***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgscoordinatereferencesystem.h"

#include <cmath>

#include <QDir>
#include <QDomNode>
#include <QDomElement>
#include <QFileInfo>
#include <QRegExp>
#include <QTextStream>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgis.h" //const vals declared here

#include <cassert>
#include <sqlite3.h>

//gdal and ogr includes (needed for == operator)
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>
#include "qgslogger.h"

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::mCustomSrsValidation = NULL;

//--------------------------

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem()
    : mMapUnits( QGis::UnknownUnit ),
    mIsValidFlag( 0 ),
    mValidationHint( 0 )
{
  mCRS = OSRNewSpatialReference( NULL );
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( QString theWkt )
    : mMapUnits( QGis::UnknownUnit ),
    mIsValidFlag( 0 ),
    mValidationHint( 0 )
{
  mCRS = OSRNewSpatialReference( NULL );
  createFromWkt( theWkt );
}


QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const long theId, CrsType theType )
    : mMapUnits( QGis::UnknownUnit ),
    mIsValidFlag( 0 ),
    mValidationHint( 0 )
{
  mCRS = OSRNewSpatialReference( NULL );
  createFromId( theId, theType );
}

QgsCoordinateReferenceSystem::~QgsCoordinateReferenceSystem()
{
  OSRDestroySpatialReference( mCRS );
}

void QgsCoordinateReferenceSystem::createFromId( const long theId, CrsType theType )
{
  switch ( theType )
  {
    case InternalCrsId:
      createFromSrsId( theId );
      break;
    case PostgisCrsId:
      createFromSrid( theId );
      break;
    case EpsgCrsId:
      createFromEpsg( theId );
      break;
    default:
      //THIS IS BAD...THIS PART OF CODE SHOULD NEVER BE REACHED...
      QgsLogger::critical( "Unexpected case reached in " + QString( __FILE__ ) + " : " + QString( __LINE__ ) );
  };
}

bool QgsCoordinateReferenceSystem::createFromOgcWmsCrs( QString theCrs )
{
  QStringList parts = theCrs.split( ":" );

  if ( parts.at( 0 ).compare( "EPSG", Qt::CaseInsensitive ) == 0 )
  {
    createFromEpsg( parts.at( 1 ).toLong() );
  }
  else if ( parts.at( 0 ).compare( "CRS", Qt::CaseInsensitive ) == 0 )
  {
    if ( parts.at( 1 ) == "84" )
    {
      //! \todo - CRS:84 is hardcoded to EPSG:4326 - see if this is appropriate
      /**
       *  See WMS 1.3 standard appendix B3 for details
       */
      createFromEpsg( 4326 );
    }
  }
  else
  {
    return FALSE;
  }

  return TRUE;
}

QgsCoordinateReferenceSystem::QgsCoordinateReferenceSystem( const QgsCoordinateReferenceSystem &srs )
{
  mCRS = OSRNewSpatialReference( NULL );
  *this = srs;
}

// Assignment operator
QgsCoordinateReferenceSystem& QgsCoordinateReferenceSystem::operator=( const QgsCoordinateReferenceSystem & srs )
{
  if ( &srs != this )
  {
    mSrsId = srs.mSrsId;
    mDescription = srs.mDescription;
    mProjectionAcronym = srs.mProjectionAcronym;
    mEllipsoidAcronym = srs.mEllipsoidAcronym;
    mGeoFlag = srs.mGeoFlag;
    mMapUnits = srs.mMapUnits;
    mSRID = srs.mSRID;
    mEpsg = srs.mEpsg;
    mIsValidFlag = srs.mIsValidFlag;
    mValidationHint = srs.mValidationHint;
    if ( mIsValidFlag )
    {
      OSRDestroySpatialReference( mCRS );
      mCRS = OSRClone( srs.mCRS );
    }
  }
  return *this;
}

// Misc helper functions -----------------------


void QgsCoordinateReferenceSystem::validate()
{
  if ( mIsValidFlag )
    return;

  // try to validate using custom validation routines
  if ( mCustomSrsValidation )
    mCustomSrsValidation( this );

  if ( !mIsValidFlag )
    // set the default
    createFromProj4( GEOPROJ4 );
}

bool QgsCoordinateReferenceSystem::createFromSrid( long id )
{
  return loadFromDb( QgsApplication::srsDbFilePath(), "srid", id );
}

bool QgsCoordinateReferenceSystem::createFromEpsg( long id )
{
  return loadFromDb( QgsApplication::srsDbFilePath(), "epsg", id );
}

bool QgsCoordinateReferenceSystem::createFromSrsId( long id )
{
  return loadFromDb( id < 100000 ? QgsApplication::srsDbFilePath() :
                     QgsApplication::qgisUserDbFilePath(), "srs_id", id );
}

bool QgsCoordinateReferenceSystem::loadFromDb( QString db, QString field, long id )
{
  QgsDebugMsg( "load CRS from " + db + " where " + field + " is " + QString::number( id ) );
  mIsValidFlag = false;

  QFileInfo myInfo( db );
  if ( !myInfo.exists() )
  {
    QgsDebugMsg( "failed : " + db + " does not exist!" );
    return mIsValidFlag;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = openDb( db, &myDatabase );
  if ( myResult )
  {
    QgsDebugMsg( "failed : " + db + " could not be opened!" );
    return mIsValidFlag;
  }

  /*
    srs_id INTEGER PRIMARY KEY,
    description text NOT NULL,
    projection_acronym text NOT NULL,
    ellipsoid_acronym NOT NULL,
    parameters text NOT NULL,
    srid integer NOT NULL,
    epsg integer NOT NULL,
    is_geo integer NOT NULL);
  */

  QString mySql = "select srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,epsg,is_geo from tbl_srs where " + field + "='" + QString::number( id ) + "'";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK && sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
  {
    mSrsId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) ).toLong();
    mDescription = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) );
    mProjectionAcronym = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 2 ) );
    mEllipsoidAcronym = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 3 ) );
    QString toProj4 = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 4 ) );
    mSRID = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 5 ) ).toLong();
    mEpsg = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 6 ) ).toLong();
    int geo = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 7 ) ).toInt();
    mGeoFlag = ( geo == 0 ? false : true );
    setProj4String( toProj4 );
    setMapUnits();
  }
  else
  {
    QgsDebugMsg( "failed : " + mySql );
  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return mIsValidFlag;
}

bool QgsCoordinateReferenceSystem::createFromWkt( QString theWkt )
{
  mIsValidFlag = false;

  if ( theWkt.isEmpty() )
  {
    QgsDebugMsg( "theWkt is uninitialised, operation failed" );
    QgsLogger::critical( "QgsCoordinateReferenceSystem::createFromWkt -- theWkt is uninitialised, operation failed" );
    return mIsValidFlag;
  }
  QgsDebugMsg( "QgsCoordinateReferenceSystem::createFromWkt(QString theWkt) using: " + theWkt );
  QByteArray ba = theWkt.toLatin1();
  const char *pWkt = ba;

  OGRErr myInputResult = OSRImportFromWkt( mCRS, ( char ** ) & pWkt );

  if ( myInputResult != OGRERR_NONE )
  {
    QgsDebugMsg( "\n---------------------------------------------------------------" );
    QgsDebugMsg( "This CRS could *** NOT *** be set from the supplied Wkt " );
    QgsDebugMsg( "INPUT: " + theWkt );
    QgsDebugMsg( "---------------------------------------------------------------\n" );
    return mIsValidFlag;
  }

  // always morph from esri as it doesn't hurt anything
  // FW: Hey, that's not right!  It can screw stuff up! Disable
  //myOgrSpatialRef.morphFromESRI();

  // create the proj4 structs needed for transforming
  char *proj4src = NULL;
  OSRExportToProj4( mCRS, &proj4src );

  //now that we have the proj4string, delegate to createFromProj4String so
  // that we can try to fill in the remaining class members...
  //create from Proj will set the isValidFlag
  createFromProj4( QString( proj4src ) );
  CPLFree( proj4src );

  return mIsValidFlag;
  //setMapunits will be called by createfromproj above
}

bool QgsCoordinateReferenceSystem::isValid() const
{
  return mIsValidFlag;
}

bool QgsCoordinateReferenceSystem::createFromProj4( const QString theProj4String )
{
  //
  // Examples:
  // +proj=tmerc +lat_0=0 +lon_0=-62 +k=0.999500 +x_0=400000 +y_0=0
  // +ellps=clrk80 +towgs84=-255,-15,71,0,0,0,0 +units=m +no_defs
  //
  // +proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=2.337229166666664 +k_0=0.99987742
  // +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356515.000000472 +units=m +no_defs
  //
  mIsValidFlag = false;

  QRegExp myProjRegExp( "\\+proj=\\S+" );
  int myStart = 0;
  int myLength = 0;
  myStart = myProjRegExp.indexIn( theProj4String, myStart );
  if ( myStart == -1 )
  {
    QgsDebugMsg( "error proj string supplied has no +proj argument" );
    return mIsValidFlag;
  }
  else
  {
    myLength = myProjRegExp.matchedLength();
  }

  mProjectionAcronym = theProj4String.mid( myStart + PROJ_PREFIX_LEN, myLength - PROJ_PREFIX_LEN );

  QRegExp myEllipseRegExp( "\\+ellps=\\S+" );
  myStart = 0;
  myLength = 0;
  myStart = myEllipseRegExp.indexIn( theProj4String, myStart );
  if ( myStart != -1 )
  {
    myLength = myEllipseRegExp.matchedLength();
    mEllipsoidAcronym = theProj4String.mid( myStart + ELLPS_PREFIX_LEN, myLength - ELLPS_PREFIX_LEN );
  }

  QRegExp myAxisRegExp( "\\+a=\\S+" );
  myStart = 0;
  myLength = 0;
  myStart = myAxisRegExp.indexIn( theProj4String, myStart );
  if ( myStart == -1 && mEllipsoidAcronym.isNull() )
  {
    QgsLogger::warning( "QgsCoordinateReferenceSystem::createFromProj4 error"
                        " proj string supplied has no +ellps or +a argument" );
    return mIsValidFlag;
  }

  /*
  * We try to match the proj string to and srsid using the following logic:
  *
  * - perform a whole text search on srs name (if not null). The srs name will
  *   have been set if this method has been delegated to from createFromWkt.
  * Normally we wouldnt expect this to work, but its worth trying first
  * as its quicker than methods below..
  */
  long mySrsId = 0;
  QgsCoordinateReferenceSystem::RecordMap myRecord;

  // *** Matching on descriptions feels iffy. Different projs can have same description. Homann ***
  // if ( !mDescription.trimmed().isEmpty() )
  //{
  //  myRecord = getRecord( "select * from tbl_srs where description='" + mDescription.trimmed() + "'" );
  //}

  /*
  * - if the above does not match perform a whole text search on proj4 string (if not null)
  */
  // QgsDebugMsg( "wholetext match on name failed, trying proj4string match" );
  myRecord = getRecord( "select * from tbl_srs where parameters='" + theProj4String.trimmed() + "'" );
  if ( !myRecord.empty() )
  {
    mySrsId = myRecord["srs_id"].toLong();
    QgsDebugMsg( "proj4string match search for srsid returned srsid: " + QString::number( mySrsId ) );
    if ( mySrsId > 0 )
    {
      createFromSrsId( mySrsId );
    }
  }
  else
  {
    // Ticket #722 - aaronr
    // Check if we can swap the lat_1 and lat_2 params (if they exist) to see if we match...
    // First we check for lat_1 and lat_2
    QRegExp myLat1RegExp( "\\+lat_1=\\S+" );
    QRegExp myLat2RegExp( "\\+lat_2=\\S+" );
    int myStart1 = 0;
    int myLength1 = 0;
    int myStart2 = 0;
    int myLength2 = 0;
    QString lat1Str = "";
    QString lat2Str = "";
    myStart1 = myLat1RegExp.indexIn( theProj4String, myStart1 );
    myStart2 = myLat2RegExp.indexIn( theProj4String, myStart2 );
    if (( myStart1 != -1 ) && ( myStart2 != -1 ) )
    {
      myLength1 = myLat1RegExp.matchedLength();
      myLength2 = myLat2RegExp.matchedLength();
      lat1Str = theProj4String.mid( myStart1 + LAT_PREFIX_LEN, myLength1 - LAT_PREFIX_LEN );
      lat2Str = theProj4String.mid( myStart2 + LAT_PREFIX_LEN, myLength2 - LAT_PREFIX_LEN );
    }
    // If we found the lat_1 and lat_2 we need to swap and check to see if we can find it...
    if (( lat1Str != "" ) && ( lat2Str != "" ) )
    {
      // Make our new string to check...
      QString theProj4StringModified = theProj4String;
      // First just swap in the lat_2 value for lat_1 value
      theProj4StringModified.replace( myStart1 + LAT_PREFIX_LEN, myLength1 - LAT_PREFIX_LEN, lat2Str );
      // Now we have to find the lat_2 location again since it has potentially moved...
      myStart2 = 0;
      myStart2 = myLat2RegExp.indexIn( theProj4String, myStart2 );
      theProj4StringModified.replace( myStart2 + LAT_PREFIX_LEN, myLength2 - LAT_PREFIX_LEN, lat1Str );
      QgsDebugMsg( "trying proj4string match with swapped lat_1,lat_2" );
      myRecord = getRecord( "select * from tbl_srs where parameters='" + theProj4StringModified.trimmed() + "'" );
      if ( !myRecord.empty() )
      {
        // Success!  We have found the proj string by swapping the lat_1 and lat_2
        setProj4String( theProj4StringModified );
        mySrsId = myRecord["srs_id"].toLong();
        QgsDebugMsg( "proj4string match search for srsid returned srsid: " + QString::number( mySrsId ) );
        if ( mySrsId > 0 )
        {
          createFromSrsId( mySrsId );
        }
      }
    }
    else
    {
      // Last ditch attempt to piece together what we know of the projection to find a match...
      QgsDebugMsg( "globbing search for srsid from this proj string" );
      setProj4String( theProj4String );
      mySrsId = findMatchingProj();
      QgsDebugMsg( "globbing search for srsid returned srsid: " + QString::number( mySrsId ) );
      if ( mySrsId > 0 )
      {
        createFromSrsId( mySrsId );
      }
      else
      {
        mIsValidFlag = false;
      }
    }
  }

  // if we failed to look up the projection in database, don't worry. we can still use it :)
  if ( !mIsValidFlag )
  {
    QgsDebugMsg( "Projection is not found in databases." );
    setProj4String( theProj4String );
    // Is the SRS is valid now, we know it's a decent +proj string that can be entered into the srs.db
    if ( mIsValidFlag )
    {
      // Try to save. If not possible, set to invalid. Problems on read only systems?
      QgsDebugMsg( "Projection appears to be valid. Save to database!" );
      mIsValidFlag = saveAsUserCRS();
      // The srsid is not set, we should do that now.
      if ( mIsValidFlag )
      {
        myRecord = getRecord( "select * from tbl_srs where parameters='" + theProj4String.trimmed() + "'" );
        if ( !myRecord.empty() )
        {
          mySrsId = myRecord["srs_id"].toLong();
          QgsDebugMsg( "proj4string match search for srsid returned srsid: " + QString::number( mySrsId ) );
          if ( mySrsId > 0 )
          {
            createFromSrsId( mySrsId );
          }
          else
          {
            QgsDebugMsg( "Couldn't find newly added proj string?" );
            mIsValidFlag = false;
          }
        }
      }
    }
  }


  return mIsValidFlag;
}

//private method meant for internal use by this class only
QgsCoordinateReferenceSystem::RecordMap QgsCoordinateReferenceSystem::getRecord( QString theSql )
{
  QString myDatabaseFileName;
  QgsCoordinateReferenceSystem::RecordMap myMap;
  QString myFieldName;
  QString myFieldValue;
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  QgsDebugMsg( "running query: " + theSql );
  // Get the full path name to the sqlite3 spatial reference database.
  myDatabaseFileName = QgsApplication::srsDbFilePath();
  QFileInfo myInfo( myDatabaseFileName );
  if ( !myInfo.exists() )
  {
    QgsDebugMsg( "failed : " + myDatabaseFileName +
                 " does not exist!" );
    return myMap;
  }

  //check the db is available
  myResult = openDb( myDatabaseFileName, &myDatabase );
  if ( myResult )
  {
    return myMap;
  }

  myResult = sqlite3_prepare( myDatabase, theSql.toUtf8(), theSql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK && sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
  {
    QgsDebugMsg( "trying system srs.db" );
    int myColumnCount = sqlite3_column_count( myPreparedStatement );
    //loop through each column in the record adding its field name and vvalue to the map
    for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
    {
      myFieldName = QString::fromUtf8(( char * )sqlite3_column_name( myPreparedStatement, myColNo ) );
      myFieldValue = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, myColNo ) );
      myMap[myFieldName] = myFieldValue;
    }
  }
  else
  {
    QgsDebugMsg( "trying system qgis.db" );
    sqlite3_finalize( myPreparedStatement );
    sqlite3_close( myDatabase );

    myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists( ) )
    {
      QgsLogger::warning( "QgsCoordinateReferenceSystem::getRecord failed :  users qgis.db not found" );
      return myMap;
    }

    //check the db is available
    myResult = openDb( myDatabaseFileName, &myDatabase );
    if ( myResult )
    {
      return myMap;
    }

    myResult = sqlite3_prepare( myDatabase, theSql.toUtf8(), theSql.toUtf8().length(), &myPreparedStatement, &myTail );
    // XXX Need to free memory from the error msg if one is set
    if ( myResult == SQLITE_OK && sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      int myColumnCount = sqlite3_column_count( myPreparedStatement );
      //loop through each column in the record adding its field name and vvalue to the map
      for ( int myColNo = 0; myColNo < myColumnCount; myColNo++ )
      {
        myFieldName = QString::fromUtf8(( char * )sqlite3_column_name( myPreparedStatement, myColNo ) );
        myFieldValue = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, myColNo ) );
        myMap[myFieldName] = myFieldValue;
      }
    }
    else
    {
      QgsLogger::warning( "QgsCoordinateReferenceSystem::getRecord failed :  " + theSql );

    }
  }
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );

#ifdef QGISDEBUG
  QgsDebugMsg( "retrieved:  " + theSql );
  RecordMap::Iterator it;
  for ( it = myMap.begin(); it != myMap.end(); ++it )
  {
    QgsDebugMsgLevel( it.key() + " => " + it.value(), 2 );
  }
#endif

  return myMap;



}

// Accessors -----------------------------------

long QgsCoordinateReferenceSystem::srsid() const
{
  return mSrsId;
}

long QgsCoordinateReferenceSystem::postgisSrid() const
{

  return mSRID;

}

long QgsCoordinateReferenceSystem::epsg() const
{
  return mEpsg;
}

QString QgsCoordinateReferenceSystem::description() const
{
  if ( mDescription.isNull() )
  {
    return "";
  }
  else
  {
    return mDescription;
  }
}

QString QgsCoordinateReferenceSystem::projectionAcronym() const
{
  if ( mProjectionAcronym.isNull() )
  {
    return "";
  }
  else
  {
    return mProjectionAcronym;
  }
}

QString QgsCoordinateReferenceSystem::ellipsoidAcronym() const
{
  if ( mEllipsoidAcronym.isNull() )
  {
    return "";
  }
  else
  {
    return mEllipsoidAcronym;
  }
}

QString QgsCoordinateReferenceSystem::toProj4() const
{
  if ( !mIsValidFlag )
    return "";

  QString toProj4;
  char *proj4src = NULL;
  OSRExportToProj4( mCRS, &proj4src );
  toProj4 = proj4src;
  CPLFree( proj4src );

  // Stray spaces at the end?
  return toProj4.trimmed();
}

bool QgsCoordinateReferenceSystem::geographicFlag() const
{
  return mGeoFlag;
}

QGis::UnitType QgsCoordinateReferenceSystem::mapUnits() const
{
  return mMapUnits;
}


// Mutators -----------------------------------


void QgsCoordinateReferenceSystem::setInternalId( long theSrsId )
{
  mSrsId = theSrsId;
}
void QgsCoordinateReferenceSystem::setSrid( long theSrid )
{
  mSRID = theSrid;
}
void QgsCoordinateReferenceSystem::setDescription( QString theDescription )
{
  mDescription = theDescription;
}
void QgsCoordinateReferenceSystem::setProj4String( QString theProj4String )
{
  const char *oldlocale = setlocale( LC_NUMERIC, NULL );

  setlocale( LC_NUMERIC, "C" );
  OSRDestroySpatialReference( mCRS );
  mCRS = OSRNewSpatialReference( NULL );
  mIsValidFlag = OSRImportFromProj4( mCRS, theProj4String.toLatin1().constData() ) == OGRERR_NONE;
  setMapUnits();
  debugPrint();

  setlocale( LC_NUMERIC, oldlocale );
}
void QgsCoordinateReferenceSystem::setGeographicFlag( bool theGeoFlag )
{
  mGeoFlag = theGeoFlag;
}
void QgsCoordinateReferenceSystem::setEpsg( long theEpsg )
{
  mEpsg = theEpsg;
}
void  QgsCoordinateReferenceSystem::setProjectionAcronym( QString theProjectionAcronym )
{
  mProjectionAcronym = theProjectionAcronym;
}
void  QgsCoordinateReferenceSystem::setEllipsoidAcronym( QString theEllipsoidAcronym )
{
  mEllipsoidAcronym = theEllipsoidAcronym;
}

void QgsCoordinateReferenceSystem::setMapUnits()
{
  if ( !mIsValidFlag )
  {
    mMapUnits = QGis::UnknownUnit;
    return;
  }

  char *unitName;

  // Of interest to us is that this call adds in a unit parameter if
  // one doesn't already exist.
  OSRFixup( mCRS );

  if ( OSRIsProjected( mCRS ) )
  {
    double toMeter = OSRGetLinearUnits( mCRS, &unitName );
    QString unit( unitName );

    // If the units parameter was created during the Fixup() call
    // above, the name of the units is likely to be 'unknown'. Try to
    // do better than that ... (but perhaps ogr should be enhanced to
    // do this instead?).

    static const double feetToMeter = 0.3048;
    static const double smallNum = 1e-3;

    if ( std::abs( toMeter - feetToMeter ) < smallNum )
      unit = "Foot";

    QgsDebugMsg( "Projection has linear units of " + unit );

    if ( unit == "Meter" )
      mMapUnits = QGis::Meters;
    else if ( unit == "Foot" )
      mMapUnits = QGis::Feet;
    else
    {
      QgsLogger::warning( "Unsupported map units of " + unit );
      mMapUnits = QGis::UnknownUnit;
    }
  }
  else
  {
    OSRGetAngularUnits( mCRS, &unitName );
    QString unit( unitName );
    if ( unit == "degree" )
      mMapUnits = QGis::Degrees;
    else
    {
      QgsLogger::warning( "Unsupported map units of " + unit );
      mMapUnits = QGis::UnknownUnit;
    }
    QgsDebugMsg( "Projection has angular units of " + unit );
  }
}

/*
*    check if srs is a geocs or a proj cs (using ogr isGeographic)
*   then sequentially walk through the database (first users qgis.db srs tbl then
*   system srs.db tbl), converting each entry into an ogr srs and using isSame
*   or isSameGeocs (essentially calling the == overloaded operator). We'll try to
*   be smart about this and first parse out the proj and ellpse strings and only
*   check for a match in entities that have the same ellps and proj entries so
*   that it doesnt munch yer cpu so much.
*/
long QgsCoordinateReferenceSystem::findMatchingProj()
{
  QgsDebugMsg( "entered." );
  if ( mEllipsoidAcronym.isNull() ||  mProjectionAcronym.isNull() || !mIsValidFlag )
  {
    QgsDebugMsg( "QgsCoordinateReferenceSystem::findMatchingProj will only work if prj acr ellipsoid acr and proj4string are set"
                 " and the current projection is valid!" );
    return 0;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  // Set up the query to retrieve the projection information needed to populate the list
  QString mySql = QString( "select srs_id,parameters from tbl_srs where projection_acronym='" +
                           mProjectionAcronym + "' and ellipsoid_acronym='" + mEllipsoidAcronym + "'" );
  // Get the full path name to the sqlite3 spatial reference database.
  QString myDatabaseFileName = QgsApplication::srsDbFilePath();

  //check the db is available
  myResult = openDb( myDatabaseFileName, &myDatabase );
  if ( myResult )
  {
    return 0;
  }

  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {

    while ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString mySrsId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      QString myProj4String = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) );
      if ( equals( myProj4String ) )
      {
        QgsDebugMsg( "-------> MATCH FOUND in srs.db srsid: " + mySrsId );
        // close the sqlite3 statement
        sqlite3_finalize( myPreparedStatement );
        sqlite3_close( myDatabase );
        return mySrsId.toLong();
      }
      else
      {
// QgsDebugMsg(QString(" Not matched : %1").arg(myProj4String));
      }
    }
  }
  QgsLogger::warning( "QgsCoordinateReferenceSystem::findMatchingProj ------->"
                      "\n no match found in srs.db, trying user db now!" );
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  //
  // Try the users db now
  //

  myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
  //check the db is available
  myResult = openDb( myDatabaseFileName, &myDatabase );
  if ( myResult )
  {
    return 0;
  }

  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {

    while ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString mySrsId = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      QString myProj4String = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 1 ) );
      if ( equals( myProj4String ) )
      {
        QgsDebugMsg( "-------> MATCH FOUND in user qgis.db srsid: " + mySrsId );
        // close the sqlite3 statement
        sqlite3_finalize( myPreparedStatement );
        sqlite3_close( myDatabase );
        return mySrsId.toLong();
      }
      else
      {
// QgsDebugMsg(QString(" Not matched : %1").arg(myProj4String));
      }
    }
  }
  QgsLogger::warning( "QgsCoordinateReferenceSystem::findMatchingProj -------> no match found in user db" );

  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return 0;
}

bool QgsCoordinateReferenceSystem::operator==( const QgsCoordinateReferenceSystem &theSrs )
{
  if ( !mIsValidFlag || !theSrs.mIsValidFlag )
  {
    return false;
  }
  char *thisStr;
  char *otherStr;

  // OSRIsSame is not relaibel when it comes to comparing +towgs84 parameters
  // Use string compare on WKT instead.
  if (( OSRExportToWkt( mCRS, &thisStr ) == OGRERR_NONE ) )
  {
    if ( OSRExportToWkt( theSrs.mCRS, &otherStr ) == OGRERR_NONE )
    {
      QgsDebugMsg( QString( "Comparing " ) + thisStr );
      QgsDebugMsg( QString( "     with " ) + otherStr );
      if ( !strcmp( thisStr, otherStr ) )
      {
        QgsDebugMsg( QString( "MATCHED!" ) + otherStr );
        CPLFree( thisStr );
        CPLFree( otherStr );
        return true;
      }
      CPLFree( otherStr );
    }
    CPLFree( thisStr );
  }
  return false;
}

bool QgsCoordinateReferenceSystem::operator!=( const QgsCoordinateReferenceSystem &theSrs )
{
  return  !( *this == theSrs );
}

bool QgsCoordinateReferenceSystem::equals( QString theProj4String )
{
  QgsCoordinateReferenceSystem r;
  r.setProj4String( theProj4String );
  return *this == r;
}

QString QgsCoordinateReferenceSystem::toWkt() const
{
  QString myWkt;
  char* Wkt;
  if ( OSRExportToWkt( mCRS, &Wkt ) == OGRERR_NONE )
  {
    myWkt = Wkt;
    OGRFree( Wkt );
  }

  return myWkt;
}

bool QgsCoordinateReferenceSystem::readXML( QDomNode & theNode )
{
  QgsDebugMsg( "Reading Spatial Ref Sys from xml ------------------------!" );
  QDomNode srsNode  = theNode.namedItem( "spatialrefsys" );

  if ( ! srsNode.isNull() )
  {
    QDomNode myNode = srsNode.namedItem( "proj4" );
    QDomElement myElement = myNode.toElement();

    if ( createFromProj4( myElement.text() ) )
    {
      // createFromProj4() sets everything, inlcuding map units
      QgsDebugMsg( "Setting from proj4 string" );
    }
    else
    {
      QgsDebugMsg( "Setting from elements one by one" );

      setProj4String( myElement.text() );

      myNode = srsNode.namedItem( "srsid" );
      myElement = myNode.toElement();
      setInternalId( myElement.text().toLong() );

      myNode = srsNode.namedItem( "srid" );
      myElement = myNode.toElement();
      setSrid( myElement.text().toLong() );

      myNode = srsNode.namedItem( "epsg" );
      myElement = myNode.toElement();
      setEpsg( myElement.text().toLong() );

      myNode = srsNode.namedItem( "description" );
      myElement = myNode.toElement();
      setDescription( myElement.text() );

      myNode = srsNode.namedItem( "projectionacronym" );
      myElement = myNode.toElement();
      setProjectionAcronym( myElement.text() );

      myNode = srsNode.namedItem( "ellipsoidacronym" );
      myElement = myNode.toElement();
      setEllipsoidAcronym( myElement.text() );

      myNode = srsNode.namedItem( "geographicflag" );
      myElement = myNode.toElement();
      if ( myElement.text().compare( "true" ) )
      {
        setGeographicFlag( true );
      }
      else
      {
        setGeographicFlag( false );
      }

      //make sure the map units have been set
      setMapUnits();

      //@TODO this srs needs to be validated!!!
      mIsValidFlag = true;//shamelessly hard coded for now
    }
  }
  else
  {
    // Return default CRS if none was found in the XML.
    createFromEpsg( GEO_EPSG_CRS_ID );
  }
  return true;
}

bool QgsCoordinateReferenceSystem::writeXML( QDomNode & theNode, QDomDocument & theDoc ) const
{

  QDomElement myLayerNode = theNode.toElement();
  QDomElement mySrsElement  = theDoc.createElement( "spatialrefsys" );

  QDomElement myProj4Element  = theDoc.createElement( "proj4" );
  myProj4Element.appendChild( theDoc.createTextNode( toProj4() ) );
  mySrsElement.appendChild( myProj4Element );

  QDomElement mySrsIdElement  = theDoc.createElement( "srsid" );
  mySrsIdElement.appendChild( theDoc.createTextNode( QString::number( srsid() ) ) );
  mySrsElement.appendChild( mySrsIdElement );

  QDomElement mySridElement  = theDoc.createElement( "srid" );
  mySridElement.appendChild( theDoc.createTextNode( QString::number( postgisSrid() ) ) );
  mySrsElement.appendChild( mySridElement );

  QDomElement myEpsgElement  = theDoc.createElement( "epsg" );
  myEpsgElement.appendChild( theDoc.createTextNode( QString::number( epsg() ) ) );
  mySrsElement.appendChild( myEpsgElement );

  QDomElement myDescriptionElement  = theDoc.createElement( "description" );
  myDescriptionElement.appendChild( theDoc.createTextNode( description() ) );
  mySrsElement.appendChild( myDescriptionElement );

  QDomElement myProjectionAcronymElement  = theDoc.createElement( "projectionacronym" );
  myProjectionAcronymElement.appendChild( theDoc.createTextNode( projectionAcronym() ) );
  mySrsElement.appendChild( myProjectionAcronymElement );

  QDomElement myEllipsoidAcronymElement  = theDoc.createElement( "ellipsoidacronym" );
  myEllipsoidAcronymElement.appendChild( theDoc.createTextNode( ellipsoidAcronym() ) );
  mySrsElement.appendChild( myEllipsoidAcronymElement );

  QDomElement myGeographicFlagElement  = theDoc.createElement( "geographicflag" );
  QString myGeoFlagText = "false";
  if ( geographicFlag() )
  {
    myGeoFlagText = "true";
  }

  myGeographicFlagElement.appendChild( theDoc.createTextNode( myGeoFlagText ) );
  mySrsElement.appendChild( myGeographicFlagElement );

  myLayerNode.appendChild( mySrsElement );

  return true;
}



//
// Static helper methods below this point only please!
//


// Returns the whole proj4 string for the selected srsid
//this is a static method! NOTE I've made it private for now to reduce API clutter TS
QString QgsCoordinateReferenceSystem::proj4FromSrsId( const int theSrsId )
{

  QString myDatabaseFileName;
  QString myProjString;
  QString mySql = "select parameters from tbl_srs where srs_id = ";
  mySql += QString::number( theSrsId );

  QgsDebugMsg( "mySrsId = " + QString::number( theSrsId ) );
  QgsDebugMsg( "USER_CRS_START_ID = " + QString::number( USER_CRS_START_ID ) );
  QgsDebugMsg( "Selection sql : " + mySql );

  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  if ( theSrsId >= USER_CRS_START_ID )
  {
    myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
    QFileInfo myFileInfo;
    myFileInfo.setFile( myDatabaseFileName );
    if ( !myFileInfo.exists( ) ) //its unlikely that this condition will ever be reached
    {
      QgsLogger::critical( "QgsCoordinateReferenceSystem::getProj4FromSrsId :  users qgis.db not found" );
      return NULL;
    }
  }
  else //must be  a system projection then
  {
    myDatabaseFileName = QgsApplication::srsDbFilePath();
  }
  QgsDebugMsg( "db = " + myDatabaseFileName );

  sqlite3 *db;
  int rc;
  rc = openDb( myDatabaseFileName, &db );
  if ( rc )
  {
    return QString();
  }
  // prepare the sql statement
  const char *pzTail;
  sqlite3_stmt *ppStmt;

  rc = sqlite3_prepare( db, mySql.toUtf8(), mySql.toUtf8().length(), &ppStmt, &pzTail );
  // XXX Need to free memory from the error msg if one is set

  if ( rc == SQLITE_OK )
  {
    if ( sqlite3_step( ppStmt ) == SQLITE_ROW )
    {
      myProjString = QString::fromUtf8(( char* )sqlite3_column_text( ppStmt, 0 ) );
    }
  }
  // close the statement
  sqlite3_finalize( ppStmt );
  // close the database
  sqlite3_close( db );

  //assert(myProjString.length() > 0);
  return myProjString;
}

int QgsCoordinateReferenceSystem::openDb( QString path, sqlite3 **db )
{
  QgsDebugMsg( "path = " + path );
  int myResult = sqlite3_open( path.toUtf8().data(), db );

  if ( myResult )
  {
    QgsLogger::critical( "Can't open database: " + QString( sqlite3_errmsg( *db ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    // ... unfortunately it happens on Windows
    QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
    output->setTitle( "Error" );
    output->setMessage( "Could not open CRS database " + path +
                        "<br>Error(" + QString::number( myResult ) + "): " +
                        QString( sqlite3_errmsg( *db ) ), QgsMessageOutput::MessageText );
    output->showMessage();
  }
  return myResult;
}

void QgsCoordinateReferenceSystem::setCustomSrsValidation( CUSTOM_CRS_VALIDATION f )
{
  mCustomSrsValidation = f;
}

CUSTOM_CRS_VALIDATION QgsCoordinateReferenceSystem::customSrsValidation()
{
  return mCustomSrsValidation;
}

void QgsCoordinateReferenceSystem::debugPrint()
{
  QgsDebugMsg( "***SpatialRefSystem***" );
  QgsDebugMsg( "* Valid : " + ( mIsValidFlag ? QString( "true" ) : QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( mSrsId ) );
  QgsDebugMsg( "* Proj4 : " + toProj4() );
  QgsDebugMsg( "* WKT   : " + toWkt() );
  QgsDebugMsg( "* Desc. : " + mDescription );
  if ( mapUnits() == QGis::Meters )
  {
    QgsDebugMsg( "* Units : meters" );
  }
  else if ( mapUnits() == QGis::Feet )
  {
    QgsDebugMsg( "* Units : feet" );
  }
  else if ( mapUnits() == QGis::Degrees )
  {
    QgsDebugMsg( "* Units : degrees" );
  }
}

void QgsCoordinateReferenceSystem::setValidationHint( QString html )
{
  mValidationHint = html;
}

QString QgsCoordinateReferenceSystem::validationHint()
{
  return mValidationHint;
}

/// Copied from QgsCustomProjectionDialog ///
/// Please refactor into SQL handler !!!  ///

bool QgsCoordinateReferenceSystem::saveAsUserCRS()
{

  if ( ! mIsValidFlag )
  {
    QgsDebugMsg( "Can't save an invalid CRS!" );
    return false;
  }

  QString mySql;
  QString myName = QString( " * %1 (%2)" )
                   .arg( QObject::tr( "Generated CRS", "A CRS automatically generated from layer info get this prefix for description" ) )
                   .arg( toProj4() );

  //if this is the first record we need to ensure that its srs_id is 10000. For
  //any rec after that sqlite3 will take care of the autonumering
  //this was done to support sqlite 3.0 as it does not yet support
  //the autoinc related system tables.
  if ( getRecordCount() == 0 )
  {
    mySql = QString( "insert into tbl_srs (srs_id,description,projection_acronym,ellipsoid_acronym,parameters,is_geo) " )
            + " values (" + QString::number( USER_CRS_START_ID ) + ",'"
            + sqlSafeString( myName ) + "','" + projectionAcronym()
            + "','" + ellipsoidAcronym()  + "','" + sqlSafeString( toProj4() )
            + "',0)"; // <-- is_geo shamelessly hard coded for now
  }
  else
  {
    mySql = "insert into tbl_srs (description,projection_acronym,ellipsoid_acronym,parameters,is_geo) values ('"
            + sqlSafeString( myName ) + "','" + projectionAcronym()
            + "','" + ellipsoidAcronym()  + "','" + sqlSafeString( toProj4() )
            + "',0)"; // <-- is_geo shamelessly hard coded for now
  }
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1 \n please notify  QGIS developers of this error \n %2 (file name) " ).arg( sqlite3_errmsg( myDatabase ) ).arg( QgsApplication::qgisUserDbFilePath() ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  QgsDebugMsg( QString( "Update or insert sql \n%1" ).arg( mySql ) );
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  sqlite3_step( myPreparedStatement );
  // XXX Need to free memory from the error msg if one is set
  return myResult == SQLITE_OK;

}

long QgsCoordinateReferenceSystem::getRecordCount()
{
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  long          myRecordCount = 0;
  //check the db is available
  myResult = sqlite3_open( QgsApplication::qgisUserDbFilePath().toUtf8().data(), &myDatabase );
  if ( myResult != SQLITE_OK )
  {
    QgsDebugMsg( QString( "Can't open database: %1" ).arg( sqlite3_errmsg( myDatabase ) ) );
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert( myResult == SQLITE_OK );
  }
  // Set up the query to retrieve the projection information needed to populate the ELLIPSOID list
  QString mySql = "select count(*) from tbl_srs";
  myResult = sqlite3_prepare( myDatabase, mySql.toUtf8(), mySql.toUtf8().length(), &myPreparedStatement, &myTail );
  // XXX Need to free memory from the error msg if one is set
  if ( myResult == SQLITE_OK )
  {
    if ( sqlite3_step( myPreparedStatement ) == SQLITE_ROW )
    {
      QString myRecordCountString = QString::fromUtf8(( char * )sqlite3_column_text( myPreparedStatement, 0 ) );
      myRecordCount = myRecordCountString.toLong();
    }
  }
  // close the sqlite3 statement
  sqlite3_finalize( myPreparedStatement );
  sqlite3_close( myDatabase );
  return myRecordCount;

}

const QString QgsCoordinateReferenceSystem::sqlSafeString( const QString theSQL )
{

  QString myRetval;
  QChar *it = ( QChar * )theSQL.unicode();
  for ( int i = 0; i < theSQL.length(); i++ )
  {
    if ( *it == '\"' )
    {
      myRetval += "\\\"";
    }
    else if ( *it == '\'' )
    {
      myRetval += "\\'";
    }
    else if ( *it == '\\' )
    {
      myRetval += "\\\\";
    }
    else if ( *it == '%' )
    {
      myRetval += "\\%";
    }
    else
    {
      myRetval += *it;
    }
    it++;
  }
  return myRetval;
}
