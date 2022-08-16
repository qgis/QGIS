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
#include <QObject>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>

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

    TestQgsSymbol() : QgsTest( QStringLiteral( "Symbol Tests" ) ) {}

  private:

    QString mTestDataDir;

    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsVectorLayer *mpLinesLayer = nullptr;
    QgsVectorLayer *mpPolysLayer = nullptr;

    bool imageCheck( QgsMapSettings &ms, const QString &testName );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    // void initStyles();

    void testCanvasClip();
    void testParseColor();
    void testParseColorList();
    void symbolProperties();
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
  QCoreApplication::setOrganizationName( QStringLiteral( "QGIS" ) );
  QCoreApplication::setOrganizationDomain( QStringLiteral( "qgis.org" ) );
  QCoreApplication::setApplicationName( QStringLiteral( "QGIS-TEST" ) );

  // initialize with a clean style
  QFile styleFile( QgsApplication::userStylePath() );
  if ( styleFile.exists() )
  {
    styleFile.remove();
    QgsDebugMsg( "removed user style file " + styleFile.fileName() );
  }

  //
  //create a point layer that will be used in all tests...
  //
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  const QString myPointsFileName = mTestDataDir + "points.shp";
  const QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );


  //
  // Create a line layer that will be used in all tests...
  //
  const QString myLinesFileName = mTestDataDir + "lines.shp";
  const QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );
  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer );
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
  ms.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput );

  //line
  ms.setLayers( QList<QgsMapLayer *>() << mpLinesLayer );

  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setPlacements( Qgis::MarkerLinePlacement::CentralPoint );
  static_cast< QgsSimpleMarkerSymbolLayer *>( markerLine->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  QgsLineSymbol *lineSymbol = new QgsLineSymbol();
  lineSymbol->changeSymbolLayer( 0, markerLine );
  QgsSingleSymbolRenderer *renderer = new QgsSingleSymbolRenderer( lineSymbol );
  mpLinesLayer->setRenderer( renderer );
  bool result;

  lineSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, QStringLiteral( "style_linecanvasclip" ) );
  QVERIFY( result );

  lineSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, QStringLiteral( "style_linecanvasclip_off" ) );
  QVERIFY( result );

  //poly
  ms.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );

  QgsCentroidFillSymbolLayer *centroidFill = new QgsCentroidFillSymbolLayer();
  static_cast< QgsSimpleMarkerSymbolLayer * >( centroidFill->subSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, centroidFill );
  renderer = new QgsSingleSymbolRenderer( fillSymbol );
  mpPolysLayer->setRenderer( renderer );

  extent = QgsRectangle( -106.0, 29.0, -94, 36.0 );
  ms.setExtent( extent );

  fillSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, QStringLiteral( "style_polycanvasclip" ) );
  QVERIFY( result );

  fillSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, QStringLiteral( "style_polycanvasclip_off" ) );
  QVERIFY( result );
}


