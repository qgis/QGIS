/***************************************************************************
                          qgsspatialrefsys.cpp
                       
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
#include "qgsspatialrefsys.h"

#include <cmath>

#include <QDir>
#include <QDomNode>
#include <QDomElement>
#include <QFileInfo>
#include <QRegExp>

#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgsmessageoutput.h"
#include "qgis.h" //const vals declared here

#include <sqlite3.h>

//gdal and ogr includes (needed for == operator)
#include <ogr_srs_api.h>
#include <cpl_error.h>
#include <cpl_conv.h>

CUSTOM_SRS_VALIDATION QgsSpatialRefSys::mCustomSrsValidation = NULL;

//--------------------------

QgsSpatialRefSys::QgsSpatialRefSys()
  : mMapUnits(QGis::UNKNOWN),
    mIsValidFlag(0)
{
  mSRS = OSRNewSpatialReference(NULL);
}

QgsSpatialRefSys::QgsSpatialRefSys(QString theWkt)
  : mMapUnits(QGis::UNKNOWN),
    mIsValidFlag(0)
{
  mSRS = OSRNewSpatialReference(NULL);
  createFromWkt(theWkt);
}


QgsSpatialRefSys::QgsSpatialRefSys(const long theId, SRS_TYPE theType)
  : mMapUnits(QGis::UNKNOWN),
    mIsValidFlag(0)
{
  mSRS = OSRNewSpatialReference(NULL);
  createFromId(theId, theType);
}

QgsSpatialRefSys::~QgsSpatialRefSys()
{
  OSRDestroySpatialReference( mSRS );
}

void QgsSpatialRefSys::createFromId(const long theId, SRS_TYPE theType)
{
  switch (theType)
  {
  case QGIS_SRSID:
    createFromSrsId(theId);
    break;
  case POSTGIS_SRID:
    createFromSrid(theId);
    break;
  case EPSG:
    createFromEpsg(theId);
    break;
  default:
    //THIS IS BAD...THIS PART OF CODE SHOULD NEVER BE REACHED...
    QgsLogger::critical("Unexpected case reached in " + QString(__FILE__) + " : " + QString(__LINE__));
  };
}

bool QgsSpatialRefSys::createFromOgcWmsCrs(QString theCrs)
{
  QStringList parts = theCrs.split(":");

  if(parts.at(0).compare("EPSG", Qt::CaseInsensitive) == 0)
  {
    createFromEpsg( parts.at(1).toLong() );
  }
  else if(parts.at(0).compare("CRS", Qt::CaseInsensitive) == 0)
  {
    if (parts.at(1) == "84")
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

QgsSpatialRefSys::QgsSpatialRefSys(const QgsSpatialRefSys &srs)
{
  mSRS = OSRNewSpatialReference(NULL);
  *this = srs;
}

// Assignment operator
QgsSpatialRefSys& QgsSpatialRefSys::operator=(const QgsSpatialRefSys& srs)
{
  if (&srs != this)
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
    if(mIsValidFlag)
    {
      OSRDestroySpatialReference(mSRS);
      mSRS = OSRClone(srs.mSRS);
    }
  }
  return *this;
}

// Misc helper functions -----------------------


void QgsSpatialRefSys::validate()
{
  if(mIsValidFlag)
    return;

  // try to validate using custom validation routines
  if (mCustomSrsValidation)
    mCustomSrsValidation(this);

  if(!mIsValidFlag)
    // set the default
    createFromProj4(GEOPROJ4);  
}

bool QgsSpatialRefSys::createFromSrid(long id)
{
  return loadFromDb( QgsApplication::srsDbFilePath(), "srid", id);
}

bool QgsSpatialRefSys::createFromEpsg(long id)
{
  return loadFromDb( QgsApplication::srsDbFilePath(), "epsg", id);
}

bool QgsSpatialRefSys::createFromSrsId(long id)
{
  return loadFromDb( id<100000 ? QgsApplication::srsDbFilePath() : QgsApplication::qgisUserDbFilePath(), "srs_id", id);
}

bool QgsSpatialRefSys::loadFromDb(QString db, QString field, long id)
{
  QgsDebugMsg("load SRS from " + db + " where " + field + " is " + QString::number(id));
  mIsValidFlag=false;

  QFileInfo myInfo(db);
  if(!myInfo.exists())
  {
    QgsDebugMsg("failed : " + db + " does not exist!");
    return mIsValidFlag;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = openDb(db, &myDatabase);
  if(myResult)
  {
    QgsDebugMsg("failed : " + db + " could not be opened!");
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

  QString mySql = "select srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,epsg,is_geo from tbl_srs where " + field + "='" + QString::number(id) + "'";
  myResult = sqlite3_prepare(myDatabase, mySql.toUtf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK && sqlite3_step(myPreparedStatement) == SQLITE_ROW)
  {
    mSrsId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0)).toLong();
    mDescription = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,1));
    mProjectionAcronym = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,2));
    mEllipsoidAcronym = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,3));
    QString proj4String = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,4));
    mSRID = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,5)).toLong();
    mEpsg = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,6)).toLong();
    int geo = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,7)).toInt();
    mGeoFlag = (geo == 0 ? false : true);
    setMapUnits();
    mIsValidFlag = true;
    const char *oldlocale = setlocale(LC_ALL, NULL);
    setlocale(LC_ALL, "C");
    OSRImportFromProj4( mSRS, proj4String.toLatin1().constData() );
    setlocale(LC_ALL, oldlocale);
  }
  else
  {
    QgsDebugMsg("failed : " + mySql);
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return mIsValidFlag;
}

bool QgsSpatialRefSys::createFromWkt(QString theWkt)
{
  mIsValidFlag = false;

  if (theWkt.isEmpty())
  {
    QgsDebugMsg("QgsSpatialRefSys::createFromWkt -- theWkt is uninitialised, operation failed")
    QgsLogger::critical("QgsSpatialRefSys::createFromWkt -- theWkt is uninitialised, operation failed");
    return mIsValidFlag;
  }
  QgsDebugMsg("QgsSpatialRefSys::createFromWkt(QString theWkt) using: " + theWkt);
  QByteArray ba = theWkt.toLatin1();
  const char *pWkt = ba;

  OGRErr myInputResult = OSRImportFromWkt( mSRS, (char **)&pWkt );

  if (myInputResult != OGRERR_NONE)
  {
    QgsDebugMsg("vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv");
    QgsDebugMsg("QgsSpatialRefSys::createFromWkt(QString theWkt) ");
    QgsDebugMsg("This SRS could *** NOT *** be set from the supplied WKT ");
    QgsDebugMsg("INPUT: " + theWkt);
    QgsDebugMsg("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^");
    return mIsValidFlag;
  }

  // always morph from esri as it doesn't hurt anything
  // FW: Hey, that's not right!  It can screw stuff up! Disable
  //myOgrSpatialRef.morphFromESRI();

  // create the proj4 structs needed for transforming
  char *proj4src = NULL;
  OSRExportToProj4(mSRS, &proj4src);

  //now that we have the proj4string, delegate to createFromProj4String so
  // that we can try to fill in the remaining class members...
  //create from Proj wil set the isValidFalg
  createFromProj4(QString(proj4src));
  CPLFree(proj4src);

  return mIsValidFlag;
  //setMapunits will be called by createfromproj above
}

bool QgsSpatialRefSys::isValid() const
{
  if (mIsValidFlag)
    return true;

  QgsDebugMsg("It's an invalid SRS without valid proj4 string");

  return false;
}

bool QgsSpatialRefSys::createFromProj4 (const QString theProj4String)
{
  //
  // Examples:
  // +proj=tmerc +lat_0=0 +lon_0=-62 +k=0.999500 +x_0=400000 +y_0=0
  // +ellps=clrk80 +towgs84=-255,-15,71,0,0,0,0 +units=m +no_defs
  //
  // +proj=lcc +lat_1=46.8 +lat_0=46.8 +lon_0=2.337229166666664 +k_0=0.99987742
  // +x_0=600000 +y_0=2200000 +a=6378249.2 +b=6356515.000000472 +units=m +no_defs
  //
  mIsValidFlag=false;

  QRegExp myProjRegExp( "\\+proj=\\S+" );
  int myStart= 0;
  int myLength=0;
  myStart = myProjRegExp.indexIn(theProj4String, myStart);
  if (myStart==-1)
  {
    QgsDebugMsg("error proj string supplied has no +proj argument");
    return mIsValidFlag;
  }
  else
  {
    myLength = myProjRegExp.matchedLength();
  }

  mProjectionAcronym = theProj4String.mid(myStart+PROJ_PREFIX_LEN,myLength-PROJ_PREFIX_LEN);

  QRegExp myEllipseRegExp( "\\+ellps=\\S+" );
  myStart= 0;
  myLength=0;
  myStart = myEllipseRegExp.indexIn(theProj4String, myStart);
  if (myStart!=-1)
  {
    myLength = myEllipseRegExp.matchedLength();
    mEllipsoidAcronym = theProj4String.mid(myStart+ELLPS_PREFIX_LEN,myLength-ELLPS_PREFIX_LEN);
  }

  QRegExp myAxisRegExp( "\\+a=\\S+" );
  myStart= 0;
  myLength=0;
  myStart = myAxisRegExp.indexIn(theProj4String, myStart);
  if (myStart==-1 && mEllipsoidAcronym.isNull())
  {
    QgsLogger::warning("QgsSpatialRefSys::createFromProj4 error proj string supplied has no +ellps or +a argument");
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
  QgsSpatialRefSys::RecordMap myRecord;
  if (!mDescription.trimmed().isEmpty())
  {
    myRecord = getRecord("select * from tbl_srs where description='" + mDescription.trimmed() + "'");
  }

  /*
  * - if the above does not match perform a whole text search on proj4 string (if not null)
  */
  QgsDebugMsg("QgsSpatialRefSys::createFromProj4 wholetext match on name failed, trying proj4string match");
  myRecord = getRecord("select * from tbl_srs where parameters='" + theProj4String.trimmed() + "'");
  if (!myRecord.empty())
  {
    mySrsId=myRecord["srs_id"].toLong();
    QgsDebugMsg("QgsSpatialRefSys::createFromProj4 proj4string match search for srsid returned srsid: " + QString::number(mySrsId));
    if (mySrsId > 0)
    {
      createFromSrsId(mySrsId);
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
    myStart1 = myLat1RegExp.indexIn(theProj4String, myStart1);
    myStart2 = myLat2RegExp.indexIn(theProj4String, myStart2);
    if ((myStart1 != -1) && (myStart2 != -1))
    {
      myLength1 = myLat1RegExp.matchedLength();
      myLength2 = myLat2RegExp.matchedLength();
      lat1Str = theProj4String.mid(myStart1+LAT_PREFIX_LEN,myLength1-LAT_PREFIX_LEN);
      lat2Str = theProj4String.mid(myStart2+LAT_PREFIX_LEN,myLength2-LAT_PREFIX_LEN);
    }
    // If we found the lat_1 and lat_2 we need to swap and check to see if we can find it...
    if ((lat1Str != "") && (lat2Str != ""))
    {
      // Make our new string to check...
      QString theProj4StringModified = theProj4String;
      // First just swap in the lat_2 value for lat_1 value
      theProj4StringModified.replace(myStart1+LAT_PREFIX_LEN,myLength1-LAT_PREFIX_LEN,lat2Str);
      // Now we have to find the lat_2 location again since it has potentially moved...
      myStart2 = 0;
      myStart2 = myLat2RegExp.indexIn(theProj4String, myStart2);
      theProj4StringModified.replace(myStart2+LAT_PREFIX_LEN,myLength2-LAT_PREFIX_LEN,lat1Str);
      QgsDebugMsg("QgsSpatialRefSys::createFromProj4 - trying proj4string match with swapped lat_1,lat_2");
      myRecord = getRecord("select * from tbl_srs where parameters='" + theProj4StringModified.trimmed() + "'");
      if (!myRecord.empty())
      {
        // Success!  We have found the proj string by swapping the lat_1 and lat_2
        const char *oldlocale = setlocale(LC_ALL, NULL);
        setlocale(LC_ALL, "C");
        OSRImportFromProj4(mSRS, theProj4StringModified.toLatin1().constData() );
        setlocale(LC_ALL, oldlocale); 
        mySrsId=myRecord["srs_id"].toLong();
        QgsDebugMsg("QgsSpatialRefSys::createFromProj4 proj4string match search for srsid returned srsid: " + QString::number(mySrsId));
        if (mySrsId > 0)
        {
          createFromSrsId(mySrsId);
        }
      }
    }
    else
    {
      // Last ditch attempt to piece together what we know of the projection to find a match...
      QgsDebugMsg("QgsSpatialRefSys::createFromProj4 globbing search for srsid from this proj string");
      mySrsId = findMatchingProj();
      QgsDebugMsg("QgsSpatialRefSys::createFromProj4 globbing search for srsid returned srsid: " + QString::number(mySrsId));
      if (mySrsId > 0)
      {
        createFromSrsId(mySrsId);
      }
    }
  }

  // NOTE: if its still empty after all the above steps then, the projection string is lost. Is that bad?
  return mIsValidFlag;
}

