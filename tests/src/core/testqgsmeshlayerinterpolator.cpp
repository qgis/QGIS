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
  QgsMeshLayer memoryLayer( mTestDataDir + "/mesh/quad_and_triangle.2dm",
                            "Triangle and Quad Mdal",
                            "mdal" );
  QVERIFY( memoryLayer.isValid() );
  QgsMeshDatasetIndex index( 0, 0 ); // bed elevation
  memoryLayer.setCrs( QgsCoordinateReferenceSystem::fromEpsgId( 27700 ) );

  QgsRasterBlock *block = QgsMeshUtils::exportRasterBlock(
                            memoryLayer,
                            index,
                            memoryLayer.crs(),
                            QgsProject::instance()->transformContext(),
                            100,
                            memoryLayer.extent()
                          );

  QCOMPARE( block->width(), 20 );
  QCOMPARE( block->height(), 10 );
  QVERIFY( block->isValid() );
  QVERIFY( !block->isEmpty() );
  QCOMPARE( block->dataType(), Qgis::Float64 );
  QVERIFY( block->hasNoDataValue() );
  QVERIFY( block->hasNoData() );

  QCOMPARE( block->value( 0, 0 ), 10.0 );
  QCOMPARE( block->value( 5, 5 ), 35.0 );
  QVERIFY( block->isNoData( 10, 10 ) );
}

QGSTEST_MAIN( TestQgsMeshLayerInterpolator )
#include "testqgsmeshlayerinterpolator.moc"
