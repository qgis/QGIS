#include "qgsspatialrefsys.h"

#include <cassert>
#include <cmath>
#include <iostream>
#include <sqlite3.h>
#include <qsettings.h>
#include <qapplication.h>
#include <qregexp.h>
#include <qfileinfo.h>
#include <qdir.h>
#include <projects.h>

#include <qgslayerprojectionselector.h>
#include <qgsproject.h>
#include <qgis.h> //const vals declared here
//--------------------------

QgsSpatialRefSys::QgsSpatialRefSys() : mMapUnits(QGis::METERS) {}

QgsSpatialRefSys::QgsSpatialRefSys(QString theWkt) : mMapUnits(QGis::METERS) 
{
  createFromWkt(theWkt);
}

QgsSpatialRefSys::QgsSpatialRefSys(long theSrsId,
                                   QString theDescription,
                                   QString theProjectionAcronym,
                                   QString theEllipsoidAcronym,
                                   QString theProj4String,
                                   long theSRID,
                                   long theEpsg,
                                   bool theGeoFlag)
  : mMapUnits(QGis::METERS)
{}

QgsSpatialRefSys::QgsSpatialRefSys(const long theId, SRS_TYPE theType) : mMapUnits(QGis::METERS) 
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
    std::cout << "Unexpected case reached in " << __FILE__ << " : " << __LINE__ << std::endl;
  };

}

// Misc helper functions -----------------------


void QgsSpatialRefSys::validate()
{
#ifdef QGISDEBUG
  std::cout << " QgsSpatialRefSys::validate" << std::endl;
#endif
  //dont bother trying to do an initial test with gdal if 
  //the proj4String is not even populated
  if (QString::null!=mProj4String && !mProj4String.isEmpty())
  {
    //first of all use gdal to test if this is an ok srs already
    //if not we will prompt the user for and srs
    //then retest using gdal
    //if the retest fails we will then set  this srs to the GEOCS/WGS84 default


    /* Here are the possible OGR error codes :
       typedef int OGRErr;

       #define OGRERR_NONE                0
       #define OGRERR_NOT_ENOUGH_DATA     1    --> not enough data to deserialize
       #define OGRERR_NOT_ENOUGH_MEMORY   2
       #define OGRERR_UNSUPPORTED_GEOMETRY_TYPE 3
       #define OGRERR_UNSUPPORTED_OPERATION 4
       #define OGRERR_CORRUPT_DATA        5
       #define OGRERR_FAILURE             6
       #define OGRERR_UNSUPPORTED_SRS     7 */

    //get the wkt into ogr
    //this is really ugly but we need to get a QString to a char**
    char *mySourceCharArrayPointer = (char *)mProj4String.latin1();
    //create the sr and populate it from a wkt proj definition
    OGRSpatialReference myOgrSpatialRef;
    OGRErr myInputResult = myOgrSpatialRef.importFromProj4(  mySourceCharArrayPointer );

    if (myInputResult==OGRERR_NONE)
    {
      //srs is valid so nothing more to do...
      return;
    }
  }
  QSettings mySettings;
  QString myDefaultProjectionOption =
    mySettings.readEntry("/qgis/projections/defaultBehaviour");
  if (myDefaultProjectionOption=="prompt")
  {
    //@note this class is not a descendent of QWidget so we cant pass
    //it in the ctor of the layer projection selector

    QgsLayerProjectionSelector * mySelector = new QgsLayerProjectionSelector();
    long myDefaultSRS =
      QgsProject::instance()->readNumEntry("SpatialRefSys","/ProjectSRSID",GEOSRS_ID);
    mySelector->setSelectedSRSID(myDefaultSRS);
    if(mySelector->exec())
    {
      createFromSrsId(mySelector->getCurrentSRSID());
    }
    else
    {
      QApplication::restoreOverrideCursor();
    }
    delete mySelector;
  }
  else if (myDefaultProjectionOption=="useProject")
  {
    // XXX TODO: Change project to store selected CS as 'projectSRS' not 'selectedWKT'
    mProj4String = QgsProject::instance()->readEntry("SpatialRefSys","//ProjectSRSProj4String",GEOPROJ4);
  }
  else ///qgis/projections/defaultBehaviour==useDefault
  {
    // XXX TODO: Change global settings to store default CS as 'defaultSRS' not 'defaultProjectionWKT'
    mProj4String = mySettings.readEntry("/qgis/projections/defaultSRS",GEOPROJ4);
  }

  //
  // This is the second check after the user assigned SRS has been retrieved
  // If it still does not work, we will simply use the QgsSpatialRefSys const GEOPROJ4 for the job
  //

  //this is really ugly but we need to get a QString to a char**
  char *mySourceCharArrayPointer = (char *)mProj4String.latin1();
  //create the sr and populate it from a wkt proj definition
  OGRSpatialReference myOgrSpatialRef;
  OGRErr myInputResult = myOgrSpatialRef.importFromProj4( mySourceCharArrayPointer );

  if (myInputResult==OGRERR_NONE)
  {
    //srs is valid so nothing more to do...
    return;
  }
  //default to proj 4..if all else fails we will use that for this srs
  else
  {
    mProj4String=GEOPROJ4;
  }

}

