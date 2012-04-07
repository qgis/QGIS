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
    void createFromSrsId();
    void createFromProj4();
    void isValid();
    void validate();
    void equality();
    void noEquality();
    void equals();
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
  private:
    void debugPrint( QgsCoordinateReferenceSystem &theCrs );
};


void TestQgsCoordinateReferenceSystem::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::setPrefixPath( INSTALL_PREFIX, true );
  QgsApplication::initQgis( );
  QgsApplication::showSettings();

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
  myCrs.createFromId( GEOSRID, QgsCoordinateReferenceSystem::EpsgCrsId );
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
void TestQgsCoordinateReferenceSystem::equals()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  debugPrint( myCrs );
  //Note: OSRImportFromProj4 (used internally by equals)
  //drops the TOWGS from the WKT which causes this test to fail
  QString myProj4( GEOPROJ4 );
  QVERIFY( myCrs.equals( myProj4 ) );
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
  //Note: this is not the same as GEOWKT as OGR strips off the TOWGS clause...
  QString myStrippedWkt("GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID"
     "[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
     "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY"
     "[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY"
     "[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]");
  QVERIFY( myWkt == myStrippedWkt );
}
void TestQgsCoordinateReferenceSystem::toProj4()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromSrid( GEOSRID );
  debugPrint( myCrs );
  QVERIFY( myCrs.toProj4() == "+proj=longlat +datum=WGS84 +no_defs" );
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

void TestQgsCoordinateReferenceSystem::debugPrint(
                              QgsCoordinateReferenceSystem &theCrs )
{
  QgsDebugMsg( "***SpatialRefSystem***" );
  QgsDebugMsg( "* Valid : " + ( theCrs.isValid() ? QString( "true" ) :
                                                   QString( "false" ) ) );
  QgsDebugMsg( "* SrsId : " + QString::number( theCrs.srsid() ) );
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
