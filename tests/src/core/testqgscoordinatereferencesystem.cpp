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
#include <QtTest>
#include <iostream>

#include <QPixmap>

#include <qgsapplication.h>
#include "qgslogger.h"

//header for class being tested
#include <qgscoordinatereferencesystem.h>
#include <qgis.h>
#include <qgsvectorlayer.h>

#include <proj_api.h>
#include <gdal.h>
#include <cpl_conv.h>

class TestQgsCoordinateReferenceSystem: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void wktCtor();
    void idCtor();
    void copyCtor();
    void assignmentCtor();
    void createFromId();
    void createFromOgcWmsCrs();
    void createFromSrid();
    void createFromWkt();
    void createFromESRIWkt();
    void createFromSrsId();
    void createFromProj4();
    void isValid();
    void validate();
    void equality();
    void noEquality();
    void equalityInvalid();
    void readXML();
    void writeXML();
    void setCustomSrsValidation();
    void customSrsValidation();
    void postgisSrid();
    void ellipsoidAcronym();
    void toWkt();
    void toProj4();
    void geographicFlag();
    void mapUnits();
    void setValidationHint();
    void axisInverted();
  private:
    void debugPrint( QgsCoordinateReferenceSystem &theCrs );
    // these used by createFromESRIWkt()
    QStringList myWktStrings;
    QList<int> myGdalVersionOK;
    QStringList myFiles;
    QStringList myProj4Strings;
    QStringList myTOWGS84Strings;
    QStringList myAuthIdStrings;
    QString testESRIWkt( int i, QgsCoordinateReferenceSystem &theCrs );
};


void TestQgsCoordinateReferenceSystem::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  qDebug() << "GEOPROJ4 constant:      " << GEOPROJ4;
  qDebug() << "GDAL version (build):   " << GDAL_RELEASE_NAME;
  qDebug() << "GDAL version (runtime): " << GDALVersionInfo( "RELEASE_NAME" );
  qDebug() << "PROJ.4 version:         " << PJ_VERSION;

  // if user set GDAL_FIX_ESRI_WKT print a warning
#if GDAL_VERSION_NUM >= 1900
  if ( strcmp( CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" ), "" ) != 0 )
  {
    qDebug() << "Warning! GDAL_FIX_ESRI_WKT =" << CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" )
    << "this might generate errors!";
  }
#endif

}

void TestQgsCoordinateReferenceSystem::wktCtor()
{
  QString myWkt = GEOWKT;
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
}
void TestQgsCoordinateReferenceSystem::assignmentCtor()
{
  QgsCoordinateReferenceSystem myCrs( GEOSRID,
                                      QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsCoordinateReferenceSystem myCrs2 = myCrs;
  debugPrint( myCrs2 );
  QVERIFY( myCrs2.isValid() );
}
void TestQgsCoordinateReferenceSystem::createFromId()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromId( GEO_EPSG_CRS_ID,
                      QgsCoordinateReferenceSystem::EpsgCrsId );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
}
void TestQgsCoordinateReferenceSystem::createFromOgcWmsCrs()
{
  QgsCoordinateReferenceSystem myCrs;
  //@todo implement this - for now we just check that if fails
  //if passed an empty string
  QVERIFY( !myCrs.createFromOgcWmsCrs( QString( "" ) ) );
}
void TestQgsCoordinateReferenceSystem::createFromSrid()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
}
void TestQgsCoordinateReferenceSystem::createFromWkt()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromWkt( GEOWKT );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
}

