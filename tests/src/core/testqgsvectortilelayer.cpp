/***************************************************************************
  testqgsvectortilelayer.cpp
  --------------------------------------
  Date                 : March 2020
  Copyright            : (C) 2020 by Martin Dobias
  Email                : wonder dot sk at gmail dot com
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

//qgis includes...
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgsrenderchecker.h"
#include "qgstiles.h"
#include "qgsvectortilelayer.h"

/**
 * \ingroup UnitTests
 * This is a unit test for a vector tile layer
 */
class TestQgsVectorTileLayer : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileLayer() = default;

  private:
    QString mDataDir;
    QgsVectorTileLayer *mLayer = nullptr;
    QString mReport;
    QgsMapSettings *mMapSettings = nullptr;

    bool imageCheck( const QString &testType, QgsVectorTileLayer *layer, QgsRectangle extent );

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_basic();
    void test_render();
};


void TestQgsVectorTileLayer::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mDataDir += "/vector_tile";

  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QString( "file://%1/{z}-{x}-{y}.pbf" ).arg( mDataDir ) );
  ds.setParam( "zmax", "1" );
  mLayer = new QgsVectorTileLayer( ds.encodedUri(), "Vector Tiles Test" );
  QVERIFY( mLayer->isValid() );

  QgsProject::instance()->addMapLayer( mLayer );

  mMapSettings = new QgsMapSettings();
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLayer );

}

void TestQgsVectorTileLayer::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsVectorTileLayer::test_basic()
{
  // tile fetch test
  QByteArray tile0rawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 0 ) );
  QCOMPARE( tile0rawData.length(), 64822 );

  QByteArray invalidTileRawData = mLayer->getRawTile( QgsTileXYZ( 0, 0, 99 ) );
  QCOMPARE( invalidTileRawData.length(), 0 );
}


bool TestQgsVectorTileLayer::imageCheck( const QString &testType, QgsVectorTileLayer *layer, QgsRectangle extent )
{
  mReport += "<h2>" + testType + "</h2>\n";
  mMapSettings->setExtent( extent );
  mMapSettings->setDestinationCrs( layer->crs() );
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "vector_tile" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( *mMapSettings );
  myChecker.setColorTolerance( 15 );
  bool myResultFlag = myChecker.runTest( testType, 0 );
  mReport += myChecker.report();
  return myResultFlag;
}

void TestQgsVectorTileLayer::test_render()
{
  QVERIFY( imageCheck( "render_test_basic", mLayer, mLayer->extent() ) );
}


QGSTEST_MAIN( TestQgsVectorTileLayer )
#include "testqgsvectortilelayer.moc"
