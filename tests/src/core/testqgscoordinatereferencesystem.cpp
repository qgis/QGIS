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
#include "qgsapplication.h"
#include "qgslogger.h"
#include "qgstest.h"

#include <QPixmap>

//header for class being tested
#include "qgscoordinatereferencesystem.h"
#include "qgscoordinatereferencesystem_p.h"
#include "qgis.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgsprojutils.h"
#include "qgsprojectionfactors.h"
#include "qgsprojoperation.h"
#include "qgscoordinatereferencesystemregistry.h"
#include <proj.h>
#include <gdal.h>
#include <cpl_conv.h>
#include <QtTest/QSignalSpy>

class TestQgsCoordinateReferenceSystem : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void wktCtor();
    void idCtor();
    void copyCtor();
    void assignmentCtor();
    void testDeprecated();
    void compoundCrs();
    void verticalCrs();
    void projectedCrs();
    void geocentricCrs();
    void geographic3d();
    void toHorizontal();
    void toVertical();
    void hasVerticalAxis();
    void coordinateEpoch();
    void createCompound();
    void saveAsUserCrs();
    void createFromId();
    void fromEpsgId();
    void createFromOgcWmsCrs();
    void fromOgcWmsCrs();
    void ogcWmsCrsCache();
    void createFromSrid();
    void sridCache();
    void createFromWkt();
    void createFromWktUnknown();
    void fromWkt();
    void wktCache();
    void createFromSrId();
    void fromSrsId();
    void srsIdCache();
    void createFromProj();
    void fromProj();
    void proj4Cache();
    void fromString();
    void fromStringCache();
    void fromProjObject();
    void fromProjObjectKnownCrs();
    void fromProjObjectNotCrs();
    void isValid();
    void validate();
    void comparison_data();
    void comparison();
    void equality();
    void noEquality();
    void equalityInvalid();
    void readWriteXml();
    void readWriteXmlNativeFormatWkt();
    void readWriteXmlNativeFormatProj();
    void setCustomSrsValidation();
    void customSrsValidation();
    void postgisSrid();
    void ellipsoidAcronym();
    void toWkt();
    void toProj();
    void isGeographic();
    void mapUnits();
    void isDynamic();
    void celestialBody();
    void operation();
    void setValidationHint();
    void hasAxisInverted();
    void createFromProjInvalid();
    void validSrsIds();
    void asVariant();
    void bounds();
    void projectWithCustomCrs();
    void projectEPSG25833();
    void geoCcsDescription();
    void geographicCrsAuthId();
    void noProj();
    void customProjString();
    void displayIdentifier();
    void createFromWktWithIdentify();
    void fromProj4EPSG20936();
    void projFactors();
    void toOgcUri();

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

    const QString mWgs84Wkt = R"""(GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],USAGE[SCOPE["unknown"],AREA["World"],BBOX[-90,-180,90,180]],ID["EPSG",4326]] )""";

    QString testESRIWkt( int i, QgsCoordinateReferenceSystem &crs );

    static bool sValidateCalled;

    static void testValidationCrs( QgsCoordinateReferenceSystem &crs )
    {
      sValidateCalled = true;

      crs.createFromString( u"EPSG:3111"_s );
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
  const QString subPath = QUuid::createUuid().toString().remove( '-' ).remove( '{' ).remove( '}' );
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
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  QSettings().clear();

  QgsApplication::showSettings();


  QgsDebugMsgLevel( u"Custom srs database: %1"_s.arg( QgsApplication::qgisUserDatabaseFilePath() ), 1 );

  qDebug() << "GDAL version (build):   " << GDAL_RELEASE_NAME;
  qDebug() << "GDAL version (runtime): " << GDALVersionInfo( "RELEASE_NAME" );
  const PJ_INFO info = proj_info();
  qDebug() << "PROJ version:           " << info.release;

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
  QgsCoordinateReferenceSystem myCrs( mWgs84Wkt );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );

  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );

  QCOMPARE( myCrs.type(), Qgis::CrsType::Geographic2d );
}
void TestQgsCoordinateReferenceSystem::idCtor()
{
  Q_NOWARN_DEPRECATED_PUSH
  const QgsCoordinateReferenceSystem myCrs( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  Q_NOWARN_DEPRECATED_POP
  QVERIFY( myCrs.isValid() );

  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}
void TestQgsCoordinateReferenceSystem::copyCtor()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QgsCoordinateReferenceSystem myCrs2( myCrs );
  debugPrint( myCrs2 );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), u"EPSG:4326"_s );

  //test implicit sharing detachment - modify original
  myCrs.createFromString( u"EPSG:3111"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:3111"_s );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), u"EPSG:4326"_s );

  // with coordinate epoch
  myCrs.createFromString( u"EPSG:4326"_s );
  myCrs.setCoordinateEpoch( 2021.3 );
  QgsCoordinateReferenceSystem myCrs3( myCrs );
  QCOMPARE( myCrs3.coordinateEpoch(), 2021.3 );
  // detach via setting coordinate epoch
  myCrs3.setCoordinateEpoch( 2021.2 );
  QCOMPARE( myCrs.coordinateEpoch(), 2021.3 );
  QCOMPARE( myCrs3.coordinateEpoch(), 2021.2 );
}

void TestQgsCoordinateReferenceSystem::assignmentCtor()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QgsCoordinateReferenceSystem myCrs2 = myCrs;
  debugPrint( myCrs2 );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), u"EPSG:4326"_s );

  //test implicit sharing detachment - modify original
  myCrs.createFromString( u"EPSG:3111"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:3111"_s );
  QVERIFY( myCrs2.isValid() );
  QCOMPARE( myCrs2.authid(), u"EPSG:4326"_s );

  // with coordinate epoch
  myCrs.createFromString( u"EPSG:4326"_s );
  myCrs.setCoordinateEpoch( 2021.3 );
  QgsCoordinateReferenceSystem myCrs3;
  myCrs3 = myCrs;
  QCOMPARE( myCrs3.coordinateEpoch(), 2021.3 );
  // detach via setting coordinate epoch
  myCrs3.setCoordinateEpoch( 2021.2 );
  QCOMPARE( myCrs.coordinateEpoch(), 2021.3 );
  QCOMPARE( myCrs3.coordinateEpoch(), 2021.2 );
}

void TestQgsCoordinateReferenceSystem::testDeprecated()
{
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:3111"_s ).isDeprecated() );
  QVERIFY( QgsCoordinateReferenceSystem( u"EPSG:4226"_s ).isDeprecated() );
}

void TestQgsCoordinateReferenceSystem::compoundCrs()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:5500"_s );
  QVERIFY( crs.isValid() );

  crs = QgsCoordinateReferenceSystem( u"EPSG:5500"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Compound );
  QVERIFY( crs.isGeographic() );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::Degrees );

  crs = QgsCoordinateReferenceSystem( u"EPSG:9388"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Compound );
  QVERIFY( !crs.isGeographic() );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::Meters );
  QVERIFY( !crs.hasAxisInverted() );
  QCOMPARE( crs.axisOrdering(), QList<Qgis::CrsAxisDirection>() << Qgis::CrsAxisDirection::East << Qgis::CrsAxisDirection::North << Qgis::CrsAxisDirection::Up );

  crs = QgsCoordinateReferenceSystem( u"EPSG:3903"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Compound );
  QVERIFY( !crs.isGeographic() );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::Meters );
  QVERIFY( crs.hasAxisInverted() );
  QCOMPARE( crs.axisOrdering(), QList<Qgis::CrsAxisDirection>() << Qgis::CrsAxisDirection::North << Qgis::CrsAxisDirection::East << Qgis::CrsAxisDirection::Up );
}

void TestQgsCoordinateReferenceSystem::verticalCrs()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:5703"_s );
  QVERIFY( crs.isValid() );

  crs = QgsCoordinateReferenceSystem( u"EPSG:5703"_s );
  QVERIFY( crs.isValid() );

  QCOMPARE( crs.type(), Qgis::CrsType::Vertical );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::Meters );

  crs = QgsCoordinateReferenceSystem( u"EPSG:6358"_s );
  QVERIFY( crs.isValid() );

  QCOMPARE( crs.type(), Qgis::CrsType::Vertical );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::FeetUSSurvey );
}

void TestQgsCoordinateReferenceSystem::projectedCrs()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:3111"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Projected );
}

void TestQgsCoordinateReferenceSystem::geocentricCrs()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:4978"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Geocentric );
}

void TestQgsCoordinateReferenceSystem::geographic3d()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:4979"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.type(), Qgis::CrsType::Geographic3d );
}

void TestQgsCoordinateReferenceSystem::toHorizontal()
{
  // invalid
  QVERIFY( !QgsCoordinateReferenceSystem().horizontalCrs().isValid() );
  // vertical only
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:5703"_s ).horizontalCrs().isValid() );
  // compound
  QCOMPARE( QgsCoordinateReferenceSystem( u"EPSG:5500"_s ).horizontalCrs().authid(), u"EPSG:4759"_s );
  // already horizontal
  QCOMPARE( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ).horizontalCrs().authid(), u"EPSG:3111"_s );
  // geographic 3d
  QCOMPARE( QgsCoordinateReferenceSystem( u"EPSG:4979"_s ).horizontalCrs().authid(), u"EPSG:4979"_s );
}

void TestQgsCoordinateReferenceSystem::toVertical()
{
  // invalid
  QVERIFY( !QgsCoordinateReferenceSystem().verticalCrs().isValid() );
  // horizontal only (projected 2d)
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:3111"_s ).verticalCrs().isValid() );
  // compound
  QCOMPARE( QgsCoordinateReferenceSystem( u"EPSG:5500"_s ).verticalCrs().authid(), u"EPSG:5703"_s );
  // already vertical
  QCOMPARE( QgsCoordinateReferenceSystem( u"EPSG:5703"_s ).verticalCrs().authid(), u"EPSG:5703"_s );
  // geographic 3d
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:4979"_s ).verticalCrs().isValid() );
}

void TestQgsCoordinateReferenceSystem::hasVerticalAxis()
{
  // invalid
  QVERIFY( !QgsCoordinateReferenceSystem().hasVerticalAxis() );
  // horizontal only (projected 2d)
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:3111"_s ).hasVerticalAxis() );
  // compound
  QVERIFY( QgsCoordinateReferenceSystem( u"EPSG:5500"_s ).hasVerticalAxis() );
  // already vertical
  QVERIFY( QgsCoordinateReferenceSystem( u"EPSG:5703"_s ).hasVerticalAxis() );
  // geographic 2d
  QVERIFY( !QgsCoordinateReferenceSystem( u"EPSG:4326"_s ).hasVerticalAxis() );
  // geographic 3d
  QVERIFY( QgsCoordinateReferenceSystem( u"EPSG:4979"_s ).hasVerticalAxis() );
  // projected 3d
  const QString projected3DWkt = QStringLiteral( "PROJCRS[\"NAD83(HARN) / Oregon GIC Lambert (ft)\",\n"
                                                 "    BASEGEOGCRS[\"NAD83(HARN)\",\n"
                                                 "        DATUM[\"NAD83 (High Accuracy Reference Network)\",\n"
                                                 "            ELLIPSOID[\"GRS 1980\",6378137,298.257222101,\n"
                                                 "                LENGTHUNIT[\"metre\",1]]],\n"
                                                 "        PRIMEM[\"Greenwich\",0,\n"
                                                 "            ANGLEUNIT[\"degree\",0.0174532925199433]],\n"
                                                 "        ID[\"EPSG\",4957]],\n"
                                                 "    CONVERSION[\"unnamed\",\n"
                                                 "        METHOD[\"Lambert Conic Conformal (2SP)\",\n"
                                                 "            ID[\"EPSG\",9802]],\n"
                                                 "        PARAMETER[\"Latitude of false origin\",41.75,\n"
                                                 "            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
                                                 "            ID[\"EPSG\",8821]],\n"
                                                 "        PARAMETER[\"Longitude of false origin\",-120.5,\n"
                                                 "            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
                                                 "            ID[\"EPSG\",8822]],\n"
                                                 "        PARAMETER[\"Latitude of 1st standard parallel\",43,\n"
                                                 "            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
                                                 "            ID[\"EPSG\",8823]],\n"
                                                 "        PARAMETER[\"Latitude of 2nd standard parallel\",45.5,\n"
                                                 "            ANGLEUNIT[\"degree\",0.0174532925199433],\n"
                                                 "            ID[\"EPSG\",8824]],\n"
                                                 "        PARAMETER[\"Easting at false origin\",1312335.958,\n"
                                                 "            LENGTHUNIT[\"foot\",0.3048],\n"
                                                 "            ID[\"EPSG\",8826]],\n"
                                                 "        PARAMETER[\"Northing at false origin\",0,\n"
                                                 "            LENGTHUNIT[\"foot\",0.3048],\n"
                                                 "            ID[\"EPSG\",8827]]],\n"
                                                 "    CS[Cartesian,3],\n"
                                                 "        AXIS[\"easting\",east,\n"
                                                 "            ORDER[1],\n"
                                                 "            LENGTHUNIT[\"foot\",0.3048]],\n"
                                                 "        AXIS[\"northing\",north,\n"
                                                 "            ORDER[2],\n"
                                                 "            LENGTHUNIT[\"foot\",0.3048]],\n"
                                                 "        AXIS[\"ellipsoidal height (h)\",up,\n"
                                                 "            ORDER[3],\n"
                                                 "            LENGTHUNIT[\"foot\",0.3048]]]" );
  const QgsCoordinateReferenceSystem projected3D = QgsCoordinateReferenceSystem::fromWkt( projected3DWkt );
  QVERIFY( projected3D.isValid() );
  QVERIFY( projected3D.hasVerticalAxis() );
}

void TestQgsCoordinateReferenceSystem::coordinateEpoch()
{
  QgsCoordinateReferenceSystem crs( u"EPSG:4326"_s );
  QVERIFY( crs.isValid() );
  QVERIFY( std::isnan( crs.coordinateEpoch() ) );
  crs.setCoordinateEpoch( 2021.3 );
  QCOMPARE( crs.coordinateEpoch(), 2021.3 );

  QVERIFY( crs.projObject() );
  // force a detach
  QgsCoordinateReferenceSystem crs2;
  crs2 = crs;
  crs.setCoordinateEpoch( 2021.2 );
  QCOMPARE( crs.coordinateEpoch(), 2021.2 );
  QCOMPARE( crs2.coordinateEpoch(), 2021.3 );
  QVERIFY( crs.projObject() );
  QVERIFY( crs2.projObject() );
}

