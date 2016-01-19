/***************************************************************************
     testadjacenttiles.cpp
     --------------------------------------
    Date                 : Jan 7  2016
    Copyright            : (C) 2016 by Marco Hugentobler
    Email                : marco at sourcepole dot ch
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QtTest/QtTest>
#include <QObject>

#include "qgsapplication.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderersequentialjob.h"
#include "qgsmapsettings.h"
#include <qgsrenderchecker.h>
#include "qgsvectorlayer.h"
#include "qgsvectorlayerrenderer.h"

/** \ingroup UnitTests
 * This unit test checks if rendering of adjacent tiles (e.g. to render images for tile caches)
 * does not result in border effects
 */
class TestQgsAdjacentTiles : public QObject
{
    Q_OBJECT
  public:
    TestQgsAdjacentTiles() {}
    ~TestQgsAdjacentTiles() {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.


#if 0 //disable for now
    void testFourAdjacentTiles_data();
    void testFourAdjacentTiles();
#endif //0
};

void TestQgsAdjacentTiles::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsAdjacentTiles::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

#if 0
void TestQgsAdjacentTiles::testFourAdjacentTiles_data()
{
  QTest::addColumn<QStringList>( "bboxList" );
  QTest::addColumn<QString>( "resultFile" );
  QTest::addColumn<QString>( "shapeFile" );
  QTest::addColumn<QString>( "qmlFile" );

  QString shapeFile = TEST_DATA_DIR + QString( "/france_parts.shp" );
  QString qmlFile = TEST_DATA_DIR + QString( "/adjacent_tiles/line_pattern_30_degree.qml" );
  QString resultFile = TEST_DATA_DIR + QString( "/adjacent_tiles/testFourAdjacentTiles1_expected.png" );

  QStringList bboxList1;
  bboxList1 << "-1.5,48,-0.5,49";
  bboxList1 << "-0.5,48,0.5,49";
  bboxList1 << "-1.5,47,-0.5,48";
  bboxList1 << "-0.5,47,0.5,48";

  QTest::newRow( "testFourAdjacentTiles1" ) << bboxList1 << resultFile << shapeFile << qmlFile;

  qmlFile = TEST_DATA_DIR + QString( "/adjacent_tiles/point_pattern_simple_marker.qml" );
  resultFile = TEST_DATA_DIR + QString( "/adjacent_tiles/testFourAdjacentTiles2_expected.png" );

  QTest::newRow( "testFourAdjacentTiles2" ) << bboxList1 << resultFile << shapeFile << qmlFile;

  shapeFile = TEST_DATA_DIR + QString( "/lines.shp" );
  qmlFile = TEST_DATA_DIR + QString( "/adjacent_tiles/simple_line_dashed.qml" );
  resultFile = TEST_DATA_DIR + QString( "/adjacent_tiles/testFourAdjacentTiles3_expected.png" );

  QStringList bboxList2;
  bboxList2 << "-105,35,-95,45";
  bboxList2 << "-95,35,-85,45";
  bboxList2 << "-105,25,-95,35";
  bboxList2 << "-95,25,-85,35";

  QTest::newRow( "testFourAdjacentTiles3" ) << bboxList2 << resultFile << shapeFile << qmlFile;
}

void TestQgsAdjacentTiles::testFourAdjacentTiles()
{
  QFETCH( QStringList, bboxList );
  QFETCH( QString, resultFile );
  QFETCH( QString, shapeFile );
  QFETCH( QString, qmlFile );

  QVERIFY( bboxList.size() == 4 );

  //create maplayer, set QML and add to maplayer registry
  QgsVectorLayer* vectorLayer = new QgsVectorLayer( shapeFile, "testshape", "ogr" );

  //todo: read QML
  QFile symbologyFile( qmlFile );
  if ( !symbologyFile.open( QIODevice::ReadOnly ) )
  {
    QFAIL( "Open symbology file failed" );
  }

  QDomDocument qmlDoc;
  if ( !qmlDoc.setContent( &symbologyFile ) )
  {
    QFAIL( "QML file not valid" );
  }

  QString errorMsg;
  if ( !vectorLayer->readSymbology( qmlDoc.documentElement(), errorMsg ) )
  {
    QFAIL( errorMsg.toLocal8Bit().data() );
  }

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << vectorLayer );

  QImage globalImage( 512, 512, QImage::Format_ARGB32_Premultiplied );
  globalImage.fill( Qt::white );
  QPainter globalPainter( &globalImage );

  for ( int i = 0; i < 4; ++i )
  {
    QgsMapSettings mapSettings;

    //extent
    QStringList rectCoords = bboxList.at( i ).split( "," );
    if ( rectCoords.size() != 4 )
    {
      QFAIL( "bbox string invalid" );
    }
    QgsRectangle rect( rectCoords[0].toDouble(), rectCoords[1].toDouble(), rectCoords[2].toDouble(), rectCoords[3].toDouble() );
    mapSettings.setExtent( rect );
    mapSettings.setOutputSize( QSize( 256, 256 ) );
    mapSettings.setLayers( QStringList() << vectorLayer->id() );
    mapSettings.setFlags( QgsMapSettings::RenderMapTile );
    mapSettings.setOutputDpi( 96 );

    QgsMapRendererSequentialJob renderJob( mapSettings );
    renderJob.start();
    renderJob.waitForFinished();
    QImage img = renderJob.renderedImage();
    int globalImageX = ( i % 2 ) * 256;
    int globalImageY = ( i < 2 ) ? 0 : 256;
    globalPainter.drawImage( globalImageX, globalImageY, img );
  }

  QgsMapLayerRegistry::instance()->removeMapLayers( QStringList() << vectorLayer->id() );

  QString renderedImagePath = QDir::tempPath() + "/" + QTest::currentDataTag() + QString( ".png" );
  globalImage.save( renderedImagePath );

  QgsRenderChecker checker;
  checker.setControlName( QTest::currentDataTag() );
  QVERIFY( checker.compareImages( QTest::currentDataTag(), 100, renderedImagePath ) );
}
#endif //0

QTEST_MAIN( TestQgsAdjacentTiles )
#include "testqgsadjacenttiles.moc"
