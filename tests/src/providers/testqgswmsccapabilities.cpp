/***************************************************************************
    testqgswmsccapabilities.cpp
    ---------------------
    begin                : July 2019
    copyright            : (C) 2019 by Maris Nartiss
    email                : maris dot nartiss at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include <QFile>
#include <QObject>
#include "qgstest.h"
#include <qgswmscapabilities.h>
#include <qgsapplication.h>

/**
 * \ingroup UnitTests
 * This is a unit test for the WMS-C capabilities parser
 * implemented as QgsWmsCapabilities::parseTileSetProfile
 */
class TestQgsWmscCapabilities: public QObject
{
    Q_OBJECT
  private slots:

    void initTestCase()
    {
      QgsApplication::init();
      QgsApplication::initQgis();

      QFile file( QStringLiteral( TEST_DATA_DIR ) + "/provider/wms-c_Capabilities.xml" );
      QVERIFY( file.open( QIODevice::ReadOnly | QIODevice::Text ) );
      const QByteArray content = file.readAll();
      QVERIFY( content.size() > 0 );
      const QgsWmsParserSettings config;

      mCapabilities = new QgsWmsCapabilities();
      QVERIFY( mCapabilities->parseResponse( content, config ) );
    }

    void cleanupTestCase()
    {
      delete mCapabilities;
      QgsApplication::exitQgis();
    }

    // WMS-C works as an ordinary WMS
    // check ordinary WMS layers
    void layers()
    {
      QCOMPARE( mCapabilities->supportedLayers().size(), 4 );

      // check identifiers
      QCOMPARE( mCapabilities->supportedLayers()[0].name, QString( "layerwtwa" ) );
      QCOMPARE( mCapabilities->supportedLayers()[1].name, QString( "layerwtna" ) );
      QCOMPARE( mCapabilities->supportedLayers()[2].name, QString( "layerntwa" ) );
      QCOMPARE( mCapabilities->supportedLayers()[3].name, QString( "layerntna" ) );

      // check layer title extraction
      QCOMPARE( mCapabilities->supportedLayers()[0].title, QString( "This is a layer title" ) );
      QCOMPARE( mCapabilities->supportedLayers()[1].title, QString( "A layer with a title and without an abstract" ) );
      QVERIFY( mCapabilities->supportedLayers()[2].title.isEmpty() );
      QVERIFY( mCapabilities->supportedLayers()[3].title.isEmpty() );

      // check layer abstract extraction
      QCOMPARE( mCapabilities->supportedLayers()[0].abstract, QString( "This is a layer abstract" ) );
      QVERIFY( mCapabilities->supportedLayers()[1].abstract.isEmpty() );
      QCOMPARE( mCapabilities->supportedLayers()[2].abstract, QString( "A layer without a title and with an abstract" ) );
      QVERIFY( mCapabilities->supportedLayers()[3].abstract.isEmpty() );

      // check bounding box
      QgsWmsBoundingBoxProperty bb;
      bb.box = QgsRectangle( 295000, 150000, 785000, 460000 );
      bb.crs = QString( "EPSG:3059" );
      QCOMPARE( mCapabilities->supportedLayers()[0].boundingBoxes.size(), 1 );
      QCOMPARE( mCapabilities->supportedLayers()[0].boundingBoxes[0].crs, bb.crs );
      QCOMPARE( mCapabilities->supportedLayers()[0].boundingBoxes[0].box, bb.box );
    }

    // check TileLayers from VendorSpecificCapabilities
    // values should be identical to layer ones
    void tileLayers()
    {
      QCOMPARE( mCapabilities->supportedTileLayers().size(), 4 );

      // check identifiers
      QCOMPARE( mCapabilities->supportedTileLayers()[0].identifier, QString( "layerwtwa" ) );
      QCOMPARE( mCapabilities->supportedTileLayers()[1].identifier, QString( "layerwtna" ) );
      QCOMPARE( mCapabilities->supportedTileLayers()[2].identifier, QString( "layerntwa" ) );
      QCOMPARE( mCapabilities->supportedTileLayers()[3].identifier, QString( "layerntna" ) );

      // check layer title extraction
      // bug #30262
      QCOMPARE( mCapabilities->supportedTileLayers()[0].title, QString( "This is a layer title" ) );
      QCOMPARE( mCapabilities->supportedTileLayers()[1].title, QString( "A layer with a title and without an abstract" ) );
      QVERIFY( mCapabilities->supportedTileLayers()[2].title.isEmpty() );
      QVERIFY( mCapabilities->supportedTileLayers()[3].title.isEmpty() );

      // check layer abstract extraction
      // bug #30262
      QCOMPARE( mCapabilities->supportedTileLayers()[0].abstract, QString( "This is a layer abstract" ) );
      QVERIFY( mCapabilities->supportedTileLayers()[1].abstract.isEmpty() );
      QCOMPARE( mCapabilities->supportedTileLayers()[2].abstract, QString( "A layer without a title and with an abstract" ) );
      QVERIFY( mCapabilities->supportedTileLayers()[3].abstract.isEmpty() );

      // check bounding box
      QgsWmsBoundingBoxProperty bb;
      bb.box = QgsRectangle( 295000, 150000, 785000, 460000 );
      bb.crs = QString( "EPSG:3059" );
      QCOMPARE( mCapabilities->supportedTileLayers()[0].boundingBoxes.size(), 1 );
      QCOMPARE( mCapabilities->supportedTileLayers()[0].boundingBoxes[0].crs, bb.crs );
      QCOMPARE( mCapabilities->supportedTileLayers()[0].boundingBoxes[0].box, bb.box );
    }

    // check TileMatrices generated from TileLayers
    void tileMatrices()
    {
      QgsWmtsTileMatrix m;

      QCOMPARE( mCapabilities->supportedTileMatrixSets().size(), 4 );
      QCOMPARE( mCapabilities->supportedTileMatrixSets()[QStringLiteral( "layerwtwa-wmsc-0" )].tileMatrices.size(), 2 );

      m = mCapabilities->supportedTileMatrixSets()[QStringLiteral( "layerwtwa-wmsc-0" )].tileMatrices[603.400];

      QCOMPARE( m.tileWidth, 256 );
      QCOMPARE( m.tileHeight, 256 );
      QCOMPARE( m.tres, 603.400 );

      QCOMPARE( m.matrixWidth, 4 );
      QCOMPARE( m.matrixHeight, 3 );
      QCOMPARE( m.topLeft, QgsPointXY( 295000, 613411.2 ) );
    }

  private:
    QgsWmsCapabilities *mCapabilities = nullptr;
};

QGSTEST_MAIN( TestQgsWmscCapabilities )
#include "testqgswmsccapabilities.moc"