QgsSpatialRefSys::RecordMap QgsSpatialRefSys::getRecord(QString theSql)
{
  QString myDatabaseFileName;
  QgsSpatialRefSys::RecordMap myMap;
  QString myFieldName;
  QString myFieldValue;
  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  QgsDebugMsg("QgsSpatialRefSys::getRecord...running query: " + theSql);
  // Get the full path name to the sqlite3 spatial reference database.
  myDatabaseFileName = QgsApplication::srsDbFilePath();
  QFileInfo myInfo (myDatabaseFileName);
  if (!myInfo.exists())
  {
    QgsDebugMsg("QgsSpatialRefSys::createFromSrid failed : " + myDatabaseFileName + 
        " does not exist!");
    return myMap;
  }

  //check the db is available
  myResult = openDb(myDatabaseFileName, &myDatabase);
  if(myResult)
  {
    return myMap;
  }

  myResult = sqlite3_prepare(myDatabase, theSql.toUtf8(), theSql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK && sqlite3_step(myPreparedStatement) == SQLITE_ROW)
  {
    QgsDebugMsg("QgsSpatialRefSys::getRecord...trying system srs.db");
    int myColumnCount = sqlite3_column_count(myPreparedStatement);
    //loop through each column in the record adding its field name and vvalue to the map
    for (int myColNo=0;myColNo < myColumnCount;myColNo++)
    {
      myFieldName = QString::fromUtf8((char *)sqlite3_column_name(myPreparedStatement,myColNo));
      myFieldValue = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,myColNo));
      myMap[myFieldName]=myFieldValue;
    }
  }
  else
  {
    QgsDebugMsg("QgsSpatialRefSys::getRecord...trying system qgis.db");
    sqlite3_finalize(myPreparedStatement);
    sqlite3_close(myDatabase);

    myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
    QFileInfo myFileInfo;
    myFileInfo.setFile(myDatabaseFileName);
    if ( !myFileInfo.exists( ) )
    {
      QgsLogger::warning("QgsSpatialRefSys::getRecord failed :  users qgis.db not found");
      return myMap;
    }

    //check the db is available
    myResult = openDb(myDatabaseFileName, &myDatabase);
    if(myResult)
    {
      return myMap;
    }

    myResult = sqlite3_prepare(myDatabase, theSql.toUtf8(), theSql.length(), &myPreparedStatement, &myTail);
    // XXX Need to free memory from the error msg if one is set
    if(myResult == SQLITE_OK && sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      int myColumnCount = sqlite3_column_count(myPreparedStatement);
      //loop through each column in the record adding its field name and vvalue to the map
      for (int myColNo=0;myColNo < myColumnCount;myColNo++)
      {
        myFieldName = QString::fromUtf8((char *)sqlite3_column_name(myPreparedStatement,myColNo));
        myFieldValue = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,myColNo));
        myMap[myFieldName]=myFieldValue;
      }
    }
    else
    {
      QgsLogger::warning("QgsSpatialRefSys::getRecord failed :  " + theSql);

    }
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);

