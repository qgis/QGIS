/***************************************************************************
     testqgsmaprotation.cpp
     --------------------------------------
    Date                 : Feb 18  2015
    Copyright            : (C) 2015 by Sandro Santilli
    Email                : strk@keybit.net
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
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>

//qgis includes...
#include "qgsrasterlayer.h"
#include "qgsvectorlayer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsproject.h"
#include "qgsapplication.h"
#include "qgspallabeling.h"
#include "qgsfontutils.h"
#include "qgsrasterdataprovider.h"
#include "qgsvectorlayerlabeling.h"

//qgis unit test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for the map rotation feature
 */
class TestQgsMapRotation : public QgsTest
{
    Q_OBJECT
  public:
    TestQgsMapRotation()
      : QgsTest( QStringLiteral( "Map Rotation Tests" ) )
    {
      mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMapRotation() override;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void rasterLayer();
    void pointsLayer();
    void linesLayer();
    // TODO: polygonsLayer

  private:
    bool render( const QString &fileName );

    QString mTestDataDir;
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mLinesLayer = nullptr;
    QgsMapSettings *mMapSettings = nullptr;
};

//runs before all tests
void TestQgsMapRotation::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  //create a raster layer that will be used in all tests...
  const QFileInfo rasterFileInfo( mTestDataDir + "rgb256x256.png" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  mRasterLayer->setRenderer( rasterRenderer );

  //create a point layer that will be used in all tests...
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //create a line layer that will be used in all tests...
  const QString myLinesFileName = mTestDataDir + "lines_cardinals.shp";
  const QFileInfo myLinesFileInfo( myLinesFileName );
  mLinesLayer = new QgsVectorLayer( myLinesFileInfo.filePath(),
                                    myLinesFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // This is needed to correctly set rotation center,
  // the actual size doesn't matter as QgsRenderChecker will
  // re-set it to the size of the expected image
  mMapSettings->setOutputSize( QSize( 256, 256 ) );

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

TestQgsMapRotation::~TestQgsMapRotation() = default;

//runs after all tests
void TestQgsMapRotation::cleanupTestCase()
{
  delete mMapSettings;
  delete mPointsLayer;
  delete mLinesLayer;
  delete mRasterLayer;
  QgsApplication::exitQgis();
}

void TestQgsMapRotation::rasterLayer()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  mMapSettings->setExtent( mRasterLayer->extent() );
  mMapSettings->setRotation( 45 );
  // This ensures rotated image is all visible by tweaking scale
  mMapSettings->setExtent( mMapSettings->visibleExtent() );
  QVERIFY( render( "raster+45" ) );

  mMapSettings->setRotation( -45 );
  QVERIFY( render( "raster-45" ) );
}

void TestQgsMapRotation::pointsLayer()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mPointsLayer );

  // SVG points, fixed (no) rotation
  QString qml = mTestDataDir + "points.qml";
  bool success = false;
  mPointsLayer->loadNamedStyle( qml, success );
  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -105.5, 37, -97.5, 45 ) );
  mMapSettings->setRotation( -60 );
  QVERIFY( render( "svgpoints-60" ) );

  // SVG points, data defined rotation
  qml = mTestDataDir + "points_single_symbol_datadefined_rotation.qml";
  success = false;
  mPointsLayer->loadNamedStyle( qml, success );
  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -116, 33, -107, 42 ) );
  mMapSettings->setRotation( 90 );
  QVERIFY( render( "svgpoints-datadefined+90" ) );

  // TODO: SVG points, fixed (defined) rotation ?

  // Simple points, data defined rotation
  qml = mTestDataDir + "points_single_symbol.qml";
  success = false;
  mPointsLayer->loadNamedStyle( qml, success );
  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -116, 33, -107, 42 ) );
  mMapSettings->setRotation( 90 );
  QVERIFY( render( "simplepoints-datadefined+90" ) );

  // Simple points, fixed (no) rotation
  qml = mTestDataDir + "points_graduated_symbol.qml";
  success = false;
  mPointsLayer->loadNamedStyle( qml, success );
  QVERIFY( success );
  mMapSettings->setExtent( QgsRectangle( -108, 26, -100, 34 ) );
  mMapSettings->setRotation( 30 );
  QVERIFY( render( "simplepoints+30" ) );

  // TODO: simple points, fixed (defined) rotation ?
}

void TestQgsMapRotation::linesLayer()
{
  mMapSettings->setLayers( QList<QgsMapLayer *>() << mLinesLayer );

  // Arrowed line with parallel labels
  const QString qml = mTestDataDir + "lines_cardinals_arrowed_parallel_label.qml";
  bool success = false;
  mLinesLayer->loadNamedStyle( qml, success );

  //use test font
  QVERIFY( mLinesLayer->labeling() );
  QVERIFY( mLinesLayer->labeling()->type() == QLatin1String( "simple" ) );
  const QgsVectorLayerSimpleLabeling *labeling = static_cast<const QgsVectorLayerSimpleLabeling *>( mLinesLayer->labeling() );
  QgsPalLayerSettings palSettings = labeling->settings();
  QgsTextFormat format = palSettings.format();
  format.setFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 16 );
  palSettings.setFormat( format );
  mLinesLayer->setLabeling( new QgsVectorLayerSimpleLabeling( palSettings ) );
  mLinesLayer->setLabelsEnabled( true );

  QVERIFY( success );
  mMapSettings->setExtent( mLinesLayer->extent() ); //QgsRectangle(-150,-150,150,150) );
  mMapSettings->setRotation( 45 );
  QVERIFY( render( "lines-parallel-label+45" ) );

  // TODO: horizontal labels
  // TODO: curved labels
}

bool TestQgsMapRotation::render( const QString &testType )
{
  mMapSettings->setOutputDpi( 96 );
  QgsMultiRenderChecker checker;
  checker.setControlPathPrefix( QStringLiteral( "maprotation" ) );
  checker.setControlName( "expected_" + testType );
  checker.setMapSettings( *mMapSettings );
  const bool result = checker.runTest( testType );
  mReport += checker.report();
  return result;
}

QGSTEST_MAIN( TestQgsMapRotation )
#include "testqgsmaprotation.moc"
