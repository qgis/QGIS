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
#include "qgstest.h"
#include "qgsproject.h"
#include "qgspointcloudlayer.h"
#include "qgspointcloudattribute.h"
#include "qgspointcloudlayerexporter.h"

class TestQgsPointCloudLayerExporter: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testScratchLayer();
    void testScratchLayerFiltered();
    void testScratchLayerExtent();
    void testScratchLayerZRange();
    void testScratchLayerAttributes();
    void testScratchLayerBadAttributes();
    void testScratchLayerSkipAttributes();
    void testScratchLayerCrs();
    void testScratchLayerSynthetic();
    void testOgrFile();
    void testPdalFile();

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

  mProject.reset( new QgsProject );

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
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  QCOMPARE( result->dataProvider()->featureCount(), 134 );
  QCOMPARE( result->crs(), mLayer->crs() );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 13 );

  QVERIFY( fieldNames.contains( QStringLiteral( "Intensity" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "ReturnNumber" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "NumberOfReturns" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "ScanDirectionFlag" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "EdgeOfFlightLine" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Classification" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "ScanAngleRank" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "UserData" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "PointSourceId" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "GpsTime" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Red" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Green" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Blue" ) ) );


  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerFiltered()
{
  mLayer->setSubsetString( QStringLiteral( "red > 150" ) );
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( QStringLiteral( "memory" ) );
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
  exp.setFormat( QStringLiteral( "memory" ) );
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
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 74 );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { QStringLiteral( "Red" ),
                      QStringLiteral( "Green" ),
                      QStringLiteral( "Blue" )
                    };
  exp.setAttributes( attrs );
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 134 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 3 );

  QVERIFY( fieldNames.contains( QStringLiteral( "Red" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Green" ) ) );
  QVERIFY( fieldNames.contains( QStringLiteral( "Blue" ) ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerBadAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { QStringLiteral( "Red" ),
                      QStringLiteral( "Red" ),
                      QStringLiteral( "MissingAttribute" )
                    };
  exp.setAttributes( attrs );
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 134 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 1 );
  QVERIFY( fieldNames.contains( QStringLiteral( "Red" ) ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerSkipAttributes()
{
  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs;
  exp.setAttributes( attrs );
  exp.setFormat( QStringLiteral( "memory" ) );
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
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  QCOMPARE( result->dataProvider()->featureCount(), 134 );
  QCOMPARE( result->crs(), differentCrs );

  delete result;
}

void TestQgsPointCloudLayerExporter::testScratchLayerSynthetic()
{
  mLayer->setSubsetString( QStringLiteral( "red > 150" ) );

  QgsPointCloudLayerExporter exp( mLayer );
  QStringList attrs { QStringLiteral( "Red" ),
                      QStringLiteral( "Red" ),
                      QStringLiteral( "MissingAttribute" )
                    };
  exp.setAttributes( attrs );
  exp.setFilterExtent( QgsRectangle( 497754, 7050888, 497755, 7050889 ) );
  exp.setZRange( QgsDoubleRange( 1, 1.1 ) );
  exp.setFormat( QStringLiteral( "memory" ) );
  exp.doExport();
  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->featureCount(), 9 );

  const auto fieldNames = result->fields().names();
  QCOMPARE( fieldNames.length(), 1 );
  QVERIFY( fieldNames.contains( QStringLiteral( "Red" ) ) );

  delete result;
}

void TestQgsPointCloudLayerExporter::testOgrFile()
{
  const QString file = QDir::tempPath() + "/filename.gpkg";
  const QString driver = QStringLiteral( "GPKG" );
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( driver );
  exp.setFileName( file );
  exp.doExport();

  QgsVectorLayer *result = qgis::down_cast<QgsVectorLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );
  QCOMPARE( result->wkbType(), QgsWkbTypes::PointZ );
  QCOMPARE( result->featureCount(), 134 );
  delete result;
}

void TestQgsPointCloudLayerExporter::testPdalFile()
{
  const QString file = QDir::tempPath() + "/filename.laz";
  const QString driver = QStringLiteral( "LAZ" );
  QgsPointCloudLayerExporter exp( mLayer );
  exp.setFormat( driver );
  exp.setFileName( file );
  exp.doExport();

  QgsPointCloudLayer *result = qgis::down_cast<QgsPointCloudLayer *>( exp.takeExportedLayer() );

  QVERIFY( result->isValid() );

  QCOMPARE( result->pointCount(), 134 );
  delete result;
}

QGSTEST_MAIN( TestQgsPointCloudLayerExporter )
#include "testqgspointcloudlayerexporter.moc"