QString TestQgsCoordinateReferenceSystem::testESRIWkt( int i, QgsCoordinateReferenceSystem &myCrs )
{
  debugPrint( myCrs );

  if ( ! myCrs.isValid() )
    return QString( "test %1 crs is invalid" );
#if 0
  if ( myCrs.toProj4() != myProj4Strings[i] )
    return QString( "test %1 PROJ.4 = [ %2 ] expecting [ %3 ]"
                  ).arg( i ).arg( myCrs.toProj4() ).arg( myProj4Strings[i] );
#endif
  if ( myCrs.toProj4().indexOf( myTOWGS84Strings[i] ) == -1 )
    return QString( "test %1 [%2] not found, PROJ.4 = [%3] expecting [%4]"
                  ).arg( i ).arg( myTOWGS84Strings[i] ).arg( myCrs.toProj4() ).arg( myProj4Strings[i] );
  if ( myCrs.authid() !=  myAuthIdStrings[i] )
    return QString( "test %1 AUTHID = [%2] expecting [%3]"
                  ).arg( i ).arg( myCrs.authid() ).arg( myAuthIdStrings[i] );

  return "";
}
void TestQgsCoordinateReferenceSystem::createFromESRIWkt()
{
  QString msg;
  QgsCoordinateReferenceSystem myCrs;
  const char* configOld = CPLGetConfigOption( "GDAL_FIX_ESRI_WKT", "" );

  // for more tests add definitions here

  // this example file taken from bug #5598
  myWktStrings << "PROJCS[\"Indian_1960_UTM_Zone_48N\",GEOGCS[\"GCS_Indian_1960\",DATUM[\"D_Indian_1960\",SPHEROID[\"Everest_Adjustment_1937\",6377276.345,300.8017]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"False_Easting\",500000.0],PARAMETER[\"False_Northing\",0.0],PARAMETER[\"Central_Meridian\",105.0],PARAMETER[\"Scale_Factor\",0.9996],PARAMETER[\"Latitude_Of_Origin\",0.0],UNIT[\"Meter\",1.0]]";
  myGdalVersionOK << 1800;
  myFiles << "bug5598.shp";
  myProj4Strings << "+proj=utm +zone=48 +a=6377276.345 +b=6356075.41314024 +towgs84=198,881,317,0,0,0,0 +units=m +no_defs";
  myTOWGS84Strings << "+towgs84=198,881,317,0,0,0,0";
  myAuthIdStrings << "EPSG:3148";

  // this example file taken from bug #5598 - geographic CRS only, supported since gdal 1.9
  myWktStrings << "GEOGCS[\"GCS_Indian_1960\",DATUM[\"D_Indian_1960\",SPHEROID[\"Everest_Adjustment_1937\",6377276.345,300.8017]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]";
  myFiles << "";
  myGdalVersionOK << 1900;
  myProj4Strings << "+proj=longlat +a=6377276.345 +b=6356075.41314024 +towgs84=198,881,317,0,0,0,0 +no_defs";
  myTOWGS84Strings << "+towgs84=198,881,317,0,0,0,0";
  myAuthIdStrings << "EPSG:4131";

  // SAD69 geographic CRS, supported since gdal 1.9
  myWktStrings << "GEOGCS[\"GCS_South_American_1969\",DATUM[\"D_South_American_1969\",SPHEROID[\"GRS_1967_Truncated\",6378160.0,298.25]],PRIMEM[\"Greenwich\",0.0],UNIT[\"Degree\",0.0174532925199433]]";
  myFiles << "";
  myGdalVersionOK << 1900;
  myProj4Strings << "+proj=longlat +ellps=aust_SA +towgs84=-57,1,-41,0,0,0,0 +no_defs";
  myTOWGS84Strings << "+towgs84=-57,1,-41,0,0,0,0";
  myAuthIdStrings << "EPSG:4618";

  // do test with WKT definitions
  for ( int i = 0; i < myWktStrings.size() ; i++ )
  {
    QgsDebugMsg( QString( "i=%1 wkt=%2" ).arg( i ).arg( myWktStrings[i] ) );
    // use createFromUserInput and add the ESRI:: prefix to force morphFromESRI
    CPLSetConfigOption( "GDAL_FIX_ESRI_WKT", configOld );
    myCrs.createFromUserInput( "ESRI::" + myWktStrings[i] );
    msg = testESRIWkt( i, myCrs );
    if ( GDAL_VERSION_NUM < myGdalVersionOK[i] )
    {
      QEXPECT_FAIL( "", QString( "expected failure with GDAL %1 : %2"
                               ).arg( GDAL_VERSION_NUM ).arg( msg ).toLocal8Bit().constData(),
                    Continue );
    }

    if ( !msg.isEmpty() )
      QVERIFY2( false, msg.toLocal8Bit().constData() );

    // do test with shapefiles
    CPLSetConfigOption( "GDAL_FIX_ESRI_WKT", configOld );
    if ( myFiles[i] != "" )
    {
      // use ogr to open file, make sure CRS is ok
      // this probably could be in another test, but leaving it here since it deals with CRS
      QString fileStr = QString( TEST_DATA_DIR ) + QDir::separator() + myFiles[i];
      QgsDebugMsg( QString( "i=%1 file=%2" ).arg( i ).arg( fileStr ) );

      QgsVectorLayer *myLayer = new QgsVectorLayer( fileStr, "", "ogr" );
      if ( !myLayer || ! myLayer->isValid() )
      {
        qWarning() << QString( "test %1 did not get valid vector layer from %2" ).arg( i ).arg( fileStr );
        QVERIFY2( false, "no valid vector layer" );
      }
      else
      {
        myCrs = myLayer->crs();
        msg = testESRIWkt( i, myCrs );
        if ( GDAL_VERSION_NUM < myGdalVersionOK[i] )
        {
          QEXPECT_FAIL( "", QString( "expected failure with GDAL %1 : %2 using layer %3"
                                   ).arg( GDAL_VERSION_NUM ).arg( msg ).arg( fileStr ).toLocal8Bit().constData(),
                        Continue );
        }
        if ( !msg.isEmpty() )
          QVERIFY2( false, msg.toLocal8Bit().constData() );
      }
      if ( myLayer )
        delete myLayer;

    }

  }

  //  QVERIFY( bOK );
}
void TestQgsCoordinateReferenceSystem::createFromSrsId()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( myCrs.createFromSrid( GEOSRID ) );
  debugPrint( myCrs );
}
void TestQgsCoordinateReferenceSystem::createFromProj4()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( myCrs.createFromProj4( GEOPROJ4 ) );
  debugPrint( myCrs );
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
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  myCrs.validate();
  QVERIFY( myCrs.isValid() );
  debugPrint( myCrs );
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
void TestQgsCoordinateReferenceSystem::readXML()
{
  //QgsCoordinateReferenceSystem myCrs;
  //myCrs.createFromSrid( GEOSRID );
  //QgsCoordinateReferenceSystem myCrs2;
  //QVERIFY( myCrs2.readXML( QDomNode & theNode ) );
}
void TestQgsCoordinateReferenceSystem::writeXML()
{
  //QgsCoordinateReferenceSystem myCrs;
  //bool writeXML( QDomNode & theNode, QDomDocument & theDoc ) const;
  //QVERIFY( myCrs.isValid() );
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
   * @todo implement this test
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
  QVERIFY( myAcronym == "WGS84" );
}
void TestQgsCoordinateReferenceSystem::toWkt()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QString myWkt = myCrs.toWkt();
  debugPrint( myCrs );
