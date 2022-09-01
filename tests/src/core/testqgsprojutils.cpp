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

class TestQgsProjUtils: public QObject
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
  QVector< int > list;
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
  QList< QgsDatumTransform::GridDetails > grids = QgsProjUtils::gridsUsed( QStringLiteral( "+proj=pipeline +step +proj=axisswap +order=2,1 +step +proj=unitconvert +xy_in=deg +xy_out=rad +step +inv +proj=hgridshift +grids=GDA94_GDA2020_conformal_and_distortion.gsb +step +proj=unitconvert +xy_in=rad +xy_out=deg +step +proj=axisswap +order=2,1" ) );
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

QGSTEST_MAIN( TestQgsProjUtils )
#include "testqgsprojutils.moc"