#ifdef QGISDEBUG
  QgsDebugMsg("QgsSpatialRefSys::getRecord retrieved:  " + theSql);
  RecordMap::Iterator it;
  for ( it = myMap.begin(); it != myMap.end(); ++it )
  {
    QgsDebugMsgLevel(it.key() + " => " + it.value(), 2);
  }
#endif

  return myMap;



}

// Accessors -----------------------------------
/*! Get the SrsId
 *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
 */
long QgsSpatialRefSys::srsid() const
{
  return mSrsId;
}
/*! Get the Postgis SRID - if possible
 *  @return  long theSRID The internal postgis SRID for this SRS
 */
long QgsSpatialRefSys::srid() const
{

  return mSRID;

}
/*! Get the Description
 * @return  QString the Description A textual description of the srs.
 */
QString QgsSpatialRefSys::description () const
{
  if (mDescription.isNull())
  {
    return "";
  }
  else
  {
    return mDescription;
  }
}
/*! Get the Projection Acronym
 * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
 */
QString QgsSpatialRefSys::projectionAcronym() const
{
  if (mProjectionAcronym.isNull())
  {
    return "";
  }
  else
  {
    return mProjectionAcronym;
  }
}
/*! Get the Ellipsoid Acronym
 * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
 */
QString QgsSpatialRefSys::ellipsoidAcronym () const
{
  if (mEllipsoidAcronym.isNull())
  {
    return "";
  }
  else
  {
    return mEllipsoidAcronym;
  }
}
/* Get the Proj Proj4String.
 * @return  QString theProj4String Proj4 format specifies that define this srs.
 */
