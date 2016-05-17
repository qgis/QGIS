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
#include <QtTest/QtTest>
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
#include "qgsmaplayerregistry.h"
#include "qgsapplication.h"
#include "qgsmaprenderer.h"
#include "qgspallabeling.h"
#include "qgsfontutils.h"

//qgis unit test includes
#include <qgsrenderchecker.h>

/** \ingroup UnitTests
 * This is a unit test for the map rotation feature
 */
class TestQgsMapRotation : public QObject
{
    Q_OBJECT
  public:
    TestQgsMapRotation()
        : mRasterLayer( 0 )
        , mPointsLayer( 0 )
        , mLinesLayer( 0 )
        , mMapSettings( 0 )
    {
      mTestDataDir = QString( TEST_DATA_DIR ) + '/';
    }

    ~TestQgsMapRotation();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void rasterLayer();
    void pointsLayer();
    void linesLayer();
    // TODO: polygonsLayer

  private:
    bool render( const QString& theFileName );

    QString mTestDataDir;
    QgsRasterLayer * mRasterLayer;
    QgsVectorLayer* mPointsLayer;
    QgsVectorLayer* mLinesLayer;
    QgsMapSettings *mMapSettings;
    QString mReport;
};

//runs before all tests
void TestQgsMapRotation::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();

  mMapSettings = new QgsMapSettings();

  QList<QgsMapLayer *> mapLayers;

  //create a raster layer that will be used in all tests...
  QFileInfo rasterFileInfo( mTestDataDir + "rgb256x256.png" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  mRasterLayer->setRenderer( rasterRenderer );
  mapLayers << mRasterLayer;

  //create a point layer that will be used in all tests...
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                     myPointFileInfo.completeBaseName(), "ogr" );
  mapLayers << mPointsLayer;

  //create a line layer that will be used in all tests...
  QString myLinesFileName = mTestDataDir + "lines_cardinals.shp";
  QFileInfo myLinesFileInfo( myLinesFileName );
  mLinesLayer = new QgsVectorLayer( myLinesFileInfo.filePath(),
                                    myLinesFileInfo.completeBaseName(), "ogr" );
  mapLayers << mLinesLayer;

  // Register all layers with the registry
  QgsMapLayerRegistry::instance()->addMapLayers( mapLayers );

  // This is needed to correctly set rotation center,
  // the actual size doesn't matter as QgsRenderChecker will
  // re-set it to the size of the expected image
  mMapSettings->setOutputSize( QSize( 256, 256 ) );

  mReport += "<h1>Map Rotation Tests</h1>\n";

  QgsFontUtils::loadStandardTestFonts( QStringList() << "Bold" );
}

TestQgsMapRotation::~TestQgsMapRotation()
{

}

//runs after all tests
void TestQgsMapRotation::cleanupTestCase()
{
  delete mMapSettings;
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsMapRotation::rasterLayer()
{
  mMapSettings->setLayers( QStringList() << mRasterLayer->id() );
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
  mMapSettings->setLayers( QStringList() << mPointsLayer->id() );

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
  mMapSettings->setLayers( QStringList() << mLinesLayer->id() );

  // Arrowed line with parallel labels
  QString qml = mTestDataDir + "lines_cardinals_arrowed_parallel_label.qml";
  bool success = false;
  mLinesLayer->loadNamedStyle( qml, success );

  //use test font
  QgsPalLayerSettings palSettings;
  palSettings.readFromLayer( mLinesLayer );
  palSettings.textFont = QgsFontUtils::getStandardTestFont( "Bold" );
  palSettings.textFont.setPointSizeF( 16 );
  palSettings.writeToLayer( mLinesLayer );

  QVERIFY( success );
  mMapSettings->setExtent( mLinesLayer->extent() ); //QgsRectangle(-150,-150,150,150) );
  mMapSettings->setRotation( 45 );
  QVERIFY( render( "lines-parallel-label+45" ) );

  // TODO: horizontal labels
  // TODO: curved labels
}

bool TestQgsMapRotation::render( const QString& theTestType )
{
  mReport += "<h2>" + theTestType + "</h2>\n";
  mMapSettings->setOutputDpi( 96 );
  QgsRenderChecker checker;
  checker.setControlPathPrefix( "maprotation" );
  checker.setControlName( "expected_" + theTestType );
  checker.setMapSettings( *mMapSettings );
  bool result = checker.runTest( theTestType );
  mReport += "\n\n\n" + checker.report();
  return result;
}

QTEST_MAIN( TestQgsMapRotation )
#include "testqgsmaprotation.moc"
