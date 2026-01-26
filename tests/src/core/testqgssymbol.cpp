/***************************************************************************
     testqgssymbol.cpp
     --------------------------------------
    Date                 : 2015-10-07
    Copyright            : (C) 2015 Nyall Dawson
    Email                : nyall dot dawson at gmail.com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#include "qgstest.h"

#include <QApplication>
#include <QFileInfo>
#include <QObject>
#include <QStringList>

//qgis includes...
#include "qgsmultirenderchecker.h"
#include <qgsapplication.h>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "qgscolorramp.h"
#include "qgscptcityarchive.h"
#include "qgsvectorlayer.h"
#include "qgsproject.h"
#include "qgslinesymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgssinglesymbolrenderer.h"
#include "qgsmarkersymbollayer.h"
#include "qgslinesymbol.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsstyle.h"

/**
 * \ingroup UnitTests
 * This is a unit test to verify that symbols are working correctly
 */
class TestQgsSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsSymbol()
      : QgsTest( u"Symbol Tests"_s ) {}

  private:
    QString mTestDataDir;

    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsVectorLayer *mpLinesLayer = nullptr;
    QgsVectorLayer *mpPolysLayer = nullptr;

    bool imageCheck( QgsMapSettings &ms, const QString &testName );

  private slots:

    // init / cleanup
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    // void initStyles();

    void testCanvasClip();
    void testParseColor();
    void testParseColorList();
    void symbolProperties();

    //
    // Regression Testing
    //

    /**
     * To check if the interval of the marker symbols of a marker line is set
 *  correctly after loading a style from a sld-file. It is a regression test
 *  for ticket #24954 which was fixed with change r22a1
 */
    void regression24954();
};

// slots
void TestQgsSymbol::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDatabase();
  mTestDataDir = QStringLiteral( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  // output test environment
  QgsApplication::showSettings();

  // Set up the QgsSettings environment
  QCoreApplication::setOrganizationName( u"QGIS"_s );
  QCoreApplication::setOrganizationDomain( u"qgis.org"_s );
  QCoreApplication::setApplicationName( u"QGIS-TEST"_s );

  // initialize with a clean style
  QFile styleFile( QgsApplication::userStylePath() );
  if ( styleFile.exists() )
  {
    styleFile.remove();
    QgsDebugMsgLevel( "removed user style file " + styleFile.fileName(), 1 );
  }

  //
  //create a point layer that will be used in all tests...
  //
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(), myPointFileInfo.completeBaseName(), u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer
  );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer
  );


  //
  // Create a line layer that will be used in all tests...
  //
  const QString myLinesFileName = mTestDataDir + "lines.shp";
  const QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(), myLineFileInfo.completeBaseName(), u"ogr"_s );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer
  );
}

void TestQgsSymbol::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

bool TestQgsSymbol::imageCheck( QgsMapSettings &ms, const QString &testName )
{
  QgsMultiRenderChecker checker;
  ms.setOutputDpi( 96 );
  checker.setControlName( "expected_" + testName );
  checker.setMapSettings( ms );
  const bool result = checker.runTest( testName, 0 );
  mReport += checker.report();
  return result;
}

void TestQgsSymbol::testCanvasClip()
{
  //test rendering with and without clip to canvas enabled
  QgsMapSettings ms;
  QgsRectangle extent( -110.0, 25.0, -90, 40.0 );
  ms.setExtent( extent );
  ms.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );

  //line
  ms.setLayers( QList<QgsMapLayer *>() << mpLinesLayer );

  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );
  static_cast<QgsSimpleMarkerSymbolLayer *>( markerLine->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, markerLine );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );
  mpLinesLayer->setRenderer( renderer );
  bool result;

  lineSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, u"style_linecanvasclip"_s );
  QVERIFY( result );

  lineSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, u"style_linecanvasclip_off"_s );
  QVERIFY( result );

  //poly
  ms.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );

  QgsCentroidFillSymbolLayer *centroidFill = new QgsCentroidFillSymbolLayer();
  static_cast<QgsSimpleMarkerSymbolLayer *>( centroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, centroidFill );
  renderer = new QgsSingleSymbolRenderer( fillSymbol );
  mpPolysLayer->setRenderer( renderer );

  extent = QgsRectangle( -106.0, 29.0, -94, 36.0 );
  ms.setExtent( extent );

  fillSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, u"style_polycanvasclip"_s );
  QVERIFY( result );

  fillSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, u"style_polycanvasclip_off"_s );
  QVERIFY( result );
}