void QgsSpatialRefSys::createFromSrid(long theSrid)
{
#ifdef QGISDEBUG
  std::cout << " QgsSpatialRefSys::createFromSrid" << std::endl;
#endif


  // Get the package data path and set the full path name to the sqlite3 spatial reference
  // database.
#if defined(Q_OS_MACX) || defined(WIN32)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QString myDatabaseFileName = PKGDATAPATH;
  myDatabaseFileName += "/resources/srs.db";


  sqlite3      *myDatabase;
  char         *myErrorMessage = 0;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(myDatabaseFileName.latin1(), &myDatabase);
  if(myResult)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl;
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(myResult == 0);
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

  QString mySql = "select srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,epsg,is_geo from tbl_srs where srid='" + QString::number(theSrid) + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    mSrsId = QString ((char *)sqlite3_column_text(myPreparedStatement,0)).toLong();
    mDescription = QString ((char *)sqlite3_column_text(myPreparedStatement,1));
    mProjectionAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,2));
    mEllipsoidAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,3));
    mProj4String = QString ((char *)sqlite3_column_text(myPreparedStatement,4));
    mSRID = QString ((char *)sqlite3_column_text(myPreparedStatement,5)).toLong();
    mEpsg = QString ((char *)sqlite3_column_text(myPreparedStatement,6)).toLong();
    int geo = QString ((char *)sqlite3_column_text(myPreparedStatement,7)).toInt();
    mGeoFlag = (geo == 0 ? false : true);
    setMapUnits();
    isValidFlag==true;

  }
  else
  {
#ifdef QGISDEBUG
    std::cout << " QgsSpatialRefSys::createFromSrid failed :  " << mySql << std::endl;
#endif
    isValidFlag==false;
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}

void QgsSpatialRefSys::createFromWkt(QString theWkt)
{
  if (!theWkt)
  {
    std::cout << "QgsSpatialRefSys::createFromWkt -- theWkt is uninitialised, operation failed" << std::endl;
    return;
  }
#ifdef QGISDEBUG
    std::cout << "QgsSpatialRefSys::createFromWkt(QString theWkt) using: \n" << theWkt << std::endl;
#endif
  //this is really ugly but we need to get a QString to a char**
  char *myCharArrayPointer = (char *)theWkt.latin1();

  /* Here are the possible OGR error codes :
     typedef int OGRErr;
     #define OGRERR_NONE                0
     #define OGRERR_NOT_ENOUGH_DATA     1    --> not enough data to deserialize 
     #define OGRERR_NOT_ENOUGH_MEMORY   2
     #define OGRERR_UNSUPPORTED_GEOMETRY_TYPE 3
     #define OGRERR_UNSUPPORTED_OPERATION 4
     #define OGRERR_CORRUPT_DATA        5
     #define OGRERR_FAILURE             6
     #define OGRERR_UNSUPPORTED_SRS     7 
  */

  OGRSpatialReference myOgrSpatialRef;

  OGRErr myInputResult = myOgrSpatialRef.importFromWkt( & myCharArrayPointer );
  if (myInputResult != OGRERR_NONE)
  {
    std::cout << "vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv"<< std::endl;
    std::cout << "QgsSpatialRefSys::createFromWkt(QString theWkt) " << __FILE__ << __LINE__ << std::endl;
    std::cout << "This SRS could *** NOT *** be set from the supplied WKT " << std::endl;
    std::cout << "INPUT: " << std::endl << theWkt << std::endl;
    std::cout << "^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^" << std::endl;
    return;
  }


  // always morph from esri as it doesn't hurt anything
  myOgrSpatialRef.morphFromESRI();
  // create the proj4 structs needed for transforming
  char *proj4src;
  myOgrSpatialRef.exportToProj4(&proj4src);

  //XXX TODO split out projext and ellipsoid acronym from the parameters
  mProj4String=QString(proj4src);

  char *unitName;
  if (myOgrSpatialRef.IsProjected())
  {
    myOgrSpatialRef.GetLinearUnits(&unitName);
    QString unit(unitName);

#ifdef QGISDEBUG
    std::cerr << "Projection has linear units of " << unit << '\n';
#endif

    if (unit == "Meters")
      mMapUnits = QGis::METERS;
  }
  else
  {
    myOgrSpatialRef.GetAngularUnits(&unitName);
    QString unit(unitName);
    if (unit == "degrees")
      mMapUnits = QGis::DEGREES;

#ifdef QGISDEBUG
    std::cerr << "Projection has angular units of " << unit << '\n';
#endif
  }
}