QString QgsSpatialRefSys::proj4String() const
{
  if(!mIsValidFlag)
    return "";

  QString proj4String;
  char *proj4src = NULL;
  OSRExportToProj4(mSRS, &proj4src);
  proj4String = proj4src;
  CPLFree(proj4src);

  return proj4String;
}
/*! Get this Geographic? flag
 * @return  bool theGeoFlag Whether this is a geographic or projected coordinate system
 */
bool QgsSpatialRefSys::geographicFlag () const
{
  return mGeoFlag;
}
/*! Get the units that the projection is in
 * @return QGis::units
 */
QGis::units QgsSpatialRefSys::mapUnits() const
{
  return mMapUnits;
}

/*! Set the postgis srid for this srs
 * @return  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
 */
long QgsSpatialRefSys::postgisSrid () const
{
  return mSRID ;
}
/*! Set the EPSG identifier for this srs
 * @return  long theEpsg the ESPG identifier for this srs (defaults to 0)
 */
long QgsSpatialRefSys::epsg () const
{
  return mEpsg;
}

// Mutators -----------------------------------


void QgsSpatialRefSys::setSrsId(long theSrsId)
{
  mSrsId = theSrsId;
}
void QgsSpatialRefSys::setSrid(long theSrid)
{
  mSRID=theSrid;
}
void QgsSpatialRefSys::setDescription (QString theDescription)
{
  mDescription = theDescription;
}
void QgsSpatialRefSys::setProj4String (QString theProj4String)
{
  const char *oldlocale = setlocale(LC_ALL, NULL);
  setlocale(LC_ALL, "C");
  mIsValidFlag = OSRImportFromProj4(mSRS, theProj4String.toLatin1().constData() )==OGRERR_NONE;
  setlocale(LC_ALL, oldlocale);
}
void QgsSpatialRefSys::setGeographicFlag (bool theGeoFlag)
{
  mGeoFlag=theGeoFlag;
}
void QgsSpatialRefSys::setEpsg (long theEpsg)
{
  mEpsg=theEpsg;
}
void  QgsSpatialRefSys::setProjectionAcronym(QString theProjectionAcronym)
{
  mProjectionAcronym=theProjectionAcronym;
}
void  QgsSpatialRefSys::setEllipsoidAcronym(QString theEllipsoidAcronym)
{
  mEllipsoidAcronym=theEllipsoidAcronym;
}
/*! Work out the projection units and set the appropriate local variable
 *
 */
