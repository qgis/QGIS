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
#include "qgselevationshadingrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterlayerelevationproperties.h"


class TestQgsElevationMap : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsElevationMap()
      : QgsTest( QStringLiteral( "Elevation Map Tests" ), QStringLiteral( "elevation_map" ) ) {}

  private slots:
    void initTestCase();
    void cleanupTestCase();
    void testRasterDemEdl();
    void testRasterDemReprojected();
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

  QGSVERIFYIMAGECHECK( "dem_edl", "dem_edl", img );
}

void TestQgsElevationMap::testRasterDemReprojected()
{
  QString testDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
  QgsRasterLayer r( testDataDir + "/analysis/dem.tif" );
  r.setOpacity( 0.0 );
  static_cast<QgsRasterLayerElevationProperties *>( r.elevationProperties() )->setEnabled( true );
  QVERIFY( r.isValid() );

  QgsElevationShadingRenderer elevationShadingRenderer;
  elevationShadingRenderer.setActive( true );
  elevationShadingRenderer.setActiveHillshading( true );

  QgsMapSettings mapSettings;
  const QSize size( 640, 480 );
  mapSettings.setOutputSize( size );
  mapSettings.setLayers( QList<QgsMapLayer *>() << &r );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( QgsCoordinateReferenceSystem( "EPSG:3857" ) );
  mapSettings.setExtent( QgsRectangle( 770583, 5609270, 781928, 5619219 ) );
  mapSettings.setElevationShadingRenderer( elevationShadingRenderer );

  QGSVERIFYRENDERMAPSETTINGSCHECK( QStringLiteral( "reprojected_raster" ), QStringLiteral( "reprojected_raster" ), mapSettings );
}


QGSTEST_MAIN( TestQgsElevationMap )
#include "testqgselevationmap.moc"
