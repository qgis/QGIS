/***************************************************************************
  testqgscoordinatereferencesystem.cpp
  --------------------------------------
Date                 : Sun Sep 16 12:22:49 AKDT 2007
Copyright            : (C) 2007 by Gary E. Sherman
Email                : sherman at mrcc dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QPixmap>

#include "qgsapplication.h"
#include "qgslogger.h"

//header for class being tested
#include "qgscoordinatereferencesystem.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#if PROJ_VERSION_MAJOR > 4
#include <proj.h>
#else
#include <proj_api.h>
#endif
#include <gdal.h>
#include <cpl_conv.h>
#include <QtTest/QSignalSpy>

class TestQgsCoordinateReferenceSystem: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void wktCtor();
    void idCtor();
    void copyCtor();
    void assignmentCtor();
    void createFromId();
    void fromEpsgId();
    void createFromOgcWmsCrs();
    void fromOgcWmsCrs();
    void ogcWmsCrsCache();
    void createFromSrid();
    void sridCache();
    void createFromWkt();
    void fromWkt();
    void wktCache();
    void createFromESRIWkt();
    void createFromSrId();
    void fromSrsId();
    void srsIdCache();
    void createFromProj4();
    void fromProj4();
    void proj4Cache();
    void fromString();
    void fromStringCache();
    void isValid();
    void validate();
    void equality();
    void noEquality();
    void equalityInvalid();
    void readWriteXml();
    void setCustomSrsValidation();
    void customSrsValidation();
    void postgisSrid();
    void ellipsoidAcronym();
    void toWkt();
    void toProj4();
    void isGeographic();
    void mapUnits();
    void setValidationHint();
    void hasAxisInverted();
    void createFromProj4Invalid();
    void validSrsIds();
    void asVariant();
    void bounds();
    void saveAsUserCrs();
    void projectWithCustomCrs();
    void projectEPSG25833();
    void geoCcsDescription();
    void geographicCrsAuthId();
    void noProj();

  private:
    void debugPrint( QgsCoordinateReferenceSystem &crs );
    // these used by createFromESRIWkt()
    QStringList myWktStrings;
    QList<int> myGdalVersionOK;
    QStringList myFiles;
    QStringList myProj4Strings;
    QStringList myTOWGS84Strings;
    QStringList myAuthIdStrings;
    QString mTempFolder;
    QString testESRIWkt( int i, QgsCoordinateReferenceSystem &crs );

    static bool sValidateCalled;

    static void testValidationCrs( QgsCoordinateReferenceSystem &crs )
    {
      sValidateCalled = true;

      crs.createFromString( QStringLiteral( "EPSG:3111" ) );
    }

    static void testNoActionValidationCrs( QgsCoordinateReferenceSystem & )
    {
      sValidateCalled = true;
    }
};

bool TestQgsCoordinateReferenceSystem::sValidateCalled = false;

void TestQgsCoordinateReferenceSystem::initTestCase()
{
  // we start from a clean profile - we don't want to mess with user custom srses
  // create temporary folder
  QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
  mTempFolder = QDir::tempPath() + '/' + subPath;
  if ( !QDir( mTempFolder ).exists() )
    QDir().mkpath( mTempFolder );

  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init( mTempFolder );
  QgsApplication::createDatabase();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QgsDebugMsg( QStringLiteral( "Custom srs database: %1" ).arg( QgsApplication::qgisUserDatabaseFilePath() ) );

  qDebug() << "geoProj4() constant:      " << geoProj4();
  qDebug() << "GDAL version (build):   " << GDAL_RELEASE_NAME;
  qDebug() << "GDAL version (runtime): " << GDALVersionInfo( "RELEASE_NAME" );
#if PROJ_VERSION_MAJOR > 4
  PJ_INFO info = proj_info();
  qDebug() << "PROJ version:           " << info.release;
#else
  qDebug() << "PROJ version:           " << PJ_VERSION;
#endif

  // if user set GDAL_FIX_ESRI_WKT print a warning
  if ( strcmp( CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" ), "" ) != 0 )
  {
    qDebug() << "Warning! GDAL_FIX_ESRI_WKT =" << CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" )
             << "this might generate errors!";
  }

}

void TestQgsCoordinateReferenceSystem::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsCoordinateReferenceSystem::wktCtor()
{
  QString myWkt = geoWkt();
  QgsCoordinateReferenceSystem myCrs( myWkt );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
}
void TestQgsCoordinateReferenceSystem::idCtor()
{
  QgsCoordinateReferenceSystem myCrs( GEOSRID,
                                      QgsCoordinateReferenceSystem::EpsgCrsId );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
}
void TestQgsCoordinateReferenceSystem::copyCtor()
{
  QgsCoordinateReferenceSystem myCrs( GEOSRID,
                                      QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem myCrs2( myCrs );
  debugPrint( myCrs2 );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), QString( "EPSG:4326" ) );

  //test implicit sharing detachment - modify original
  myCrs.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:3111" ) );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), QString( "EPSG:4326" ) );
}

void TestQgsCoordinateReferenceSystem::assignmentCtor()
{
  QgsCoordinateReferenceSystem myCrs( GEOSRID,
                                      QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem myCrs2 = myCrs;
  debugPrint( myCrs2 );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), QString( "EPSG:4326" ) );

  //test implicit sharing detachment - modify original
  myCrs.createFromId( 3111, QgsCoordinateReferenceSystem::EpsgCrsId );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:3111" ) );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), QString( "EPSG:4326" ) );
}

void TestQgsCoordinateReferenceSystem::createFromId()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromId( GEO_EPSG_CRS_ID,
                      QgsCoordinateReferenceSystem::EpsgCrsId );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
}

void TestQgsCoordinateReferenceSystem::fromEpsgId()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromEpsgId( GEO_EPSG_CRS_ID );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
  myCrs = QgsCoordinateReferenceSystem::fromEpsgId( -999 );
  QVERIFY( !myCrs.isValid() );

  // using an ESRI: code. This worked in pre-proj 6 builds, so we need to keep compatibility
  myCrs = QgsCoordinateReferenceSystem::fromEpsgId( 54030 );
  QVERIFY( myCrs.isValid() );
#if PROJ_VERSION_MAJOR>=6
  QCOMPARE( myCrs.authid(), QStringLiteral( "ESRI:54030" ) );
#endif
}
void TestQgsCoordinateReferenceSystem::createFromOgcWmsCrs()
{
  QgsCoordinateReferenceSystem myCrs;
  //check fails if passed an empty string
  QVERIFY( !myCrs.createFromOgcWmsCrs( QString() ) );

  myCrs.createFromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:4326" ) );

  myCrs.createFromOgcWmsCrs( QStringLiteral( "http://www.opengis.net/def/crs/EPSG/0/4326" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:4326" ) );

  myCrs.createFromOgcWmsCrs( QStringLiteral( "urn:ogc:def:crs:EPSG::4326" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:4326" ) );

  myCrs.createFromOgcWmsCrs( QStringLiteral( "i am not a CRS" ) );
  QVERIFY( !myCrs.isValid() );

  myCrs.createFromOgcWmsCrs( QStringLiteral( "http://www.opengis.net/def/crs/OGC/1.3/CRS84" ) );
  QVERIFY( myCrs.isValid() );
  QVERIFY( !myCrs.hasAxisInverted() );
#if PROJ_VERSION_MAJOR>=6
  QCOMPARE( myCrs.authid(), QString( "OGC:CRS84" ) );
#endif
}

void TestQgsCoordinateReferenceSystem::fromOgcWmsCrs()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QString( "EPSG:4326" ) );
  myCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "not a crs" ) );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::ogcWmsCrsCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QString( "EPSG:4326" ) );
  QVERIFY( QgsCoordinateReferenceSystem::ogcCache().contains( "EPSG:4326" ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), QString( "EPSG:4326" ) );

  // invalid
  QgsCoordinateReferenceSystem crs3 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "not a CRS" ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::ogcCache().contains( "not a CRS" ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4 = QgsCoordinateReferenceSystem::fromOgcWmsCrs( QStringLiteral( "not a CRS" ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::ogcCache().contains( "EPSG:4326" ) );
}

void TestQgsCoordinateReferenceSystem::createFromSrid()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
}

void TestQgsCoordinateReferenceSystem::sridCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 3112 );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QString( "EPSG:3112" ) );
  QVERIFY( QgsCoordinateReferenceSystem::srIdCache().contains( 3112 ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromSrid( 3112 );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), QString( "EPSG:3112" ) );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  crs3.createFromSrid( -3141 );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::srIdCache().contains( -3141 ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  crs4.createFromSrid( -3141 );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::srIdCache().contains( 3112 ) );
}

void TestQgsCoordinateReferenceSystem::createFromWkt()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromWkt( geoWkt() );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
}

void TestQgsCoordinateReferenceSystem::fromWkt()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromWkt( geoWkt() );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
  myCrs = QgsCoordinateReferenceSystem::fromWkt( QStringLiteral( "not wkt" ) );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::wktCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromWkt( geoWkt() );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), GEOCRS_ID );
  QVERIFY( QgsCoordinateReferenceSystem::wktCache().contains( geoWkt() ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromWkt( geoWkt() );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), GEOCRS_ID );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  crs3.createFromWkt( QStringLiteral( "bad wkt" ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::wktCache().contains( "bad wkt" ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  crs4.createFromWkt( QStringLiteral( "bad wkt" ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::wktCache().contains( geoWkt() ) );
}

QString TestQgsCoordinateReferenceSystem::testESRIWkt( int i, QgsCoordinateReferenceSystem &myCrs )
{
  debugPrint( myCrs );

  if ( ! myCrs.isValid() )
    return QStringLiteral( "test %1 crs is invalid" );
#if 0
  if ( myCrs.toProj4() != myProj4Strings[i] )
    return QString( "test %1 PROJ = [ %2 ] expecting [ %3 ]"
                  ).arg( i ).arg( myCrs.toProj4() ).arg( myProj4Strings[i] );
#endif
  if ( myCrs.toProj4().indexOf( myTOWGS84Strings[i] ) == -1 )
    return QStringLiteral( "test %1 [%2] not found, PROJ = [%3] expecting [%4]"
                         ).arg( i ).arg( myTOWGS84Strings[i], myCrs.toProj4(), myProj4Strings[i] );
  if ( myCrs.authid() != myAuthIdStrings[i] )
    return QStringLiteral( "test %1 AUTHID = [%2] expecting [%3]"
                         ).arg( i ).arg( myCrs.authid(), myAuthIdStrings[i] );

  return QString();
}
void TestQgsCoordinateReferenceSystem::createFromESRIWkt()
{
  // disabled for proj >= 6 -- all this belongs in proj, not in QGIS
#if PROJ_VERSION_MAJOR<6
  QString msg;
  QgsCoordinateReferenceSystem myCrs;
  const char *configOld = CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" );

  // for more tests add definitions here

  // this example file taken from bug #5598
  myWktStrings << QStringLiteral( "PROJCS[\"Indian_1960_UTM_Zone_48N\",GEOGCS[\"GCS_Indian_1960\",DATUM[\"D_Indian_1960\",SPHEROID[\"Everest_Adjustment_1937\",6377276.345,300.8017]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",105.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]" );
  myGdalVersionOK << 1800;
  myFiles << QStringLiteral( "bug5598.shp" );
  myProj4Strings << QStringLiteral( "+proj=utm +zone=48 +a=6377276.345 +b=6356075.41314024 +towgs84=198,881,317,0,0,0,0 +units=m +no_defs" );
  myTOWGS84Strings << QStringLiteral( "+towgs84=198,881,317,0,0,0,0" );
  myAuthIdStrings << QStringLiteral( "EPSG:3148" );

  // this example file taken from bug #5598 - geographic CRS only, supported since gdal 1.9
  myWktStrings << QStringLiteral( "GEOGCS[\"GCS_Indian_1960\",DATUM[\"D_Indian_1960\",SPHEROID[\"Everest_Adjustment_1937\",6377276.345,300.8017]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]" );
  myFiles << QString();
  myGdalVersionOK << 1900;
  myProj4Strings << QStringLiteral( "+proj=longlat +a=6377276.345 +b=6356075.41314024 +towgs84=198,881,317,0,0,0,0 +no_defs" );
  myTOWGS84Strings << QStringLiteral( "+towgs84=198,881,317,0,0,0,0" );
  myAuthIdStrings << QStringLiteral( "EPSG:4131" );

  // SAD69 geographic CRS, supported since gdal 1.9
  myWktStrings << QStringLiteral( "GEOGCS[\"GCS_South_American_1969\",DATUM[\"D_South_American_1969\",SPHEROID[\"GRS_1967_Truncated\",6378160.0,298.25]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]" );
  myFiles << QString();
  myGdalVersionOK << 1900;

  //proj definition for EPSG:4618 was updated in GDAL 2.0 - see https://github.com/OSGeo/proj.4/issues/241
  myProj4Strings << QStringLiteral( "+proj=longlat +ellps=aust_SA +towgs84=-66.87,4.37,-38.52,0,0,0,0 +no_defs" );
  myTOWGS84Strings << QStringLiteral( "+towgs84=-66.87,4.37,-38.52,0,0,0,0" );
  myAuthIdStrings << QStringLiteral( "EPSG:4618" );

  // do test with WKT definitions
  for ( int i = 0; i < myWktStrings.size() ; i++ )
  {
    QgsDebugMsg( QStringLiteral( "i=%1 wkt=%2" ).arg( i ).arg( myWktStrings[i] ) );
    // use createFromUserInput and add the ESRI:: prefix to force morphFromESRI
    CPLSetConfigOption( "GDAL_FIX_ESRI_WKT", configOld );
    myCrs.createFromUserInput( "ESRI::" + myWktStrings[i] );
    msg = testESRIWkt( i, myCrs );
    if ( GDAL_VERSION_NUM < myGdalVersionOK.at( i ) )
    {
      QEXPECT_FAIL( "", QString( "expected failure with GDAL %1 : %2"
                               ).arg( GDAL_VERSION_NUM ).arg( msg ).toLocal8Bit().constData(),
                    Continue );
    }

    if ( !msg.isEmpty() )
      QVERIFY2( false, msg.toLocal8Bit().constData() );

    // do test with shapefiles
    CPLSetConfigOption( "GDAL_FIX_ESRI_WKT", configOld );
    if ( !myFiles[i].isEmpty() )
    {
      // use ogr to open file, make sure CRS is OK
      // this probably could be in another test, but leaving it here since it deals with CRS
      QString fileStr = QStringLiteral( TEST_DATA_DIR ) + '/' + myFiles[i];
      QgsDebugMsg( QStringLiteral( "i=%1 file=%2" ).arg( i ).arg( fileStr ) );

      QgsVectorLayer *myLayer = new QgsVectorLayer( fileStr, QString(), QStringLiteral( "ogr" ) );
      if ( !myLayer || ! myLayer->isValid() )
      {
        qWarning() << QStringLiteral( "test %1 did not get valid vector layer from %2" ).arg( i ).arg( fileStr );
        QVERIFY2( false, "no valid vector layer" );
      }
      else
      {
        myCrs = myLayer->crs();
        msg = testESRIWkt( i, myCrs );
        if ( GDAL_VERSION_NUM < myGdalVersionOK.at( i ) )
        {
          QEXPECT_FAIL( "", QString( "expected failure with GDAL %1 : %2 using layer %3"
                                   ).arg( GDAL_VERSION_NUM ).arg( msg, fileStr ).toLocal8Bit().constData(),
                        Continue );
        }
        if ( !msg.isEmpty() )
          QVERIFY2( false, msg.toLocal8Bit().constData() );
      }
      if ( myLayer )
        delete myLayer;

    }

  }
#endif
  //  QVERIFY( bOK );
}
void TestQgsCoordinateReferenceSystem::createFromSrId()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( myCrs.createFromSrid( GEOSRID ) );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
}

void TestQgsCoordinateReferenceSystem::fromSrsId()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromSrsId( GEOCRS_ID );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
  myCrs = QgsCoordinateReferenceSystem::fromSrsId( -9999 );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::srsIdCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrsId( GEOCRS_ID );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), GEOCRS_ID );
  QVERIFY( QgsCoordinateReferenceSystem::srsIdCache().contains( GEOCRS_ID ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromSrsId( GEOCRS_ID );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), GEOCRS_ID );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  crs3.createFromSrsId( -5141 );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::srsIdCache().contains( -5141 ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  crs4.createFromSrsId( -5141 );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::srsIdCache().contains( GEOCRS_ID ) );
}


void TestQgsCoordinateReferenceSystem::createFromProj4()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( !myCrs.createFromProj4( QString() ) );
  QVERIFY( !myCrs.isValid() );

  QVERIFY( myCrs.createFromProj4( geoProj4() ) );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
}

void TestQgsCoordinateReferenceSystem::fromProj4()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromProj4( geoProj4() );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), GEOCRS_ID );
  myCrs = QgsCoordinateReferenceSystem::fromProj4( QString() );
  QVERIFY( !myCrs.isValid() );


  myCrs = QgsCoordinateReferenceSystem::fromProj4( "+proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs" );
  QCOMPARE( myCrs.authid(), QStringLiteral( "EPSG:20936" ) );
}

void TestQgsCoordinateReferenceSystem::proj4Cache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromProj4( geoProj4() );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), GEOCRS_ID );
  QVERIFY( QgsCoordinateReferenceSystem::proj4Cache().contains( geoProj4() ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromProj4( geoProj4() );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), GEOCRS_ID );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  crs3.createFromProj4( QStringLiteral( "bad proj4" ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::proj4Cache().contains( "bad proj4" ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  crs4.createFromProj4( QStringLiteral( "bad proj4" ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::proj4Cache().contains( geoProj4() ) );
}

void TestQgsCoordinateReferenceSystem::fromString()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( QStringLiteral( "woohooo" ) );
  QVERIFY( !crs.isValid() );
  crs.createFromString( QStringLiteral( "EPSG:3111" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:3111" ) );
  crs.createFromString( QStringLiteral( "epsg:3111" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:3111" ) );

#if PROJ_VERSION_MAJOR>=6
  crs.createFromString( QStringLiteral( "esri:102499" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "ESRI:102499" ) );
  crs.createFromString( QStringLiteral( "OSGEO:41001" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "OSGEO:41001" ) );
  crs.createFromString( QStringLiteral( "IGNF:LAMB1" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "IGNF:LAMB1" ) );
  crs.createFromString( QStringLiteral( "IAU2000:69918" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "IAU2000:69918" ) );
#endif
}

void TestQgsCoordinateReferenceSystem::fromStringCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( QStringLiteral( "EPSG:3113" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QString( "EPSG:3113" ) );
  QVERIFY( QgsCoordinateReferenceSystem::stringCache().contains( "EPSG:3113" ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromString( QStringLiteral( "EPSG:3113" ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), QString( "EPSG:3113" ) );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  crs3.createFromString( QStringLiteral( "bad string" ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::stringCache().contains( "bad string" ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  crs4.createFromString( QStringLiteral( "bad string" ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::stringCache().contains( "EPSG:3113" ) );
}

void TestQgsCoordinateReferenceSystem::isValid()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QVERIFY( myCrs.isValid() );
  debugPrint( myCrs );
}

void TestQgsCoordinateReferenceSystem::validate()
{
  // no validator set
  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );
  QgsCoordinateReferenceSystem myCrs( QStringLiteral( "EPSG:28356" ) );
  myCrs.validate();
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QStringLiteral( "EPSG:28356" ) );
  myCrs = QgsCoordinateReferenceSystem();
  myCrs.validate();
  // no change, no custom function
  QVERIFY( !myCrs.isValid() );

  QgsCoordinateReferenceSystem::setCustomCrsValidation( &TestQgsCoordinateReferenceSystem::testValidationCrs );
  myCrs.validate();
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), QStringLiteral( "EPSG:3111" ) );
  QVERIFY( sValidateCalled );
  sValidateCalled = false;

  myCrs = QgsCoordinateReferenceSystem();
  QgsCoordinateReferenceSystem::setCustomCrsValidation( &TestQgsCoordinateReferenceSystem::testNoActionValidationCrs );
  myCrs.validate();
  QVERIFY( !myCrs.isValid() );
  QVERIFY( sValidateCalled );

  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );
}

void TestQgsCoordinateReferenceSystem::equality()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QgsCoordinateReferenceSystem myCrs2;
  myCrs2.createFromSrsId( GEOCRS_ID );
  debugPrint( myCrs );
  QVERIFY( myCrs == myCrs2 );
}
void TestQgsCoordinateReferenceSystem::noEquality()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QgsCoordinateReferenceSystem myCrs2;
  myCrs2.createFromSrsId( 4327 );
  debugPrint( myCrs );
  QVERIFY( myCrs != myCrs2 );
}
void TestQgsCoordinateReferenceSystem::equalityInvalid()
{
  QgsCoordinateReferenceSystem invalidCrs1;
  QgsCoordinateReferenceSystem invalidCrs2;
  QVERIFY( invalidCrs1 == invalidCrs2 );
}
void TestQgsCoordinateReferenceSystem::readWriteXml()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QVERIFY( myCrs.isValid() );
  QDomDocument document( "test" );
  QDomElement node = document.createElement( QStringLiteral( "crs" ) );
  document.appendChild( node );

  // start with invalid node
  QgsCoordinateReferenceSystem badCrs;
  QVERIFY( !badCrs.readXml( node ) );
  QVERIFY( !badCrs.isValid() );
  QDomElement badSrsElement  = document.createElement( QStringLiteral( "spatialrefsys" ) );
  QDomElement badNode = document.createElement( QStringLiteral( "crs" ) );
  document.appendChild( badNode );
  badNode.appendChild( badSrsElement );
  // should return true, because it's ok to write/read invalid crs to xml
  QVERIFY( badCrs.readXml( badNode ) );
  QVERIFY( !badCrs.isValid() );

  QVERIFY( myCrs.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs2;
  QVERIFY( myCrs2.readXml( node ) );
  QVERIFY( myCrs == myCrs2 );

  // Empty XML made from writeXml operation
  QgsCoordinateReferenceSystem myCrs3;
  QDomDocument document2( "test" );
  QDomElement node2 = document2.createElement( QStringLiteral( "crs" ) );
  document2.appendChild( node2 );
  QVERIFY( ! myCrs3.isValid() );
  QVERIFY( myCrs3.writeXml( node2, document2 ) );
  QgsCoordinateReferenceSystem myCrs4;
  QVERIFY( myCrs4.readXml( node2 ) );
  QVERIFY( ! myCrs4.isValid() );
  QVERIFY( myCrs3 == myCrs4 );

  // Empty XML node
  QDomDocument document3( "test" );
  QDomElement node3 = document3.createElement( QStringLiteral( "crs" ) );
  document3.appendChild( node3 );
  QgsCoordinateReferenceSystem myCrs5;
  QVERIFY( ! myCrs5.readXml( node3 ) );
  QVERIFY( myCrs5 == QgsCoordinateReferenceSystem() );
}
void TestQgsCoordinateReferenceSystem::setCustomSrsValidation()
{
  //QgsCoordinateReferenceSystem myCrs;
  //static void setCustomSrsValidation( CUSTOM_CRS_VALIDATION f );
  //QVERIFY( myCrs.isValid() );
}
void TestQgsCoordinateReferenceSystem::customSrsValidation()
{

  /**
   * \todo implement this test
  "QgsCoordinateReferenceSystem myCrs;
  static CUSTOM_CRS_VALIDATION customSrsValidation();
  QVERIFY( myCrs.isValid() );
  */
}
void TestQgsCoordinateReferenceSystem::postgisSrid()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QVERIFY( myCrs.postgisSrid() == GEOSRID );
  debugPrint( myCrs );
}
void TestQgsCoordinateReferenceSystem::ellipsoidAcronym()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QString myAcronym = myCrs.ellipsoidAcronym();
  debugPrint( myCrs );
  QCOMPARE( myAcronym, QStringLiteral( "WGS84" ) );
}
void TestQgsCoordinateReferenceSystem::toWkt()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QString myWkt = myCrs.toWkt();
  debugPrint( myCrs );
  //Note: this is not the same as geoWkt() as OGR strips off the TOWGS clause...
  QString myStrippedWkt( "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID"
                         "[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
                         "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY"
                         "[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY"
                         "[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]" );
  qDebug() << "wkt:      " << myWkt;
  qDebug() << "stripped: " << myStrippedWkt;
  QVERIFY( myWkt == myStrippedWkt );
}
void TestQgsCoordinateReferenceSystem::toProj4()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  debugPrint( myCrs );
  //first proj string produced by gdal 1.8-1.9
  //second by gdal 1.7
  QCOMPARE( myCrs.toProj4(), geoProj4() );
}
void TestQgsCoordinateReferenceSystem::isGeographic()
{
  QgsCoordinateReferenceSystem geographic;
  geographic.createFromSrid( GEOSRID );
  QVERIFY( geographic.isGeographic() );
  debugPrint( geographic );

  QgsCoordinateReferenceSystem nonGeographic;
  nonGeographic.createFromId( 3857, QgsCoordinateReferenceSystem::EpsgCrsId );
  QVERIFY( !nonGeographic.isGeographic() );
}
void TestQgsCoordinateReferenceSystem::mapUnits()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromString( QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( myCrs.mapUnits(), QgsUnitTypes::DistanceDegrees );
  debugPrint( myCrs );
  myCrs.createFromString( QStringLiteral( "EPSG:28355" ) );
  QCOMPARE( myCrs.mapUnits(), QgsUnitTypes::DistanceMeters );
  debugPrint( myCrs );
  myCrs.createFromString( QStringLiteral( "EPSG:26812" ) );
  QCOMPARE( myCrs.mapUnits(), QgsUnitTypes::DistanceFeet );
  myCrs.createFromString( QStringLiteral( "EPSG:4619" ) );
  QCOMPARE( myCrs.mapUnits(), QgsUnitTypes::DistanceDegrees );

  // an invalid crs should return unknown unit
  QCOMPARE( QgsCoordinateReferenceSystem().mapUnits(), QgsUnitTypes::DistanceUnknownUnit );
}
void TestQgsCoordinateReferenceSystem::setValidationHint()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.setValidationHint( QStringLiteral( "<head>" ) );
  QVERIFY( myCrs.validationHint() == "<head>" );
  debugPrint( myCrs );
}