void QgsSpatialRefSys::createFromEpsg(long theEpsg)
{
#ifdef QGISDEBUG
  std::cout << " QgsSpatialRefSys::createFromEpsg" << std::endl;
#endif
  // Get the package data path and set the full path name to the sqlite3 spatial reference
  // database.
#if defined(Q_OS_MACX) || defined(WIN32)
  QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
  QString myDatabaseFileName = PKGDATAPATH;
  myDatabaseFileName += "/resources/srs.db";


  sqlite3      *myDatabase;
  char         *myErrorMessage = 0;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(myDatabaseFileName.latin1(), &myDatabase);
  if(myResult)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl;
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(myResult == 0);
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

  QString mySql = "select srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,epsg,is_geo from tbl_srs where epsg_id='" + QString::number(theEpsg) + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    mSrsId = QString ((char *)sqlite3_column_text(myPreparedStatement,0)).toLong();
    mDescription = QString ((char *)sqlite3_column_text(myPreparedStatement,1));
    mProjectionAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,2));
    mEllipsoidAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,3));
    mProj4String = QString ((char *)sqlite3_column_text(myPreparedStatement,4));
    mSRID = QString ((char *)sqlite3_column_text(myPreparedStatement,5)).toLong();
    mEpsg = QString ((char *)sqlite3_column_text(myPreparedStatement,6)).toLong();
    int geo = QString ((char *)sqlite3_column_text(myPreparedStatement,7)).toInt();
    mGeoFlag = (geo == 0 ? false : true);
    setMapUnits();
    isValidFlag==true;

  }
  else
  {
#ifdef QGISDEBUG
    std::cout << " QgsSpatialRefSys::createFromEpsg failed :  " << mySql << std::endl;
#endif
    isValidFlag==false;
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}


void QgsSpatialRefSys::createFromSrsId (long theSrsId)
{
#ifdef QGISDEBUG
  std::cout << " QgsSpatialRefSys::createFromSrsId" << std::endl;
#endif 
  QString myDatabaseFileName;
  //
  // Determine if this is a user projection or a system on
  // user projection defs all have srs_id >= 100000
  //
  if (theSrsId>= USER_PROJECTION_START_ID)
  {
    myDatabaseFileName = QDir::homeDirPath () + "/.qgis/qgis.db";
    QFileInfo myFileInfo;
    myFileInfo.setFile(myDatabaseFileName);
    if ( !myFileInfo.exists( ) )
    {
      isValidFlag==false;
      std::cout << " QgsSpatialRefSys::createFromSrid failed :  users qgis.db not found" << std::endl;
      return;
    }
  }
  else //must be  a system projection then
  {
    // Get the package data path and set the full path name to the sqlite3 spatial reference
    // database.
#if defined(Q_OS_MACX) || defined(WIN32)
    QString PKGDATAPATH = qApp->applicationDirPath() + "/share/qgis";
#endif
    myDatabaseFileName = PKGDATAPATH;
    myDatabaseFileName += "/resources/srs.db";
  }


  sqlite3      *myDatabase;
  char         *myErrorMessage = 0;
  const char   *myTail;
  sqlite3_stmt *myPreparedStatement;
  int           myResult;
  //check the db is available
  myResult = sqlite3_open(myDatabaseFileName.latin1(), &myDatabase);
  if(myResult)
  {
    std::cout <<  "Can't open database: " <<  sqlite3_errmsg(myDatabase) << std::endl;
    // XXX This will likely never happen since on open, sqlite creates the
    //     database if it does not exist.
    assert(myResult == 0);
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

  QString mySql = "select srs_id,description,projection_acronym,ellipsoid_acronym,parameters,srid,epsg,is_geo from tbl_srs where srs_id='" + QString::number(theSrsId) + "'";
  myResult = sqlite3_prepare(myDatabase, (const char *)mySql, mySql.length(), &myPreparedStatement, &myTail);
  // XXX Need to free memory from the error msg if one is set
  if(myResult == SQLITE_OK)
  {
    sqlite3_step(myPreparedStatement) == SQLITE_ROW;
    mSrsId = QString ((char *)sqlite3_column_text(myPreparedStatement,0)).toLong();
    mDescription = QString ((char *)sqlite3_column_text(myPreparedStatement,1));
    mProjectionAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,2));
    mEllipsoidAcronym = QString ((char *)sqlite3_column_text(myPreparedStatement,3));
    mProj4String = QString ((char *)sqlite3_column_text(myPreparedStatement,4));
    mSRID = QString ((char *)sqlite3_column_text(myPreparedStatement,5)).toLong();
    mEpsg = QString ((char *)sqlite3_column_text(myPreparedStatement,6)).toLong();
    int geo = QString ((char *)sqlite3_column_text(myPreparedStatement,7)).toInt();
    mGeoFlag = (geo == 0 ? false : true);
    setMapUnits();
    isValidFlag==true;
  }
  else
  {
#ifdef QGISDEBUG
    std::cout << " QgsSpatialRefSys::createFromSrsId failed :  " << mySql << std::endl;
#endif
    isValidFlag==false;
  }
  sqlite3_finalize(myPreparedStatement);
  sqlite3_close(myDatabase);
}





