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
  QMap< QString, QVector< QgsLayoutGeoPdfExporter::RenderedFeature > > renderedFeatures = geoPdfExporter.renderedFeatures();
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
  QCOMPARE( lineGeometry1.asWkt( 1 ), QStringLiteral( "MultiLineString ((935.3 4.3, 943.9 25.9, 948.2 34.5, 952.6 40.9, 967.6 53.9, 1010.7 75.4, 1056 94.8, 1068.9 105.6, 1090.5 127.2, 1092.6 131.5, 1097 157.3, 1099.1 170.3, 1105.6 189.6, 1107.7 198.3, 1109.9 219.8, 1114.2 226.3, 1120.7 239.2, 1127.1 252.1, 1142.2 269.4, 1144.4 273.7, 1146.5 286.6, 1146.5 306, 1146.5 314.6, 1150.8 331.9, 1155.1 340.5, 1172.4 353.4, 1187.5 362.1, 1202.6 370.7, 1217.6 377.1, 1219.8 390.1, 1219.8 398.7, 1211.2 413.8, 1206.9 418.1, 1200.4 431, 1193.9 439.6, 1187.5 446.1, 1185.3 450.4, 1183.2 461.2, 1181 469.8, 1181 478.4, 1187.5 493.5, 1191.8 502.1, 1191.8 502.1, 1213.3 549.6, 1202.6 571.1, 1193.9 579.7, 1178.8 605.6, 1178.8 622.8, 1159.4 640.1, 1153 650.8, 1142.2 668.1, 1142.2 676.7, 1142.2 687.5, 1142.2 696.1, 1142.2 706.9, 1148.7 715.5, 1150.8 726.3, 1155.1 734.9, 1163.8 743.5, 1172.4 756.4, 1185.3 771.5, 1206.9 812.5, 1209 814.6, 1224.1 829.7, 1230.6 836.2, 1245.7 857.7, 1256.4 866.4, 1275.8 879.3, 1288.8 890.1, 1303.8 905.1, 1314.6 911.6, 1329.7 924.5, 1349.1 935.3, 1364.2 946.1, 1387.9 963.3, 1396.5 972, 1403 987, 1403 1000, 1403 1012.9, 1400.8 1017.2, 1392.2 1030.1, 1379.3 1049.5, 1370.6 1058.2, 1368.5 1066.8, 1368.5 1077.6, 1370.6 1086.2, 1377.1 1103.4, 1379.3 1112, 1392.2 1127.1, 1398.7 1133.6, 1407.3 1146.5, 1420.2 1165.9, 1454.7 1181))" ) );

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
  QCOMPARE( pointGeometry3.asWkt( 1 ), QStringLiteral( "Polygon ((1226.4 516.9, 1359.2 516.9, 1359.2 649.7, 1226.4 649.7, 1226.4 516.9))" ) );
}

QGSTEST_MAIN( TestQgsLayoutGeoPdfExport )
#include "testqgslayoutgeopdfexport.moc"
