/***************************************************************************
     testqgspointcloudlayerexporter.cpp
     -------------------
    Date                 : July 2022
    Copyright            : (C) 2022 Stefanos Natsis
    Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgsconfig.h"

#include <memory>

#include "qgspointcloudlayer.h"
#include "qgspointcloudlayerexporter.h"
#include "qgsproject.h"
#include "qgstest.h"

class TestQgsPointCloudLayerExporter : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.
    void testScratchLayer();
    void testScratchLayerFiltered();
    void testScratchLayerExtent();
    void testScratchLayerZRange();
    void testScratchLayerFilteredByGeometry();
    void testScratchLayerFilteredByLayer();
    void testScratchLayerFilteredByLayerSelected();
    void testScratchLayerFilteredByLayerDifferentCrs();
    void testScratchLayerAttributes();
    void testScratchLayerBadAttributes();
    void testScratchLayerSkipAttributes();
    void testScratchLayerCrs();
    void testScratchLayerSynthetic();
    void testOgrFile();
#ifdef HAVE_PDAL_QGIS
    void testPdalFile();
#endif
  private:
    std::unique_ptr<QgsProject> mProject;
    QgsPointCloudLayer *mLayer;
    QString mTestDataDir;
};

void TestQgsPointCloudLayerExporter::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::registerOgrDrivers();

  mProject = std::make_unique<QgsProject>();

  const QString dataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt

  mLayer = new QgsPointCloudLayer( dataDir + "/point_clouds/ept/rgb/ept.json", "test", "ept" );
  QVERIFY( mLayer->isValid() );
  mProject->addMapLayer( mLayer );
  mProject->setCrs( mLayer->crs() );
}

void TestQgsPointCloudLayerExporter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointCloudLayerExporter::init()
{
}

void TestQgsPointCloudLayerExporter::cleanup()
{
  mLayer->setSubsetString( QString() );
}

void TestQgsPointCloudLayerExporter::testScratchLayer()
{
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), Qgis::WkbType::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  QCOMPARE( result->dataProvider()->featureCount(), 134 );
  QCOMPARE( result->crs(), mLayer->crs() );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 13 );

  QVERIFY( fieldNames.contains( u"Intensity"_s ) );
  QVERIFY( fieldNames.contains( u"ReturnNumber"_s ) );
  QVERIFY( fieldNames.contains( u"NumberOfReturns"_s ) );
  QVERIFY( fieldNames.contains( u"ScanDirectionFlag"_s ) );
  QVERIFY( fieldNames.contains( u"EdgeOfFlightLine"_s ) );
  QVERIFY( fieldNames.contains( u"Classification"_s ) );
  QVERIFY( fieldNames.contains( u"ScanAngleRank"_s ) );
  QVERIFY( fieldNames.contains( u"UserData"_s ) );
  QVERIFY( fieldNames.contains( u"PointSourceId"_s ) );
  QVERIFY( fieldNames.contains( u"GpsTime"_s ) );
  QVERIFY( fieldNames.contains( u"Red"_s ) );
  QVERIFY( fieldNames.contains( u"Green"_s ) );
  QVERIFY( fieldNames.contains( u"Blue"_s ) );


  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFiltered()
{
  mLayer->setSubsetString( u"red > 150"_s );
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 45 );
  QCOMPARE( result->crs(), mLayer->crs() );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerExtent()
{
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFilterExtent( QgsRectangle( 497754, 7050888, 497755, 7050889 ) );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 46 );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerZRange()
{
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setZRange( QgsDoubleRange( 1, 1.1 ) );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 74 );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFilteredByGeometry()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QgsMultiPolygon *geom = new QgsMultiPolygon();
  geom->fromWkt( u"MultiPolygon (((497753.68054185633081943 7050888.42151333577930927, 497753.68262159946607426 7050888.20937953889369965, 497753.93531038763467222 7050888.2218579975888133, 497753.93219077296089381 7050888.28736990503966808, 497753.86355925025418401 7050888.28840977698564529, 497753.85835989244515076 7050888.41215449199080467, 497753.68054185633081943 7050888.42151333577930927)),((497753.63665927684633061 7050887.68445237725973129, 497753.66327998862834647 7050887.93235775642096996, 497753.85794394387630746 7050887.95232329051941633, 497753.86293532734271139 7050887.84084905963391066, 497753.89621121709933504 7050887.66781443264335394, 497753.63665927684633061 7050887.68445237725973129)),((497753.69655587844317779 7050888.15697001200169325, 497753.82134046510327607 7050888.16528898477554321, 497753.87624568323371932 7050888.10871997196227312, 497753.98439232504460961 7050888.13534068409353495, 497754.18072007474256679 7050888.12702171131968498, 497754.25226323772221804 7050888.07544408272951841, 497754.23229770385660231 7050888.00889230240136385, 497754.17739248572615907 7050887.9822715912014246, 497753.99437509197741747 7050887.95731467381119728, 497753.9211681344313547 7050887.94899570103734732, 497753.834650820994284 7050887.98393538594245911, 497753.70154726190958172 7050887.96064226236194372, 497753.69655587844317779 7050888.15697001200169325)))"_s );
  exp.setFilterGeometry( geom );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 56 );

  delete geom;
  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFilteredByLayer()
{
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( u"Polygon?crs=EPSG:32756"_s, u"layer polygon"_s, u"memory"_s );
  QVERIFY( polygonLayer->isValid() );
  polygonLayer->startEditing();
  QgsFeature polygonF1, polygonF2, polygonF3;
  polygonF1.setGeometry( QgsGeometry::fromWkt( u"Polygon ((497753.68054185633081943 7050888.42151333577930927, 497753.68262159946607426 7050888.20937953889369965, 497753.93531038763467222 7050888.2218579975888133, 497753.93219077296089381 7050888.28736990503966808, 497753.86355925025418401 7050888.28840977698564529, 497753.85835989244515076 7050888.41215449199080467, 497753.68054185633081943 7050888.42151333577930927))"_s ) );
  polygonF2.setGeometry( QgsGeometry::fromWkt( u"Polygon ((497753.69655587844317779 7050888.15697001200169325, 497753.82134046510327607 7050888.16528898477554321, 497753.87624568323371932 7050888.10871997196227312, 497753.98439232504460961 7050888.13534068409353495, 497754.18072007474256679 7050888.12702171131968498, 497754.25226323772221804 7050888.07544408272951841, 497754.23229770385660231 7050888.00889230240136385, 497754.17739248572615907 7050887.9822715912014246, 497753.99437509197741747 7050887.95731467381119728, 497753.9211681344313547 7050887.94899570103734732, 497753.834650820994284 7050887.98393538594245911, 497753.70154726190958172 7050887.96064226236194372, 497753.69655587844317779 7050888.15697001200169325))"_s ) );
  polygonF3.setGeometry( QgsGeometry::fromWkt( u"Polygon (((497753.63665927684633061 7050887.68445237725973129, 497753.66327998862834647 7050887.93235775642096996, 497753.85794394387630746 7050887.95232329051941633, 497753.86293532734271139 7050887.84084905963391066, 497753.89621121709933504 7050887.66781443264335394, 497753.63665927684633061 7050887.68445237725973129))"_s ) );
  polygonLayer->addFeature( polygonF1 );
  polygonLayer->addFeature( polygonF2 );
  polygonLayer->addFeature( polygonF3 );
  polygonLayer->commitChanges();

  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFilterGeometry( polygonLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 56 );

  delete polygonLayer;
  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFilteredByLayerSelected()
{
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( u"Polygon?crs=EPSG:32756"_s, u"layer polygon"_s, u"memory"_s );
  QVERIFY( polygonLayer->isValid() );
  polygonLayer->startEditing();
  QgsFeature polygonF1, polygonF2, polygonF3;
  polygonF1.setGeometry( QgsGeometry::fromWkt( u"Polygon ((497753.68054185633081943 7050888.42151333577930927, 497753.68262159946607426 7050888.20937953889369965, 497753.93531038763467222 7050888.2218579975888133, 497753.93219077296089381 7050888.28736990503966808, 497753.86355925025418401 7050888.28840977698564529, 497753.85835989244515076 7050888.41215449199080467, 497753.68054185633081943 7050888.42151333577930927))"_s ) );
  polygonF2.setGeometry( QgsGeometry::fromWkt( u"Polygon ((497753.69655587844317779 7050888.15697001200169325, 497753.82134046510327607 7050888.16528898477554321, 497753.87624568323371932 7050888.10871997196227312, 497753.98439232504460961 7050888.13534068409353495, 497754.18072007474256679 7050888.12702171131968498, 497754.25226323772221804 7050888.07544408272951841, 497754.23229770385660231 7050888.00889230240136385, 497754.17739248572615907 7050887.9822715912014246, 497753.99437509197741747 7050887.95731467381119728, 497753.9211681344313547 7050887.94899570103734732, 497753.834650820994284 7050887.98393538594245911, 497753.70154726190958172 7050887.96064226236194372, 497753.69655587844317779 7050888.15697001200169325))"_s ) );
  polygonF3.setGeometry( QgsGeometry::fromWkt( u"Polygon (((497753.63665927684633061 7050887.68445237725973129, 497753.66327998862834647 7050887.93235775642096996, 497753.85794394387630746 7050887.95232329051941633, 497753.86293532734271139 7050887.84084905963391066, 497753.89621121709933504 7050887.66781443264335394, 497753.63665927684633061 7050887.68445237725973129))"_s ) );
  polygonLayer->addFeature( polygonF1 );
  polygonLayer->addFeature( polygonF2 );
  polygonLayer->addFeature( polygonF3 );
  polygonLayer->commitChanges();

  const QgsFeatureIds ids { 1, 2 };
  polygonLayer->selectByIds( ids );

  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFilterGeometry( polygonLayer, true );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 46 );

  delete polygonLayer;
  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFilteredByLayerDifferentCrs()
{
  QgsVectorLayer *polygonLayer = new QgsVectorLayer( u"Polygon?crs=EPSG:32757"_s, u"layer polygon"_s, u"memory"_s );
  QVERIFY( polygonLayer->isValid() );
  polygonLayer->startEditing();
  QgsFeature polygonF1, polygonF2, polygonF3;
  polygonF1.setGeometry( QgsGeometry::fromWkt( u"Polygon ((-99965.72217222594190389 7036704.40407544746994972, -99965.71004615712445229 7036704.19133298099040985, -99965.4571056473068893 7036704.21581172198057175, -99965.46333606250118464 7036704.28139435965567827, -99965.53224556404165924 7036704.27918965928256512, -99965.54331856453791261 7036704.40310078114271164, -99965.72217222594190389 7036704.40407544746994972))"_s ) );
  polygonF2.setGeometry( QgsGeometry::fromWkt( u"Polygon ((-99965.69358511152677238 7036704.13940821029245853, -99965.5687782890163362 7036704.1536604380235076, -99965.51101288676727563 7036704.09950129874050617, -99965.40376561181619763 7036704.13132886588573456, -99965.20638975047040731 7036704.13227352499961853, -99965.13216716633178294 7036704.08390980958938599, -99965.1490496383048594 7036704.01619131304323673, -99965.20287802210077643 7036703.9868834363296628, -99965.38532435230445117 7036703.95318189449608326, -99965.45838162454310805 7036703.94137061573565006, -99965.54684086830820888 7036703.97233226522803307, -99965.67928573116660118 7036703.94266227260231972, -99965.69358511152677238 7036704.13940821029245853))"_s ) );
  polygonF3.setGeometry( QgsGeometry::fromWkt( u"Polygon ((-99965.73131910467054695 7036703.66248090378940105, -99965.71634197188541293 7036703.91247245855629444, -99965.52197403809987009 7036703.94171716459095478, -99965.51169041451066732 7036703.83010758273303509, -99965.47011460980866104 7036703.65807099640369415, -99965.73131910467054695 7036703.66248090378940105))"_s ) );
  polygonLayer->addFeature( polygonF1 );
  polygonLayer->addFeature( polygonF2 );
  polygonLayer->addFeature( polygonF3 );
  polygonLayer->commitChanges();

  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFilterGeometry( polygonLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 56 );

  delete polygonLayer;
  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { u"Red"_s, u"Green"_s, u"Blue"_s };
  exp.setAttributes( attrs );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 134 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 3 );

  QVERIFY( fieldNames.contains( u"Red"_s ) );
  QVERIFY( fieldNames.contains( u"Green"_s ) );
  QVERIFY( fieldNames.contains( u"Blue"_s ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerBadAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { u"Red"_s, u"Red"_s, u"MissingAttribute"_s };
  exp.setAttributes( attrs );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 134 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 1 );
  QVERIFY( fieldNames.contains( u"Red"_s ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerSkipAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs;
  exp.setAttributes( attrs );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 134 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 0 );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerCrs()
{
  QgsPointCloudLayerExporter exp( mLayer );

  QgsCoordinateReferenceSystem differentCrs = QgsCoordinateReferenceSystem::fromEpsgId( 2100 );
  exp.setCrs( differentCrs );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), Qgis::WkbType::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  QCOMPARE( result->dataProvider()->featureCount(), 134 );
  QCOMPARE( result->crs(), differentCrs );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerSynthetic()
{
  mLayer->setSubsetString( u"red > 150"_s );

  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { u"Red"_s, u"Red"_s, u"MissingAttribute"_s };
  exp.setAttributes( attrs );
  exp.setFilterExtent( QgsRectangle( 497754, 7050888, 497755, 7050889 ) );
  exp.setZRange( QgsDoubleRange( 1, 1.1 ) );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Memory );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 9 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 1 );
  QVERIFY( fieldNames.contains( u"Red"_s ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testOgrFile()
{
  const QString file = QDir::tempPath() + "/filename.gpkg";
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Gpkg );
  exp.setFileName( file );
  exp.doExport();

  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), Qgis::WkbType::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  delete result;
}

#ifdef HAVE_PDAL_QGIS
void TestQgsPointCloudLayerExporter::testPdalFile()
{
  const QString file = QDir::tempPath() + "/filename.laz";
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( QgsPointCloudLayerExporter::ExportFormat::Las );
  exp.setFileName( file );
  exp.doExport();

  QgsPointCloudLayer *result = qgis::down_cast<QgsPointCloudLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );

  QCOMPARE( result->pointCount(), 134 );
  delete result;
}
#endif

QGSTEST_MAIN( TestQgsPointCloudLayerExporter )
#include "testqgspointcloudlayerexporter.moc"