#if GDAL_VERSION_NUM >= 1800
  //Note: this is not the same as GEOWKT as OGR strips off the TOWGS clause...
  QString myStrippedWkt( "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID"
                         "[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
                         "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY"
                         "[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY"
                         "[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]" );
#else
  // for GDAL <1.8
  QString myStrippedWkt( "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID"
                         "[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
                         "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY"
                         "[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.01745329251994328,AUTHORITY"
                         "[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]" );
#endif
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
  QVERIFY( myCrs.toProj4() == GEOPROJ4 );
}
void TestQgsCoordinateReferenceSystem::geographicFlag()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QVERIFY( myCrs.geographicFlag() );
  debugPrint( myCrs );
}
void TestQgsCoordinateReferenceSystem::mapUnits()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  QVERIFY( myCrs.mapUnits() == QGis::Degrees );
  debugPrint( myCrs );
}
void TestQgsCoordinateReferenceSystem::setValidationHint()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.setValidationHint( "<head>" );
  QVERIFY( myCrs.validationHint() == QString( "<head>" ) );
  debugPrint( myCrs );
}

void TestQgsCoordinateReferenceSystem::axisInverted()
{
  // this is used by WMS 1.3 to determine whether to switch axes or not

  QgsCoordinateReferenceSystem crs;
  crs.createFromOgcWmsCrs( "EPSG:4326" ); // WGS 84 with inverted axes
  QVERIFY( crs.axisInverted() );

  crs.createFromOgcWmsCrs( "CRS:84" ); // WGS 84 without inverted axes
  QVERIFY( !crs.axisInverted() );

  crs.createFromOgcWmsCrs( "EPSG:32633" ); // "WGS 84 / UTM zone 33N" - projected CRS without invertex axes
  QVERIFY( !crs.axisInverted() );
}


void TestQgsCoordinateReferenceSystem::debugPrint(
  QgsCoordinateReferenceSystem &theCrs )
{
  QgsDebugMsg( "***SpatialRefSystem***" );
  QgsDebugMsg( "* Valid : " + ( theCrs.isValid() ? QString( "true" ) :
                                QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( theCrs.srsid() ) );
  QgsDebugMsg( "* EPSG ID : " + theCrs.authid() );
  QgsDebugMsg( "* PGIS ID : " + QString::number( theCrs.postgisSrid() ) );
  QgsDebugMsg( "* Proj4 : " + theCrs.toProj4() );
  QgsDebugMsg( "* WKT   : " + theCrs.toWkt() );
  QgsDebugMsg( "* Desc. : " + theCrs.description() );
  if ( theCrs.mapUnits() == QGis::Meters )
  {
    QgsDebugMsg( "* Units : meters" );
  }
  else if ( theCrs.mapUnits() == QGis::Feet )
  {
    QgsDebugMsg( "* Units : feet" );
  }
  else if ( theCrs.mapUnits() == QGis::Degrees )
  {
    QgsDebugMsg( "* Units : degrees" );
  }
}

QTEST_MAIN( TestQgsCoordinateReferenceSystem )
#include "moc_testqgscoordinatereferencesystem.cxx"
