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
#include "qgsmarkersymbol.h"
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for raster marker types.
 */
class TestQgsRasterMarker : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterMarker()
      : QgsTest( QStringLiteral( "Raster Marker Renderer Tests" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void rasterMarkerSymbol();
    void anchor();
    void alpha();
    void symbolOpacity();
    void dataDefinedOpacity();
    void rotation();
    void fixedAspectRatio();

    // Tests for percentage value of size unit.
    void percentage();
    void percentageAnchor();
    void percentageAlpha();
    void percentageRotation();
    void percentageFixedAspectRatio();
    void percentageOffset();

  private:
    bool mTestHasError = false;
    bool imageCheck( const QString &type );

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mPointLayer = nullptr;
    QgsRasterMarkerSymbolLayer *mRasterMarker = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsRasterMarker::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //create a marker layer that will be used in all tests
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  mPointLayer = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mPointLayer
  );

  //setup the raster marker symbol
  mRasterMarker = new QgsRasterMarkerSymbolLayer();
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mRasterMarker );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mPointLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mPointLayer );
}

void TestQgsRasterMarker::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterMarker::init()
{
  mRasterMarker->setPath( mTestDataDir + QStringLiteral( "sample_image.png" ) );
  mRasterMarker->setSize( 30.0 );
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Pixels );
}

void TestQgsRasterMarker::cleanup()
{
}

void TestQgsRasterMarker::rasterMarkerSymbol()
{
  const bool result = imageCheck( QStringLiteral( "rastermarker" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::anchor()
{
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::Right );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::Bottom );
  const bool result = imageCheck( QStringLiteral( "rastermarker_anchor" ) );
  QVERIFY( result );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HCenter );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VCenter );
}

void TestQgsRasterMarker::alpha()
{
  mRasterMarker->setOpacity( 0.5 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_alpha" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::symbolOpacity()
{
  // test combination of layer opacity AND symbol level opacity
  mRasterMarker->setOpacity( 0.5 );
  mMarkerSymbol->setOpacity( 0.5 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_opacity" ) );
  mMarkerSymbol->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsRasterMarker::dataDefinedOpacity()
{
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  const bool result = imageCheck( QStringLiteral( "rastermarker_ddopacity" ) );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty() );
  QVERIFY( result );
}

void TestQgsRasterMarker::rotation()
{
  mRasterMarker->setAngle( 45.0 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_rotation" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::fixedAspectRatio()
{
  mRasterMarker->setFixedAspectRatio( 0.2 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_fixedaspectratio" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentage()
{
  mRasterMarker->setOffset( QPointF( 0, 0 ) );
  mRasterMarker->setAngle( 0.0 );
  mRasterMarker->setFixedAspectRatio( 0.0 );
  mRasterMarker->setOpacity( 1.0 );

  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_percentage" ) );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentageAnchor()
{
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::Right );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::Bottom );
  const bool result = imageCheck( QStringLiteral( "rastermarker_anchor_percentage" ) );
  mRasterMarker->setHorizontalAnchorPoint( QgsMarkerSymbolLayer::HCenter );
  mRasterMarker->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VCenter );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentageAlpha()
{
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  mRasterMarker->setOpacity( 0.5 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_alpha_percentage" ) );
  mRasterMarker->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentageRotation()
{
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  mRasterMarker->setAngle( 45.0 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_rotation_percentage" ) );
  mRasterMarker->setAngle( 0.0 );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentageFixedAspectRatio()
{
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  mRasterMarker->setFixedAspectRatio( 1.0 );
  const bool result = imageCheck( QStringLiteral( "rastermarker_fixedaspectratio_percentage" ) );
  mRasterMarker->setFixedAspectRatio( 0.0 );
  QVERIFY( result );
}

void TestQgsRasterMarker::percentageOffset()
{
  mRasterMarker->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterMarker->setSize( 6.3 );
  mRasterMarker->setOffsetUnit( Qgis::RenderUnit::Pixels );
  mRasterMarker->setOffset( QPointF( 12, 15 ) );
  const bool result = imageCheck( QStringLiteral( "rastermarker_offset_percentage" ) );
  mRasterMarker->setOffset( QPointF( 0, 0 ) );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRasterMarker::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mPointLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_rastermarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  const bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsRasterMarker )
#include "testqgsrastermarker.moc"
