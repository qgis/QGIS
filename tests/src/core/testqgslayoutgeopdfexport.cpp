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
#include "qgsmapthemecollection.h"
#include <gdal.h>

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
    void skipLayers();
    void layerOrder();

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
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/polys.shp" ),
      QStringLiteral( "polys" ), QStringLiteral( "ogr" ) );
  QVERIFY( polygonLayer->isValid() );
  pointsLayer->setDisplayExpression( QStringLiteral( "Staff" ) );

  QgsProject p;
  p.addMapLayer( linesLayer );
  p.addMapLayer( pointsLayer );
  p.addMapLayer( polygonLayer );

  QgsMapThemeCollection::MapThemeRecord rec;
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( linesLayer )
                     );

  p.mapThemeCollection()->insert( QStringLiteral( "test preset" ), rec );
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( linesLayer )
                       << QgsMapThemeCollection::MapThemeLayerRecord( pointsLayer )
                     );
  p.mapThemeCollection()->insert( QStringLiteral( "test preset2" ), rec );
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( polygonLayer )
                     );
  p.mapThemeCollection()->insert( QStringLiteral( "test preset3" ), rec );

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
  QgsFeatureList lineFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( linesLayer->id() );
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
  QgsDebugMsg( lineGeometry1.asWkt( 0 ) );
  QCOMPARE( lineGeometry1.asWkt( 0 ), QStringLiteral( "MultiLineString ((281 538, 283 532, 284 530, 285 529, 289 526, 299 520, 310 516, 313 513, 318 508, 319 507, 320 501, 320 498, 322 493, 323 491, 323 486, 324 484, 326 481, 327 478, 331 474, 331 473, 332 470, 332 465, 332 463, 333 459, 334 457, 338 454, 342 452, 345 450, 349 448, 349 445, 349 443, 347 439, 346 438, 345 435, 343 433, 342 432, 341 430, 341 428, 340 426, 340 424, 342 420, 343 418, 343 418, 348 407, 345 402, 343 399, 340 393, 340 389, 335 385, 333 382, 331 378, 331 376, 331 374, 331 372, 331 369, 332 367, 333 364, 334 362, 336 360, 338 357, 341 353, 346 344, 347 343, 350 339, 352 338, 356 333, 358 331, 363 328, 366 325, 370 321, 372 320, 376 317, 380 314, 384 312, 390 307, 392 305, 393 302, 393 299, 393 295, 393 294, 391 291, 388 287, 386 285, 385 283, 385 280, 386 278, 387 274, 388 272, 391 268, 392 267, 394 263, 398 259, 406 255))" ) );

  QgsFeatureList  pointFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( pointsLayer->id() );
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
  QCOMPARE( pointGeometry3.asWkt( 0 ), QStringLiteral( "MultiPolygon (((473 306, 505 306, 505 274, 473 274, 473 306)))" ) );

  // check second map
  QgsFeatureList polyFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( polygonLayer->id() );
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
  QgsDebugMsg( polyGeometry3b.asWkt( 0 ) );
  QCOMPARE( polyGeometry3b.asWkt( 0 ), QStringLiteral( "MultiPolygon (((469 306, 469 305, 468 305, 468 305, 467 305, 466 304, 466 304, 466 303, 467 303, 467 302, 468 302, 469 302, 470 302, 470 303, 471 303, 472 303, 473 303, 474 303, 474 303, 475 303, 476 303, 476 303, 478 300, 478 299, 478 299, 478 298, 478 296, 477 296, 476 295, 476 295, 475 294, 474 294, 474 294, 473 295, 472 295, 472 296, 471 296, 470 297, 469 297, 468 297, 466 296, 464 296, 463 296, 462 297, 462 298, 462 298, 461 299, 460 299, 459 300, 458 300, 458 301, 458 301, 458 302, 459 303, 459 303, 458 304, 458 304, 458 305, 458 306, 458 307, 458 308, 458 308, 459 309, 460 309, 460 310, 461 311, 462 311, 462 312, 463 312, 464 312, 465 312, 465 311, 467 310, 467 309, 468 308, 469 307, 469 306)))" ) );

  // finalize and test collation
  QgsAbstractGeoPdfExporter::ExportDetails details;
  details.pageSizeMm = QSizeF( 297, 210 );
  const bool expected = true;
  QCOMPARE( geoPdfExporter.finalize( QList<QgsAbstractGeoPdfExporter::ComponentLayerDetail>(), outputFile, details ), expected );
  QVERIFY( geoPdfExporter.errorMessage().isEmpty() );

  QgsAbstractGeoPdfExporter::VectorComponentDetail vectorDetail;
  for ( const auto &it :  geoPdfExporter.mVectorComponents )
  {
    if ( it.mapLayerId == linesLayer->id() )
      vectorDetail = it;
  }

  // read in as vector
  std::unique_ptr< QgsVectorLayer > layer1 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer1->isValid() );
  QCOMPARE( layer1->featureCount(), 6L );
  for ( const auto &it :  geoPdfExporter.mVectorComponents )
  {
    if ( it.mapLayerId == pointsLayer->id() )
      vectorDetail = it;
  }
  std::unique_ptr< QgsVectorLayer > layer2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer2->isValid() );
  QCOMPARE( layer2->featureCount(), 32L );
  for ( const auto &it :  geoPdfExporter.mVectorComponents )
  {
    if ( it.mapLayerId == polygonLayer->id() )
      vectorDetail = it;
  }
  std::unique_ptr< QgsVectorLayer > layer3 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer3->isValid() );
  QCOMPARE( layer3->featureCount(), 10L );

  // test for theme based export here!
  map2->setFollowVisibilityPreset( true );
  map2->setFollowVisibilityPresetName( QStringLiteral( "test preset3" ) );

  QgsLayoutGeoPdfExporter geoPdfExporter2( &l );
  settings = QgsLayoutExporter::PdfExportSettings();
  settings.writeGeoPdf = true;
  settings.exportMetadata = false;
  settings.exportThemes = QStringList() << QStringLiteral( "test preset2" ) << QStringLiteral( "test preset" ) << QStringLiteral( "test preset3" );
  exporter.exportToPdf( outputFile, settings );

  // check that features were collected
  lineFeatures = geoPdfExporter2.mCollatedFeatures.value( QString() ).value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 0 );
  lineFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset2" ) ).value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 6 );
  lineFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset" ) ).value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 6 );
  lineFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset3" ) ).value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 0 );

  pointFeatures = geoPdfExporter2.mCollatedFeatures.value( QString() ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 0 );
  pointFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset2" ) ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 15 );
  pointFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset" ) ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 0 );
  pointFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset3" ) ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 0 );

  polyFeatures = geoPdfExporter2.mCollatedFeatures.value( QString() ).value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 10 );
  polyFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset2" ) ).value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 0 );
  polyFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset" ) ).value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 0 );
  polyFeatures = geoPdfExporter2.mCollatedFeatures.value( QStringLiteral( "test preset3" ) ).value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 10 );

  // finalize and test collation
  details = QgsAbstractGeoPdfExporter::ExportDetails();
  details.pageSizeMm = QSizeF( 297, 210 );
  QCOMPARE( geoPdfExporter2.finalize( QList<QgsAbstractGeoPdfExporter::ComponentLayerDetail>(), outputFile, details ), expected );
  QVERIFY( geoPdfExporter2.errorMessage().isEmpty() );

  vectorDetail = QgsAbstractGeoPdfExporter::VectorComponentDetail();
  for ( const auto &it : geoPdfExporter2.mVectorComponents )
  {
    if ( it.mapLayerId == linesLayer->id() && it.group == QLatin1String( "test preset2" ) )
      vectorDetail = it;
  }

  // read in as vector
  layer1 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
           QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer1->isValid() );
  QCOMPARE( layer1->featureCount(), 6L );
  vectorDetail = QgsAbstractGeoPdfExporter::VectorComponentDetail();
  for ( const auto &it : geoPdfExporter2.mVectorComponents )
  {
    if ( it.mapLayerId == linesLayer->id() && it.group == QLatin1String( "test preset" ) )
      vectorDetail = it;
  }

  // read in as vector
  layer1 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
           QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer1->isValid() );
  QCOMPARE( layer1->featureCount(), 6L );
  vectorDetail = QgsAbstractGeoPdfExporter::VectorComponentDetail();
  for ( const auto &it :  geoPdfExporter2.mVectorComponents )
  {
    if ( it.mapLayerId == pointsLayer->id() && it.group == QLatin1String( "test preset2" ) )
      vectorDetail = it;
  }
  layer2 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
           QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer2->isValid() );
  QCOMPARE( layer2->featureCount(), 15L );
  vectorDetail = QgsAbstractGeoPdfExporter::VectorComponentDetail();
  for ( const auto &it :  geoPdfExporter2.mVectorComponents )
  {
    if ( it.mapLayerId == polygonLayer->id()  && it.group.isEmpty() )
      vectorDetail = it;
  }
  layer3 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
           QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer3->isValid() );
  QCOMPARE( layer3->featureCount(), 10L );
  vectorDetail = QgsAbstractGeoPdfExporter::VectorComponentDetail();
  for ( const auto &it :  geoPdfExporter2.mVectorComponents )
  {
    if ( it.mapLayerId == polygonLayer->id() && it.group == QLatin1String( "test preset3" ) )
      vectorDetail = it;
  }
  layer3 = std::make_unique< QgsVectorLayer >( QStringLiteral( "%1|layername=%2" ).arg( vectorDetail.sourceVectorPath, vectorDetail.sourceVectorLayer ),
           QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( layer3->isValid() );
  QCOMPARE( layer3->featureCount(), 10L );
}