bool QgsSpatialRefSys::isValid() const
{
  //this is really ugly but we need to get a QString to a char**
  char *mySourceCharArrayPointer = (char *)mProj4String.latin1();
  //create the sr and populate it from a wkt proj definition
  OGRSpatialReference myOgrSpatialRef;
  OGRErr myResult = myOgrSpatialRef.importFromProj4( mySourceCharArrayPointer );
  if (myResult==OGRERR_NONE)
  {
    //srs is valid so nothing more to do...
    return true;
  }
  else
  {
    return false;
  }
}

void QgsSpatialRefSys::createFromProj4 (const QString theProj4String)
{
  //
  // Example:
  // +proj=tmerc +lat_0=0 +lon_0=-62 +k=0.999500 +x_0=400000 +y_0=0 
  // +ellps=clrk80 +towgs84=-255,-15,71,0,0,0,0 +units=m +no_defs
  // 

  QRegExp myProjRegExp( "\\+proj=[a-zA-Z]* " );    
  int myStart= 0;
  int myLength=0;
  myStart = myProjRegExp.search(theProj4String, myStart);
  if (myStart==-1)
  {
    std::cout << "QgsSpatialRefSys::createFromProj4 error proj string supplied has no +proj argument" << std::endl;
  }
  else
  {
    myLength = myProjRegExp.matchedLength();
  }
  mProjectionAcronym = theProj4String.mid(myStart+PROJ_PREFIX_LEN,myLength);
  
  QRegExp myEllipseRegExp( "\\+ellps=[a-zA-Z]* " );    
  myStart= 0;
  myLength=0;
  myStart = myEllipseRegExp.search(theProj4String, myStart);
  if (myStart==-1)
  {
    std::cout << "QgsSpatialRefSys::createFromProj4 error proj string supplied has no +ellps argument" << std::endl;
  }
  else
  {
    myLength = myEllipseRegExp.matchedLength();
  }
  mEllipsoidAcronym = theProj4String.mid(myStart+ELLPS_PREFIX_LEN,myLength);
  setMapUnits();
}
// Accessors -----------------------------------

/*! Get the SrsId
 *  @return  long theSrsId The internal sqlite3 srs.db primary key for this srs 
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
  return mDescription;
}
/*! Get the Projection Acronym
 * @return  QString theProjectionAcronym The official proj4 acronym for the projection family
 */
QString QgsSpatialRefSys::projectionAcronym() const
{
  return mProjectionAcronym;
}
/*! Get the Ellipsoid Acronym
 * @return  QString theEllipsoidAcronym The official proj4 acronym for the ellipoid
 */
QString QgsSpatialRefSys::ellipsoidAcronym () const
{
  return mEllipsoidAcronym;
}
/* Get the Proj Proj4String. 
 * @return  QString theProj4String Proj4 format specifies that define this srs.
 */
