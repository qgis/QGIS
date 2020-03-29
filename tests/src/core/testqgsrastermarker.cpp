/***************************************************************************
     testqgsrastermarker.cpp
     ---------------------
    Date                 : December 2018
    Copyright            : (C) 2014 Mathieu Pellerin
    Email                : nirvn dot asia at gmail dot com
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
#include <QDesktopServices>

#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsmarkersymbollayer.h>

#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for raster marker types.
 */
class TestQgsRasterMarker : public QObject
{
    Q_OBJECT

  public:
    TestQgsRasterMarker() = default;

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void rasterMarkerSymbol();
    void offset();
    void anchor();
    void rotation();
    void fixedAspectRatio();
    void alpha();

    void percentage();
    void percentageOffset() { offset(); }
    void percentageAnchor() { anchor(); }
    void percentageRotation() { rotation(); }
    void percentageFixedAspectRatio() { fixedAspectRatio(); }
    void percentageAlpha() { alpha(); }

  private:
    bool mTestHasError = false;
    bool imageCheck( const QString &type );

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mPointLayer = nullptr;
    QgsRasterMarkerSymbolLayer *mRasterMarker = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
    QString mPercentageName;
};


void TestQgsRasterMarker::initTestCase()
{
  mTestHasError = false;

  // Init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  // Create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); // defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  // Create a marker layer that will be used in all tests
  QString pointFileName = mTestDataDir + "points.shp";
  QFileInfo pointFileInfo( pointFileName );
  mPointLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                    pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mPointLayer );

  // Setup the raster marker symbol
  mRasterMarker = new QgsRasterMarkerSymbolLayer();
  mRasterMarker->setPath( mTestDataDir + QStringLiteral( "sample_image.png" ) );
  mRasterMarker->setSizeUnit( QgsUnitTypes::RenderPixels );
  mRasterMarker->setSize( 30.0 );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HCenter );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VCenter );
  mRasterMarker->setOffsetUnit( QgsUnitTypes::RenderPixels );

  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mRasterMarker );

  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mPointLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
  mReport += QLatin1String( "<h1>Raster Marker Renderer Tests</h1>\n" );
}

void TestQgsRasterMarker::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsRasterMarker::init()
{
}

void TestQgsRasterMarker::cleanup()
{
}

void TestQgsRasterMarker::rasterMarkerSymbol()
{
  mReport += QLatin1String( "<h2>Raster marker symbol renderer (30 px)</h2>\n" );
  bool result = imageCheck( QStringLiteral( "rastermarker" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::offset()
{
  mReport += QString( "<h2>Raster marker" + mPercentageName + " offset (12 px; 15 px)</h2>\n" );
  mRasterMarker->setOffset( QPointF( 12, 15 ) );
  bool result = imageCheck( QStringLiteral( "rastermarker_offset" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterMarker::anchor()
{
  mReport += QString( "<h2>Raster marker" + mPercentageName + " anchor (Left; Top)</h2>\n" );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::Left );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::Top );
  bool result = imageCheck( QStringLiteral( "rastermarker_anchor" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterMarker::rotation()
{
  mReport += QString( "<h2>Raster marker" + mPercentageName + " rotation (45.0)</h2>\n" );
  mRasterMarker->setAngle( 45.0 );
  bool result = imageCheck( QStringLiteral( "rastermarker_rotation" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterMarker::fixedAspectRatio()
{
  mReport += QString( "<h2>Raster marker" + mPercentageName + " fixed aspect ratio (1.0)</h2>\n" );
  mRasterMarker->setFixedAspectRatio( 1.0 );
  bool result = imageCheck( QStringLiteral( "rastermarker_fixedaspectratio" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterMarker::alpha()
{
  mReport += QString( "<h2>Raster marker" + mPercentageName + " alpha (0.5)</h2>\n" );
  mRasterMarker->setOpacity( 0.5 );
  bool result = imageCheck( QStringLiteral( "rastermarker_alpha" ) + mPercentageName );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentage()
{
  mPercentageName = "_percentage";
  mRasterMarker->setOffset( QPointF( 0, 0 ) );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HCenter );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VCenter );
  mRasterMarker->setAngle( 0.0 );
  mRasterMarker->setFixedAspectRatio( 0.0 );
  mRasterMarker->setOpacity( 1.0 );

  mReport += QLatin1String( "<h2>Raster marker_percentage (12.3 %)</h2>\n" );
  mRasterMarker->setSizeUnit( QgsUnitTypes::RenderPercentage );
  mRasterMarker->setSize( 12.3 );
  bool result = imageCheck( QStringLiteral( "rastermarker_percentage" ) );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRasterMarker::imageCheck( const QString &testType )
{
  // Use the QgsRenderChecker test utility class to
  // ensure the rendered output matches our control image
  mMapSettings.setExtent( mPointLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_rastermarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsRasterMarker )
#include "testqgsrastermarker.moc"