void TestQgsLayoutGeoPdfExport::skipLayers()
{
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
  linesLayer->setCustomProperty( QStringLiteral( "geopdf/includeFeatures" ), false );
  pointsLayer->setCustomProperty( QStringLiteral( "geopdf/includeFeatures" ), true );
  // nothing specifically set for polygonLayer => should be included

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer << pointsLayer << polygonLayer );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  map->setBackgroundColor( QColor( 200, 220, 230 ) );
  map->setBackgroundEnabled( true );
  l.addLayoutItem( map );

  const QgsLayoutGeoPdfExporter geoPdfExporter( &l );

  // trigger render
  QgsLayoutExporter exporter( &l );

  const QString outputFile = geoPdfExporter.generateTemporaryFilepath( QStringLiteral( "test_src.pdf" ) );
  QgsLayoutExporter::PdfExportSettings settings;
  settings.writeGeoPdf = true;
  settings.exportMetadata = false;
  exporter.exportToPdf( outputFile, settings );

  // check that features were collected
  const QgsFeatureList lineFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( linesLayer->id() );
  QCOMPARE( lineFeatures.count(), 0 ); // should be nothing, layer is set to skip
  const QgsFeatureList  pointFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( pointsLayer->id() );
  QCOMPARE( pointFeatures.count(), 15 ); // should be features, layer was set to export
  const QgsFeatureList polyFeatures = geoPdfExporter.mCollatedFeatures.value( QString() ).value( polygonLayer->id() );
  QCOMPARE( polyFeatures.count(), 10 ); // should be features, layer did not have any setting set
}

void TestQgsLayoutGeoPdfExport::layerOrder()
{
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

  const QgsLayoutGeoPdfExporter geoPdfExporter( &l );
  // by default we should follow project layer order
  QCOMPARE( geoPdfExporter.layerOrder(), QStringList() << polygonLayer->id() << pointsLayer->id() << linesLayer->id() );

  // but if a custom order is specified, respected that
  l.setCustomProperty( QStringLiteral( "pdfLayerOrder" ), QStringLiteral( "%1~~~%2" ).arg( linesLayer->id(), polygonLayer->id() ) );
  const QgsLayoutGeoPdfExporter geoPdfExporter2( &l );
  QCOMPARE( geoPdfExporter2.layerOrder(), QStringList() << linesLayer->id() << polygonLayer->id() << pointsLayer->id() );
}

QGSTEST_MAIN( TestQgsLayoutGeoPdfExport )
#include "testqgslayoutgeopdfexport.moc"
