/***************************************************************************
  testqgsvectortilewriter.cpp
  --------------------------------------
  Date                 : April 2020
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
#include "qgstiles.h"
#include "qgsvectorlayer.h"
#include "qgsvectortilemvtdecoder.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilewriter.h"

#include <QTemporaryDir>

/**
 * \ingroup UnitTests
 * This is a unit test for vector tile writing
 */
class TestQgsVectorTileWriter : public QObject
{
    Q_OBJECT

  public:
    TestQgsVectorTileWriter() = default;

  private:
    QString mDataDir;
    //QgsVectorTileLayer *mLayer = nullptr;
    QString mReport;
    QgsMapSettings *mMapSettings = nullptr;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void test_basic();
};


void TestQgsVectorTileWriter::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();
  mDataDir = QString( TEST_DATA_DIR ); //defined in CmakeLists.txt
}

void TestQgsVectorTileWriter::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsVectorTileWriter::test_basic()
{
  QTemporaryDir dir;
  dir.setAutoRemove( false );  // so that we can inspect the results later
  QString tmpDir = dir.path();

  QgsDataSourceUri ds;
  ds.setParam( "type", "xyz" );
  ds.setParam( "url", QUrl::fromLocalFile( tmpDir ).toString() + "/{z}-{x}-{y}.pbf" );

  QgsVectorLayer *vlPoints = new QgsVectorLayer( mDataDir + "/points.shp", "points", "ogr" );
  QgsVectorLayer *vlLines = new QgsVectorLayer( mDataDir + "/lines.shp", "lines", "ogr" );
  QgsVectorLayer *vlPolys = new QgsVectorLayer( mDataDir + "/polys.shp", "polys", "ogr" );

  QList<QgsVectorTileWriter::Layer> layers;
  layers << QgsVectorTileWriter::Layer( vlPoints );
  layers << QgsVectorTileWriter::Layer( vlLines );
  layers << QgsVectorTileWriter::Layer( vlPolys );

  QgsVectorTileWriter writer;
  writer.setDestinationUri( ds.encodedUri() );
  writer.setMaxZoom( 3 );
  writer.setLayers( layers );

  bool res = writer.writeTiles();
  QVERIFY( res );
  QVERIFY( writer.errorMessage().isEmpty() );

  delete vlPoints;
  delete vlLines;
  delete vlPolys;

  // check on the file level
  QDir dirInfo( tmpDir );
  QStringList dirFiles = dirInfo.entryList( QStringList( "*.pbf" ) );
  QCOMPARE( dirFiles.count(), 8 );   // 1 tile at z0, 1 tile at z1, 2 tiles at z2, 4 tiles at z3
  QVERIFY( dirFiles.contains( "0-0-0.pbf" ) );

  QgsVectorTileLayer *vtLayer = new QgsVectorTileLayer( ds.encodedUri(), "output" );

  QByteArray tile0 = vtLayer->getRawTile( QgsTileXYZ( 0, 0, 0 ) );
  QgsVectorTileMVTDecoder decoder;
  bool resDecode0 = decoder.decode( QgsTileXYZ( 0, 0, 0 ), tile0 );
  QVERIFY( resDecode0 );
  QStringList layerNames = decoder.layers();
  QCOMPARE( layerNames, QStringList() << "points" << "lines" << "polys" );
  QStringList fieldNamesLines = decoder.layerFieldNames( "lines" );
  QCOMPARE( fieldNamesLines, QStringList() << "Name" << "Value" );

  QgsFields fieldsPolys;
  fieldsPolys.append( QgsField( "Name", QVariant::String ) );
  QMap<QString, QgsFields> perLayerFields;
  perLayerFields["polys"] = fieldsPolys;
  perLayerFields["lines"] = QgsFields();
  perLayerFields["points"] = QgsFields();
  QgsVectorTileFeatures features0 = decoder.layerFeatures( perLayerFields, QgsCoordinateTransform() );
  QCOMPARE( features0["points"].count(), 17 );
  QCOMPARE( features0["lines"].count(), 6 );
  QCOMPARE( features0["polys"].count(), 10 );

  QCOMPARE( features0["points"][0].geometry().wkbType(), QgsWkbTypes::Point );
  QCOMPARE( features0["lines"][0].geometry().wkbType(), QgsWkbTypes::LineString );
  QCOMPARE( features0["polys"][0].geometry().wkbType(), QgsWkbTypes::MultiPolygon );   // source geoms in shp are multipolygons

  QgsAttributes attrsPolys0_0 = features0["polys"][0].attributes();
  QCOMPARE( attrsPolys0_0.count(), 1 );
  QString attrNamePolys0_0 = attrsPolys0_0[0].toString();
  QVERIFY( attrNamePolys0_0 == "Dam" || attrNamePolys0_0 == "Lake" );

  delete vtLayer;
}


QGSTEST_MAIN( TestQgsVectorTileWriter )
#include "testqgsvectortilewriter.moc"