void TestQgsCoordinateReferenceSystem::hasAxisInverted()
{
  // this is used by WMS 1.3 to determine whether to switch axes or not

  QgsCoordinateReferenceSystem crs;
  crs.createFromOgcWmsCrs( QStringLiteral( "EPSG:4326" ) ); // WGS 84 with inverted axes
  QVERIFY( crs.hasAxisInverted() );

  crs.createFromOgcWmsCrs( QStringLiteral( "CRS:84" ) ); // WGS 84 without inverted axes
  QVERIFY( !crs.hasAxisInverted() );

  crs.createFromOgcWmsCrs( QStringLiteral( "EPSG:32633" ) ); // "WGS 84 / UTM zone 33N" - projected CRS without invertex axes
  QVERIFY( !crs.hasAxisInverted() );
}


void TestQgsCoordinateReferenceSystem::debugPrint(
  QgsCoordinateReferenceSystem &crs )
{
  QgsDebugMsg( QStringLiteral( "***SpatialRefSystem***" ) );
  QgsDebugMsg( "* Valid : " + ( crs.isValid() ? QString( "true" ) :
                                QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( crs.srsid() ) );
  QgsDebugMsg( "* EPSG ID : " + crs.authid() );
  QgsDebugMsg( "* PGIS ID : " + QString::number( crs.postgisSrid() ) );
  QgsDebugMsg( "* Proj4 : " + crs.toProj4() );
  QgsDebugMsg( "* WKT   : " + crs.toWkt() );
  QgsDebugMsg( "* Desc. : " + crs.description() );
  if ( crs.mapUnits() == QgsUnitTypes::DistanceMeters )
  {
    QgsDebugMsg( QStringLiteral( "* Units : meters" ) );
  }
  else if ( crs.mapUnits() == QgsUnitTypes::DistanceFeet )
  {
    QgsDebugMsg( QStringLiteral( "* Units : feet" ) );
  }
  else if ( crs.mapUnits() == QgsUnitTypes::DistanceDegrees )
  {
    QgsDebugMsg( QStringLiteral( "* Units : degrees" ) );
  }
}

void TestQgsCoordinateReferenceSystem::createFromProj4Invalid()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( !myCrs.createFromProj4( "+proj=longlat +no_defs" ) );
}

