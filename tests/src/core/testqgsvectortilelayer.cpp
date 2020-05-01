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
#include "qgsvectortilebasicrenderer.h"
#include "qgsvectortilelayer.h"
#include "qgsvectortilebasiclabeling.h"
#include "qgsfontutils.h"

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
    void test_labeling();
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

  // let's have some standard style config for the layer
  QColor polygonFillColor = Qt::blue;
  QColor polygonStrokeColor = polygonFillColor;
  polygonFillColor.setAlpha( 100 );
  double polygonStrokeWidth = DEFAULT_LINE_WIDTH * 2;
  QColor lineStrokeColor = Qt::blue;
  double lineStrokeWidth = DEFAULT_LINE_WIDTH * 2;
  QColor pointFillColor = Qt::red;
  QColor pointStrokeColor = pointFillColor;
  pointFillColor.setAlpha( 100 );
  double pointSize = DEFAULT_POINT_SIZE;

  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QgsVectorTileBasicRenderer::simpleStyle(
                     polygonFillColor, polygonStrokeColor, polygonStrokeWidth,
                     lineStrokeColor, lineStrokeWidth,
                     pointFillColor, pointStrokeColor, pointSize ) );
  mLayer->setRenderer( rend );  // takes ownership
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

void TestQgsVectorTileLayer::test_labeling()
{
  QgsTextFormat format;
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ).family() );
  format.setSize( 12 );
  format.setNamedStyle( QStringLiteral( "Bold" ) );
  format.setColor( QColor( 200, 0, 200 ) );

  QgsPalLayerSettings labelSettings;
  labelSettings.drawLabels = true;
  labelSettings.fieldName = "name:en";
  labelSettings.placement = QgsPalLayerSettings::OverPoint;
  labelSettings.setFormat( format );

  QgsVectorTileBasicLabelingStyle st;
  st.setStyleName( "st1" );
  st.setLayerName( "place" );
  st.setFilterExpression( "rank = 1 AND class = 'country'" );
  st.setGeometryType( QgsWkbTypes::PointGeometry );
  st.setLabelSettings( labelSettings );

  QgsVectorTileBasicLabeling *labeling = new QgsVectorTileBasicLabeling;
  QList<QgsVectorTileBasicLabelingStyle> lst;
  lst << st;
  labeling->setStyles( lst );

  mLayer->setLabeling( labeling );

  QgsVectorTileRenderer *oldRenderer = mLayer->renderer()->clone();

  // use a different renderer to make the labels stand out more
  QgsVectorTileBasicRenderer *rend = new QgsVectorTileBasicRenderer;
  rend->setStyles( QgsVectorTileBasicRenderer::simpleStyle(
                     Qt::transparent, Qt::white, DEFAULT_LINE_WIDTH * 2,
                     Qt::transparent, 0,
                     Qt::transparent, Qt::transparent, 0 ) );
  mLayer->setRenderer( rend );  // takes ownership

  bool res = imageCheck( "render_test_labeling", mLayer, mLayer->extent() );

  mLayer->setRenderer( oldRenderer );

  QVERIFY( res );
}


QGSTEST_MAIN( TestQgsVectorTileLayer )
#include "testqgsvectortilelayer.moc"
