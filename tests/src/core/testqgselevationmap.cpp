/***************************************************************************
  testqgselevationmap.cpp
  --------------------------------------
Date                 : August 2022
Copyright            : (C) 2022 by Martin Dobias
Email                : wonder dot sk at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include "qgsapplication.h"
#include "qgselevationmap.h"
#include "qgsrasterlayer.h"


class TestQgsElevationMap : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsElevationMap() : QgsTest( QStringLiteral( "Elevation Map Tests" ), QStringLiteral( "elevation_map" ) ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testRasterDemEdl();

};


void TestQgsElevationMap::initTestCase()
{
  //
  // Runs once before any tests are run
  //
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

}

void TestQgsElevationMap::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsElevationMap::testRasterDemEdl()
{

  QString testDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QgsRasterLayer r( testDataDir + "/analysis/dem.tif" );
  QVERIFY( r.isValid() );

  const QgsRectangle fullExtent = r.extent();
  const int width = r.width();
  const int height = r.height();

  std::unique_ptr<QgsRasterBlock> block( r.dataProvider()->block( 1, fullExtent, width, height ) );
  QVERIFY( block );

  QImage img( block->width(), block->height(), QImage::Format_RGB32 );
  img.fill( Qt::cyan );

  std::unique_ptr<QgsElevationMap> elevationMap( QgsElevationMap::fromRasterBlock( block.get() ) );
  elevationMap->applyEyeDomeLighting( img, 2, 100, 10000 );

  QVERIFY( imageCheck( "dem_edl", "dem_edl", img ) );
}


QGSTEST_MAIN( TestQgsElevationMap )
#include "testqgselevationmap.moc"