void TestQgsCoordinateReferenceSystem::createCompound()
{
  //horizontal invalid / vertical invalid
  QString error;
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem(), error ).isValid() );
  // horizontal valid / vertical invalid
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ), QgsCoordinateReferenceSystem(), error ).isValid() );
  // horizontal invalid / vertical valid
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem(), QgsCoordinateReferenceSystem( u"EPSG:5703"_s ), error ).isValid() );
  // horizontal valid / vertical valid
  const QgsCoordinateReferenceSystem compound = QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ), QgsCoordinateReferenceSystem( u"EPSG:5703"_s ), error );
  QVERIFY( compound.isValid() );
  QCOMPARE( compound.description(), u"unnamed"_s );
  QCOMPARE( compound.type(), Qgis::CrsType::Compound );
  QVERIFY( error.isEmpty() );
  // horizontal / vertical flipped
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem( u"EPSG:5703"_s ), QgsCoordinateReferenceSystem( u"EPSG:3111"_s ), error ).isValid() );
  QCOMPARE( error.left( 79 ), u"components of the compound CRS do not belong to one of the allowed combinations"_s );
  // horizontal valid / not vertical
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ), QgsCoordinateReferenceSystem( u"EPSG:3113"_s ), error ).isValid() );
  QCOMPARE( error.left( 79 ), u"components of the compound CRS do not belong to one of the allowed combinations"_s );
  // horizontal already a compound
  QVERIFY( !QgsCoordinateReferenceSystem::createCompoundCrs( QgsCoordinateReferenceSystem( u"EPSG:5500"_s ), QgsCoordinateReferenceSystem( u"EPSG:5703"_s ), error ).isValid() );
  QCOMPARE( error.left( 79 ), u"components of the compound CRS do not belong to one of the allowed combinations"_s );
}

