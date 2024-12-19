/***************************************************************************
     testqgsmeshlayerinterpolator.cpp
     --------------------------------
    Date                 : January 2019
    Copyright            : (C) 2019 by Peter Petrik
    Email                : zilolv at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QApplication>

//qgis includes...
#include <qgsrasterblock.h>
#include <qgsmeshlayer.h>
#include <qgsmeshlayerinterpolator.h>
#include <qgsapplication.h>
#include <qgscoordinatereferencesystem.h>
#include <qgsproject.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the TestQgsMeshLayerInterpolator class
 */
class TestQgsMeshLayerInterpolator: public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void testExportRasterBand();
  private:
    QString mTestDataDir;
};

//runs before all tests
void TestQgsMeshLayerInterpolator::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ); //defined in CmakeLists.txt
}
//runs after all tests
void TestQgsMeshLayerInterpolator::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsMeshLayerInterpolator::testExportRasterBand()
{
  QgsMeshLayer meshLayer( mTestDataDir + "/mesh/quad_and_triangle.2dm",
                          "Triangle and Quad Mdal",
                          "mdal" );
  QVERIFY( meshLayer.isValid() );
  QgsMeshDatasetIndex index( 0, 0 ); // bed elevation
  meshLayer.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ) );

  std::unique_ptr<QgsRasterBlock> block;
  block.reset( QgsMeshUtils::exportRasterBlock(
                 meshLayer,
                 index,
                 meshLayer.crs(),
                 QgsProject::instance()->transformContext(),
                 100,
                 meshLayer.extent() ) );

  QVERIFY( block );

  QCOMPARE( block->width(), 20 );
  QCOMPARE( block->height(), 10 );
  QVERIFY( block->isValid() );
  QVERIFY( !block->isEmpty() );
  QCOMPARE( block->dataType(), Qgis::DataType::Float64 );
  QVERIFY( block->hasNoDataValue() );
  QVERIFY( block->hasNoData() );

  QCOMPARE( block->value( 0, 0 ), 10.0 );
  QCOMPARE( block->value( 5, 5 ), 35.0 );
  QVERIFY( block->isNoData( 10, 10 ) );

  std::unique_ptr<QgsMeshMemoryDatasetGroup> virtualGroup = std::make_unique<QgsMeshMemoryDatasetGroup>( QStringLiteral( "on face" ), QgsMeshDatasetGroupMetadata::DataOnFaces );
  std::shared_ptr<QgsMeshMemoryDataset> dataset = std::make_shared < QgsMeshMemoryDataset>();
  dataset->values.resize( 2 );
  dataset->values[0] = 12;
  dataset->values[1] = 36;
  dataset->active.resize( 2 );
  dataset->active[0] = 1;
  dataset->active[1] = 1;
  dataset->time = 0;
  dataset->valid = true;
  virtualGroup->addDataset( dataset );

  meshLayer.addDatasets( virtualGroup.release() );

  QCOMPARE( meshLayer.datasetGroupCount(), 2 );

  index = QgsMeshDatasetIndex( 1, 0 );

  block.reset( QgsMeshUtils::exportRasterBlock(
                 meshLayer,
                 index,
                 meshLayer.crs(),
                 QgsProject::instance()->transformContext(),
                 100,
                 meshLayer.extent() ) );

  QVERIFY( block );

  QCOMPARE( block->width(), 20 );
  QCOMPARE( block->height(), 10 );
  QVERIFY( block->isValid() );
  QVERIFY( !block->isEmpty() );
  QCOMPARE( block->dataType(), Qgis::DataType::Float64 );
  QVERIFY( block->hasNoDataValue() );
  QVERIFY( block->hasNoData() );

  QCOMPARE( block->value( 0, 0 ), 12.0 );
  QCOMPARE( block->value( 5, 15 ), 36.0 );
  QVERIFY( block->isNoData( 10, 20 ) );
}

QGSTEST_MAIN( TestQgsMeshLayerInterpolator )
#include "testqgsmeshlayerinterpolator.moc"