void TestQgsCoordinateReferenceSystem::validSrsIds()
{
  const QList< long > ids = QgsCoordinateReferenceSystem::validSrsIds();
  QVERIFY( ids.contains( 3857 ) );
  QVERIFY( ids.contains( 28356 ) );

  int validCount = 0;

  // check that all returns ids are valid
  for ( long id : ids )
  {
    QgsCoordinateReferenceSystem c = QgsCoordinateReferenceSystem::fromSrsId( id );
    if ( c.isValid() )
      validCount++;
    else
      qDebug() << QStringLiteral( "QgsCoordinateReferenceSystem::fromSrsId( %1 ) is not valid (%2 of %3 IDs returned by QgsCoordinateReferenceSystem::validSrsIds())." ).arg( id ).arg( ids.indexOf( id ) ).arg( ids.length() );
  }

  QVERIFY( validCount > ids.size() - 100 );
}

void TestQgsCoordinateReferenceSystem::asVariant()
{
  QgsCoordinateReferenceSystem original;
  original.createFromSrid( 3112 );

  //convert to and from a QVariant
  QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  QgsCoordinateReferenceSystem fromVar = qvariant_cast<QgsCoordinateReferenceSystem>( var );
  QCOMPARE( fromVar.authid(), original.authid() );
}