void TestQgsSymbol::testParseColor()
{
  // values for color tests
  QMap<QString, QPair<QColor, bool>> colorTests;

  colorTests.insert( u"bad color"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"red"_s, qMakePair( QColor( 255, 0, 0 ), false ) );
  colorTests.insert( u"#ff00ff"_s, qMakePair( QColor( 255, 0, 255 ), false ) );
  colorTests.insert( u"#99AA00"_s, qMakePair( QColor( 153, 170, 0 ), false ) );
  colorTests.insert( u"#GG0000"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"000000"_s, qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( u"00ff00"_s, qMakePair( QColor( 0, 255, 0 ), false ) );
  colorTests.insert( u"00gg00"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"00ff000"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"fff"_s, qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( u"fff0"_s, qMakePair( QColor(), false ) );

  // hex rrggbbaa colors
  colorTests.insert( u"#ff00ffaa"_s, qMakePair( QColor( 255, 0, 255, 170 ), true ) );
  colorTests.insert( u"#99AA0099"_s, qMakePair( QColor( 153, 170, 0, 153 ), true ) );
  colorTests.insert( u"#GG0000aa"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"00000000"_s, qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( u"00ff0011"_s, qMakePair( QColor( 0, 255, 0, 17 ), true ) );
  colorTests.insert( u"00gg0011"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"00ff00000"_s, qMakePair( QColor(), false ) );

  colorTests.insert( u"0,0,0"_s, qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( u"127,60,0"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"255,255,255"_s, qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( u"256,60,0"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u"rgb(127,60,0)"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"rgb(127.5,60.5,0.5)"_s, qMakePair( QColor::fromRgbF( 127.5 / 255.0, 60.5 / 255.0, 0.5 / 255.0 ), false ) );
  colorTests.insert( u"rgb(255,255,255)"_s, qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( u"rgb(256,60,0)"_s, qMakePair( QColor(), false ) );
  colorTests.insert( u" rgb(  127, 60 ,  0 ) "_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"rgb(127,60,0);"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"hsl(30,19%,90%)"_s, qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( u"hsl( 30 , 19 % , 90 % )"_s, qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( u" hsl(  30, 19%,  90% ) "_s, qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( u"hsl(30,19%,90%);"_s, qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( u"hsl(30,19%,90%);"_s, qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( u"hsl(74.5, 15.6%, 77.1%)"_s, qMakePair( QColor::fromHslF( 0.206944444, 0.156, 0.771 ), false ) );
  colorTests.insert( u"hsla(30,19%,90%,0.4)"_s, qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( u"hsla( 30 , 19 % , 90 % , 0.4 )"_s, qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( u" hsla(  30, 19%,  90%, 0.4 ) "_s, qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( u"hsla(30,19%,90%,0.4);"_s, qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( u"hsla(74.5, 15.6%, 77.1%, 0.44)"_s, qMakePair( QColor::fromHslF( 0.206944444, 0.156, 0.771, 0.44 ), true ) );
  colorTests.insert( u"(127,60,0);"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"(127,60,0)"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"127,060,000"_s, qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( u"0,0,0,0"_s, qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( u"127,60,0,0.5"_s, qMakePair( QColor( 127, 60, 0, 128 ), true ) );
  colorTests.insert( u"255,255,255,0.1"_s, qMakePair( QColor( 255, 255, 255, 26 ), true ) );
  colorTests.insert( u"rgba(127,60,0,1.0)"_s, qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( u"rgba(255,255,255,0.0)"_s, qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( u" rgba(  127, 60 ,  0  , 0.2 ) "_s, qMakePair( QColor( 127, 60, 0, 51 ), true ) );
  colorTests.insert( u"rgba(127,60,0,0.1);"_s, qMakePair( QColor( 127, 60, 0, 26 ), true ) );
  colorTests.insert( u"rgba(127.5,60.5,0.5,0.1)"_s, qMakePair( QColor::fromRgbF( 127.5 / 255.0, 60.5 / 255.0, 0.5 / 255.0, 0.1 ), true ) );
  colorTests.insert( u"(127,60,0,1);"_s, qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( u"(127,60,0,1.0)"_s, qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( u"127,060,000,1"_s, qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( u"0%,0%,0%"_s, qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( u"50 %,60 %,0 %"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"100%, 100%, 100%"_s, qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( u"rgb(50%,60%,0%)"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"rgb(100%, 100%, 100%)"_s, qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( u" rgb(  50 % , 60 % ,  0  % ) "_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"rgb(50%,60%,0%);"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"rgb(50.5%,60.6%,0.8%)"_s, qMakePair( QColor::fromRgbF( 0.505, 0.606, 0.008 ), false ) );
  colorTests.insert( u"(50%,60%,0%);"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"(50%,60%,0%)"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"050%,060%,000%"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0 ), false ) );
  colorTests.insert( u"0%,0%,0%,0"_s, qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( u"50 %,60 %,0 %,0.5"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 0.5 ), true ) );
  colorTests.insert( u"100%, 100%, 100%, 1.0"_s, qMakePair( QColor( 255, 255, 255, 255 ), true ) );
  colorTests.insert( u"rgba(50%,60%,0%, 1.0)"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 1.0 ), true ) );
  colorTests.insert( u"rgba(100%, 100%, 100%, 0.0)"_s, qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( u" rgba(  50 % , 60 % ,  0  %, 0.5 ) "_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 0.5 ), true ) );
  colorTests.insert( u"rgba(50%,60%,0%,0);"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 0 ), true ) );
  colorTests.insert( u"rgba(50.5%,60.6%,0.8%, 0.5)"_s, qMakePair( QColor::fromRgbF( 0.505, 0.606, 0.008, 0.5 ), true ) );
  colorTests.insert( u"(50%,60%,0%,1);"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 1.0 ), true ) );
  colorTests.insert( u"(50%,60%,0%,1.0)"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 1.0 ), true ) );
  colorTests.insert( u"050%,060%,000%,0"_s, qMakePair( QColor::fromRgbF( 0.5, 0.6, 0, 0 ), true ) );

  QMap<QString, QPair<QColor, bool>>::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsgLevel( "color string: " + i.key(), 1 );
    bool hasAlpha = false;
    const QColor result = QgsSymbolLayerUtils::parseColorWithAlpha( i.key(), hasAlpha );
    QCOMPARE( result, i.value().first );
    QCOMPARE( hasAlpha, i.value().second );
    ++i;
  }
}

void TestQgsSymbol::testParseColorList()
{
  //ensure that majority of single parseColor tests work for lists
  //note that some are not possible, as the colors may be ambiguous when treated as a list
  QMap<QString, QColor> colorTests;
  colorTests.insert( u"bad color"_s, QColor() );
  colorTests.insert( u"red"_s, QColor( 255, 0, 0 ) );
  colorTests.insert( u"#ff00ff"_s, QColor( 255, 0, 255 ) );
  colorTests.insert( u"#99AA00"_s, QColor( 153, 170, 0 ) );
  colorTests.insert( u"#GG0000"_s, QColor() );
  //colorTests.insert( "000000", QColor( 0, 0, 0 ) );
  //colorTests.insert( "00ff00", QColor( 0, 255, 0 ) );
  //colorTests.insert( "00gg00", QColor() );
  colorTests.insert( u"00ff000"_s, QColor() );
  //colorTests.insert( "fff", QColor( 255, 255, 255 ) );
  colorTests.insert( u"fff0"_s, QColor() );

  // hex rrggbbaa colors
  colorTests.insert( u"#ff00ffaa"_s, QColor( 255, 0, 255, 170 ) );
  colorTests.insert( u"#99AA0099"_s, QColor( 153, 170, 0, 153 ) );
  colorTests.insert( u"#GG0000aa"_s, QColor() );
  colorTests.insert( u"00000000"_s, QColor( 0, 0, 0, 0 ) );
  colorTests.insert( u"00ff0011"_s, QColor( 0, 255, 0, 17 ) );
  colorTests.insert( u"00gg0011"_s, QColor() );
  colorTests.insert( u"00ff00000"_s, QColor() );

  colorTests.insert( u"0,0,0"_s, QColor( 0, 0, 0 ) );
  colorTests.insert( u"127,60,0"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"255,255,255"_s, QColor( 255, 255, 255 ) );
  //colorTests.insert( "256,60,0", QColor() );
  colorTests.insert( u"rgb(127,60,0)"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"rgb(255,255,255)"_s, QColor( 255, 255, 255 ) );
  colorTests.insert( u"rgb(256,60,0)"_s, QColor() );
  colorTests.insert( u" rgb(  127, 60 ,  0 ) "_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"rgb(127,60,0);"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"(127,60,0);"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"(127,60,0)"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"127,060,000"_s, QColor( 127, 60, 0 ) );
  colorTests.insert( u"0,0,0,0"_s, QColor( 0, 0, 0, 0 ) );
  colorTests.insert( u"127,60,0,0.5"_s, QColor( 127, 60, 0, 128 ) );
  colorTests.insert( u"255,255,255,0.1"_s, QColor( 255, 255, 255, 26 ) );
  colorTests.insert( u"rgba(127,60,0,1.0)"_s, QColor( 127, 60, 0, 255 ) );
  colorTests.insert( u"rgba(255,255,255,0.0)"_s, QColor( 255, 255, 255, 0 ) );
  colorTests.insert( u" rgba(  127, 60 ,  0  , 0.2 ) "_s, QColor( 127, 60, 0, 51 ) );
  colorTests.insert( u"rgba(127,60,0,0.1);"_s, QColor( 127, 60, 0, 26 ) );
  colorTests.insert( u"(127,60,0,1);"_s, QColor( 127, 60, 0, 255 ) );
  colorTests.insert( u"(127,60,0,1.0)"_s, QColor( 127, 60, 0, 255 ) );
  colorTests.insert( u"127,060,000,1"_s, QColor( 127, 60, 0, 255 ) );
  colorTests.insert( u"0%,0%,0%"_s, QColor( 0, 0, 0 ) );
  colorTests.insert( u"50 %,60 %,0 %"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"100%, 100%, 100%"_s, QColor( 255, 255, 255 ) );
  colorTests.insert( u"rgb(50%,60%,0%)"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"rgb(100%, 100%, 100%)"_s, QColor( 255, 255, 255 ) );
  colorTests.insert( u" rgb(  50 % , 60 % ,  0  % ) "_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"rgb(50%,60%,0%);"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"(50%,60%,0%);"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"(50%,60%,0%)"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"050%,060%,000%"_s, QColor::fromRgbF( 0.5, 0.6, 0 ) );
  colorTests.insert( u"0%,0%,0%,0"_s, QColor( 0, 0, 0, 0 ) );
  colorTests.insert( u"50 %,60 %,0 %,0.5"_s, QColor::fromRgbF( 0.5, 0.6, 0, 0.5 ) );
  colorTests.insert( u"100%, 100%, 100%, 1.0"_s, QColor( 255, 255, 255, 255 ) );
  colorTests.insert( u"rgba(50%,60%,0%, 1.0)"_s, QColor::fromRgbF( 0.5, 0.6, 0, 1.0 ) );
  colorTests.insert( u"rgba(100%, 100%, 100%, 0.0)"_s, QColor( 255, 255, 255, 0 ) );
  colorTests.insert( u" rgba(  50 % , 60 % ,  0  %, 0.5 ) "_s, QColor::fromRgbF( 0.5, 0.6, 0, 0.5 ) );
  colorTests.insert( u"rgba(50%,60%,0%,0);"_s, QColor::fromRgbF( 0.5, 0.6, 0, 0 ) );
  colorTests.insert( u"(50%,60%,0%,1);"_s, QColor::fromRgbF( 0.5, 0.6, 0, 1 ) );
  colorTests.insert( u"(50%,60%,0%,1.0)"_s, QColor::fromRgbF( 0.5, 0.6, 0, 1 ) );
  colorTests.insert( u"050%,060%,000%,0"_s, QColor::fromRgbF( 0.5, 0.6, 0, 0 ) );

  QMap<QString, QColor>::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsgLevel( "color list string: " + i.key(), 1 );
    const QList<QColor> result = QgsSymbolLayerUtils::parseColorList( i.key() );
    if ( i.value().isValid() )
    {
      QCOMPARE( result.length(), 1 );
      QVERIFY( result.at( 0 ) == i.value() );
    }
    else
    {
      QCOMPARE( result.length(), 0 );
    }
    ++i;
  }

  QVector<QPair<QString, QList<QColor>>> colorListTests;
  QList<QColor> list1;
  list1 << QColor( u"blue"_s ) << QColor( u"red"_s ) << QColor( u"green"_s );
  colorListTests.append( qMakePair( u"blue red green"_s, list1 ) );
  colorListTests.append( qMakePair( u"blue,red,green"_s, list1 ) );
  colorListTests.append( qMakePair( u"blue\nred\ngreen"_s, list1 ) );
  colorListTests.append( qMakePair( u"blue\nred green"_s, list1 ) );
  colorListTests.append( qMakePair( u"blue\nred,green"_s, list1 ) );
  QList<QColor> list2;
  list2 << QColor( u"#ff0000"_s ) << QColor( u"#00ff00"_s ) << QColor( u"#0000ff"_s );
  colorListTests.append( qMakePair( u"#ff0000 #00ff00 #0000ff"_s, list2 ) );
  colorListTests.append( qMakePair( u"#ff0000,#00ff00,#0000ff"_s, list2 ) );
  colorListTests.append( qMakePair( u"#ff0000\n#00ff00\n#0000ff"_s, list2 ) );
  colorListTests.append( qMakePair( u"#ff0000\n#00ff00 #0000ff"_s, list2 ) );
  colorListTests.append( qMakePair( u"#ff0000\n#00ff00,#0000ff"_s, list2 ) );
  QList<QColor> list3;
  list3 << QColor( u"#ff0000"_s ) << QColor( u"#00ff00"_s ) << QColor( u"#0000ff"_s );
  colorListTests.append( qMakePair( u"rgb(255,0,0) rgb(0,255,0) rgb(0,0,255)"_s, list3 ) );
  colorListTests.append( qMakePair( u"rgb(255,0,0)\nrgb(0,255,0)\nrgb(0,0,255)"_s, list3 ) );
  colorListTests.append( qMakePair( u"rgb(255,0,0)\nrgb(0,255,0) rgb(0,0,255)"_s, list3 ) );

  QVector<QPair<QString, QList<QColor>>>::const_iterator it = colorListTests.constBegin();
  while ( it != colorListTests.constEnd() )
  {
    QgsDebugMsgLevel( "color list string: " + ( *it ).first, 1 );
    const QList<QColor> result = QgsSymbolLayerUtils::parseColorList( ( *it ).first );
    if ( ( *it ).second.length() > 0 )
    {
      QCOMPARE( result.length(), ( *it ).second.length() );
      int index = 0;
      for ( QList<QColor>::const_iterator colorIt = ( *it ).second.constBegin(); colorIt != ( *it ).second.constEnd(); ++colorIt )
      {
        QVERIFY( result.at( index ) == ( *colorIt ) );
        index++;
      }
    }
    else
    {
      QCOMPARE( result.length(), 0 );
    }
    ++it;
  }
}

void TestQgsSymbol::symbolProperties()
{
  //test QgsSymbolLayerUtils::symbolProperties

  //make a symbol
  QgsSimpleFillSymbolLayer *fill = new QgsSimpleFillSymbolLayer();
  fill->setColor( QColor( 25, 125, 225 ) );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, fill );

  QgsFillSymbol *fillSymbol2 = static_cast<QgsFillSymbol *>( fillSymbol->clone() );

  //test that two different symbol pointers return same properties
  QCOMPARE( QgsSymbolLayerUtils::symbolProperties( fillSymbol ), QgsSymbolLayerUtils::symbolProperties( fillSymbol2 ) );

  //modify one of the symbols
  fillSymbol2->symbolLayer( 0 )->setColor( QColor( 235, 135, 35 ) );
  QVERIFY( QgsSymbolLayerUtils::symbolProperties( fillSymbol ) != QgsSymbolLayerUtils::symbolProperties( fillSymbol2 ) );

  delete fillSymbol;
  delete fillSymbol2;
}

void TestQgsSymbol::regression24954()
{
  //test if the interval value for marker placement is read from an sld file
  QString sldFileName( mTestDataDir + "symbol_layer/QgsMarkerLineSymbolLayer.sld" );

  bool defaultLoadedFlag = false;

  //load style from sld
  mpLinesLayer->loadSldStyle( sldFileName, defaultLoadedFlag );

  //create a symbol and convert it's first layer to a marker line symbol layer
  QgsFeatureRenderer *renderer = mpLinesLayer->renderer();
  QgsRenderContext renderContext;
  QgsSymbol *symbol = renderer->symbols( renderContext ).at( 0 );
  QgsMarkerLineSymbolLayer *symbolLayer = static_cast<QgsMarkerLineSymbolLayer *>( symbol->symbolLayer( 0 ) );

  QVERIFY( symbolLayer->interval() == 3.3 );
}

QGSTEST_MAIN( TestQgsSymbol )
#include "testqgssymbol.moc"