QString QgsSpatialRefSys::proj4String() const
{
  return mProj4String;
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


/*! Set the SrsId
 *  @param  long theSrsId The internal sqlite3 srs.db primary key for this srs 
 */
void QgsSpatialRefSys::setSrid(long theSrid)
{
  mSRID=theSrid;
}
/*! Set the Description
 * @param  QString the Description A textual description of the srs.
 */
void QgsSpatialRefSys::setDescription (QString theDescription)
{
  mDescription = theDescription;
}

void QgsSpatialRefSys::setProj4String (QString theProj4String)
{
  mProj4String = theProj4String;
}
/*! Set this Geographic? flag
 * @param  bool theGeoFlag Whether this is a geographic or projected coordinate system
 */
void QgsSpatialRefSys::setGeographicFlag (bool theGeoFlag)
{
  mGeoFlag=theGeoFlag;
}

/*! Set the postgis srid for this srs
 * @param  long theSRID the Postgis spatial_ref_sys identifier for this srs (defaults to 0)
 */
void QgsSpatialRefSys::setPostgisSrid (long theSrid)
{
  mSRID=theSrid;
}
/*! Set the EPSG identifier for this srs
 * @param  long theEpsg the ESPG identifier for this srs (defaults to 0)
 */
void QgsSpatialRefSys::setEpsg (long theEpsg)
{
  mEpsg=theEpsg;
}
/*! Work out the projection units and set the appropriate local variable
 *
 */
void QgsSpatialRefSys::setMapUnits()
{
  // Need to to extract the units from the mProj4String variable. If
  // the coordinate system is geographic it seems that the answer is
  // always degrees, and if it is not geographic, we need to search
  // mProj4String for a +units= section and interpret that.

#ifdef QGISEBUG
  std::cerr << "Proj4 string is: " << mProj4String << '\n';
  if (geographicFlag())
    std::cerr << " and is geographic\n";
  else
    std::cerr << " and is projected\n";
#endif

  // This code will list all of the linear units that proj4 knows
  // about. A future enhancement will be for qgis to support all of
  // these, and allow conversion between them. This code is here as a
  // reminder to support these other units.
  /*
  for (int i = 0; pj_units[i].name != (char*)0; ++i)
    std::cerr << pj_units[i].id << ", "
              << pj_units[i].to_meter << ", "
              << pj_units[i].name << '\n';
  */

  // Default units.
  mMapUnits = QGis::UNKNOWN;

  if (geographicFlag())
    mMapUnits = QGis::DEGREES;
  else
  {
    if (mProj4String.isEmpty())
      qWarning("No proj4 projection string. Can't determine units.");
    else
    {
      // Look for a units statement
      static const int fieldLength = 7; // length of '+units='
      QRegExp unitPattern("\\+units=\\S+");
      int pos = unitPattern.search(mProj4String);
      if (pos != -1)
      {
        QString unit = mProj4String.mid(pos + fieldLength, 
                            unitPattern.matchedLength()-fieldLength);
        if (unit == "m")
          mMapUnits = QGis::METERS;
        else if (unit == "us-ft" || unit == "ft" || unit == "ind-ft")
          mMapUnits = QGis::FEET;
        else
          qWarning("Unsupported projection unit: " + unit);
      }
      else
      {
        // There may be a to_meter statement from which the units
        // could be deduced.
        static const int fieldLength = 10; // length of '+to_meter='
        QRegExp meterPattern("\\+to_meter=\\S+");
        int pos = meterPattern.search(mProj4String);
        if (pos != -1)
        {
          static const double mPerFeet = 0.3048;
          static const double smallValue = 1e-5;

          QString tmp = mProj4String.mid(pos + fieldLength, 
                               meterPattern.matchedLength()-fieldLength);
          double conversion = tmp.toDouble();
          if (std::abs(conversion - 1.0) < smallValue)
            mMapUnits = QGis::METERS;
          else if (std::abs(conversion - mPerFeet) < smallValue)
            mMapUnits = QGis::FEET;
          else
            qWarning("Unsupported to_meter conversion factor(" + 
                     tmp + ").");
        }
        else
          qWarning("Unable to determine units for the output projection.");
      }
    }
  }
}

bool QgsSpatialRefSys::operator==(const QgsSpatialRefSys &theSrs)
{
   qWarning("QgsSpatialRefSys::operator== called ");
   //placeholder to be replaced with ogr tests
   return (proj4String() == theSrs.proj4String());
}
