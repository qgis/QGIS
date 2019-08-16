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
  if ( !QgsAbstractGeoPdfExporter::geoPDFCreationAvailable() )
  {
    QSKIP( "This test requires GeoPDF creation abilities", SkipSingle );
  }

  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ),
      QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QVERIFY( polygonLayer->isValid() );
  pointsLayer->setDisplayExpression( QStringLiteral( "Staff" ) );

  QgsProject p;
  p.addMapLayer( linesLayer );
  p.addMapLayer( pointsLayer );
  p.addMapLayer( polygonLayer );

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer << pointsLayer );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  map->setBackgroundColor( QColor( 200, 220, 230 ) );
  map->setBackgroundEnabled( true );
  l.addLayoutItem( map );

  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 150, 80, 40, 50 ) );
  map2->setFrameEnabled( true );
  map2->setLayers( QList<QgsMapLayer *>() << pointsLayer << polygonLayer );
  map2->setCrs( linesLayer->crs() );
  map2->zoomToExtent( pointsLayer->extent() );
  map2->setMapRotation( 45 );
  map2->setBackgroundColor( QColor( 240, 230, 200 ) );
  map2->setBackgroundEnabled( true );
  l.addLayoutItem( map2 );

  QgsLayoutGeoPdfExporter geoPdfExporter( &l );

  // trigger render
  QgsLayoutExporter exporter( &l );

  const QString outputFile = geoPdfExporter.generateTemporaryFilepath( QStringLiteral( "test_src.pdf" ) );
  QgsLayoutExporter::PdfExportSettings settings;
  settings.writeGeoPdf = true;
  settings.exportMetadata = false;
  exporter.exportToPdf( outputFile, settings );

  // check that features were collected
  QgsFeatureList lineFeatures = geoPdfExporter.mCollatedFeatures.value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 6 );

  QgsFeature lineFeature1;
  QgsGeometry lineGeometry1;
  for ( auto it = lineFeatures.constBegin(); it != lineFeatures.constEnd(); ++it )
  {
    if ( it->id() == 1 )
    {
      lineFeature1 = *it;
      lineGeometry1 = it->geometry();
    }
  }
  QVERIFY( lineFeature1.isValid() );
  QCOMPARE( lineFeature1.attribute( 0 ).toString(), QStringLiteral( "Highway" ) );
  QCOMPARE( lineFeature1.attribute( 1 ).toDouble(), 1.0 );
  QgsDebugMsg( lineGeometry1.asWkt( 1 ) );
  QCOMPARE( lineGeometry1.asWkt( 1 ), QStringLiteral( "MultiLineString ((281.2 537.3, 283.3 532.1, 284.3 530.1, 285.3 528.5, 289 525.4, 299.3 520.2, 310.2 515.6, 313.3 513, 318.5 507.8, 319 506.8, 320 500.6, 320.5 497.5, 322.1 492.8, 322.6 490.8, 323.1 485.6, 324.1 484, 325.7 480.9, 327.2 477.8, 330.9 473.7, 331.4 472.7, 331.9 469.6, 331.9 464.9, 331.9 462.9, 332.9 458.7, 334 456.6, 338.1 453.5, 341.7 451.5, 345.4 449.4, 349 447.9, 349.5 444.8, 349.5 442.7, 347.4 439.1, 346.4 438, 344.8 434.9, 343.3 432.9, 341.7 431.3, 341.2 430.3, 340.7 427.7, 340.2 425.6, 340.2 423.6, 341.7 419.9, 342.8 417.9, 342.8 417.9, 347.9 406.5, 345.4 401.3, 343.3 399.3, 339.7 393.1, 339.7 388.9, 335 384.8, 333.5 382.2, 330.9 378.1, 330.9 376, 330.9 373.4, 330.9 371.3, 330.9 368.8, 332.4 366.7, 332.9 364.1, 334 362, 336 360, 338.1 356.9, 341.2 353.3, 346.4 343.4, 346.9 342.9, 350.5 339.3, 352.1 337.7, 355.7 332.6, 358.3 330.5, 362.9 327.4, 366 324.8, 369.7 321.2, 372.2 319.6, 375.9 316.5, 380.5 314, 384.1 311.4, 389.8 307.2, 391.9 305.2, 393.5 301.6, 393.5 298.5, 393.5 295.3, 392.9 294.3, 390.9 291.2, 387.8 286.6, 385.7 284.5, 385.2 282.4, 385.2 279.8, 385.7 277.8, 387.3 273.6, 387.8 271.6, 390.9 267.9, 392.4 266.4, 394.5 263.3, 397.6 258.6, 405.9 255))" ) );

  QgsFeatureList  pointFeatures = geoPdfExporter.mCollatedFeatures.value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 32 );

  QgsFeature pointFeature3;
  QgsGeometry pointGeometry3;
  for ( auto it = pointFeatures.constBegin(); it != pointFeatures.constEnd(); ++it )
  {
    if ( it->id() == 3 )
    {
      pointFeature3 = *it;
      pointGeometry3 = it->geometry();
    }
  }
  QVERIFY( pointFeature3.isValid() );
  QCOMPARE( pointFeature3.attribute( 0 ).toString(), QStringLiteral( "Jet" ) );
  QCOMPARE( pointFeature3.attribute( 1 ).toInt(), 95 );
  QCOMPARE( pointFeature3.attribute( 2 ).toDouble(), 3.0 );
  QCOMPARE( pointFeature3.attribute( 3 ).toInt(), 1 );
  QCOMPARE( pointFeature3.attribute( 4 ).toInt(), 1 );
  QCOMPARE( pointFeature3.attribute( 5 ).toInt(), 2 );
  QCOMPARE( pointGeometry3.asWkt( 1 ), QStringLiteral( "MultiPolygon (((473.4 305.9, 505.3 305.9, 505.3 274.1, 473.4 274.1, 473.4 305.9)))" ) );

  // check second map
  QgsFeatureList polyFeatures = geoPdfExporter.mCollatedFeatures.value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 10 );

  QgsFeature polyFeature3b;
  QgsGeometry polyGeometry3b;
  for ( auto it = polyFeatures.constBegin(); it != polyFeatures.constEnd(); ++it )
  {
    if ( it->id() == 3 )
    {
      polyFeature3b = *it;
      polyGeometry3b = it->geometry();
    }
  }
  QVERIFY( polyFeature3b.isValid() );
  QCOMPARE( polyFeature3b.attribute( 0 ).toString(), QStringLiteral( "Dam" ) );
  QCOMPARE( polyFeature3b.attribute( 1 ).toInt(), 8 );
  QgsDebugMsg( polyGeometry3b.asWkt( 1 ) );
  QCOMPARE( polyGeometry3b.asWkt( 1 ), QStringLiteral( "MultiPolygon (((468.8 306.3, 468.8 305.2, 468.4 305, 467.8 305, 467.1 305, 466.4 304.2, 466.4 303.8, 466.2 303.3, 466.7 302.6, 467.2 302, 467.7 301.6, 468.6 301.5, 469.6 302.1, 470.4 302.5, 470.9 302.7, 472.1 302.7, 472.8 302.9, 473.6 302.7, 474.2 303, 474.9 303.3, 475.7 303.3, 476.5 303, 478 300.3, 478.1 299.3, 478.1 298.7, 477.8 297.5, 477.7 296.3, 477.5 296.1, 476.3 294.8, 475.6 294.4, 475.4 294.3, 473.9 294, 473.7 294.1, 473 294.4, 472.2 295, 471.6 295.6, 470.8 296.4, 469.6 297, 469.5 297.1, 467.8 296.7, 466 296.3, 463.9 296.1, 463.2 296.1, 462.3 296.7, 461.8 297.7, 461.6 298.1, 460.9 298.5, 459.8 299.3, 459.2 299.5, 458.5 300.1, 458.2 300.5, 458.3 301.2, 458.5 301.6, 458.7 302.4, 458.7 302.8, 458.3 303.6, 457.8 304.1, 457.7 305.1, 458 306, 458.2 306.6, 458.5 307.6, 458.5 307.6, 459.4 308.9, 459.7 309.2, 460.5 310, 461.1 310.4, 461.9 311, 462.4 311.4, 463.2 311.6, 464 311.7, 464.6 311.7, 465.3 311.1, 466.6 309.9, 467.3 309.1, 468.3 307.8, 468.9 306.7, 468.8 306.3)))" ) );

  // finalize and test collation
  QgsAbstractGeoPdfExporter::ExportDetails details;
  QVERIFY( geoPdfExporter.finalize( QList<QgsAbstractGeoPdfExporter::ComponentLayerDetail>(), outputFile, details ) );
  QVERIFY( geoPdfExporter.errorMessage().isEmpty() );

  // read in as vector
  std::unique_ptr< QgsVectorLayer > layer1 = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=lines" ).arg( outputFile ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer1->isValid() );
  QCOMPARE( layer1->featureCount(), 6L );
  std::unique_ptr< QgsVectorLayer > layer2 = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=points" ).arg( outputFile ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer2->isValid() );
  QCOMPARE( layer2->featureCount(), 32L );
  std::unique_ptr< QgsVectorLayer > layer3 = qgis::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=polys" ).arg( outputFile ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer3->isValid() );
  QCOMPARE( layer3->featureCount(), 10L );
}

QGSTEST_MAIN( TestQgsLayoutGeoPdfExport )
#include "testqgslayoutgeopdfexport.moc"