void QgsSpatialRefSys::setMapUnits()
{
  if (!mIsValidFlag)
  {
    QgsLogger::warning("No valid projection. Unable to set map units.");
    mMapUnits = QGis::UNKNOWN;
    return;
  }

  char *unitName;

  // Of interest to us is that this call adds in a unit parameter if
  // one doesn't already exist.
  OSRFixup(mSRS);

  if (OSRIsProjected(mSRS))
  {
    double toMeter = OSRGetLinearUnits(mSRS,&unitName);
    QString unit(unitName);

    // If the units parameter was created during the Fixup() call
    // above, the name of the units is likely to be 'unknown'. Try to
    // do better than that ... (but perhaps ogr should be enhanced to
    // do this instead?).

    static const double feetToMeter = 0.3048;
    static const double smallNum = 1e-3;

    if (std::abs(toMeter - feetToMeter) < smallNum)
      unit = "Foot";

    QgsDebugMsg("Projection has linear units of " + unit);

    if (unit == "Meter")
      mMapUnits = QGis::METERS;
    else if (unit == "Foot")
      mMapUnits = QGis::FEET;
    else
    {
      QgsLogger::warning("Unsupported map units of " + unit);
      mMapUnits = QGis::UNKNOWN;
    }
  }
  else
  {
    OSRGetAngularUnits(mSRS, &unitName);
    QString unit(unitName);
    if (unit == "degree")
      mMapUnits = QGis::DEGREES;
    else
    {
      QgsLogger::warning("Unsupported map units of " + unit);
      mMapUnits = QGis::UNKNOWN;
    }
    QgsDebugMsg("Projection has angular units of " + unit);
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
long QgsSpatialRefSys::findMatchingProj()
{
  QgsDebugMsg("QgsSpatialRefSys::findMatchingProj...");
  if (mEllipsoidAcronym.isNull() ||  mProjectionAcronym.isNull() || !mIsValidFlag)
  {
    QgsLogger::warning("QgsSpatialRefSys::findMatchingProj will only work if prj acr ellipsoid acr and proj4string are set!...");
    return 0;
  }

  sqlite3      *myDatabase;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;

  // Set up the query to retreive the projection information needed to populate the list
  QString mySql = QString ("select srs_id,parameters from tbl_srs where projection_acronym='" +
                           mProjectionAcronym + "' and ellipsoid_acronym='" + mEllipsoidAcronym + "'");
  // Get the full path name to the sqlite3 spatial reference database.
  QString myDatabaseFileName = QgsApplication::srsDbFilePath();

  //check the db is available
  myResult = openDb(myDatabaseFileName, &myDatabase);
  if(myResult)
  {
    return 0;
  }

  myResult = sqlite3_prepare(myDatabase, mySql.toUtf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {

    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      QString mySrsId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      QString myProj4String = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement, 1));
      if (equals(myProj4String))
      {
        QgsDebugMsg("QgsSpatialRefSys::findMatchingProj -------> MATCH FOUND in srs.db srsid: " + mySrsId);
        // close the sqlite3 statement
        sqlite3_finalize(myPreparedStatement);
        sqlite3_close(myDatabase);
        return mySrsId.toLong();
      }
      else
      {
        //std::cout << " Not matched : " << myProj4String << std::endl;
      }
    }
  }
  QgsLogger::warning("QgsSpatialRefSys::findMatchingProj ------->"
                      "\n no match found in srs.db, trying user db now!");
  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  //
  // Try the users db now
  //

  myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
  //check the db is available
  myResult = openDb(myDatabaseFileName, &myDatabase);
  if(myResult)
  {
    return 0;
  }

  myResult = sqlite3_prepare(myDatabase, mySql.toUtf8(), mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {

    while(sqlite3_step(myPreparedStatement) == SQLITE_ROW)
    {
      QString mySrsId = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement,0));
      QString myProj4String = QString::fromUtf8((char *)sqlite3_column_text(myPreparedStatement, 1));
      if (equals(myProj4String))
      {
        QgsDebugMsg("QgsSpatialRefSys::findMatchingProj -------> MATCH FOUND in user qgis.db srsid: " + mySrsId);
        // close the sqlite3 statement
        sqlite3_finalize(myPreparedStatement);
        sqlite3_close(myDatabase);
        return mySrsId.toLong();
      }
      else
      {
        //std::cout << " Not matched : " << myProj4String << std::endl;
      }
    }
  }
  QgsLogger::warning("QgsSpatialRefSys::findMatchingProj -------> no match found in user db");

  // close the sqlite3 statement
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
  return 0;
}

