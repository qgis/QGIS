/***************************************************************************
     testqgsprojutils.cpp
     --------------------------------------
    Date                 : March 2019
    Copyright            : (C) 2019 Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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

#include <proj.h>

//header for class being tested
#include "qgsprojutils.h"
#include <QtConcurrent>

class TestQgsProjUtils : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();
    void cleanupTestCase();
    void threadSafeContext();
    void usesAngularUnits();
    void axisOrderIsSwapped();
    void searchPath();
    void gridsUsed();
    void toHorizontalCrs();
    void toUnboundCrs();
    void hasVerticalAxis();
};


void TestQgsProjUtils::initTestCase()
{
  QgsApplication::init();
  QgsApplication::createDatabase();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
}

void TestQgsProjUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


struct ProjContextWrapper
{
    explicit ProjContextWrapper()
    {}

    void operator()( int )
    {
      QVERIFY( QgsProjContext::get() );
      // TODO - do something with the context?
    }
};

void TestQgsProjUtils::threadSafeContext()
{
  // smash proj context generation over many threads
  QVector<int> list;
  list.resize( 100 );
  QtConcurrent::blockingMap( list, ProjContextWrapper() );
}

void TestQgsProjUtils::usesAngularUnits()
{
  QVERIFY( !QgsProjUtils::usesAngularUnit( QString() ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QString( "" ) ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QStringLiteral( "x" ) ) );
  QVERIFY( QgsProjUtils::usesAngularUnit( QStringLiteral( "+proj=longlat +ellps=WGS60 +no_defs" ) ) );
  QVERIFY( !QgsProjUtils::usesAngularUnit( QStringLiteral( "+proj=tmerc +lat_0=0 +lon_0=147 +k_0=0.9996 +x_0=500000 +y_0=10000000 +ellps=GRS80 +units=m +no_defs" ) ) );
}

void TestQgsProjUtils::axisOrderIsSwapped()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  QVERIFY( !QgsProjUtils::axisOrderIsSwapped( nullptr ) );

  QgsProjUtils::proj_pj_unique_ptr crs( proj_create( context, "urn:ogc:def:crs:EPSG::3111" ) );
  QVERIFY( !QgsProjUtils::axisOrderIsSwapped( crs.get() ) );
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::4326" ) );
  QVERIFY( QgsProjUtils::axisOrderIsSwapped( crs.get() ) );
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::3903" ) );
  QVERIFY( QgsProjUtils::axisOrderIsSwapped( crs.get() ) );
}

void TestQgsProjUtils::searchPath()
{
  // ensure local user-writable path is present in Proj search paths
  const QStringList paths = QgsProjUtils::searchPaths();
  QVERIFY( paths.contains( QgsApplication::qgisSettingsDirPath() + QStringLiteral( "proj" ) ) );
}

void TestQgsProjUtils::gridsUsed()
{
  // ensure local user-writable path is present in Proj search paths
  QList<QgsDatumTransform::GridDetails> grids = QgsProjUtils::gridsUsed( QStringLiteral( "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1" ) );
  QCOMPARE( grids.count(), 1 );
  QCOMPARE( grids.at( 0 ).shortName, QStringLiteral( "GDA94_GDA2020_conformal_and_distortion.gsb" ) );
  QVERIFY( grids.at( 0 ).directDownload );
  QVERIFY( !grids.at( 0 ).url.isEmpty() );
  // using tif grid
  grids = QgsProjUtils::gridsUsed( QStringLiteral( "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=au_icsm_GDA94_GDA2020_conformal_and_distortion.tif +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1" ) );
  QCOMPARE( grids.count(), 1 );
  QCOMPARE( grids.at( 0 ).shortName, QStringLiteral( "au_icsm_GDA94_GDA2020_conformal_and_distortion.tif" ) );
  QVERIFY( grids.at( 0 ).directDownload );
  QVERIFY( !grids.at( 0 ).url.isEmpty() );
}

void TestQgsProjUtils::toHorizontalCrs()
{
  PJ_CONTEXT *context = QgsProjContext::get();

  // compound crs
  QgsProjUtils::proj_pj_unique_ptr crs( proj_create( context, "urn:ogc:def:crs:EPSG::5500" ) );
  QgsProjUtils::proj_pj_unique_ptr horizontalCrs( QgsProjUtils::crsToHorizontalCrs( crs.get() ) );
  QCOMPARE( QString( proj_get_id_code( horizontalCrs.get(), 0 ) ), QStringLiteral( "4759" ) );

  // horizontal CRS
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::4759" ) );
  horizontalCrs = QgsProjUtils::crsToHorizontalCrs( crs.get() );
  QCOMPARE( QString( proj_get_id_code( horizontalCrs.get(), 0 ) ), QStringLiteral( "4759" ) );

  // vertical only CRS
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::5703" ) );
  horizontalCrs = QgsProjUtils::crsToHorizontalCrs( crs.get() );
  QVERIFY( !horizontalCrs );
}

void TestQgsProjUtils::toUnboundCrs()
{
  PJ_CONTEXT *context = QgsProjContext::get();

  // compound crs
  QgsProjUtils::proj_pj_unique_ptr crs( proj_create( context, "urn:ogc:def:crs:EPSG::5500" ) );
  QgsProjUtils::proj_pj_unique_ptr unbound( QgsProjUtils::unboundCrs( crs.get() ) );
  QCOMPARE( QString( proj_get_id_code( unbound.get(), 0 ) ), QStringLiteral( "5500" ) );

  // horizontal CRS
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::4759" ) );
  unbound = QgsProjUtils::unboundCrs( crs.get() );
  QCOMPARE( QString( proj_get_id_code( unbound.get(), 0 ) ), QStringLiteral( "4759" ) );

  // vertical only CRS
  crs.reset( proj_create( context, "urn:ogc:def:crs:EPSG::5703" ) );
  unbound = QgsProjUtils::unboundCrs( crs.get() );
  QCOMPARE( QString( proj_get_id_code( unbound.get(), 0 ) ), QStringLiteral( "5703" ) );
}

void TestQgsProjUtils::hasVerticalAxis()
{
  PJ_CONTEXT *context = QgsProjContext::get();
  // compound crs
  QgsProjUtils::proj_pj_unique_ptr crs( proj_create( context, "EPSG:5500" ) );
  QVERIFY( QgsProjUtils::hasVerticalAxis( crs.get() ) );

  // horizontal crs
  crs.reset( proj_create( context, "EPSG:4759" ) );
  QVERIFY( !QgsProjUtils::hasVerticalAxis( crs.get() ) );

  // vertical crs
  crs.reset( proj_create( context, "EPSG:5703" ) );
  QVERIFY( QgsProjUtils::hasVerticalAxis( crs.get() ) );

  // projected 3d crs
  crs.reset( proj_create( context, "PROJCRS[\"NAD83(HARN) / Oregon GIC Lambert (ft)\",\n"
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
                                   "            LENGTHUNIT[\"foot\",0.3048]]]" ) );
  QVERIFY( QgsProjUtils::hasVerticalAxis( crs.get() ) );
}

QGSTEST_MAIN( TestQgsProjUtils )
#include "testqgsprojutils.moc"