void TestQgsCoordinateReferenceSystem::createFromId()
{
  QgsCoordinateReferenceSystem myCrs;
  Q_NOWARN_DEPRECATED_PUSH
  myCrs.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  Q_NOWARN_DEPRECATED_POP
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::fromEpsgId()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromEpsgId( 4326 );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  myCrs = QgsCoordinateReferenceSystem::fromEpsgId( -999 );
  QVERIFY( !myCrs.isValid() );

  // using an ESRI: code. This worked in pre-proj 6 builds, so we need to keep compatibility
  myCrs = QgsCoordinateReferenceSystem::fromEpsgId( 54030 );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"ESRI:54030"_s );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::createFromOgcWmsCrs()
{
  QgsCoordinateReferenceSystem myCrs;
  //check fails if passed an empty string
  QVERIFY( !myCrs.createFromOgcWmsCrs( QString() ) );

  myCrs.createFromOgcWmsCrs( u"EPSG:4326"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:4326"_s );

  myCrs.createFromOgcWmsCrs( u"http://www.opengis.net/def/crs/EPSG/0/4326"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:4326"_s );

  myCrs.createFromOgcWmsCrs( u"urn:ogc:def:crs:EPSG::4326"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:4326"_s );

  myCrs.createFromOgcWmsCrs( u"i am not a CRS"_s );
  QVERIFY( !myCrs.isValid() );

  myCrs.createFromOgcWmsCrs( u"http://www.opengis.net/def/crs/OGC/1.3/CRS84"_s );
  QVERIFY( myCrs.isValid() );
  QVERIFY( !myCrs.hasAxisInverted() );
  QCOMPARE( myCrs.authid(), u"OGC:CRS84"_s );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::fromOgcWmsCrs()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"EPSG:4326"_s );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:4326"_s );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );

  myCrs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"not a crs"_s );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::ogcWmsCrsCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  QVERIFY( crs.createFromOgcWmsCrs( u"EPSG:4326"_s ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:4326"_s );
  QVERIFY( QgsCoordinateReferenceSystem::ogcCache().contains( u"EPSG:4326"_s ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.createFromOgcWmsCrs( u"EPSG:4326"_s ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), u"EPSG:4326"_s );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromOgcWmsCrs( u"not a CRS"_s ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::ogcCache().contains( u"not a CRS"_s ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromOgcWmsCrs( u"not a CRS"_s ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::ogcCache().contains( u"EPSG:4326"_s ) );
}

void TestQgsCoordinateReferenceSystem::createFromSrid()
{
  QgsCoordinateReferenceSystem myCrs;
  Q_NOWARN_DEPRECATED_PUSH
  myCrs.createFromSrid( 4326 );
  Q_NOWARN_DEPRECATED_POP
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::sridCache()
{
  Q_NOWARN_DEPRECATED_PUSH

  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  crs.createFromSrid( 3112 );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3112"_s );
  QVERIFY( QgsCoordinateReferenceSystem::srIdCache().contains( 3112 ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  crs2.createFromSrid( 3112 );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), u"EPSG:3112"_s );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromSrid( -3141 ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::srIdCache().contains( -3141 ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromSrid( -3141 ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::srIdCache().contains( 3112 ) );

  Q_NOWARN_DEPRECATED_POP
}

void TestQgsCoordinateReferenceSystem::createFromWkt()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromWkt( mWgs84Wkt );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::createFromWktWithIdentify()
{
  QgsCoordinateReferenceSystem crs;
  // see https://github.com/qgis/QGIS/issues/33007, WKT string from a MapInfo tabfile in GDA2020 projection

  // older GDAL gave this BoundCRS WKT, but that was an issue in GDAL -- see https://github.com/OSGeo/PROJ/issues/1752
  crs.createFromWkt( u"BOUNDCRS[SOURCECRS[PROJCRS[\"unnamed\",BASEGEOGCRS[\"unnamed\",DATUM[\"Geocentric Datum of Australia 2020\",ELLIPSOID[\"GRS 80\",6378137,298.257222101,LENGTHUNIT[\"metre\",1,ID[\"EPSG\",9001]]]],PRIMEM[\"Greenwich\",0,ANGLEUNIT[\"degree\",0.0174532925199433,ID[\"EPSG\",9122]]]],CONVERSION[\"UTM zone 55S\",METHOD[\"Transverse Mercator\",ID[\"EPSG\",9807]],PARAMETER[\"Latitude of natural origin\",0,ANGLEUNIT[\"degree\",0.0174532925199433],ID[\"EPSG\",8801]],PARAMETER[\"Longitude of natural origin\",147,ANGLEUNIT[\"degree\",0.0174532925199433],ID[\"EPSG\",8802]],PARAMETER[\"Scale factor at natural origin\",0.9996,SCALEUNIT[\"unity\",1],ID[\"EPSG\",8805]],PARAMETER[\"False easting\",500000,LENGTHUNIT[\"metre\",1],ID[\"EPSG\",8806]],PARAMETER[\"False northing\",10000000,LENGTHUNIT[\"metre\",1],ID[\"EPSG\",8807]],ID[\"EPSG\",17055]],CS[Cartesian,2],AXIS[\"easting\",east,ORDER[1],LENGTHUNIT[\"metre\",1,ID[\"EPSG\",9001]]],AXIS[\"northing\",north,ORDER[2],LENGTHUNIT[\"metre\",1,ID[\"EPSG\",9001]]]]],TARGETCRS[GEOGCRS[\"WGS 84\",DATUM[\"World Geodetic System 1984\",ELLIPSOID[\"WGS 84\",6378137,298.257223563,LENGTHUNIT[\"metre\",1]]],PRIMEM[\"Greenwich\",0,ANGLEUNIT[\"degree\",0.0174532925199433]],CS[ellipsoidal,2],AXIS[\"geodetic latitude (Lat)\",north,ORDER[1],ANGLEUNIT[\"degree\",0.0174532925199433]],AXIS[\"geodetic longitude (Lon)\",east,ORDER[2],ANGLEUNIT[\"degree\",0.0174532925199433]],USAGE[SCOPE[\"unknown\"],AREA[\"World\"],BBOX[-90,-180,90,180]],ID[\"EPSG\",4326]]],ABRIDGEDTRANSFORMATION[\"Transformation to WGS84\",METHOD[\"Position Vector transformation (geog2D domain)\",ID[\"EPSG\",9606]],PARAMETER[\"X-axis translation\",-0.06155,ID[\"EPSG\",8605]],PARAMETER[\"Y-axis translation\",0.01087,ID[\"EPSG\",8606]],PARAMETER[\"Z-axis translation\",0.04019,ID[\"EPSG\",8607]],PARAMETER[\"X-axis rotation\",-0.0394924,ID[\"EPSG\",8608]],PARAMETER[\"Y-axis rotation\",-0.0327221,ID[\"EPSG\",8609]],PARAMETER[\"Z-axis rotation\",-0.0328979,ID[\"EPSG\",8610]],PARAMETER[\"Scale difference\",1.000000009994,ID[\"EPSG\",8611]]]]"_s );
  QVERIFY( crs.isValid() );
  // this must be a user CRS -- it's a boundcrs of EPSG:7855, not EPSG:7855 itself
  QVERIFY( crs.authid().isEmpty() );
  QCOMPARE( crs.ellipsoidAcronym(), u"PARAMETER:6378137:6356752.31414035614579916"_s );

  // here's the correct (desirable) WKT we want (and get) from GDAL
  crs.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.9996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:7855"_s );

  QCOMPARE( crs.ellipsoidAcronym(), u"EPSG:7019"_s );
}

void TestQgsCoordinateReferenceSystem::fromProj4EPSG20936()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=utm +zone=36 +south +a=6378249.145 +b=6356514.966398753 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs"_s );

  // For consistency with GDAL, we treat BoundCRS defined from a proj string as equivalent to their underlying CRS (ie. in this case EPSG:20936)

  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:20936"_s );
  // must NOT be a BoundCRS!
  QCOMPARE( crs.toWkt(), QStringLiteral( R"""(PROJCS["Arc 1950 / UTM zone 36S",GEOGCS["Arc 1950",DATUM["Arc_1950",SPHEROID["Clarke 1880 (Arc)",6378249.145,293.4663077,AUTHORITY["EPSG","7013"]],AUTHORITY["EPSG","6209"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4209"]],PROJECTION["Transverse_Mercator"],PARAMETER["latitude_of_origin",0],PARAMETER["central_meridian",33],PARAMETER["scale_factor",0.9996],PARAMETER["false_easting",500000],PARAMETER["false_northing",10000000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","20936"]])""" ) );
  QCOMPARE( crs.toProj(), u"+proj=utm +zone=36 +south +a=6378249.145 +rf=293.4663077 +towgs84=-143,-90,-294,0,0,0,0 +units=m +no_defs"_s );

  QCOMPARE( crs.ellipsoidAcronym(), u"EPSG:7013"_s );
}

void TestQgsCoordinateReferenceSystem::projFactors()
{
  QgsProjectionFactors factors = QgsCoordinateReferenceSystem().factors( QgsPoint( 0, 0 ) );
  QVERIFY( !factors.isValid() );

  factors = QgsCoordinateReferenceSystem( u"EPSG:3717"_s ).factors( QgsPoint( -120, 34 ) );
  QVERIFY( factors.isValid() );
  QGSCOMPARENEAR( factors.meridionalScale(), 1.0005466, 0.0000001 );
  QGSCOMPARENEAR( factors.parallelScale(), 1.0005466, 0.0000001 );
  QGSCOMPARENEAR( factors.arealScale(), 1.00109349, 0.00000001 );
  QGSCOMPARENEAR( factors.angularDistortion(), 0.0, 0.00001 );
  QGSCOMPARENEAR( factors.meridianParallelAngle(), 90.0, 0.01 );
  QGSCOMPARENEAR( factors.meridianConvergence(), 1.67864770, 0.00000001 );
  QGSCOMPARENEAR( factors.tissotSemimajor(), 1.00055, 0.00001 );
  QGSCOMPARENEAR( factors.tissotSemiminor(), 1.00055, 0.00001 );
  QGSCOMPARENEAR( factors.dxDlam(), 0.8300039, 0.000001 );
  QGSCOMPARENEAR( factors.dxDphi(), -0.0292052, 0.000001 );
  QGSCOMPARENEAR( factors.dyDlam(), 0.0243244, 0.000001 );
  QGSCOMPARENEAR( factors.dyDphi(), 0.9965495, 0.000001 );
}

void TestQgsCoordinateReferenceSystem::createFromWktUnknown()
{
  QgsCoordinateReferenceSystem crs;
  // try creating a crs from a non-standard WKT string (in this case, the invalid WKT definition of EPSG:31370 used by
  // some ArcGIS versions: see https://github.com/OSGeo/PROJ/issues/1781
  const QString wkt = QStringLiteral( R"""(PROJCS["Belge 1972 / Belgian Lambert 72",GEOGCS["Belge 1972",DATUM["Reseau_National_Belge_1972",SPHEROID["International 1924",6378388,297],AUTHORITY["EPSG","6313"]],PRIMEM["Greenwich",0],UNIT["Degree",0.0174532925199433]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",90],PARAMETER["central_meridian",4.36798666666667],PARAMETER["standard_parallel_1",49.8333339],PARAMETER["standard_parallel_2",51.1666672333333],PARAMETER["false_easting",150000.01256],PARAMETER["false_northing",5400088.4378],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" );
  const QString expectedWkt = QStringLiteral( R"""(PROJCRS["Belge 1972 / Belgian Lambert 72",BASEGEOGCRS["Belge 1972",DATUM["Reseau National Belge 1972",ELLIPSOID["International 1924",6378388,297,LENGTHUNIT["metre",1]],ID["EPSG",6313]],PRIMEM["Greenwich",0,ANGLEUNIT["Degree",0.0174532925199433]]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Latitude of false origin",90,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Longitude of false origin",4.36798666666667,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Latitude of 1st standard parallel",49.8333339,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Latitude of 2nd standard parallel",51.1666672333333,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Easting at false origin",150000.01256,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",5400088.4378,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["northing",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]])""" );

  QgsDebugMsgLevel( expectedWkt, 1 );

  crs.createFromWkt( wkt );
  QVERIFY( crs.isValid() );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Preferred ), 1 );
  crs.saveAsUserCrs( u"Test CRS"_s );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Preferred ), 1 );
  QCOMPARE( crs.toWkt( Qgis::CrsWktVariant::Preferred ), expectedWkt );
  QCOMPARE( crs.srsid(), static_cast<long>( Qgis::USER_CRS_START_ID + 1 ) );
  QCOMPARE( crs.authid(), u"USER:100001"_s );
  QCOMPARE( crs.mapUnits(), Qgis::DistanceUnit::Meters );
  QCOMPARE( crs.ellipsoidAcronym().left( 30 ), u"PARAMETER:6378388:6356911.9461"_s );

  // try creating new ones with same def
  const QgsCoordinateReferenceSystem crs2( u"USER:100001"_s );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs2.mapUnits(), Qgis::DistanceUnit::Meters );

  QgsCoordinateReferenceSystem crs3;
  crs3.createFromWkt( wkt );
  QVERIFY( crs3.isValid() );
  QCOMPARE( crs3.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs3.mapUnits(), Qgis::DistanceUnit::Meters );

  // force reads from database
  QgsCoordinateReferenceSystem::invalidateCache();
  QgsCoordinateReferenceSystem crs4( u"USER:100001"_s );
  QVERIFY( crs4.isValid() );
  QCOMPARE( crs4.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs4.mapUnits(), Qgis::DistanceUnit::Meters );
  QgsCoordinateReferenceSystem::invalidateCache();
  crs4.createFromOgcWmsCrs( u"USER:100001"_s );
  QVERIFY( crs4.isValid() );
  QCOMPARE( crs4.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs4.mapUnits(), Qgis::DistanceUnit::Meters );

  QgsCoordinateReferenceSystem crs5;
  crs5.createFromWkt( expectedWkt );
  QVERIFY( crs5.isValid() );
  QCOMPARE( crs5.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs5.mapUnits(), Qgis::DistanceUnit::Meters );
  QCOMPARE( crs5.authid(), u"USER:100001"_s );

  // try creating with a different parameter order, should still be matched to existing user crs
  QgsCoordinateReferenceSystem crs6;
  crs6.createFromWkt( QStringLiteral( R"""(PROJCRS["Belge 1972 / Belgian Lambert 72",BASEGEOGCRS["Belge 1972",DATUM["Reseau National Belge 1972",ELLIPSOID["International 1924",6378388,297,LENGTHUNIT["metre",1]],ID["EPSG",6313]],PRIMEM["Greenwich",0,ANGLEUNIT["Degree",0.0174532925199433]]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Latitude of false origin",90,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Latitude of 1st standard parallel",49.8333339,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Longitude of false origin",4.36798666666667,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Latitude of 2nd standard parallel",51.1666672333333,ANGLEUNIT["Degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Easting at false origin",150000.01256,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",5400088.4378,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["northing",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]])""" ) );
  QVERIFY( crs6.isValid() );
  QCOMPARE( crs6.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), expectedWkt );
  QCOMPARE( crs6.mapUnits(), Qgis::DistanceUnit::Meters );
  QCOMPARE( crs6.authid(), u"USER:100001"_s );
}

void TestQgsCoordinateReferenceSystem::fromWkt()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromWkt( mWgs84Wkt );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  myCrs = QgsCoordinateReferenceSystem::fromWkt( u"not wkt"_s );
  QVERIFY( !myCrs.isValid() );

  // wkt with embedded name
  myCrs = QgsCoordinateReferenceSystem::fromWkt( R"""(PROJCRS["some locally made crs",BASEGEOGCRS["unknown",DATUM["Unknown based on WGS84 ellipsoid",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1],ID["EPSG",7030]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8901]]],CONVERSION["unknown",METHOD["Hotine Oblique Mercator (variant B)",ID["EPSG",9815]],PARAMETER["Latitude of projection centre",47.2,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8811]],PARAMETER["Longitude of projection centre",9,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8812]],PARAMETER["Azimuth of initial line",39.4,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8813]],PARAMETER["Angle from Rectified to Skew Grid",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8814]],PARAMETER["Scale factor on initial line",1,SCALEUNIT["unity",1],ID["EPSG",8815]],PARAMETER["Easting at projection centre",750,LENGTHUNIT["metre",1],ID["EPSG",8816]],PARAMETER["Northing at projection centre",250,LENGTHUNIT["metre",1],ID["EPSG",8817]]],CS[Cartesian,2],AXIS["(E)",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["(N)",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]])""" );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.description(), u"some locally made crs"_s );


  // wkt with ID set, but not a known one:
  myCrs = QgsCoordinateReferenceSystem::fromWkt( R"""(GEODCRS["IAU_2015MoonXYZ",
  DATUM["Moon(2015)-Sphere",
    ELLIPSOID["Moon(2015)-Sphere",1737400,0,LENGTHUNIT["metre",1,ID["EPSG",9001]]]],
  CS[Cartesian,3],
  AXIS["(X)",geocentricX,ORDER[1],LENGTHUNIT["metre",1]],
  AXIS["(Y)",geocentricY,ORDER[2],LENGTHUNIT["metre",1]],
  AXIS["(Z)",geocentricZ,ORDER[3],LENGTHUNIT["metre",1]],
  ID["IAU",30000,2015]
])""" );
  QVERIFY( myCrs.isValid() );
  // we should ignore the authid from the WKT, it is not a valid one
  QCOMPARE( myCrs.authid(), QString() );
  QCOMPARE( myCrs.description(), u"IAU_2015MoonXYZ"_s );
  QCOMPARE( myCrs.type(), Qgis::CrsType::Geocentric );
  QCOMPARE( myCrs.celestialBodyName(), u"Moon"_s );

  // retrieve from cache
  QgsCoordinateReferenceSystem fromCache = QgsCoordinateReferenceSystem::fromWkt( R"""(GEODCRS["IAU_2015MoonXYZ",
  DATUM["Moon(2015)-Sphere",
    ELLIPSOID["Moon(2015)-Sphere",1737400,0,LENGTHUNIT["metre",1,ID["EPSG",9001]]]],
  CS[Cartesian,3],
  AXIS["(X)",geocentricX,ORDER[1],LENGTHUNIT["metre",1]],
  AXIS["(Y)",geocentricY,ORDER[2],LENGTHUNIT["metre",1]],
  AXIS["(Z)",geocentricZ,ORDER[3],LENGTHUNIT["metre",1]],
  ID["IAU",30000,2015]
])""" );
  QCOMPARE( fromCache, myCrs );

  // wkt with ID set, but not a known one:
  myCrs = QgsCoordinateReferenceSystem::fromWkt( R"""(PROJCRS["RGF93 v2b / Lambert-LSBB",
    BASEGEOGCRS["RGF93 v2b",
        DATUM["Reseau Geodesique Francais 1993 v2b",
            ELLIPSOID["GRS 1980",6378137,298.257222101,
                LENGTHUNIT["metre",1]]],
        PRIMEM["Greenwich",0,
            ANGLEUNIT["degree",0.0174532925199433]],
        ID["EPSG",9782]],
    CONVERSION["Lambert-LSBB",
        METHOD["Lambert Conic Conformal (1SP)",
            ID["EPSG",9801]],
        PARAMETER["Latitude of natural origin",43.93333333333333,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8801]],
        PARAMETER["Longitude of natural origin",5.475000,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8802]],
        PARAMETER["Scale factor at natural origin",1.00008468,
            SCALEUNIT["unity",1],
            ID["EPSG",8805]],
        PARAMETER["False easting",10000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8806]],
        PARAMETER["False northing",70000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8807]]],
    CS[Cartesian,2],
        AXIS["easting (X)",east,
            ORDER[1],
            LENGTHUNIT["metre",1]],
        AXIS["northing (Y)",north,
            ORDER[2],
            LENGTHUNIT["metre",1]],
    USAGE[
        SCOPE["Engineering survey, topographic mapping."],
        AREA["France - onshore and offshore, mainland and Corsica."],
        BBOX[43,4,45,6]],
    ID["EPSG",100001]])""" );
  QVERIFY( myCrs.isValid() );
  // we should ignore the authid from the WKT, it is not a valid one
  QCOMPARE( myCrs.authid(), QString() );
  QCOMPARE( myCrs.description(), u"RGF93 v2b / Lambert-LSBB"_s );
  QCOMPARE( myCrs.type(), Qgis::CrsType::Projected );
  QCOMPARE( myCrs.celestialBodyName(), u"Earth"_s );

  // retrieve from cache
  fromCache = QgsCoordinateReferenceSystem::fromWkt( R"""(PROJCRS["RGF93 v2b / Lambert-LSBB",
    BASEGEOGCRS["RGF93 v2b",
        DATUM["Reseau Geodesique Francais 1993 v2b",
            ELLIPSOID["GRS 1980",6378137,298.257222101,
                LENGTHUNIT["metre",1]]],
        PRIMEM["Greenwich",0,
            ANGLEUNIT["degree",0.0174532925199433]],
        ID["EPSG",9782]],
    CONVERSION["Lambert-LSBB",
        METHOD["Lambert Conic Conformal (1SP)",
            ID["EPSG",9801]],
        PARAMETER["Latitude of natural origin",43.93333333333333,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8801]],
        PARAMETER["Longitude of natural origin",5.475000,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8802]],
        PARAMETER["Scale factor at natural origin",1.00008468,
            SCALEUNIT["unity",1],
            ID["EPSG",8805]],
        PARAMETER["False easting",10000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8806]],
        PARAMETER["False northing",70000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8807]]],
    CS[Cartesian,2],
        AXIS["easting (X)",east,
            ORDER[1],
            LENGTHUNIT["metre",1]],
        AXIS["northing (Y)",north,
            ORDER[2],
            LENGTHUNIT["metre",1]],
    USAGE[
        SCOPE["Engineering survey, topographic mapping."],
        AREA["France - onshore and offshore, mainland and Corsica."],
        BBOX[43,4,45,6]],
    ID["EPSG",100001]])""" );
  QCOMPARE( fromCache, myCrs );
}

void TestQgsCoordinateReferenceSystem::wktCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  QVERIFY( crs.createFromWkt( mWgs84Wkt ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), 3452 );
  QVERIFY( QgsCoordinateReferenceSystem::wktCache().contains( mWgs84Wkt ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.createFromWkt( mWgs84Wkt ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), 3452 );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromWkt( u"bad wkt"_s ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::wktCache().contains( u"bad wkt"_s ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromWkt( u"bad wkt"_s ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::wktCache().contains( mWgs84Wkt ) );
}

QString TestQgsCoordinateReferenceSystem::testESRIWkt( int i, QgsCoordinateReferenceSystem &myCrs )
{
  debugPrint( myCrs );

  if ( !myCrs.isValid() )
    return u"test %1 crs is invalid"_s;
#if 0
  if ( myCrs.toProj() != myProj4Strings[i] )
    return QString( "test %1 PROJ = [ %2 ] expecting [ %3 ]"
                  ).arg( i ).arg( myCrs.toProj() ).arg( myProj4Strings[i] );
#endif
  if ( myCrs.toProj().indexOf( myTOWGS84Strings[i] ) == -1 )
    return QStringLiteral( "test %1 [%2] not found, PROJ = [%3] expecting [%4]"
    )
      .arg( i )
      .arg( myTOWGS84Strings[i], myCrs.toProj(), myProj4Strings[i] );
  if ( myCrs.authid() != myAuthIdStrings[i] )
    return QStringLiteral( "test %1 AUTHID = [%2] expecting [%3]"
    )
      .arg( i )
      .arg( myCrs.authid(), myAuthIdStrings[i] );

  return QString();
}

void TestQgsCoordinateReferenceSystem::createFromSrId()
{
  QgsCoordinateReferenceSystem myCrs;
  Q_NOWARN_DEPRECATED_PUSH
  QVERIFY( myCrs.createFromSrid( 4326 ) );
  Q_NOWARN_DEPRECATED_POP
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
}

void TestQgsCoordinateReferenceSystem::fromSrsId()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromSrsId( 3452 );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  myCrs = QgsCoordinateReferenceSystem::fromSrsId( -9999 );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::srsIdCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  QVERIFY( crs.createFromSrsId( 3452 ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), 3452 );
  QVERIFY( QgsCoordinateReferenceSystem::srsIdCache().contains( 3452 ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.createFromSrsId( 3452 ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), 3452 );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromSrsId( -5141 ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::srsIdCache().contains( -5141 ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromSrsId( -5141 ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::srsIdCache().contains( 3452 ) );
}


void TestQgsCoordinateReferenceSystem::createFromProj()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( !myCrs.createFromProj( QString() ) );
  QVERIFY( !myCrs.isValid() );

  QVERIFY( myCrs.createFromProj( u"+proj=longlat +datum=WGS84 +no_defs"_s ) );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );
}

void TestQgsCoordinateReferenceSystem::fromProj()
{
  QgsCoordinateReferenceSystem myCrs = QgsCoordinateReferenceSystem::fromProj( u"+proj=longlat +datum=WGS84 +no_defs"_s );
  debugPrint( myCrs );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.srsid(), 3452 );
  myCrs = QgsCoordinateReferenceSystem::fromProj( QString() );
  QVERIFY( !myCrs.isValid() );
}

void TestQgsCoordinateReferenceSystem::proj4Cache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  QVERIFY( crs.createFromProj( u"+proj=longlat +datum=WGS84 +no_defs"_s ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.srsid(), 3452 );
  QVERIFY( QgsCoordinateReferenceSystem::projCache().contains( u"+proj=longlat +datum=WGS84 +no_defs"_s ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.createFromProj( u"+proj=longlat +datum=WGS84 +no_defs"_s ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.srsid(), 3452 );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromProj( u"bad proj4"_s ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::projCache().contains( u"bad proj4"_s ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromProj( u"bad proj4"_s ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::projCache().contains( u"+proj=longlat +datum=WGS84 +no_defs"_s ) );
}

void TestQgsCoordinateReferenceSystem::fromString()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"woohooo"_s );
  QVERIFY( !crs.isValid() );
  crs.createFromString( u"EPSG:3111"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );
  crs.createFromString( u"epsg:3111"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );

  crs.createFromString( u"proj4:+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"_s );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );

  crs.createFromString( u"PROJ4:+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );
  crs.createFromString( u"proj:+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );
  crs.createFromString( u"PROJ:+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +towgs84=0,0,0,0,0,0,0 +units=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );

  crs.createFromString( u"esri:102499"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"ESRI:102499"_s );
  crs.createFromString( u"IGNF:LAMB1"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"IGNF:LAMB1"_s );
}

void TestQgsCoordinateReferenceSystem::fromStringCache()
{
  // test that crs can be retrieved correctly from cache
  QgsCoordinateReferenceSystem crs;
  QVERIFY( crs.createFromString( u"EPSG:3113"_s ) );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3113"_s );
  QVERIFY( QgsCoordinateReferenceSystem::stringCache().contains( u"EPSG:3113"_s ) );
  // a second time, so crs is fetched from cache
  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.createFromString( u"EPSG:3113"_s ) );
  QVERIFY( crs2.isValid() );
  QCOMPARE( crs2.authid(), u"EPSG:3113"_s );

  // invalid
  QgsCoordinateReferenceSystem crs3;
  QVERIFY( !crs3.createFromString( u"bad string"_s ) );
  QVERIFY( !crs3.isValid() );
  QVERIFY( QgsCoordinateReferenceSystem::stringCache().contains( u"bad string"_s ) );
  // a second time, so invalid crs is fetched from cache
  QgsCoordinateReferenceSystem crs4;
  QVERIFY( !crs4.createFromString( u"bad string"_s ) );
  QVERIFY( !crs4.isValid() );

  QgsCoordinateReferenceSystem::invalidateCache();
  QVERIFY( !QgsCoordinateReferenceSystem::stringCache().contains( u"EPSG:3113"_s ) );
}

void TestQgsCoordinateReferenceSystem::fromProjObject()
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProjObject( nullptr );
  QVERIFY( !crs.isValid() );

  // test creating a QgsCoordinateReferenceSystem from a proj object which is custom crs
  const QString def = u"+proj=ortho +lat_0=-20 +lon_0=11.11111 +x_0=0 +y_0=0 +R=6371000 +units=m +no_defs +type=crs"_s;
  const QgsProjUtils::proj_pj_unique_ptr pj( proj_create( QgsProjContext::get(), def.toUtf8().constData() ) );
  QVERIFY( pj.get() );

  crs = QgsCoordinateReferenceSystem::fromProjObject( pj.get() );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), QString() );
  QCOMPARE( crs.toProj(), u"+proj=ortho +lat_0=-20 +lon_0=11.11111 +x_0=0 +y_0=0 +R=6371000 +units=m +no_defs +type=crs"_s );
}

void TestQgsCoordinateReferenceSystem::fromProjObjectKnownCrs()
{
  // test creating a QgsCoordinateReferenceSystem from a proj object which is known crs
  const QString crsWkt = QStringLiteral( R"""(
PROJCRS["GDA94 / Vicgrid",
    BASEGEOGCRS["GDA94",
        DATUM["Geocentric Datum of Australia 1994",
            ELLIPSOID["GRS 1980",6378137,298.257222101,
                LENGTHUNIT["metre",1]]],
        PRIMEM["Greenwich",0,
            ANGLEUNIT["degree",0.0174532925199433]],
        ID["EPSG",4283]],
    CONVERSION["Vicgrid",
        METHOD["Lambert Conic Conformal (2SP)",
            ID["EPSG",9802]],
        PARAMETER["Latitude of false origin",-37,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8821]],
        PARAMETER["Longitude of false origin",145,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8822]],
        PARAMETER["Latitude of 1st standard parallel",-36,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8823]],
        PARAMETER["Latitude of 2nd standard parallel",-38,
            ANGLEUNIT["degree",0.0174532925199433],
            ID["EPSG",8824]],
        PARAMETER["Easting at false origin",2500000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8826]],
        PARAMETER["Northing at false origin",2500000,
            LENGTHUNIT["metre",1],
            ID["EPSG",8827]]],
    CS[Cartesian,2],
        AXIS["(E)",east,
            ORDER[1],
            LENGTHUNIT["metre",1]],
        AXIS["(N)",north,
            ORDER[2],
            LENGTHUNIT["metre",1]],
    USAGE[
        SCOPE["State-wide spatial data management."],
        AREA["Australia - Victoria."],
        BBOX[-39.2,140.96,-33.98,150.04]],
    ID["EPSG",3111]]
          )""" );

  const QgsProjUtils::proj_pj_unique_ptr pj( proj_create_from_wkt( QgsProjContext::get(), crsWkt.toUtf8().constData(), nullptr, nullptr, nullptr ) );
  QVERIFY( pj.get() );

  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProjObject( pj.get() );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3111"_s );
}

void TestQgsCoordinateReferenceSystem::fromProjObjectNotCrs()
{
  // test creating a QgsCoordinateReferenceSystem from a proj object which is NOT a crs
  const QString operation = u"+proj=pipeline +step +inv +proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80 +step +proj=utm +zone=56 +south +ellps=GRS80"_s;
  const QgsProjUtils::proj_pj_unique_ptr pj( proj_create( QgsProjContext::get(), operation.toUtf8().constData() ) );
  QVERIFY( pj.get() );

  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProjObject( pj.get() );
  QVERIFY( !crs.isValid() );
}

void TestQgsCoordinateReferenceSystem::isValid()
{
  QgsCoordinateReferenceSystem crs( u"EPSG:4326"_s );
  QVERIFY( crs.isValid() );
  crs = QgsCoordinateReferenceSystem( u"xxxxxxxxxxxxxxx"_s );
  QVERIFY( !crs.isValid() );
}

void TestQgsCoordinateReferenceSystem::validate()
{
  // no validator set
  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );
  QgsCoordinateReferenceSystem myCrs( u"EPSG:28356"_s );
  myCrs.validate();
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:28356"_s );
  myCrs = QgsCoordinateReferenceSystem();
  myCrs.validate();
  // no change, no custom function
  QVERIFY( !myCrs.isValid() );

  QgsCoordinateReferenceSystem::setCustomCrsValidation( &TestQgsCoordinateReferenceSystem::testValidationCrs );
  myCrs.validate();
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.authid(), u"EPSG:3111"_s );
  QVERIFY( sValidateCalled );
  sValidateCalled = false;

  myCrs = QgsCoordinateReferenceSystem();
  QgsCoordinateReferenceSystem::setCustomCrsValidation( &TestQgsCoordinateReferenceSystem::testNoActionValidationCrs );
  myCrs.validate();
  QVERIFY( !myCrs.isValid() );
  QVERIFY( sValidateCalled );

  QgsCoordinateReferenceSystem::setCustomCrsValidation( nullptr );
}

void TestQgsCoordinateReferenceSystem::comparison_data()
{
  QTest::addColumn<QgsCoordinateReferenceSystem>( "lhs" );
  QTest::addColumn<QgsCoordinateReferenceSystem>( "rhs" );
  QTest::addColumn<bool>( "equal" );

  QgsCoordinateReferenceSystem noAuthId;
  noAuthId.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.1996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );

  QgsCoordinateReferenceSystem noAuthId2;
  noAuthId2.createFromProj( u"+proj=ortho +lat_0=47.686 +lon_0=16.548 +x_0=0 +y_0=0 +datum=WGS84 +units=m +no_defs +type=crs"_s );

  const QgsCoordinateReferenceSystem epsg4326( u"EPSG:4326"_s );
  const QgsCoordinateReferenceSystem epsg3111( u"EPSG:3111"_s );

  QgsCoordinateReferenceSystem userCrs1;
  userCrs1.createFromProj( u"+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10.123 +y_0=3 +datum=WGS84 +units=m +no_defs"_s );
  userCrs1.saveAsUserCrs( u"test1"_s );
  Q_ASSERT( userCrs1.isValid() );

  QgsCoordinateReferenceSystem userCrs2;
  userCrs2.createFromProj( u"+proj=aea +lat_1=21 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10.123 +y_0=3 +datum=WGS84 +units=m +no_defs"_s );
  userCrs2.saveAsUserCrs( u"test2"_s );
  Q_ASSERT( userCrs2.isValid() );

  // lhs is always < rhs. Or equal is set to true
  QTest::newRow( "invalid == invalid" ) << QgsCoordinateReferenceSystem() << QgsCoordinateReferenceSystem() << true;
  QTest::newRow( "epsg4326 == epsg4326" ) << epsg4326 << epsg4326 << true;
  QTest::newRow( "user == user" ) << userCrs1 << userCrs1 << true;
  QTest::newRow( "noAuthId == noAuthId" ) << noAuthId << noAuthId << true;
  QTest::newRow( "noAuthId == noAuthId" ) << noAuthId2 << noAuthId2 << true;
  QTest::newRow( "noAuthId2 < noAuthId" ) << noAuthId2 << noAuthId << false;

  QTest::newRow( "invalid < epsg4326" ) << QgsCoordinateReferenceSystem() << epsg4326 << false;
  QTest::newRow( "invalid < user" ) << QgsCoordinateReferenceSystem() << userCrs1 << false;
  QTest::newRow( "invalid < noAuthId" ) << QgsCoordinateReferenceSystem() << noAuthId << false;

  QTest::newRow( "epsg3111 < epsg4326" ) << epsg3111 << epsg4326 << false;
  QTest::newRow( "user1 < user2" ) << userCrs1 << userCrs2 << false;
}

void TestQgsCoordinateReferenceSystem::comparison()
{
  QFETCH( QgsCoordinateReferenceSystem, lhs );
  QFETCH( QgsCoordinateReferenceSystem, rhs );
  QFETCH( bool, equal );

  if ( equal )
  {
    QVERIFY( lhs <= rhs );
    QVERIFY( lhs >= rhs );
    QVERIFY( !( lhs > rhs ) );
    QVERIFY( !( lhs < rhs ) );

    if ( lhs.isValid() )
    {
      rhs.setCoordinateEpoch( 2021.2 );
      QVERIFY( lhs <= rhs );
      QVERIFY( rhs >= lhs );
      QVERIFY( lhs < rhs );
      QVERIFY( rhs > lhs );

      lhs.setCoordinateEpoch( 2021.1 );
      QVERIFY( lhs <= rhs );
      QVERIFY( rhs >= lhs );
      QVERIFY( lhs < rhs );
      QVERIFY( rhs > lhs );

      rhs.setCoordinateEpoch( std::numeric_limits<double>::quiet_NaN() );
      QVERIFY( lhs >= rhs );
      QVERIFY( rhs <= lhs );
      QVERIFY( lhs > rhs );
      QVERIFY( rhs < lhs );
    }
  }
  else
  {
    QVERIFY( lhs <= rhs );
    QVERIFY( rhs >= lhs );
    QVERIFY( lhs < rhs );
    QVERIFY( rhs > lhs );
  }
}

void TestQgsCoordinateReferenceSystem::equality()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QgsCoordinateReferenceSystem myCrs2( u"EPSG:4326"_s );
  const QgsCoordinateReferenceSystem myCrs3 = myCrs2;
  QVERIFY( myCrs == myCrs2 );
  QVERIFY( myCrs3 == myCrs2 );
  QVERIFY( QgsCoordinateReferenceSystem() == QgsCoordinateReferenceSystem() );

  // with coordinate epoch
  myCrs.setCoordinateEpoch( 2021.3 );
  QVERIFY( !( myCrs == myCrs2 ) );
  myCrs2.setCoordinateEpoch( 2021.3 );
  QVERIFY( myCrs == myCrs2 );
  myCrs.setCoordinateEpoch( std::numeric_limits<double>::quiet_NaN() );
  myCrs2.setCoordinateEpoch( std::numeric_limits<double>::quiet_NaN() );

  // custom crs, no authid
  myCrs.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.1996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );
  myCrs2.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.1996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );
  QVERIFY( myCrs == myCrs2 );
}

void TestQgsCoordinateReferenceSystem::noEquality()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QgsCoordinateReferenceSystem myCrs2( u"EPSG:4327"_s );
  QgsCoordinateReferenceSystem myCrs3( u"EPSG:4326"_s );
  QVERIFY( myCrs != myCrs2 );
  QVERIFY( myCrs != QgsCoordinateReferenceSystem() );
  QVERIFY( QgsCoordinateReferenceSystem() != myCrs );

  // with coordinate epoch
  myCrs.setCoordinateEpoch( 2021.3 );
  QVERIFY( myCrs != myCrs3 );
  myCrs3.setCoordinateEpoch( 2021.3 );
  QVERIFY( !( myCrs != myCrs3 ) );
  myCrs.setCoordinateEpoch( std::numeric_limits<double>::quiet_NaN() );
  myCrs3.setCoordinateEpoch( std::numeric_limits<double>::quiet_NaN() );

  // custom crs, no authid
  myCrs.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.1996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );
  myCrs2.createFromWkt( u"PROJCS[\"unnamed\",GEOGCS[\"unnamed\",DATUM[\"Geocentric_Datum_of_Australia_2020\",SPHEROID[\"GRS 80\",6378137,298.257222101]],PRIMEM[\"Greenwich\",0],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]]],PROJECTION[\"Transverse_Mercator\"],PARAMETER[\"latitude_of_origin\",0],PARAMETER[\"central_meridian\",147],PARAMETER[\"scale_factor\",0.2996],PARAMETER[\"false_easting\",500000],PARAMETER[\"false_northing\",10000000],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AXIS[\"Easting\",EAST],AXIS[\"Northing\",NORTH]]"_s );
  QVERIFY( myCrs != myCrs2 );

  // fake as though the myCrs and myCrs2 were saved custom crses, with the same authid but different wkt
  // ie. one was created before the user modified the custom crs definition.
  // they should be reported as different!
  myCrs.d->mAuthId = u"USER:100001"_s;
  myCrs.d->mSrsId = 100001;
  myCrs2.d->mAuthId = u"USER:100001"_s;
  myCrs2.d->mSrsId = 100001;
  QVERIFY( myCrs != myCrs2 );
}

void TestQgsCoordinateReferenceSystem::equalityInvalid()
{
  const QgsCoordinateReferenceSystem invalidCrs1;
  const QgsCoordinateReferenceSystem invalidCrs2;
  QVERIFY( invalidCrs1 == invalidCrs2 );
}
void TestQgsCoordinateReferenceSystem::readWriteXml()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QVERIFY( myCrs.isValid() );
  QDomDocument document( u"test"_s );
  QDomElement node = document.createElement( u"crs"_s );
  document.appendChild( node );

  // start with invalid node
  QgsCoordinateReferenceSystem badCrs;
  QVERIFY( !badCrs.readXml( node ) );
  QVERIFY( !badCrs.isValid() );
  const QDomElement badSrsElement = document.createElement( u"spatialrefsys"_s );
  QDomElement badNode = document.createElement( u"crs"_s );
  document.appendChild( badNode );
  badNode.appendChild( badSrsElement );
  // should return true, because it's ok to write/read invalid crs to xml
  QVERIFY( badCrs.readXml( badNode ) );
  QVERIFY( !badCrs.isValid() );

  QVERIFY( myCrs.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs2;
  QVERIFY( myCrs2.readXml( node ) );
  QVERIFY( myCrs == myCrs2 );
  QVERIFY( std::isnan( myCrs2.coordinateEpoch() ) );

  // with coordinate epoch
  myCrs.setCoordinateEpoch( 2021.3 );
  QDomElement node1a = document.createElement( u"crs"_s );
  document.appendChild( node1a );
  QVERIFY( myCrs.writeXml( node1a, document ) );
  const QgsCoordinateReferenceSystem myCrs2a;
  QVERIFY( myCrs2.readXml( node1a ) );
  QVERIFY( myCrs == myCrs2 );

  // Empty XML made from writeXml operation
  const QgsCoordinateReferenceSystem myCrs3;
  QDomDocument document2( u"test"_s );
  QDomElement node2 = document2.createElement( u"crs"_s );
  document2.appendChild( node2 );
  QVERIFY( !myCrs3.isValid() );
  QVERIFY( myCrs3.writeXml( node2, document2 ) );
  QgsCoordinateReferenceSystem myCrs4;
  QVERIFY( myCrs4.readXml( node2 ) );
  QVERIFY( !myCrs4.isValid() );
  QVERIFY( myCrs3 == myCrs4 );

  // Empty XML node
  QDomDocument document3( u"test"_s );
  const QDomElement node3 = document3.createElement( u"crs"_s );
  document3.appendChild( node3 );
  QgsCoordinateReferenceSystem myCrs5;
  QVERIFY( !myCrs5.readXml( node3 ) );
  QVERIFY( myCrs5 == QgsCoordinateReferenceSystem() );

  // valid CRS using auth/code
  QgsCoordinateReferenceSystem myCrs6( u"EPSG:3111"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs6.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs7;
  QVERIFY( myCrs7.readXml( node ) );
  QCOMPARE( myCrs7.authid(), u"EPSG:3111"_s );
  QCOMPARE( myCrs7.toProj().left( 92 ), u"+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80"_s );
  QCOMPARE( myCrs7.toWkt(), QStringLiteral( R"""(PROJCS["GDA94 / Vicgrid",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",-37],PARAMETER["central_meridian",145],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["false_easting",2500000],PARAMETER["false_northing",2500000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","3111"]])""" ) );
  // with coordinate epoch
  myCrs6.setCoordinateEpoch( 2021.3 );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs6.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs7a;
  QVERIFY( myCrs7a.readXml( node ) );
  QCOMPARE( myCrs7a.coordinateEpoch(), 2021.3 );

  // valid CRS from proj string
  QgsCoordinateReferenceSystem myCrs8;
  myCrs8.createFromProj( u"+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10.123 +y_0=3 +datum=WGS84 +units=m +no_defs"_s );
  myCrs8.saveAsUserCrs( u"test"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs8.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs9;
  QVERIFY( myCrs9.readXml( node ) );
  myCrs9.saveAsUserCrs( u"test2"_s );

  QCOMPARE( myCrs9.authid(), u"USER:%1"_s.arg( myCrs9.srsid() ) );
  QCOMPARE( myCrs9.toProj(), u"+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10.123 +y_0=3 +datum=WGS84 +units=m +no_defs"_s );
  QCOMPARE( myCrs9.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), QStringLiteral( R"""(PROJCRS["unknown",BASEGEOGCRS["unknown",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]],ID["EPSG",6326]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8901]]],CONVERSION["unknown",METHOD["Albers Equal Area",ID["EPSG",9822]],PARAMETER["Latitude of false origin",4,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Longitude of false origin",29,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Latitude of 1st standard parallel",20,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Latitude of 2nd standard parallel",-23,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Easting at false origin",10.123,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",3,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["(E)",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["(N)",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]])""" ) );

  // valid CRS from WKT string
  QgsCoordinateReferenceSystem myCrs10;
  myCrs10.createFromWkt( QStringLiteral( R"""(PROJCS["xxx",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[1,2,3,4,5,6,7],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37.2],PARAMETER["central_meridian",145.1],PARAMETER["false_easting",2510000],PARAMETER["false_northing",2520000],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" ) );
  myCrs10.saveAsUserCrs( u"test3"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs10.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs11;
  QVERIFY( myCrs11.readXml( node ) );
  myCrs11.saveAsUserCrs( u"test4"_s );
  QCOMPARE( myCrs11.authid(), u"USER:%1"_s.arg( myCrs11.srsid() ) );
  QCOMPARE( myCrs11.toProj(), u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2510000 +y_0=2520000 +ellps=GRS80 +towgs84=1,2,3,4,5,6,7 +units=m +no_defs +type=crs"_s );
  QCOMPARE( myCrs11.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), QStringLiteral( R"""(BOUNDCRS[SOURCECRS[PROJCRS["xxx",BASEGEOGCRS["GDA94",DATUM["Geocentric Datum of Australia 1994",ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4283]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Latitude of 1st standard parallel",-36,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Latitude of 2nd standard parallel",-38,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Latitude of false origin",-37.2,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Longitude of false origin",145.1,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Easting at false origin",2510000,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",2520000,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["northing",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]],TARGETCRS[GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4326]]],ABRIDGEDTRANSFORMATION["Transformation from GDA94 to WGS84",METHOD["Position Vector transformation (geog2D domain)",ID["EPSG",9606]],PARAMETER["X-axis translation",1,ID["EPSG",8605]],PARAMETER["Y-axis translation",2,ID["EPSG",8606]],PARAMETER["Z-axis translation",3,ID["EPSG",8607]],PARAMETER["X-axis rotation",4,ID["EPSG",8608]],PARAMETER["Y-axis rotation",5,ID["EPSG",8609]],PARAMETER["Z-axis rotation",6,ID["EPSG",8610]],PARAMETER["Scale difference",1.000007,ID["EPSG",8611]]]])""" ) );

  // try reloading, make sure it gets the same user crs assigned
  QgsCoordinateReferenceSystem myCrs11b;
  QVERIFY( myCrs11b.readXml( node ) );
  myCrs11b.saveAsUserCrs( u"test4"_s );
  QCOMPARE( myCrs11b.authid(), u"USER:%1"_s.arg( myCrs11b.srsid() ) );
  QCOMPARE( myCrs11b.toProj(), u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2510000 +y_0=2520000 +ellps=GRS80 +towgs84=1,2,3,4,5,6,7 +units=m +no_defs +type=crs"_s );
  QCOMPARE( myCrs11b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), QStringLiteral( R"""(BOUNDCRS[SOURCECRS[PROJCRS["xxx",BASEGEOGCRS["GDA94",DATUM["Geocentric Datum of Australia 1994",ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4283]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Latitude of 1st standard parallel",-36,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Latitude of 2nd standard parallel",-38,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Latitude of false origin",-37.2,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Longitude of false origin",145.1,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Easting at false origin",2510000,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",2520000,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["northing",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]],TARGETCRS[GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4326]]],ABRIDGEDTRANSFORMATION["Transformation from GDA94 to WGS84",METHOD["Position Vector transformation (geog2D domain)",ID["EPSG",9606]],PARAMETER["X-axis translation",1,ID["EPSG",8605]],PARAMETER["Y-axis translation",2,ID["EPSG",8606]],PARAMETER["Z-axis translation",3,ID["EPSG",8607]],PARAMETER["X-axis rotation",4,ID["EPSG",8608]],PARAMETER["Y-axis rotation",5,ID["EPSG",8609]],PARAMETER["Z-axis rotation",6,ID["EPSG",8610]],PARAMETER["Scale difference",1.000007,ID["EPSG",8611]]]])""" ) );

  // fudge an dom element without the wkt element
  QgsCoordinateReferenceSystem myCrs12;
  myCrs12.createFromWkt( QStringLiteral( R"""(PROJCS["xxx",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[1,2,3,4,5,6,7],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37.2],PARAMETER["central_meridian",145.1],PARAMETER["false_easting",2510000],PARAMETER["false_northing",2520000],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" ) );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs12.writeXml( node, document ) );
  QDomNodeList nodeList = node.toElement().elementsByTagName( u"wkt"_s );
  nodeList.at( 0 ).parentNode().removeChild( nodeList.at( 0 ) );
  QgsCoordinateReferenceSystem myCrs13;
  QVERIFY( myCrs13.readXml( node ) );
  myCrs13.saveAsUserCrs( u"test6"_s );
  QCOMPARE( myCrs13.authid(), u"USER:%1"_s.arg( myCrs13.srsid() ) );
  QCOMPARE( myCrs13.toProj(), u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2510000 +y_0=2520000 +ellps=GRS80 +towgs84=1,2,3,4,5,6,7 +units=m +no_defs"_s );
  QgsDebugMsgLevel( myCrs13.toWkt(), 1 );
  QVERIFY( myCrs13.toWkt().contains( QLatin1String( R"""(SPHEROID["GRS 1980",)""" ) ) );
  QVERIFY( myCrs13.toWkt().contains( QLatin1String( R"""(TOWGS84[1,2,3,4,5,6,7])""" ) ) );
  QVERIFY( myCrs13.toWkt().contains( QLatin1String( R"""(PROJECTION["Lambert_Conformal_Conic_2SP"])""" ) ) );
  QVERIFY( myCrs13.toWkt().contains( QLatin1String( R"""(PARAMETER["latitude_of_origin",-37.2],PARAMETER["central_meridian",145.1],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["false_easting",2510000],PARAMETER["false_northing",2520000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH])""" ) ) );

  // fudge a dom element with conflicting proj and wkt, wkt should be preferred
  QgsCoordinateReferenceSystem myCrs14;
  myCrs14.createFromWkt( QStringLiteral( R"""(PROJCS["xxx",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[1,2,3,4,5,6,7],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37.2],PARAMETER["central_meridian",145.1],PARAMETER["false_easting",2510000],PARAMETER["false_northing",2520000],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" ) );
  myCrs14.saveAsUserCrs( u"test5"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs14.writeXml( node, document ) );
  nodeList = node.toElement().elementsByTagName( u"proj4"_s );
  nodeList.at( 0 ).parentNode().removeChild( nodeList.at( 0 ) );
  QDomElement proj4Element = document.createElement( u"proj4"_s );
  proj4Element.appendChild( document.createTextNode( u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2530000 +y_0=2540000 +ellps=GRS80  +units=m +no_defs"_s ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( proj4Element );
  QgsCoordinateReferenceSystem myCrs15;
  QVERIFY( myCrs15.readXml( node ) );
  myCrs15.saveAsUserCrs( u"test6"_s );
  QCOMPARE( myCrs15.authid(), u"USER:%1"_s.arg( myCrs15.srsid() ) );
  QCOMPARE( myCrs15.toProj(), u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2510000 +y_0=2520000 +ellps=GRS80 +towgs84=1,2,3,4,5,6,7 +units=m +no_defs +type=crs"_s );
  QCOMPARE( myCrs15.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), QStringLiteral( R"""(BOUNDCRS[SOURCECRS[PROJCRS["xxx",BASEGEOGCRS["GDA94",DATUM["Geocentric Datum of Australia 1994",ELLIPSOID["GRS 1980",6378137,298.257222101,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4283]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Latitude of 1st standard parallel",-36,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Latitude of 2nd standard parallel",-38,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]],PARAMETER["Latitude of false origin",-37.2,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Longitude of false origin",145.1,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Easting at false origin",2510000,LENGTHUNIT["metre",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",2520000,LENGTHUNIT["metre",1],ID["EPSG",8827]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["northing",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]],TARGETCRS[GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["geodetic latitude (Lat)",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["geodetic longitude (Lon)",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4326]]],ABRIDGEDTRANSFORMATION["Transformation from GDA94 to WGS84",METHOD["Position Vector transformation (geog2D domain)",ID["EPSG",9606]],PARAMETER["X-axis translation",1,ID["EPSG",8605]],PARAMETER["Y-axis translation",2,ID["EPSG",8606]],PARAMETER["Z-axis translation",3,ID["EPSG",8607]],PARAMETER["X-axis rotation",4,ID["EPSG",8608]],PARAMETER["Y-axis rotation",5,ID["EPSG",8609]],PARAMETER["Z-axis rotation",6,ID["EPSG",8610]],PARAMETER["Scale difference",1.000007,ID["EPSG",8611]]]])""" ) );

  // fudge a dom element with auth/code and conflicting proj, auth/code should be preferred
  const QgsCoordinateReferenceSystem myCrs16( u"EPSG:3111"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs16.writeXml( node, document ) );
  nodeList = node.toElement().elementsByTagName( u"proj4"_s );
  nodeList.at( 0 ).parentNode().removeChild( nodeList.at( 0 ) );
  proj4Element = document.createElement( u"proj4"_s );
  proj4Element.appendChild( document.createTextNode( u"+proj=lcc +lat_0=-37.2 +lon_0=145.1 +lat_1=-36 +lat_2=-38 +x_0=2530000 +y_0=2540000 +ellps=GRS80  +units=m +no_defs"_s ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( proj4Element );
  QgsCoordinateReferenceSystem myCrs17;
  QVERIFY( myCrs17.readXml( node ) );
  QCOMPARE( myCrs17.authid(), u"EPSG:3111"_s );
  QCOMPARE( myCrs17.toProj().left( 92 ), u"+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80"_s );
  QCOMPARE( myCrs17.toWkt(), QStringLiteral( R"""(PROJCS["GDA94 / Vicgrid",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",-37],PARAMETER["central_meridian",145],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["false_easting",2500000],PARAMETER["false_northing",2500000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","3111"]])""" ) );

  // fudge a dom element with auth/code and conflicting wkt, auth/code should be preferred
  const QgsCoordinateReferenceSystem myCrs18( u"EPSG:3111"_s );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs18.writeXml( node, document ) );
  nodeList = node.toElement().elementsByTagName( u"wkt"_s );
  nodeList.at( 0 ).parentNode().removeChild( nodeList.at( 0 ) );
  QDomElement wktElement = document.createElement( u"wkt"_s );
  wktElement.appendChild( document.createTextNode( QStringLiteral( R"""(PROJCS["xxx",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[1,2,3,4,5,6,7],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37.2],PARAMETER["central_meridian",145.1],PARAMETER["false_easting",2510000],PARAMETER["false_northing",2520000],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" ) ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( wktElement );
  QgsCoordinateReferenceSystem myCrs19;
  QVERIFY( myCrs19.readXml( node ) );
  QCOMPARE( myCrs19.authid(), u"EPSG:3111"_s );
  QCOMPARE( myCrs19.toProj().left( 92 ), u"+proj=lcc +lat_0=-37 +lon_0=145 +lat_1=-36 +lat_2=-38 +x_0=2500000 +y_0=2500000 +ellps=GRS80"_s );
  QCOMPARE( myCrs19.toWkt(), QStringLiteral( R"""(PROJCS["GDA94 / Vicgrid",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["latitude_of_origin",-37],PARAMETER["central_meridian",145],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["false_easting",2500000],PARAMETER["false_northing",2500000],UNIT["metre",1,AUTHORITY["EPSG","9001"]],AXIS["Easting",EAST],AXIS["Northing",NORTH],AUTHORITY["EPSG","3111"]])""" ) );

  // valid CRS from WKT string, not matching a local user CRS
  QgsCoordinateReferenceSystem myCrs20;
  myCrs20.createFromWkt( QStringLiteral( R"""(PROJCS["",GEOGCS["GDA94",DATUM["Geocentric_Datum_of_Australia_1994",SPHEROID["GRS 1980",6378137,298.257222101,AUTHORITY["EPSG","7019"]],TOWGS84[1,2,3,4,5,16,17],AUTHORITY["EPSG","6283"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.01745329251994328,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4283"]],UNIT["metre",1,AUTHORITY["EPSG","9001"]],PROJECTION["Lambert_Conformal_Conic_2SP"],PARAMETER["standard_parallel_1",-36],PARAMETER["standard_parallel_2",-38],PARAMETER["latitude_of_origin",-37.5],PARAMETER["central_meridian",145.5],PARAMETER["false_easting",2533000],PARAMETER["false_northing",2533000],AXIS["Easting",EAST],AXIS["Northing",NORTH]])""" ) );
  QVERIFY( myCrs20.isValid() );
  node = document.createElement( u"crs"_s );
  document.appendChild( node );
  QVERIFY( myCrs20.writeXml( node, document ) );
  QgsCoordinateReferenceSystem myCrs21;
  QVERIFY( myCrs21.readXml( node ) );
  QVERIFY( myCrs21.isValid() );
  QgsDebugMsgLevel( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(BOUNDCRS[SOURCECRS[PROJCRS["")""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",1,ID["EPSG",8605]],PARAMETER["Y-axis translation",2,ID["EPSG",8606]],PARAMETER["Z-axis translation",3,ID["EPSG",8607]],PARAMETER["X-axis rotation",4,ID["EPSG",8608]],PARAMETER["Y-axis rotation",5,ID["EPSG",8609]],PARAMETER["Z-axis rotation",16,ID["EPSG",8610]])""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of 1st standard parallel",-36,)""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of 2nd standard parallel",-38,)""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of false origin",-37.5,)""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Longitude of false origin",145.5,)""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Easting at false origin",2533000,)""" ) ) );
  QVERIFY( myCrs21.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Northing at false origin",2533000,)""" ) ) );
  QCOMPARE( myCrs21.description(), QString() );

  // now fudge in the description node to mimic as though the XML came from a different QGIS install where this CRS was a user-defined CRS with a name
  QDomElement descriptionElement = document.createElement( u"description"_s );
  descriptionElement.appendChild( document.createTextNode( u"someone else's previously saved CRS"_s ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().removeChild( node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().elementsByTagName( u"description"_s ).at( 0 ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( descriptionElement );
  QgsCoordinateReferenceSystem myCrs22;
  QVERIFY( myCrs22.readXml( node ) );
  QVERIFY( myCrs22.isValid() );
  QgsDebugMsgLevel( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );

  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(BOUNDCRS[SOURCECRS[PROJCRS["")""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",1,ID["EPSG",8605]],PARAMETER["Y-axis translation",2,ID["EPSG",8606]],PARAMETER["Z-axis translation",3,ID["EPSG",8607]],PARAMETER["X-axis rotation",4,ID["EPSG",8608]],PARAMETER["Y-axis rotation",5,ID["EPSG",8609]],PARAMETER["Z-axis rotation",16,ID["EPSG",8610]])""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of 1st standard parallel",-36,)""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of 2nd standard parallel",-38,)""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of false origin",-37.5,)""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Longitude of false origin",145.5,)""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Easting at false origin",2533000,)""" ) ) );
  QVERIFY( myCrs22.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Northing at false origin",2533000,)""" ) ) );

  // description should be restored, even though it's not a user-defined CRS on this install...
  QCOMPARE( myCrs22.description(), u"someone else's previously saved CRS"_s );

  // a different WKT string, which doesn't match any CRS previously used this session
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().removeChild( node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().elementsByTagName( u"wkt"_s ).at( 0 ) );
  QDomElement wktElementNew = document.createElement( u"wkt"_s );
  wktElementNew.appendChild( document.createTextNode( R"""(PROJCRS["",BASEGEOGCRS["unknown",DATUM["Unknown based on WGS84 ellipsoid",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1],ID["EPSG",7030]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8901]]],CONVERSION["unknown",METHOD["Hotine Oblique Mercator (variant B)",ID["EPSG",9815]],PARAMETER["Latitude of projection centre",47.173836897,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8811]],PARAMETER["Longitude of projection centre",8.4550705414,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8812]],PARAMETER["Azimuth of initial line",39.3,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8813]],PARAMETER["Angle from Rectified to Skew Grid",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8814]],PARAMETER["Scale factor on initial line",1,SCALEUNIT["unity",1],ID["EPSG",8815]],PARAMETER["Easting at projection centre",750,LENGTHUNIT["metre",1],ID["EPSG",8816]],PARAMETER["Northing at projection centre",250,LENGTHUNIT["metre",1],ID["EPSG",8817]]],CS[Cartesian,2],AXIS["(E)",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["(N)",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]])""" ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( wktElementNew );
  QDomElement descriptionElementB = document.createElement( u"description"_s );
  descriptionElementB.appendChild( document.createTextNode( u"a new CRS"_s ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().removeChild( node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().elementsByTagName( u"description"_s ).at( 0 ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( descriptionElementB );
  QgsCoordinateReferenceSystem myCrs21b;
  QVERIFY( myCrs21b.readXml( node ) );
  QVERIFY( myCrs21b.isValid() );
  QgsDebugMsgLevel( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PROJCRS["",)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(BASEGEOGCRS["unknown",DATUM["Unknown based on WGS84 ellipsoid",)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(CONVERSION["unknown")""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Hotine Oblique Mercator (variant B)",)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of projection centre",47.173836897)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Longitude of projection centre",8.4550705414,)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Azimuth of initial line",39.3,)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Angle from Rectified to Skew Grid",0,)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Scale factor on initial line",1,)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Easting at projection centre",750,)""" ) ) );
  QVERIFY( myCrs21b.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Northing at projection centre",250,)""" ) ) );
  QCOMPARE( myCrs21b.description(), u"a new CRS"_s );

  // a different WKT string, which doesn't match any CRS previously used this session, and which includes a name in the WKT but which should be overridden with the user set name
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().removeChild( node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().elementsByTagName( u"wkt"_s ).at( 0 ) );
  QDomElement wktElementNewC = document.createElement( u"wkt"_s );
  wktElementNewC.appendChild( document.createTextNode( R"""(PROJCRS["XXYYZZ",BASEGEOGCRS["unknown",DATUM["Unknown based on WGS84 ellipsoid",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1],ID["EPSG",7030]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8901]]],CONVERSION["unknown",METHOD["Hotine Oblique Mercator (variant B)",ID["EPSG",9815]],PARAMETER["Latitude of projection centre",47.2,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8811]],PARAMETER["Longitude of projection centre",8.4550705414,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8812]],PARAMETER["Azimuth of initial line",39.3,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8813]],PARAMETER["Angle from Rectified to Skew Grid",0,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8814]],PARAMETER["Scale factor on initial line",1,SCALEUNIT["unity",1],ID["EPSG",8815]],PARAMETER["Easting at projection centre",750,LENGTHUNIT["metre",1],ID["EPSG",8816]],PARAMETER["Northing at projection centre",250,LENGTHUNIT["metre",1],ID["EPSG",8817]]],CS[Cartesian,2],AXIS["(E)",east,ORDER[1],LENGTHUNIT["metre",1,ID["EPSG",9001]]],AXIS["(N)",north,ORDER[2],LENGTHUNIT["metre",1,ID["EPSG",9001]]]]])""" ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( wktElementNewC );
  QDomElement descriptionElementC = document.createElement( u"description"_s );
  descriptionElementC.appendChild( document.createTextNode( u"a new CRS C"_s ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().removeChild( node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().elementsByTagName( u"description"_s ).at( 0 ) );
  node.toElement().elementsByTagName( u"spatialrefsys"_s ).at( 0 ).toElement().appendChild( descriptionElementC );
  QgsCoordinateReferenceSystem myCrs21c;
  QVERIFY( myCrs21c.readXml( node ) );
  QVERIFY( myCrs21c.isValid() );
  QgsDebugMsgLevel( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );

  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PROJCRS["XXYYZZ",)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(BASEGEOGCRS["unknown",DATUM["Unknown based on WGS84 ellipsoid",)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(CONVERSION["unknown")""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Hotine Oblique Mercator (variant B)",)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Latitude of projection centre",47.2)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Longitude of projection centre",8.4550705414,)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Azimuth of initial line",39.3,)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Angle from Rectified to Skew Grid",0,)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Scale factor on initial line",1,)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Easting at projection centre",750,)""" ) ) );
  QVERIFY( myCrs21c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["Northing at projection centre",250,)""" ) ) );
  QCOMPARE( myCrs21c.description(), u"a new CRS C"_s );
}

void TestQgsCoordinateReferenceSystem::readWriteXmlNativeFormatWkt()
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=ortho +lat_0=-11 +lon_0=34 +x_0=0 +y_0=0 +ellps=sphere +units=m +no_defs +type=crs"_s );
  crs.setNativeFormat( Qgis::CrsDefinitionFormat::Wkt );

  QDomDocument document( u"test"_s );
  QDomElement node = document.createElement( u"crs"_s );
  document.appendChild( node );

  QVERIFY( crs.writeXml( node, document ) );

  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.readXml( node ) );
  QVERIFY( crs == crs2 );
  QCOMPARE( crs2.nativeFormat(), Qgis::CrsDefinitionFormat::Wkt );
}

void TestQgsCoordinateReferenceSystem::readWriteXmlNativeFormatProj()
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=ortho +lat_0=-11.6 +lon_0=34 +x_0=0 +y_0=0 +ellps=sphere +units=m +no_defs +type=crs"_s );
  crs.setNativeFormat( Qgis::CrsDefinitionFormat::Proj );

  QDomDocument document( u"test"_s );
  QDomElement node = document.createElement( u"crs"_s );
  document.appendChild( node );

  QVERIFY( crs.writeXml( node, document ) );

  QgsCoordinateReferenceSystem crs2;
  QVERIFY( crs2.readXml( node ) );
  QVERIFY( crs == crs2 );
  QCOMPARE( crs2.nativeFormat(), Qgis::CrsDefinitionFormat::Proj );
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
  *  "QgsCoordinateReferenceSystem myCrs;
  *  static CUSTOM_CRS_VALIDATION customSrsValidation();
  *  QVERIFY( myCrs.isValid() );
  */
}
void TestQgsCoordinateReferenceSystem::postgisSrid()
{
  QgsCoordinateReferenceSystem myCrs;
  Q_NOWARN_DEPRECATED_PUSH
  myCrs.createFromSrid( 4326 );
  Q_NOWARN_DEPRECATED_POP
  QVERIFY( myCrs.postgisSrid() == 4326 );
  debugPrint( myCrs );
}
void TestQgsCoordinateReferenceSystem::ellipsoidAcronym()
{
  QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7030"_s );

  myCrs.createFromString( u"EPSG:2951"_s );
  QCOMPARE( myCrs.ellipsoidAcronym(), u"EPSG:7019"_s );
}
void TestQgsCoordinateReferenceSystem::toWkt()
{
  const QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  const QString myWkt = myCrs.toWkt();
  const QString myStrippedWkt( "GEOGCS[\"WGS 84\",DATUM[\"WGS_1984\",SPHEROID"
                               "[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
                               "AUTHORITY[\"EPSG\",\"6326\"]],PRIMEM[\"Greenwich\",0,AUTHORITY"
                               "[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY"
                               "[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4326\"]]" );
  QCOMPARE( myWkt, myStrippedWkt );
}
void TestQgsCoordinateReferenceSystem::toProj()
{
  const QgsCoordinateReferenceSystem myCrs( u"EPSG:4326"_s );
  //first proj string produced by gdal 1.8-1.9
  //second by gdal 1.7
  QCOMPARE( myCrs.toProj(), u"+proj=longlat +datum=WGS84 +no_defs"_s );
}
void TestQgsCoordinateReferenceSystem::isGeographic()
{
  const QgsCoordinateReferenceSystem geographic( u"EPSG:4326"_s );
  QVERIFY( geographic.isGeographic() );

  const QgsCoordinateReferenceSystem nonGeographic( u"EPSG:3857"_s );
  QVERIFY( !nonGeographic.isGeographic() );
}
void TestQgsCoordinateReferenceSystem::mapUnits()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.createFromString( u"EPSG:4326"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Degrees );
  debugPrint( myCrs );
  myCrs.createFromString( u"EPSG:28355"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Meters );
  debugPrint( myCrs );
  myCrs.createFromString( u"EPSG:26812"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::FeetUSSurvey );
  myCrs.createFromString( u"EPSG:4619"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Degrees );

  // compound CRS
  myCrs.createFromString( u"EPSG:5500"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Degrees );

  // vertical CRS
  myCrs = QgsCoordinateReferenceSystem( u"EPSG:5703"_s );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Meters );

  // custom CRS using "m" unit keyword
  myCrs.createFromWkt( QStringLiteral( R"""(PROJCS["MGI / Austria Lambert", GEOGCS["MGI", DATUM["Militar-Geographische Institut", SPHEROID["Bessel 1841", 6377397.155, 299.1528128, AUTHORITY["EPSG","7004"]], TOWGS84[601.705, 84.263, 485.227, 4.7354, -1.3145, -5.393, -2.3887], AUTHORITY["EPSG","6312"]], PRIMEM["Greenwich", 0.0, AUTHORITY["EPSG","8901"]], UNIT["degree", 0.017453292519943295], AXIS["Geodetic longitude", EAST], AXIS["Geodetic latitude", NORTH], AUTHORITY["EPSG","4312"]], PROJECTION["Lambert_Conformal_Conic_2SP", AUTHORITY["EPSG","9802"]], PARAMETER["central_meridian", 13.333333333333336], PARAMETER["latitude_of_origin", 47.5], PARAMETER["standard_parallel_1", 48.99999999999999], PARAMETER["false_easting", 400000.0], PARAMETER["false_northing", 400000.0], PARAMETER["scale_factor", 1.0], PARAMETER["standard_parallel_2", 46.0], UNIT["m", 1.0], AXIS["Easting", EAST], AXIS["Northing", NORTH], AUTHORITY["EPSG","31287"]])""" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Meters );

  // bound CRS using "m" unit keyword
  myCrs.createFromWkt( QStringLiteral( R"""(BOUNDCRS[SOURCECRS[PROJCRS["MGI / Austria Lambert",BASEGEOGCRS["MGI",DATUM["Militar-Geographische Institut",ELLIPSOID["Bessel 1841",6377397.155,299.1528128,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]]],CONVERSION["unnamed",METHOD["Lambert Conic Conformal (2SP)",ID["EPSG",9802]],PARAMETER["Longitude of false origin",13.3333333333333,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8822]],PARAMETER["Latitude of false origin",47.5,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8821]],PARAMETER["Latitude of 1st standard parallel",49,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8823]],PARAMETER["Easting at false origin",400000,LENGTHUNIT["m",1],ID["EPSG",8826]],PARAMETER["Northing at false origin",400000,LENGTHUNIT["m",1],ID["EPSG",8827]],PARAMETER["scale_factor",1,SCALEUNIT["unity",1]],PARAMETER["Latitude of 2nd standard parallel",46,ANGLEUNIT["degree",0.0174532925199433],ID["EPSG",8824]]],CS[Cartesian,2],AXIS["easting",east,ORDER[1],LENGTHUNIT["m",1]],AXIS["northing",north,ORDER[2],LENGTHUNIT["m",1]],ID["EPSG",31287]]],TARGETCRS[GEOGCRS["WGS 84",DATUM["World Geodetic System 1984",ELLIPSOID["WGS 84",6378137,298.257223563,LENGTHUNIT["metre",1]]],PRIMEM["Greenwich",0,ANGLEUNIT["degree",0.0174532925199433]],CS[ellipsoidal,2],AXIS["latitude",north,ORDER[1],ANGLEUNIT["degree",0.0174532925199433]],AXIS["longitude",east,ORDER[2],ANGLEUNIT["degree",0.0174532925199433]],ID["EPSG",4326]]],ABRIDGEDTRANSFORMATION["Transformation from MGI to WGS84",METHOD["Position Vector transformation (geog2D domain)",ID["EPSG",9606]],PARAMETER["X-axis translation",601.705,ID["EPSG",8605]],PARAMETER["Y-axis translation",84.263,ID["EPSG",8606]],PARAMETER["Z-axis translation",485.227,ID["EPSG",8607]],PARAMETER["X-axis rotation",4.7354,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.3145,ID["EPSG",8609]],PARAMETER["Z-axis rotation",-5.393,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999976113,ID["EPSG",8611]]]])""" ) );
  QVERIFY( myCrs.isValid() );
  QCOMPARE( myCrs.mapUnits(), Qgis::DistanceUnit::Meters );

  // an invalid crs should return unknown unit
  QCOMPARE( QgsCoordinateReferenceSystem().mapUnits(), Qgis::DistanceUnit::Unknown );
}

void TestQgsCoordinateReferenceSystem::isDynamic()
{
  QgsCoordinateReferenceSystem crs( u"EPSG:7665"_s );
  QVERIFY( crs.isDynamic() );

  crs = QgsCoordinateReferenceSystem( u"EPSG:4171"_s );
  QVERIFY( !crs.isDynamic() );

  // WGS84 (generic), using datum ensemble
  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QVERIFY( crs.isDynamic() );

  // Test generic geodetic CRS using datum ensemble
  QVERIFY( crs.createFromWkt( QStringLiteral( R"""(GEOGCRS["test",
    ENSEMBLE["test",
             MEMBER["member1"],
        MEMBER["member2"],
        ELLIPSOID["GRS 1980",6378137,298.257222101,
            LENGTHUNIT["metre",1]],
        ENSEMBLEACCURACY[0.1]],
    PRIMEM["Greenwich",0,
        ANGLEUNIT["degree",0.0174532925199433]],
    CS[ellipsoidal,2],
        AXIS["geodetic latitude (Lat)",north,
            ORDER[1],
            ANGLEUNIT["degree",0.0174532925199433]],
        AXIS["geodetic longitude (Lon)",east,
            ORDER[2],
            ANGLEUNIT["degree",0.0174532925199433]]])""" ) ) );
  QVERIFY( crs.isValid() );
  QVERIFY( !crs.isDynamic() );

  QVERIFY( crs.createFromWkt( QStringLiteral( R"""(GEOGCS["WGS 84",
      DATUM["WGS_1984",
          SPHEROID["WGS 84",6378137,298.257223563,
              AUTHORITY["EPSG","7030"]],
          AUTHORITY["EPSG","6326"]],
      PRIMEM["Greenwich",0,
          AUTHORITY["EPSG","8901"]],
      UNIT["degree",0.0174532925199433,
          AUTHORITY["EPSG","9122"]],
      AXIS["Latitude",NORTH],
      AXIS["Longitude",EAST],
      AUTHORITY["EPSG","4326"]])""" ) ) );
  QVERIFY( crs.isValid() );
  QVERIFY( crs.isDynamic() );
}

void TestQgsCoordinateReferenceSystem::celestialBody()
{
  QgsCoordinateReferenceSystem crs;
  QCOMPARE( crs.celestialBodyName(), QString() );

  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QCOMPARE( crs.celestialBodyName(), u"Earth"_s );

  crs = QgsCoordinateReferenceSystem( u"ESRI:104903"_s );
  QCOMPARE( crs.celestialBodyName(), u"Moon"_s );
}

void TestQgsCoordinateReferenceSystem::operation()
{
  QgsCoordinateReferenceSystem crs;
  QVERIFY( !crs.operation().isValid() );

  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QVERIFY( crs.operation().isValid() );
  QCOMPARE( crs.operation().id(), u"longlat"_s );
  QCOMPARE( crs.operation().description(), u"Lat/long (Geodetic alias)"_s );

  crs = QgsCoordinateReferenceSystem( u"EPSG:28356"_s );
  QVERIFY( crs.operation().isValid() );
  QCOMPARE( crs.operation().id(), u"utm"_s );
  QCOMPARE( crs.operation().description(), u"Universal Transverse Mercator (UTM)"_s );
  QVERIFY( crs.operation().details().contains( u"south approx"_s ) );
}

void TestQgsCoordinateReferenceSystem::setValidationHint()
{
  QgsCoordinateReferenceSystem myCrs;
  myCrs.setValidationHint( u"<head>"_s );
  QVERIFY( myCrs.validationHint() == "<head>"_L1 );
  debugPrint( myCrs );
}

void TestQgsCoordinateReferenceSystem::hasAxisInverted()
{
  // this is used by WMS 1.3 to determine whether to switch axes or not

  QgsCoordinateReferenceSystem crs;
  crs.createFromOgcWmsCrs( u"EPSG:4326"_s ); // WGS 84 with inverted axes
  QVERIFY( crs.hasAxisInverted() );

  crs.createFromOgcWmsCrs( u"CRS:84"_s ); // WGS 84 without inverted axes
  QVERIFY( !crs.hasAxisInverted() );
  QCOMPARE( crs.toWkt(), QStringLiteral( R"""(GEOGCS["WGS 84",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["EPSG","4326"]])""" ) );

  crs.createFromOgcWmsCrs( u"OGC:CRS84"_s ); // WGS 84 without inverted axes
  QVERIFY( !crs.hasAxisInverted() );
  QgsDebugMsgLevel( crs.toWkt(), 1 );
  QCOMPARE( crs.toWkt(), QStringLiteral( R"""(GEOGCS["WGS 84 (CRS84)",DATUM["WGS_1984",SPHEROID["WGS 84",6378137,298.257223563,AUTHORITY["EPSG","7030"]],AUTHORITY["EPSG","6326"]],PRIMEM["Greenwich",0,AUTHORITY["EPSG","8901"]],UNIT["degree",0.0174532925199433,AUTHORITY["EPSG","9122"]],AUTHORITY["OGC","CRS84"]])""" ) );
  crs.createFromOgcWmsCrs( u"EPSG:32633"_s ); // "WGS 84 / UTM zone 33N" - projected CRS without invertex axes
  QVERIFY( !crs.hasAxisInverted() );
}


void TestQgsCoordinateReferenceSystem::debugPrint(
  QgsCoordinateReferenceSystem &crs
)
{
  QgsDebugMsgLevel( u"***SpatialRefSystem***"_s, 1 );
  QgsDebugMsgLevel( "* Valid : " + ( crs.isValid() ? u"true"_s : u"false"_s ), 1 );
  QgsDebugMsgLevel( "* SrsId : " + QString::number( crs.srsid() ), 1 );
  QgsDebugMsgLevel( "* EPSG ID : " + crs.authid(), 1 );
  QgsDebugMsgLevel( "* PGIS ID : " + QString::number( crs.postgisSrid() ), 1 );
  QgsDebugMsgLevel( "* Proj4 : " + crs.toProj(), 1 );
  QgsDebugMsgLevel( "* WKT   : " + crs.toWkt(), 1 );
  QgsDebugMsgLevel( "* Desc. : " + crs.description(), 1 );
  if ( crs.mapUnits() == Qgis::DistanceUnit::Meters )
  {
    QgsDebugMsgLevel( u"* Units : meters"_s, 1 );
  }
  else if ( crs.mapUnits() == Qgis::DistanceUnit::Feet )
  {
    QgsDebugMsgLevel( u"* Units : feet"_s, 1 );
  }
  else if ( crs.mapUnits() == Qgis::DistanceUnit::Degrees )
  {
    QgsDebugMsgLevel( u"* Units : degrees"_s, 1 );
  }
}

void TestQgsCoordinateReferenceSystem::createFromProjInvalid()
{
  QgsCoordinateReferenceSystem myCrs;
  QVERIFY( !myCrs.createFromProj( u"+proj=longlat +no_defs"_s ) );
}

void TestQgsCoordinateReferenceSystem::validSrsIds()
{
  const QList<long> ids = QgsCoordinateReferenceSystem::validSrsIds();
  QVERIFY( ids.contains( 3857 ) );
  QVERIFY( ids.contains( 28356 ) );

  int validCount = 0;

  if ( QgsTest::isCIRun() )
    return; // the next part is too time consuming to run on the CI infrastructure

  // check that all returns ids are valid
  for ( const long id : ids )
  {
    const QgsCoordinateReferenceSystem c = QgsCoordinateReferenceSystem::fromSrsId( id );
    if ( c.isValid() )
      validCount++;
    else
      qDebug() << u"QgsCoordinateReferenceSystem::fromSrsId( %1 ) is not valid (%2 of %3 IDs returned by QgsCoordinateReferenceSystem::validSrsIds())."_s.arg( id ).arg( ids.indexOf( id ) ).arg( ids.length() );

    // round trip via WKT strings
    const QgsProjUtils::proj_pj_unique_ptr crs( proj_create_from_database( QgsProjContext::get(), c.authid().split( ':' ).at( 0 ).toLatin1(), c.authid().split( ':' ).at( 1 ).toLatin1(), PJ_CATEGORY_CRS, false, nullptr ) );
    if ( !crs || proj_get_type( crs.get() ) == PJ_TYPE_COMPOUND_CRS )
      continue;

    const QgsCoordinateReferenceSystem viaWkt = QgsCoordinateReferenceSystem::fromWkt( c.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ) );
    QCOMPARE( viaWkt.authid(), c.authid() );
#if 0 // NOT POSSIBLE -- too costly to test for all CRSes !
    // round trip via proj strings
    QgsCoordinateReferenceSystem viaProj = QgsCoordinateReferenceSystem::fromProj( c.toProj() );
    QCOMPARE( viaProj.authid(), c.authid() );
    QCOMPARE( viaProj.toProj(), c.toProj() );
#endif
  }

  QVERIFY( validCount > ids.size() - 100 );
}

void TestQgsCoordinateReferenceSystem::asVariant()
{
  const QgsCoordinateReferenceSystem original( u"EPSG:3112"_s );

  //convert to and from a QVariant
  const QVariant var = QVariant::fromValue( original );
  QVERIFY( var.isValid() );

  const QgsCoordinateReferenceSystem fromVar = qvariant_cast<QgsCoordinateReferenceSystem>( var );
  QCOMPARE( fromVar.authid(), original.authid() );
}

void TestQgsCoordinateReferenceSystem::bounds()
{
  const QgsCoordinateReferenceSystem invalid;
  QVERIFY( invalid.bounds().isNull() );

  const QgsCoordinateReferenceSystem crs3111( u"EPSG:3111"_s );
  QgsRectangle bounds = crs3111.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 140.960000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 150.040000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -39.200000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), -33.980000, 0.0001 );

  const QgsCoordinateReferenceSystem crs28356( u"EPSG:28356"_s );
  bounds = crs28356.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 150.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 156.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -58.960000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), -13.870000, 0.0001 );

  const QgsCoordinateReferenceSystem crs3857( u"EPSG:3857"_s );
  bounds = crs3857.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), -180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -85.060000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 85.060000, 0.0001 );

  const QgsCoordinateReferenceSystem crs4326( u"EPSG:4326"_s );
  bounds = crs4326.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), -180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), 180.000000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), -90.00000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 90.00000, 0.0001 );

  const QgsCoordinateReferenceSystem crs2163( u"EPSG:2163"_s );
  bounds = crs2163.bounds();
  QGSCOMPARENEAR( bounds.xMinimum(), 167.65000, 0.0001 );
  QGSCOMPARENEAR( bounds.xMaximum(), -65.69000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMinimum(), 15.56000, 0.0001 );
  QGSCOMPARENEAR( bounds.yMaximum(), 74.71000, 0.0001 );
}

void TestQgsCoordinateReferenceSystem::saveAsUserCrs()
{
  const QString madeUpProjection = u"+proj=aea +lat_1=20 +lat_2=-23 +lat_0=4 +lon_0=29 +x_0=10 +y_0=3 +datum=WGS84 +units=m +no_defs"_s;
  QgsCoordinateReferenceSystem userCrs = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs.isValid() );
  QCOMPARE( userCrs.toProj(), madeUpProjection );
  QCOMPARE( userCrs.srsid(), 0L ); // not saved to database yet

  const long newId = userCrs.saveAsUserCrs( u"babies first projection"_s );
  QCOMPARE( newId, static_cast<long>( Qgis::USER_CRS_START_ID ) );
  QCOMPARE( userCrs.srsid(), newId );
  QCOMPARE( userCrs.authid(), u"USER:100000"_s );
  QCOMPARE( userCrs.description(), u"babies first projection"_s );

  // new CRS with same definition, check that it's matched to user crs
  const QgsCoordinateReferenceSystem userCrs2 = QgsCoordinateReferenceSystem::fromProj( madeUpProjection );
  QVERIFY( userCrs2.isValid() );
  QCOMPARE( userCrs2.toProj(), madeUpProjection );
  QCOMPARE( userCrs2.srsid(), userCrs.srsid() );
  QCOMPARE( userCrs2.authid(), u"USER:100000"_s );
  QCOMPARE( userCrs2.description(), u"babies first projection"_s );

  // createFromString with user crs
  QgsCoordinateReferenceSystem userCrs3;
  userCrs3.createFromString( u"USER:100000"_s );
  QVERIFY( userCrs3.isValid() );
  QCOMPARE( userCrs3.authid(), u"USER:100000"_s );
  QCOMPARE( userCrs3.toProj(), madeUpProjection );
  QCOMPARE( userCrs3.description(), u"babies first projection"_s );
}

void TestQgsCoordinateReferenceSystem::projectWithCustomCrs()
{
  // tests loading a 2.x project with a custom CRS defined
  QgsProject p;
  const QSignalSpy spyCrsChanged( &p, &QgsProject::crsChanged );
  QVERIFY( p.read( TEST_DATA_DIR + u"/projects/custom_crs.qgs"_s ) );
  QVERIFY( p.crs().isValid() );
  QCOMPARE( p.crs().toProj(), u"+proj=ortho +lat_0=42.1 +lon_0=12.8 +x_0=0 +y_0=0 +a=6371000 +b=6371000 +units=m +no_defs"_s );
  QCOMPARE( spyCrsChanged.count(), 1 );
}

void TestQgsCoordinateReferenceSystem::projectEPSG25833()
{
  // tests loading a 2.x project with a predefined EPSG that has non unique proj.4 string
  QgsProject p;
  const QSignalSpy spyCrsChanged( &p, &QgsProject::crsChanged );
  QVERIFY( p.read( TEST_DATA_DIR + u"/projects/epsg25833.qgs"_s ) );
  QVERIFY( p.crs().isValid() );
  QCOMPARE( p.crs().authid(), u"EPSG:25833"_s );
  QCOMPARE( spyCrsChanged.count(), 1 );
}

void TestQgsCoordinateReferenceSystem::geoCcsDescription()
{
  // test that geoccs crs descriptions are correctly imported from GDAL
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:3822"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:3822"_s );
  QCOMPARE( crs.description(), u"TWD97"_s );

  crs.createFromString( u"EPSG:4340"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:4340"_s );
  QCOMPARE( crs.description(), u"Australian Antarctic (geocentric)"_s );

  crs.createFromString( u"EPSG:4348"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"EPSG:4348"_s );
  QCOMPARE( crs.description(), u"GDA94 (geocentric)"_s );
}

void TestQgsCoordinateReferenceSystem::geographicCrsAuthId()
{
  QgsCoordinateReferenceSystem crs;
  crs.createFromString( u"EPSG:4326"_s );
  QCOMPARE( crs.authid(), u"EPSG:4326"_s );
  QCOMPARE( crs.geographicCrsAuthId(), u"EPSG:4326"_s );
  QCOMPARE( crs.toGeographicCrs(), crs );
  QVERIFY( crs.toGeographicCrs().isGeographic() );

  crs.createFromString( u"EPSG:3825"_s );
  QCOMPARE( crs.authid(), u"EPSG:3825"_s );
  QCOMPARE( crs.geographicCrsAuthId(), u"EPSG:3824"_s );
  QCOMPARE( crs.toGeographicCrs().toProj().replace( "+towgs84=0,0,0,0,0,0,0 "_L1, QString() ).replace( " +type=crs"_L1, QString() ), QgsCoordinateReferenceSystem( u"EPSG:3824"_s ).toProj().replace( "+towgs84=0,0,0,0,0,0,0 "_L1, QString() ).replace( " +type=crs"_L1, QString() ) );
  QVERIFY( crs.toGeographicCrs().isGeographic() );
}

void TestQgsCoordinateReferenceSystem::noProj()
{
  // test a crs which cannot be represented by a proj string
  QgsCoordinateReferenceSystem crs( u"EPSG:2218"_s );
  QCOMPARE( crs.authid(), u"EPSG:2218"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toWkt(), u"PROJCS[\"Scoresbysund 1952 / Greenland zone 5 east\",GEOGCS[\"Scoresbysund 1952\",DATUM[\"Scoresbysund_1952\",SPHEROID[\"International 1924\",6378388,297,AUTHORITY[\"EPSG\",\"7022\"]],AUTHORITY[\"EPSG\",\"6195\"]],PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],AUTHORITY[\"EPSG\",\"4195\"]],PROJECTION[\"Lambert_Conic_Conformal_(West_Orientated)\"],PARAMETER[\"Latitude of natural origin\",70.5],PARAMETER[\"Longitude of natural origin\",-24],PARAMETER[\"Scale factor at natural origin\",1],PARAMETER[\"False easting\",0],PARAMETER[\"False northing\",0],UNIT[\"metre\",1,AUTHORITY[\"EPSG\",\"9001\"]],AUTHORITY[\"EPSG\",\"2218\"]]"_s );
  crs = QgsCoordinateReferenceSystem::fromOgcWmsCrs( u"ESRI:54091"_s );
  QCOMPARE( crs.authid(), u"ESRI:54091"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019Simplified ).left( 30 ), u"PROJCRS[\"WGS_1984_Peirce_quinc"_s );
  crs = QgsCoordinateReferenceSystem( u"EPSG:22300"_s );
  QCOMPARE( crs.authid(), u"EPSG:22300"_s );
  QVERIFY( crs.isValid() );
  QgsDebugMsgLevel( crs.toWkt(), 1 );
  QVERIFY( crs.toWkt().startsWith( "PROJCS[\"Carthage (Paris) / Tunisia Mining Grid\","_L1 ) );
#if ( PROJ_VERSION_MAJOR > 9 || ( PROJ_VERSION_MAJOR == 9 && PROJ_VERSION_MINOR >= 2 ) )
  QVERIFY( crs.toWkt().contains( "PROJECTION[\"Tunisia_Mining_Grid\"]"_L1 ) );
#else
  QVERIFY( crs.toWkt().contains( "PROJECTION[\"Tunisia_Mapping_Grid\"]"_L1 ) );
#endif
}

void TestQgsCoordinateReferenceSystem::customProjString()
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toProj(), u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).startsWith( QLatin1String( R"""(BOUNDCRS[SOURCECRS[)""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Oblique Stereographic",ID["EPSG",9809]])""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",595.75,ID["EPSG",8605]],PARAMETER["Y-axis translation",121.09,ID["EPSG",8606]],PARAMETER["Z-axis translation",515.5,ID["EPSG",8607]],PARAMETER["X-axis rotation",8.227,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.5193,ID["EPSG",8609]],PARAMETER["Z-axis rotation",5.5971,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999973271,ID["EPSG",8611]])""" ) ) );
  const long id = crs.saveAsUserCrs( u"custom proj crs"_s, Qgis::CrsDefinitionFormat::Proj );
  QVERIFY( id );

  // try again and make sure it's matched to user crs
  crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toProj(), u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).startsWith( QLatin1String( R"""(BOUNDCRS[SOURCECRS[)""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Oblique Stereographic",ID["EPSG",9809]])""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",595.75,ID["EPSG",8605]],PARAMETER["Y-axis translation",121.09,ID["EPSG",8606]],PARAMETER["Z-axis translation",515.5,ID["EPSG",8607]],PARAMETER["X-axis rotation",8.227,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.5193,ID["EPSG",8609]],PARAMETER["Z-axis rotation",5.5971,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999973271,ID["EPSG",8611]])""" ) ) );

  QCOMPARE( crs.authid(), u"USER:%1"_s.arg( id ) );

  // make sure it works without cache
  QgsCoordinateReferenceSystem::invalidateCache();
  crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toProj(), u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).startsWith( QLatin1String( R"""(BOUNDCRS[SOURCECRS[)""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Oblique Stereographic",ID["EPSG",9809]])""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",595.75,ID["EPSG",8605]],PARAMETER["Y-axis translation",121.09,ID["EPSG",8606]],PARAMETER["Z-axis translation",515.5,ID["EPSG",8607]],PARAMETER["X-axis rotation",8.227,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.5193,ID["EPSG",8609]],PARAMETER["Z-axis rotation",5.5971,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999973271,ID["EPSG",8611]])""" ) ) );

  QCOMPARE( crs.authid(), u"USER:%1"_s.arg( id ) );

  // make sure it matches to user crs when parameter order is different
  QgsCoordinateReferenceSystem::invalidateCache();
  crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=sterea +lon_0=19.0491441390302 +lat_0=47.4860018439082 +k=1 +y_0=500000 +ellps=bessel +x_0=500000 +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.authid(), u"USER:%1"_s.arg( id ) );
  QCOMPARE( crs.toProj(), u"+proj=sterea +lat_0=47.4860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QgsDebugMsgLevel( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ), 1 );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).startsWith( QLatin1String( R"""(BOUNDCRS[SOURCECRS[)""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(METHOD["Oblique Stereographic",ID["EPSG",9809]])""" ) ) );
  QVERIFY( crs.toWkt( Qgis::CrsWktVariant::Wkt2_2019 ).contains( QLatin1String( R"""(PARAMETER["X-axis translation",595.75,ID["EPSG",8605]],PARAMETER["Y-axis translation",121.09,ID["EPSG",8606]],PARAMETER["Z-axis translation",515.5,ID["EPSG",8607]],PARAMETER["X-axis rotation",8.227,ID["EPSG",8608]],PARAMETER["Y-axis rotation",-1.5193,ID["EPSG",8609]],PARAMETER["Z-axis rotation",5.5971,ID["EPSG",8610]],PARAMETER["Scale difference",0.9999973271,ID["EPSG",8611]])""" ) ) );
}

void TestQgsCoordinateReferenceSystem::displayIdentifier()
{
  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem();
  QCOMPARE( crs.userFriendlyIdentifier(), QString() );
  QCOMPARE( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ), u"Unknown CRS"_s );
  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QCOMPARE( crs.userFriendlyIdentifier(), u"EPSG:4326 - WGS 84"_s );
  crs.setCoordinateEpoch( 2021.3 );
  QCOMPARE( crs.userFriendlyIdentifier(), u"EPSG:4326 - WGS 84 @ 2021.3"_s );
  crs = QgsCoordinateReferenceSystem( u"EPSG:3111"_s );
  QCOMPARE( crs.userFriendlyIdentifier(), u"EPSG:3111 - GDA94 / Vicgrid"_s );
  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QCOMPARE( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ), u"EPSG:4326"_s );

  // non registered custom CRS
  crs = QgsCoordinateReferenceSystem::fromProj( u"+proj=sterea +lat_0=47.9860018439082 +lon_0=19.0491441390302 +k=1 +x_0=500000 +y_0=500000 +ellps=bessel +towgs84=595.75,121.09,515.50,8.2270,-1.5193,5.5971,-2.6729 +units=m +vunits=m +no_defs"_s );
  QgsDebugMsgLevel( crs.userFriendlyIdentifier(), 1 );
  QVERIFY( crs.userFriendlyIdentifier().startsWith( "Custom CRS: BOUNDCRS["_L1 ) );
  QVERIFY( !crs.userFriendlyIdentifier().contains( "PARAMETER[\"Scale difference\",0.9999973271,ID[\"EPSG\",8611]]"_L1 ) );
  QVERIFY( crs.userFriendlyIdentifier().endsWith( QChar( 0x2026 ) ) ); //#spellok

  QgsDebugMsgLevel( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::FullString ), 1 );
  QVERIFY( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::FullString ).startsWith( "Custom CRS: BOUNDCRS["_L1 ) );
  QVERIFY( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::FullString ).contains( "PARAMETER[\"Scale difference\",0.9999973271,ID[\"EPSG\",8611]]"_L1 ) );
  QCOMPARE( crs.userFriendlyIdentifier( Qgis::CrsIdentifierType::ShortString ), u"Custom CRS"_s );

  crs.saveAsUserCrs( u"my test"_s );
  QCOMPARE( crs.userFriendlyIdentifier(), u"USER:%1 - my test"_s.arg( crs.srsid() ) );
}

void TestQgsCoordinateReferenceSystem::toOgcUri()
{
  QgsCoordinateReferenceSystem crs( u"EPSG:3717"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toOgcUri(), "http://www.opengis.net/def/crs/EPSG/0/3717" );

  crs = QgsCoordinateReferenceSystem( u"EPSG:4326"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toOgcUri(), "http://www.opengis.net/def/crs/EPSG/0/4326" );

  crs = QgsCoordinateReferenceSystem( u"OGC:CRS84"_s );
  QVERIFY( crs.isValid() );
  QCOMPARE( crs.toOgcUri(), "http://www.opengis.net/def/crs/OGC/1.3/CRS84" );
}

QGSTEST_MAIN( TestQgsCoordinateReferenceSystem )
#include "testqgscoordinatereferencesystem.moc"
