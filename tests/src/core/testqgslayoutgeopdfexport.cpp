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
  QCOMPARE( lineGeometry1.asWkt( 1 ), QStringLiteral( "MultiLineString ((79.2 0.4, 79.9 2.2, 80.3 2.9, 80.6 3.5, 81.9 4.6, 85.6 6.4, 89.4 8, 90.5 8.9, 92.3 10.8, 92.5 11.1, 92.9 13.3, 93.1 14.4, 93.6 16.1, 93.8 16.8, 94 18.6, 94.3 19.2, 94.9 20.3, 95.4 21.3, 96.7 22.8, 96.9 23.2, 97.1 24.3, 97.1 25.9, 97.1 26.6, 97.4 28.1, 97.8 28.8, 99.3 29.9, 100.5 30.7, 101.8 31.4, 103.1 31.9, 103.3 33, 103.3 33.8, 102.5 35, 102.2 35.4, 101.6 36.5, 101.1 37.2, 100.5 37.8, 100.4 38.1, 100.2 39, 100 39.8, 100 40.5, 100.5 41.8, 100.9 42.5, 100.9 42.5, 102.7 46.5, 101.8 48.4, 101.1 49.1, 99.8 51.3, 99.8 52.7, 98.2 54.2, 97.6 55.1, 96.7 56.6, 96.7 57.3, 96.7 58.2, 96.7 58.9, 96.7 59.8, 97.3 60.6, 97.4 61.5, 97.8 62.2, 98.5 63, 99.3 64, 100.4 65.3, 102.2 68.8, 102.4 69, 103.6 70.2, 104.2 70.8, 105.5 72.6, 106.4 73.4, 108 74.4, 109.1 75.4, 110.4 76.6, 111.3 77.2, 112.6 78.3, 114.2 79.2, 115.5 80.1, 117.5 81.6, 118.2 82.3, 118.8 83.6, 118.8 84.7, 118.8 85.8, 118.6 86.1, 117.9 87.2, 116.8 88.9, 116 89.6, 115.9 90.3, 115.9 91.2, 116 92, 116.6 93.4, 116.8 94.2, 117.9 95.4, 118.4 96, 119.2 97.1, 120.2 98.7, 123.2 100))" ) );

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
  QCOMPARE( pointGeometry3.asWkt( 1 ), QStringLiteral( "Polygon ((103.8 43.8, 115.1 43.8, 115.1 55, 103.8 55, 103.8 43.8))" ) );
}

QGSTEST_MAIN( TestQgsLayoutGeoPdfExport )
#include "testqgslayoutgeopdfexport.moc"
