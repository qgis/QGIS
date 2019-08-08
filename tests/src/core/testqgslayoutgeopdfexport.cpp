/***************************************************************************
                         testqgslayoutgeopdfexport.cpp
                         ----------------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgslayoutexporter.h"
#include "qgslayoutgeopdfexporter.h"
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutGeoPdfExport : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutGeoPdfExport() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testCollectingFeatures();

  private:

    QString mReport;
};

void TestQgsLayoutGeoPdfExport::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mReport = QStringLiteral( "<h1>GeoPDF Export Tests</h1>\n" );
}

void TestQgsLayoutGeoPdfExport::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsLayoutGeoPdfExport::init()
{
}

void TestQgsLayoutGeoPdfExport::cleanup()
{
}

void TestQgsLayoutGeoPdfExport::testCollectingFeatures()
{
  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsProject p;
  p.addMapLayer( linesLayer );
  p.addMapLayer( pointsLayer );

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer << pointsLayer );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  l.addLayoutItem( map );

  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 150, 80, 40, 50 ) );
  map2->setFrameEnabled( true );
  map2->setLayers( QList<QgsMapLayer *>() << pointsLayer );
  map2->setCrs( linesLayer->crs() );
  map2->zoomToExtent( pointsLayer->extent() );
  map2->setMapRotation( 45 );
  l.addLayoutItem( map2 );

  QgsLayoutGeoPdfExporter geoPdfExporter( &l );

  // trigger render
  QgsLayoutExporter exporter( &l );
  exporter.renderPageToImage( 0 );

  // check that features were collected
  QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures = geoPdfExporter.renderedFeatures( map );
  QCOMPARE( renderedFeatures.count(), 2 );
  QVector< QgsLayoutGeoPdfExporter::RenderedFeature > lineFeatures = renderedFeatures.value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 6 );

  QgsFeature lineFeature1;
  QgsGeometry lineGeometry1;
  for ( auto it = lineFeatures.constBegin(); it != lineFeatures.constEnd(); ++it )
  {
    if ( it->feature.id() == 1 )
    {
      lineFeature1 = it->feature;
      lineGeometry1 = it->renderedBounds;
    }
  }
  QVERIFY( lineFeature1.isValid() );
  QCOMPARE( lineFeature1.attribute( 0 ).toString(), QStringLiteral( "Highway" ) );
  QCOMPARE( lineFeature1.attribute( 1 ).toDouble(), 1.0 );
  QgsDebugMsg( lineGeometry1.asWkt( 1 ) );
  QCOMPARE( lineGeometry1.asWkt( 1 ), QStringLiteral( "MultiLineString ((99.2 20.4, 99.9 22.2, 100.3 22.9, 100.6 23.5, 101.9 24.6, 105.6 26.4, 109.4 28, 110.5 28.9, 112.3 30.8, 112.5 31.1, 112.9 33.3, 113.1 34.4, 113.6 36.1, 113.8 36.8, 114 38.6, 114.3 39.2, 114.9 40.3, 115.4 41.3, 116.7 42.8, 116.9 43.2, 117.1 44.3, 117.1 45.9, 117.1 46.6, 117.4 48.1, 117.8 48.8, 119.3 49.9, 120.5 50.7, 121.8 51.4, 123.1 51.9, 123.3 53, 123.3 53.8, 122.5 55, 122.2 55.4, 121.6 56.5, 121.1 57.2, 120.5 57.8, 120.4 58.1, 120.2 59, 120 59.8, 120 60.5, 120.5 61.8, 120.9 62.5, 120.9 62.5, 122.7 66.5, 121.8 68.4, 121.1 69.1, 119.8 71.3, 119.8 72.7, 118.2 74.2, 117.6 75.1, 116.7 76.6, 116.7 77.3, 116.7 78.2, 116.7 78.9, 116.7 79.8, 117.3 80.6, 117.4 81.5, 117.8 82.2, 118.5 83, 119.3 84, 120.4 85.3, 122.2 88.8, 122.4 89, 123.6 90.2, 124.2 90.8, 125.5 92.6, 126.4 93.4, 128 94.4, 129.1 95.4, 130.4 96.6, 131.3 97.2, 132.6 98.3, 134.2 99.2, 135.5 100.1, 137.5 101.6, 138.2 102.3, 138.8 103.6, 138.8 104.7, 138.8 105.8, 138.6 106.1, 137.9 107.2, 136.8 108.9, 136 109.6, 135.9 110.3, 135.9 111.2, 136 112, 136.6 113.4, 136.8 114.2, 137.9 115.4, 138.4 116, 139.2 117.1, 140.2 118.7, 143.2 120))" ) );

  QVector< QgsLayoutGeoPdfExporter::RenderedFeature > pointFeatures = renderedFeatures.value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 15 );

  QgsFeature pointFeature3;
  QgsGeometry pointGeometry3;
  for ( auto it = pointFeatures.constBegin(); it != pointFeatures.constEnd(); ++it )
  {
    if ( it->feature.id() == 3 )
    {
      pointFeature3 = it->feature;
      pointGeometry3 = it->renderedBounds;
    }
  }
  QVERIFY( pointFeature3.isValid() );
  QCOMPARE( pointFeature3.attribute( 0 ).toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( pointFeature3.attribute( 1 ).toInt(), 95 );
  QCOMPARE( pointFeature3.attribute( 2 ).toDouble(), 3 );
  QCOMPARE( pointFeature3.attribute( 3 ).toInt(), 1 );
  QCOMPARE( pointFeature3.attribute( 4 ).toInt(), 1 );
  QCOMPARE( pointFeature3.attribute( 5 ).toInt(), 2 );
  QCOMPARE( pointGeometry3.asWkt( 1 ), QStringLiteral( "Polygon ((123.8 63.8, 135.1 63.8, 135.1 75, 123.8 75, 123.8 63.8))" ) );

  // check second map
  pointFeatures = geoPdfExporter.renderedFeatures( map2 ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 17 );

  QgsFeature pointFeature3b;
  QgsGeometry pointGeometry3b;
  for ( auto it = pointFeatures.constBegin(); it != pointFeatures.constEnd(); ++it )
  {
    if ( it->feature.id() == 3 )
    {
      pointFeature3b = it->feature;
      pointGeometry3b = it->renderedBounds;
    }
  }
  QVERIFY( pointFeature3b.isValid() );
  QCOMPARE( pointFeature3b.attribute( 0 ).toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( pointFeature3b.attribute( 1 ).toInt(), 95 );
  QCOMPARE( pointFeature3b.attribute( 2 ).toDouble(), 3 );
  QCOMPARE( pointFeature3b.attribute( 3 ).toInt(), 1 );
  QCOMPARE( pointFeature3b.attribute( 4 ).toInt(), 1 );
  QCOMPARE( pointFeature3b.attribute( 5 ).toInt(), 2 );
  QCOMPARE( pointGeometry3b.asWkt( 1 ), QStringLiteral( "Polygon ((167 102, 178.2 102, 178.2 113.3, 167 113.3, 167 102))" ) );
}

QGSTEST_MAIN( TestQgsLayoutGeoPdfExport )
#include "testqgslayoutgeopdfexport.moc"