bool QgsSpatialRefSys::operator==(const QgsSpatialRefSys &theSrs)
{
  if (!mIsValidFlag || !theSrs.mIsValidFlag)
  {
    return false;
  }
  else if (OSRIsGeographic(mSRS) && OSRIsGeographic(theSrs.mSRS))
  {
    // qWarning("QgsSpatialRefSys::operator== srs1 and srs2 are geographic ");
    return OSRIsSameGeogCS(mSRS,theSrs.mSRS);
  }
  else if (OSRIsProjected(mSRS) && OSRIsProjected(theSrs.mSRS))
  {
    // qWarning("QgsSpatialRefSys::operator== srs1 and srs2 are projected ");
    return OSRIsSame(mSRS, theSrs.mSRS);
  }
  else
  {
    return false;
  }
}

bool QgsSpatialRefSys::operator!=(const QgsSpatialRefSys &theSrs)
{
  return  ! (*this == theSrs);
}

bool QgsSpatialRefSys::equals(QString theProj4String)
{
  QgsSpatialRefSys r;
  r.setProj4String(theProj4String);
  return *this==r;
}

QString QgsSpatialRefSys::toWkt() const
{
  QString myWkt;
  char* WKT;
  if(OSRExportToWkt(mSRS,&WKT) == OGRERR_NONE)
  {
    myWkt = WKT;
    OGRFree(WKT);
  }    

  return myWkt;
}

