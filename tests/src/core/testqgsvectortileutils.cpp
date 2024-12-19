/***************************************************************************
  testqgsvectortileutils.cpp
  --------------------------------------
  Date                 : November 2021
  Copyright            : (C) 2021 by Mathieu Pellerin
  Email                : nirvn dot asia at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QTemporaryFile>

//qgis includes...
#include "qgsapplication.h"
#include "qgsvectortileutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the vector tile utils class
 */
class TestQgsVectorTileUtils : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileUtils() = default;

  private:
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void test_scaleToZoomLevel();
    void test_urlsFromStyle();
};


void TestQgsVectorTileUtils::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsVectorTileUtils::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorTileUtils::test_scaleToZoomLevel()
{
  // test zoom level logic
  int zoomLevel = QgsVectorTileUtils::scaleToZoomLevel( 288896, 0, 20 );
  QCOMPARE( zoomLevel, 10 );
}

void TestQgsVectorTileUtils::test_urlsFromStyle()
{
  QString dataDir( TEST_DATA_DIR );
  QFile style1File( dataDir + "/vector_tile/styles/style1.json" );
  style1File.open( QIODevice::Text | QIODevice::ReadOnly );
  QString style1Content = style1File.readAll();
  style1File.close();
  style1Content.replace( QString( "_TILE_SOURCE_TEST_PATH_" ), "file://" + dataDir + "/vector_tile/styles" );
  QFile fixedStyleFilePath( QDir::tempPath() + QStringLiteral( "/style1.json" ) );
  if ( fixedStyleFilePath.open( QFile::WriteOnly | QFile::Truncate ) )
  {
    QTextStream out( &fixedStyleFilePath );
    out << style1Content;
  }
  fixedStyleFilePath.close();

  auto sources = QgsVectorTileUtils::parseStyleSourceUrl( "file://" + fixedStyleFilePath.fileName() );
  QCOMPARE( sources.count(), 2 );
  QVERIFY( sources.contains( "base_v1.0.0" ) );
  QString sourceUrl = sources.value( "base_v1.0.0" );
  sourceUrl.replace( QRegularExpression( "vectortiles[0-9]" ), QStringLiteral( "vectortilesX" ) );
  QCOMPARE( sourceUrl, "https://vectortilesX.geo.admin.ch/tiles/ch.swisstopo.base.vt/v1.0.0/{z}/{x}/{y}.pbf" );
  QVERIFY( sources.contains( "terrain_v1.0.0" ) );
  sourceUrl = sources.value( "terrain_v1.0.0" );
  sourceUrl.replace( QRegularExpression( "vectortiles[0-9]" ), QStringLiteral( "vectortilesX" ) );
  QCOMPARE( sourceUrl, "https://vectortilesX.geo.admin.ch/tiles/ch.swisstopo.relief.vt/v1.0.0/{z}/{x}/{y}.pbf" );

  sources = QgsVectorTileUtils::parseStyleSourceUrl( "file://" + dataDir + "/vector_tile/styles/style2.json" );
  QCOMPARE( sources.count(), 1 );
  QVERIFY( sources.contains( "plan_ign" ) );
  QCOMPARE( sources.value( "plan_ign" ), "https://data.geopf.fr/tms/1.0.0/PLAN.IGN/{z}/{x}/{y}.pbf" );
}

QGSTEST_MAIN( TestQgsVectorTileUtils )
#include "testqgsvectortileutils.moc"