void TestQgsSymbol::testParseColor()
{
  // values for color tests
  QMap< QString, QPair< QColor, bool> > colorTests;
  colorTests.insert( QStringLiteral( "bad color" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "red" ), qMakePair( QColor( 255, 0, 0 ), false ) );
  colorTests.insert( QStringLiteral( "#ff00ff" ), qMakePair( QColor( 255, 0, 255 ), false ) );
  colorTests.insert( QStringLiteral( "#99AA00" ), qMakePair( QColor( 153, 170, 0 ), false ) );
  colorTests.insert( QStringLiteral( "#GG0000" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "000000" ), qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( QStringLiteral( "00ff00" ), qMakePair( QColor( 0, 255, 0 ), false ) );
  colorTests.insert( QStringLiteral( "00gg00" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "00ff000" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "fff" ), qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( QStringLiteral( "fff0" ), qMakePair( QColor(), false ) );

  // hex rrggbbaa colors
  colorTests.insert( QStringLiteral( "#ff00ffaa" ), qMakePair( QColor( 255, 0, 255, 170 ), true ) );
  colorTests.insert( QStringLiteral( "#99AA0099" ), qMakePair( QColor( 153, 170, 0, 153 ), true ) );
  colorTests.insert( QStringLiteral( "#GG0000aa" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "00000000" ), qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( QStringLiteral( "00ff0011" ), qMakePair( QColor( 0, 255, 0, 17 ), true ) );
  colorTests.insert( QStringLiteral( "00gg0011" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "00ff00000" ), qMakePair( QColor(), false ) );

  colorTests.insert( QStringLiteral( "0,0,0" ), qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( QStringLiteral( "127,60,0" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "255,255,255" ), qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( QStringLiteral( "256,60,0" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( "rgb(127,60,0)" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(255,255,255)" ), qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(256,60,0)" ), qMakePair( QColor(), false ) );
  colorTests.insert( QStringLiteral( " rgb(  127, 60 ,  0 ) " ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(127,60,0);" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "hsl(30,19%,90%)" ), qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( QStringLiteral( "hsl( 30 , 19 % , 90 % )" ), qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( QStringLiteral( " hsl(  30, 19%,  90% ) " ), qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( QStringLiteral( "hsl(30,19%,90%);" ), qMakePair( QColor::fromHsl( 30, 48, 229 ), false ) );
  colorTests.insert( QStringLiteral( "hsla(30,19%,90%,0.4)" ), qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( QStringLiteral( "hsla( 30 , 19 % , 90 % , 0.4 )" ), qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( QStringLiteral( " hsla(  30, 19%,  90%, 0.4 ) " ), qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( QStringLiteral( "hsla(30,19%,90%,0.4);" ), qMakePair( QColor::fromHsl( 30, 48, 229, 102 ), true ) );
  colorTests.insert( QStringLiteral( "(127,60,0);" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "(127,60,0)" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "127,060,000" ), qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( QStringLiteral( "0,0,0,0" ), qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( QStringLiteral( "127,60,0,0.5" ), qMakePair( QColor( 127, 60, 0, 128 ), true ) );
  colorTests.insert( QStringLiteral( "255,255,255,0.1" ), qMakePair( QColor( 255, 255, 255, 26 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(127,60,0,1.0)" ), qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(255,255,255,0.0)" ), qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( QStringLiteral( " rgba(  127, 60 ,  0  , 0.2 ) " ), qMakePair( QColor( 127, 60, 0, 51 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(127,60,0,0.1);" ), qMakePair( QColor( 127, 60, 0, 26 ), true ) );
  colorTests.insert( QStringLiteral( "(127,60,0,1);" ), qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "(127,60,0,1.0)" ), qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "127,060,000,1" ), qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "0%,0%,0%" ), qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( QStringLiteral( "50 %,60 %,0 %" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "100%, 100%, 100%" ), qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(50%,60%,0%)" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(100%, 100%, 100%)" ), qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( QStringLiteral( " rgb(  50 % , 60 % ,  0  % ) " ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "rgb(50%,60%,0%);" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%);" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%)" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "050%,060%,000%" ), qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( QStringLiteral( "0%,0%,0%,0" ), qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( QStringLiteral( "50 %,60 %,0 %,0.5" ), qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( QStringLiteral( "100%, 100%, 100%, 1.0" ), qMakePair( QColor( 255, 255, 255, 255 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(50%,60%,0%, 1.0)" ), qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(100%, 100%, 100%, 0.0)" ), qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( QStringLiteral( " rgba(  50 % , 60 % ,  0  %, 0.5 ) " ), qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( QStringLiteral( "rgba(50%,60%,0%,0);" ), qMakePair( QColor( 127, 153, 0, 0 ), true ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%,1);" ), qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%,1.0)" ), qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( QStringLiteral( "050%,060%,000%,0" ), qMakePair( QColor( 127, 153, 0, 0 ), true ) );

  QMap<QString, QPair< QColor, bool> >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color string: " +  i.key() );
    bool hasAlpha = false;
    const QColor result = QgsSymbolLayerUtils::parseColorWithAlpha( i.key(), hasAlpha );
    QVERIFY( result == i.value().first );
    QVERIFY( hasAlpha == i.value().second );
    ++i;
  }
}

void TestQgsSymbol::testParseColorList()
{
  //ensure that majority of single parseColor tests work for lists
  //note that some are not possible, as the colors may be ambiguous when treated as a list
  QMap< QString, QColor > colorTests;
  colorTests.insert( QStringLiteral( "bad color" ), QColor() );
  colorTests.insert( QStringLiteral( "red" ), QColor( 255, 0, 0 ) );
  colorTests.insert( QStringLiteral( "#ff00ff" ), QColor( 255, 0, 255 ) );
  colorTests.insert( QStringLiteral( "#99AA00" ), QColor( 153, 170, 0 ) );
  colorTests.insert( QStringLiteral( "#GG0000" ), QColor() );
  //colorTests.insert( "000000", QColor( 0, 0, 0 ) );
  //colorTests.insert( "00ff00", QColor( 0, 255, 0 ) );
  //colorTests.insert( "00gg00", QColor() );
  colorTests.insert( QStringLiteral( "00ff000" ), QColor() );
  //colorTests.insert( "fff", QColor( 255, 255, 255 ) );
  colorTests.insert( QStringLiteral( "fff0" ), QColor() );

  // hex rrggbbaa colors
  colorTests.insert( QStringLiteral( "#ff00ffaa" ), QColor( 255, 0, 255, 170 ) );
  colorTests.insert( QStringLiteral( "#99AA0099" ), QColor( 153, 170, 0, 153 ) );
  colorTests.insert( QStringLiteral( "#GG0000aa" ), QColor() );
  colorTests.insert( QStringLiteral( "00000000" ), QColor( 0, 0, 0, 0 ) );
  colorTests.insert( QStringLiteral( "00ff0011" ), QColor( 0, 255, 0, 17 ) );
  colorTests.insert( QStringLiteral( "00gg0011" ), QColor() );
  colorTests.insert( QStringLiteral( "00ff00000" ),  QColor() );

  colorTests.insert( QStringLiteral( "0,0,0" ), QColor( 0, 0, 0 ) );
  colorTests.insert( QStringLiteral( "127,60,0" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "255,255,255" ), QColor( 255, 255, 255 ) );
  //colorTests.insert( "256,60,0", QColor() );
  colorTests.insert( QStringLiteral( "rgb(127,60,0)" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "rgb(255,255,255)" ), QColor( 255, 255, 255 ) );
  colorTests.insert( QStringLiteral( "rgb(256,60,0)" ), QColor() );
  colorTests.insert( QStringLiteral( " rgb(  127, 60 ,  0 ) " ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "rgb(127,60,0);" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "(127,60,0);" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "(127,60,0)" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "127,060,000" ), QColor( 127, 60, 0 ) );
  colorTests.insert( QStringLiteral( "0,0,0,0" ), QColor( 0, 0, 0, 0 ) );
  colorTests.insert( QStringLiteral( "127,60,0,0.5" ), QColor( 127, 60, 0, 128 ) );
  colorTests.insert( QStringLiteral( "255,255,255,0.1" ), QColor( 255, 255, 255, 26 ) );
  colorTests.insert( QStringLiteral( "rgba(127,60,0,1.0)" ), QColor( 127, 60, 0, 255 ) );
  colorTests.insert( QStringLiteral( "rgba(255,255,255,0.0)" ), QColor( 255, 255, 255, 0 ) );
  colorTests.insert( QStringLiteral( " rgba(  127, 60 ,  0  , 0.2 ) " ), QColor( 127, 60, 0, 51 ) );
  colorTests.insert( QStringLiteral( "rgba(127,60,0,0.1);" ), QColor( 127, 60, 0, 26 ) );
  colorTests.insert( QStringLiteral( "(127,60,0,1);" ), QColor( 127, 60, 0, 255 ) );
  colorTests.insert( QStringLiteral( "(127,60,0,1.0)" ), QColor( 127, 60, 0, 255 ) );
  colorTests.insert( QStringLiteral( "127,060,000,1" ), QColor( 127, 60, 0, 255 ) );
  colorTests.insert( QStringLiteral( "0%,0%,0%" ), QColor( 0, 0, 0 ) );
  colorTests.insert( QStringLiteral( "50 %,60 %,0 %" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "100%, 100%, 100%" ), QColor( 255, 255, 255 ) );
  colorTests.insert( QStringLiteral( "rgb(50%,60%,0%)" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "rgb(100%, 100%, 100%)" ), QColor( 255, 255, 255 ) );
  colorTests.insert( QStringLiteral( " rgb(  50 % , 60 % ,  0  % ) " ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "rgb(50%,60%,0%);" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%);" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%)" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "050%,060%,000%" ), QColor( 127, 153, 0 ) );
  colorTests.insert( QStringLiteral( "0%,0%,0%,0" ), QColor( 0, 0, 0, 0 ) );
  colorTests.insert( QStringLiteral( "50 %,60 %,0 %,0.5" ), QColor( 127, 153, 0, 128 ) );
  colorTests.insert( QStringLiteral( "100%, 100%, 100%, 1.0" ), QColor( 255, 255, 255, 255 ) );
  colorTests.insert( QStringLiteral( "rgba(50%,60%,0%, 1.0)" ), QColor( 127, 153, 0, 255 ) );
  colorTests.insert( QStringLiteral( "rgba(100%, 100%, 100%, 0.0)" ), QColor( 255, 255, 255, 0 ) );
  colorTests.insert( QStringLiteral( " rgba(  50 % , 60 % ,  0  %, 0.5 ) " ), QColor( 127, 153, 0, 128 ) );
  colorTests.insert( QStringLiteral( "rgba(50%,60%,0%,0);" ), QColor( 127, 153, 0, 0 ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%,1);" ), QColor( 127, 153, 0, 255 ) );
  colorTests.insert( QStringLiteral( "(50%,60%,0%,1.0)" ), QColor( 127, 153, 0, 255 ) );
  colorTests.insert( QStringLiteral( "050%,060%,000%,0" ), QColor( 127, 153, 0, 0 ) );

  QMap<QString, QColor >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " +  i.key() );
    const QList< QColor > result = QgsSymbolLayerUtils::parseColorList( i.key() );
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

  QVector< QPair< QString, QList<QColor> > > colorListTests;
  QList<QColor> list1;
  list1 << QColor( QStringLiteral( "blue" ) ) << QColor( QStringLiteral( "red" ) ) << QColor( QStringLiteral( "green" ) );
  colorListTests.append( qMakePair( QStringLiteral( "blue red green" ), list1 ) );
  colorListTests.append( qMakePair( QStringLiteral( "blue,red,green" ), list1 ) );
  colorListTests.append( qMakePair( QStringLiteral( "blue\nred\ngreen" ), list1 ) );
  colorListTests.append( qMakePair( QStringLiteral( "blue\nred green" ), list1 ) );
  colorListTests.append( qMakePair( QStringLiteral( "blue\nred,green" ), list1 ) );
  QList<QColor> list2;
  list2 << QColor( QStringLiteral( "#ff0000" ) ) << QColor( QStringLiteral( "#00ff00" ) ) << QColor( QStringLiteral( "#0000ff" ) );
  colorListTests.append( qMakePair( QStringLiteral( "#ff0000 #00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QStringLiteral( "#ff0000,#00ff00,#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QStringLiteral( "#ff0000\n#00ff00\n#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QStringLiteral( "#ff0000\n#00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QStringLiteral( "#ff0000\n#00ff00,#0000ff" ), list2 ) );
  QList<QColor> list3;
  list3 << QColor( QStringLiteral( "#ff0000" ) ) << QColor( QStringLiteral( "#00ff00" ) ) << QColor( QStringLiteral( "#0000ff" ) );
  colorListTests.append( qMakePair( QStringLiteral( "rgb(255,0,0) rgb(0,255,0) rgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QStringLiteral( "rgb(255,0,0)\nrgb(0,255,0)\nrgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QStringLiteral( "rgb(255,0,0)\nrgb(0,255,0) rgb(0,0,255)" ), list3 ) );

  QVector< QPair< QString, QList<QColor> > >::const_iterator it = colorListTests.constBegin();
  while ( it != colorListTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " + ( *it ).first );
    const QList< QColor > result = QgsSymbolLayerUtils::parseColorList( ( *it ).first );
    if ( ( *it ).second.length() > 0 )
    {
      QCOMPARE( result.length(), ( *it ).second.length() );
      int index = 0;
      for ( QList<QColor>::const_iterator colorIt = ( *it ).second.constBegin();  colorIt != ( *it ).second.constEnd(); ++colorIt )
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

  QgsFillSymbol *fillSymbol2 = static_cast< QgsFillSymbol * >( fillSymbol->clone() );

  //test that two different symbol pointers return same properties
  QCOMPARE( QgsSymbolLayerUtils::symbolProperties( fillSymbol ),
            QgsSymbolLayerUtils::symbolProperties( fillSymbol2 ) );

  //modify one of the symbols
  fillSymbol2->symbolLayer( 0 )->setColor( QColor( 235, 135, 35 ) );
  QVERIFY( QgsSymbolLayerUtils::symbolProperties( fillSymbol ) !=
           QgsSymbolLayerUtils::symbolProperties( fillSymbol2 ) );

  delete fillSymbol;
  delete fillSymbol2;
}

QGSTEST_MAIN( TestQgsSymbol )
#include "testqgssymbol.moc"