void TestQgsCoordinateReferenceSystem::bounds()
{
  QgsCoordinateReferenceSystem invalid;
  QVERIFY( invalid.bounds().isNull() );

  QgsCoordinateReferenceSystem crs3111( "EPSG:3111" );
  QgsRectangle bounds = crs3111.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 140.960000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 150.040000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -39.200000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), -33.980000, 0.0001 );

  QgsCoordinateReferenceSystem crs28356( "EPSG:28356" );
  bounds = crs28356.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 150.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 156.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -58.960000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), -13.870000, 0.0001 );

  QgsCoordinateReferenceSystem crs3857( "EPSG:3857" );
  bounds = crs3857.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), -180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -85.060000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 85.060000, 0.0001 );

  QgsCoordinateReferenceSystem crs4326( "EPSG:4326" );
  bounds = crs4326.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), -180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -90.00000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 90.00000, 0.0001 );

  QgsCoordinateReferenceSystem crs2163( "EPSG:2163" );
  bounds = crs2163.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 167.65000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), -65.69000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), 15.56000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 74.71000, 0.0001 );
}

void TestQgsCoordinateReferenceSystem::saveAsUserCrs()
{
  QString madeUpProjection = QStringLiteral( "+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs" );
  QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj4( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj4(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  long newId = userCrs.saveAsUserCrs( QStringLiteral( "babies first projection" ) );
  QCOMPARE( newId, static_cast< long >( USER_CRS_START_ID ) );
  QCOMPARE( userCrs.srsid(), newId );
  QCOMPARE( userCrs.authid(), QStringLiteral( "USER:100000" ) );
  QCOMPARE( userCrs.description(), QStringLiteral( "babies first projection" ) );

  // new CRS with same definition, check that it's matched to user crs
  QgsCoordinateReferenceSystem userCrs2 = QgsCoordinateReferenceSystem::fromProj4( madeUpProjection );
  QVERIFY( userCrs2.isValid() );
  QCOMPARE( userCrs2.toProj4(), madeUpProjection );
  QCOMPARE( userCrs2.srsid(), userCrs.srsid() );
  QCOMPARE( userCrs2.authid(), QStringLiteral( "USER:100000" ) );
  QCOMPARE( userCrs2.description(), QStringLiteral( "babies first projection" ) );

  // createFromString with user crs
  QgsCoordinateReferenceSystem userCrs3;
  userCrs3.createFromString( QStringLiteral( "USER:100000" ) );
  QVERIFY( userCrs3.isValid() );
  QCOMPARE( userCrs3.authid(), QString( "USER:100000" ) );
  QCOMPARE( userCrs3.toProj4(), madeUpProjection );
  QCOMPARE( userCrs3.description(), QStringLiteral( "babies first projection" ) );
}

void TestQgsCoordinateReferenceSystem::projectWithCustomCrs()
{
  // tests loading a 2.x project with a custom CRS defined
  QgsProject p;
  QSignalSpy spyCrsChanged( &p, &QgsProject::crsChanged );
  QVERIFY( p.read( TEST_DATA_DIR + QStringLiteral( "/projects/custom_crs.qgs" ) ) );
  QVERIFY( p.crs().isValid() );
  QCOMPARE( p.crs().toProj4(), QStringLiteral( "+proj=ortho +lat_0=42.1 +lon_0=12.8 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs" ) );
  QCOMPARE( spyCrsChanged.count(), 1 );
}

void TestQgsCoordinateReferenceSystem::projectEPSG25833()
{
  // tests loading a 2.x project with a predefined EPSG that has non unique proj.4 string
  QgsProject p;
  QSignalSpy spyCrsChanged( &p, &QgsProject::crsChanged );
  QVERIFY( p.read( TEST_DATA_DIR + QStringLiteral( "/projects/epsg25833.qgs" ) ) );
  QVERIFY( p.crs().isValid() );
  QCOMPARE( p.crs().authid(), QStringLiteral( "EPSG:25833" ) );
  QCOMPARE( spyCrsChanged.count(), 1 );
}

void TestQgsCoordinateReferenceSystem::geoCcsDescription()
{
  // test that geoccs crs descriptions are correctly imported from GDAL
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( QStringLiteral( "EPSG:3822" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:3822" ) );
  QCOMPARE( crs.description(), QStringLiteral( "TWD97" ) );

  crs.createFromString( QStringLiteral( "EPSG:4340" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:4340" ) );
  QCOMPARE( crs.description(), QStringLiteral( "Australian Antarctic (geocentric)" ) );

  crs.createFromString( QStringLiteral( "EPSG:4348" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:4348" ) );
  QCOMPARE( crs.description(), QStringLiteral( "GDA94 (geocentric)" ) );
}

void TestQgsCoordinateReferenceSystem::geographicCrsAuthId()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:4326" ) );
  QCOMPARE( crs.geographicCrsAuthId(), QStringLiteral( "EPSG:4326" ) );
  crs.createFromString( QStringLiteral( "EPSG:3825" ) );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:3825" ) );
  QCOMPARE( crs.geographicCrsAuthId(), QStringLiteral( "EPSG:3824" ) );
}

void TestQgsCoordinateReferenceSystem::noProj()
{
#if PROJ_VERSION_MAJOR>=6
  // test a crs which cannot be represented by a proj string
  QgsCoordinateReferenceSystem crs( "EPSG:2218" );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:2218" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"Scoresbysund 1952 / Greenland zone 5 east\",GEOGCS[\"Scoresbysund 1952\",DATUM[\"Scoresbysund_1952\",SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]],AUTHORITY[\"EPSG\",\"6195\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4195\"]],PROJECTION[\"Lambert_Conic_Conformal_(West_Orientated)\"],PARAMETER[\"Latitude of natural origin\",70.5],PARAMETER[\"Longitude of natural origin\",-24],PARAMETER[\"Scale factor at natural origin\",1],PARAMETER[\"False easting\",0],PARAMETER[\"False northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"2218\"]]" ) );
  crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( "ESRI:54091" );
  QCOMPARE( crs.authid(), QStringLiteral( "ESRI:54091" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"WGS_1984_Peirce_quincuncial_North_Pole_diamond\",GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Peirce_Quincuncial\"],PARAMETER[\"False_Easting\",0],PARAMETER[\"False_Northing\",0],PARAMETER[\"Central_Meridian\",0],PARAMETER[\"Scale_Factor\",1],PARAMETER[\"Latitude_Of_Origin\",90],PARAMETER[\"Option\",1],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"ESRI\",\"54091\"]]" ) );
  crs = QgsCoordinateReferenceSystem( "EPSG:22300" );
  QCOMPARE( crs.authid(), QStringLiteral( "EPSG:22300" ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toWkt(), QStringLiteral( "PROJCS[\"Carthage (Paris) / Tunisia Mining Grid\",GEOGCS[\"Carthage (Paris)\",DATUM[\"Carthage_Paris\",SPHEROID[\"Clarke 1880 (IGN)\",6378249.2,293.466021293627,AUTHORITY[\"EPSG\",\"7011\"]],AUTHORITY[\"EPSG\",\"6816\"]],PRIMEM[\"Paris\",2.33722916999999,AUTHORITY[\"EPSG\",\"8903\"]],UNIT[\"grad\",0.0157079632679489,AUTHORITY[\"EPSG\",\"9105\"]],AUTHORITY[\"EPSG\",\"4816\"]],PROJECTION[\"Tunisia_Mapping_Grid\"],PARAMETER[\"latitude_of_origin\",36.5964],PARAMETER[\"central_meridian\",7.83445],PARAMETER[\"false_easting\",270],PARAMETER[\"false_northing\",360],UNIT[\"kilometre\",1000,AUTHORITY[\"EPSG\",\"9036\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH],AUTHORITY[\"EPSG\",\"22300\"]]" ) );  //#spellok
#endif

}
QGSTEST_MAIN( TestQgsCoordinateReferenceSystem )
#include "testqgscoordinatereferencesystem.moc"