bool QgsSpatialRefSys::readXML( QDomNode & theNode )
{
  QgsDebugMsg("Reading Spatial Ref Sys from xml ------------------------!");
  QDomNode srsNode  = theNode.namedItem( "spatialrefsys" );

  if ( ! srsNode.isNull() )
  {
     QDomNode myNode = srsNode.namedItem("proj4");
     QDomElement myElement = myNode.toElement();
     setProj4String(myElement.text());

     myNode = srsNode.namedItem("srsid");
     myElement = myNode.toElement();
     setSrsId(myElement.text().toLong());

     myNode = srsNode.namedItem("srid");
     myElement = myNode.toElement();
     setSrid(myElement.text().toLong());

     myNode = srsNode.namedItem("epsg");
     myElement = myNode.toElement();
     setEpsg(myElement.text().toLong());

     myNode = srsNode.namedItem("description");
     myElement = myNode.toElement();
     setDescription(myElement.text());

     myNode = srsNode.namedItem("projectionacronym");
     myElement = myNode.toElement();
     setProjectionAcronym(myElement.text());

     myNode = srsNode.namedItem("ellipsoidacronym");
     myElement = myNode.toElement();
     setEllipsoidAcronym(myElement.text());
 
     myNode = srsNode.namedItem("geographicflag");
     myElement = myNode.toElement();
     if (myElement.text().compare("true"))
     {
       setGeographicFlag(true);
     }
     else
     {
       setGeographicFlag(false);
     }
     //make sure the map units have been set
     setMapUnits();

     //@TODO this srs needs to be validated!!!
     mIsValidFlag=true;//shamelessly hard coded for now
  }
  else
  {
    // Return default SRS if none was found in the XML.
    createFromSrsId(GEOSRS_ID);
  }
  return true;
}

bool QgsSpatialRefSys::writeXML( QDomNode & theNode, QDomDocument & theDoc ) const
{

  QDomElement myLayerNode = theNode.toElement();
  QDomElement mySrsElement  = theDoc.createElement( "spatialrefsys" );
  
  QDomElement myProj4Element  = theDoc.createElement( "proj4" );
  myProj4Element.appendChild(theDoc.createTextNode( proj4String()));
  mySrsElement.appendChild(myProj4Element);
  
  QDomElement mySrsIdElement  = theDoc.createElement( "srsid" );
  mySrsIdElement.appendChild(theDoc.createTextNode( QString::number(srsid())));
  mySrsElement.appendChild(mySrsIdElement);

  QDomElement mySridElement  = theDoc.createElement( "srid" );
  mySridElement.appendChild(theDoc.createTextNode( QString::number(srid())));
  mySrsElement.appendChild(mySridElement);

  QDomElement myEpsgElement  = theDoc.createElement( "epsg" );
  myEpsgElement.appendChild(theDoc.createTextNode( QString::number(epsg())));
  mySrsElement.appendChild(myEpsgElement);

  QDomElement myDescriptionElement  = theDoc.createElement( "description" );
  myDescriptionElement.appendChild(theDoc.createTextNode( description()));
  mySrsElement.appendChild(myDescriptionElement);

  QDomElement myProjectionAcronymElement  = theDoc.createElement( "projectionacronym" );
  myProjectionAcronymElement.appendChild(theDoc.createTextNode( projectionAcronym()));
  mySrsElement.appendChild(myProjectionAcronymElement);

  QDomElement myEllipsoidAcronymElement  = theDoc.createElement( "ellipsoidacronym" );
  myEllipsoidAcronymElement.appendChild(theDoc.createTextNode( ellipsoidAcronym()));
  mySrsElement.appendChild(myEllipsoidAcronymElement);

  QDomElement myGeographicFlagElement  = theDoc.createElement( "geographicflag" );
  QString myGeoFlagText = "false";
  if (geographicFlag())
  {
    myGeoFlagText="true";
  }
    
  myGeographicFlagElement.appendChild(theDoc.createTextNode( myGeoFlagText ));
  mySrsElement.appendChild(myGeographicFlagElement);

  myLayerNode.appendChild( mySrsElement );

  return true;
}



//
// Static helper methods below this point only please!
//


// Returns the whole proj4 string for the selected srsid
//this is a static method!
QString QgsSpatialRefSys::getProj4FromSrsId(const int theSrsId)
{

      QString myDatabaseFileName;
      QString myProjString;
      QString mySql = "select parameters from tbl_srs where srs_id = ";
      mySql += QString::number(theSrsId);

      QgsDebugMsg("QgsSpatialRefSys::getProj4FromSrsId :  mySrsId = " + QString::number(theSrsId));
      QgsDebugMsg("QgsSpatialRefSys::getProj4FromSrsId :  USER_PROJECTION_START_ID = " + QString::number(USER_PROJECTION_START_ID));
      QgsDebugMsg("QgsSpatialRefSys::getProj4FromSrsId :Selection sql : " + mySql);

      //
      // Determine if this is a user projection or a system on
      // user projection defs all have srs_id >= 100000
      //
      if (theSrsId >= USER_PROJECTION_START_ID)
      {
        myDatabaseFileName = QgsApplication::qgisUserDbFilePath();
        QFileInfo myFileInfo;
        myFileInfo.setFile(myDatabaseFileName);
        if ( !myFileInfo.exists( ) ) //its unlikely that this condition will ever be reached
        {
          QgsLogger::critical("QgsSpatialRefSys::getProj4FromSrsId :  users qgis.db not found");
          return NULL;
        }
      }
      else //must be  a system projection then
      {
        myDatabaseFileName = QgsApplication::srsDbFilePath();
      }
      QgsDebugMsg("QgsSpatialRefSys::getProj4FromSrsId db = " + myDatabaseFileName);

      sqlite3 *db;
      int rc;
      rc = openDb(myDatabaseFileName, &db);
      if(rc)
      {
        return QString();
      }
      // prepare the sql statement
      const char *pzTail;
      sqlite3_stmt *ppStmt;

      rc = sqlite3_prepare(db, mySql.toUtf8(), mySql.length(), &ppStmt, &pzTail);
      // XXX Need to free memory from the error msg if one is set

      if(rc == SQLITE_OK)
      {
        if(sqlite3_step(ppStmt) == SQLITE_ROW)
        {
          myProjString = QString::fromUtf8((char*)sqlite3_column_text(ppStmt, 0));
        }
      }
      // close the statement
      sqlite3_finalize(ppStmt);
      // close the database
      sqlite3_close(db);

      //assert(myProjString.length() > 0);
      return myProjString;
}

int QgsSpatialRefSys::openDb(QString path, sqlite3 **db)
{
  QgsDebugMsg("QgsSpatialRefSys::openDb path = " + path);
  int myResult = sqlite3_open(path.toUtf8().data(), db);

  if(myResult)
  {
    QgsLogger::critical("Can't open database: " + QString(sqlite3_errmsg(*db)));
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    // ... unfortunately it happens on Windows
    QgsMessageOutput* output = QgsMessageOutput::createMessageOutput();
    output->setTitle("Error");
    output->setMessage("Could not open SRS database " + path +
        "<br>Error(" + QString::number(myResult) + "): " +
        QString(sqlite3_errmsg(*db)), QgsMessageOutput::MessageText); 
    output->showMessage();
  }
  return myResult;
}

void QgsSpatialRefSys::setCustomSrsValidation(CUSTOM_SRS_VALIDATION f)
{
  mCustomSrsValidation = f;
}

void QgsSpatialRefSys::debugPrint()
{
  QgsDebugMsg("***SpatialRefSystem***");
  QgsDebugMsg("* Valid : " + (mIsValidFlag?QString("true"):QString("false")));
  QgsDebugMsg("* SrsId : " + QString::number(mSrsId));
  QgsDebugMsg("* Proj4 : " + proj4String() );
  QgsDebugMsg("* Desc. : " + mDescription);
}
