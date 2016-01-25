/***************************************************************************
     testqgssymbolv2.cpp
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
#include <QtTest/QtTest>
#include <QObject>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>

//qgis includes...
#include "qgsmultirenderchecker.h"
#include <qgsapplication.h>
#include "qgsconfig.h"
#include "qgslogger.h"
#include "qgsvectorcolorrampv2.h"
#include "qgscptcityarchive.h"
#include "qgsvectorlayer.h"
#include "qgsmaplayerregistry.h"
#include "qgslinesymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgssinglesymbolrendererv2.h"

#include "qgsstylev2.h"

/** \ingroup UnitTests
 * This is a unit test to verify that symbols are working correctly
 */
class TestQgsSymbolV2 : public QObject
{
    Q_OBJECT

  public:

    TestQgsSymbolV2();

  private:

    QString mReport;
    QString mTestDataDir;

    QgsVectorLayer * mpPointsLayer;
    QgsVectorLayer * mpLinesLayer;
    QgsVectorLayer * mpPolysLayer;

    bool imageCheck( QgsMapSettings &ms, const QString &testName );

  private slots:

    // init / cleanup
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {}// will be called before each testfunction is executed.
    void cleanup() {}// will be called after every testfunction.
    // void initStyles();

    void testCanvasClip();
    void testParseColor();
    void testParseColorList();
    void symbolProperties();
};

TestQgsSymbolV2::TestQgsSymbolV2()
    : mpPointsLayer( 0 )
    , mpLinesLayer( 0 )
    , mpPolysLayer( 0 )
{

}

// slots
void TestQgsSymbolV2::initTestCase()
{
  // initialize with test settings directory so we don't mess with user's stuff
  QgsApplication::init( QDir::tempPath() + "/dot-qgis" );
  QgsApplication::initQgis();
  QgsApplication::createDB();
  mTestDataDir = QString( TEST_DATA_DIR ) + '/'; //defined in CmakeLists.txt

  // output test environment
  QgsApplication::showSettings();

  // Set up the QSettings environment
  QCoreApplication::setOrganizationName( "QGIS" );
  QCoreApplication::setOrganizationDomain( "qgis.org" );
  QCoreApplication::setApplicationName( "QGIS-TEST" );

  // initialize with a clean style
  QFile styleFile( QgsApplication::userStyleV2Path() );
  if ( styleFile.exists() )
  {
    styleFile.remove();
    QgsDebugMsg( "removed user style file " + styleFile.fileName() );
  }

  //
  //create a point layer that will be used in all tests...
  //
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';
  QString myPointsFileName = mTestDataDir + "points.shp";
  QFileInfo myPointFileInfo( myPointsFileName );
  mpPointsLayer = new QgsVectorLayer( myPointFileInfo.filePath(),
                                      myPointFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );


  //
  // Create a line layer that will be used in all tests...
  //
  QString myLinesFileName = mTestDataDir + "lines.shp";
  QFileInfo myLineFileInfo( myLinesFileName );
  mpLinesLayer = new QgsVectorLayer( myLineFileInfo.filePath(),
                                     myLineFileInfo.completeBaseName(), "ogr" );
  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpLinesLayer );

  mReport += "<h1>StyleV2 Tests</h1>\n";
}

void TestQgsSymbolV2::cleanupTestCase()
{
  QgsApplication::exitQgis();

  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
    //QDesktopServices::openUrl( "file:///" + myReportFile );
  }
}

bool TestQgsSymbolV2::imageCheck( QgsMapSettings& ms, const QString& testName )
{
  QgsMultiRenderChecker checker;
  ms.setOutputDpi( 96 );
  checker.setControlName( "expected_" + testName );
  checker.setMapSettings( ms );
  bool result = checker.runTest( testName, 0 );
  mReport += checker.report();
  return result;
}

void TestQgsSymbolV2::testCanvasClip()
{
  //test rendering with and without clip to canvas enabled
  QgsMapSettings ms;
  QgsRectangle extent( -110.0, 25.0, -90, 40.0 );
  ms.setExtent( extent );
  ms.setFlag( QgsMapSettings::ForceVectorOutput );

  //line
  mReport += "<h2>Line canvas clip</h2>\n";
  ms.setLayers( QStringList() << mpLinesLayer->id() );

  QgsMarkerLineSymbolLayerV2* markerLine = new QgsMarkerLineSymbolLayerV2();
  markerLine->setPlacement( QgsMarkerLineSymbolLayerV2:: CentralPoint );
  QgsLineSymbolV2* lineSymbol = new QgsLineSymbolV2();
  lineSymbol->changeSymbolLayer( 0, markerLine );
  QgsSingleSymbolRendererV2* renderer = new QgsSingleSymbolRendererV2( lineSymbol );
  mpLinesLayer->setRendererV2( renderer );
  bool result;

  lineSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, "stylev2_linecanvasclip" );
  QVERIFY( result );

  lineSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, "stylev2_linecanvasclip_off" );
  QVERIFY( result );

  //poly
  mReport += "<h2>Polygon canvas clip</h2>\n";
  ms.setLayers( QStringList() << mpPolysLayer->id() );

  QgsCentroidFillSymbolLayerV2* centroidFill = new QgsCentroidFillSymbolLayerV2();
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, centroidFill );
  renderer = new QgsSingleSymbolRendererV2( fillSymbol );
  mpPolysLayer->setRendererV2( renderer );

  extent = QgsRectangle( -106.0, 29.0, -94, 36.0 );
  ms.setExtent( extent );

  fillSymbol->setClipFeaturesToExtent( true );
  result = imageCheck( ms, "stylev2_polycanvasclip" );
  QVERIFY( result );

  fillSymbol->setClipFeaturesToExtent( false );
  result = imageCheck( ms, "stylev2_polycanvasclip_off" );
  QVERIFY( result );
}


void TestQgsSymbolV2::testParseColor()
{
  // values for color tests
  QMap< QString, QPair< QColor, bool> > colorTests;
  colorTests.insert( "bad color", qMakePair( QColor(), false ) );
  colorTests.insert( "red", qMakePair( QColor( 255, 0, 0 ), false ) );
  colorTests.insert( "#ff00ff", qMakePair( QColor( 255, 0, 255 ), false ) );
  colorTests.insert( "#99AA00", qMakePair( QColor( 153, 170, 0 ), false ) );
  colorTests.insert( "#GG0000", qMakePair( QColor(), false ) );
  colorTests.insert( "000000", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "00ff00", qMakePair( QColor( 0, 255, 0 ), false ) );
  colorTests.insert( "00gg00", qMakePair( QColor(), false ) );
  colorTests.insert( "00ff000", qMakePair( QColor(), false ) );
  colorTests.insert( "fff", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "fff0", qMakePair( QColor(), false ) );

  // hex rrggbbaa colors
  colorTests.insert( "#ff00ffaa", qMakePair( QColor( 255, 0, 255, 170 ), true ) );
  colorTests.insert( "#99AA0099", qMakePair( QColor( 153, 170, 0, 153 ), true ) );
  colorTests.insert( "#GG0000aa", qMakePair( QColor(), false ) );
  colorTests.insert( "00000000", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "00ff0011", qMakePair( QColor( 0, 255, 0, 17 ), true ) );
  colorTests.insert( "00gg0011", qMakePair( QColor(), false ) );
  colorTests.insert( "00ff00000", qMakePair( QColor(), false ) );

  colorTests.insert( "0,0,0", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "127,60,0", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "255,255,255", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "256,60,0", qMakePair( QColor(), false ) );
  colorTests.insert( "rgb(127,60,0)", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "rgb(255,255,255)", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "rgb(256,60,0)", qMakePair( QColor(), false ) );
  colorTests.insert( " rgb(  127, 60 ,  0 ) ", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "rgb(127,60,0);", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "(127,60,0);", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "(127,60,0)", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "127,060,000", qMakePair( QColor( 127, 60, 0 ), false ) );
  colorTests.insert( "0,0,0,0", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "127,60,0,0.5", qMakePair( QColor( 127, 60, 0, 128 ), true ) );
  colorTests.insert( "255,255,255,0.1", qMakePair( QColor( 255, 255, 255, 26 ), true ) );
  colorTests.insert( "rgba(127,60,0,1.0)", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "rgba(255,255,255,0.0)", qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( " rgba(  127, 60 ,  0  , 0.2 ) ", qMakePair( QColor( 127, 60, 0, 51 ), true ) );
  colorTests.insert( "rgba(127,60,0,0.1);", qMakePair( QColor( 127, 60, 0, 26 ), true ) );
  colorTests.insert( "(127,60,0,1);", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "(127,60,0,1.0)", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "127,060,000,1", qMakePair( QColor( 127, 60, 0, 255 ), true ) );
  colorTests.insert( "0%,0%,0%", qMakePair( QColor( 0, 0, 0 ), false ) );
  colorTests.insert( "50 %,60 %,0 %", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "100%, 100%, 100%", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( "rgb(50%,60%,0%)", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "rgb(100%, 100%, 100%)", qMakePair( QColor( 255, 255, 255 ), false ) );
  colorTests.insert( " rgb(  50 % , 60 % ,  0  % ) ", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "rgb(50%,60%,0%);", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "(50%,60%,0%);", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "(50%,60%,0%)", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "050%,060%,000%", qMakePair( QColor( 127, 153, 0 ), false ) );
  colorTests.insert( "0%,0%,0%,0", qMakePair( QColor( 0, 0, 0, 0 ), true ) );
  colorTests.insert( "50 %,60 %,0 %,0.5", qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( "100%, 100%, 100%, 1.0", qMakePair( QColor( 255, 255, 255, 255 ), true ) );
  colorTests.insert( "rgba(50%,60%,0%, 1.0)", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "rgba(100%, 100%, 100%, 0.0)", qMakePair( QColor( 255, 255, 255, 0 ), true ) );
  colorTests.insert( " rgba(  50 % , 60 % ,  0  %, 0.5 ) ", qMakePair( QColor( 127, 153, 0, 128 ), true ) );
  colorTests.insert( "rgba(50%,60%,0%,0);", qMakePair( QColor( 127, 153, 0, 0 ), true ) );
  colorTests.insert( "(50%,60%,0%,1);", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "(50%,60%,0%,1.0)", qMakePair( QColor( 127, 153, 0, 255 ), true ) );
  colorTests.insert( "050%,060%,000%,0", qMakePair( QColor( 127, 153, 0, 0 ), true ) );

  QMap<QString, QPair< QColor, bool> >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color string: " +  i.key() );
    bool hasAlpha = false;
    QColor result = QgsSymbolLayerV2Utils::parseColorWithAlpha( i.key(), hasAlpha );
    QVERIFY( result == i.value().first );
    QVERIFY( hasAlpha == i.value().second );
    ++i;
  }
}

void TestQgsSymbolV2::testParseColorList()
{
  //ensure that majority of single parseColor tests work for lists
  //note that some are not possible, as the colors may be ambiguous when treated as a list
  QMap< QString, QColor > colorTests;
  colorTests.insert( "bad color", QColor() );
  colorTests.insert( "red", QColor( 255, 0, 0 ) );
  colorTests.insert( "#ff00ff", QColor( 255, 0, 255 ) );
  colorTests.insert( "#99AA00", QColor( 153, 170, 0 ) );
  colorTests.insert( "#GG0000", QColor() );
  //colorTests.insert( "000000", QColor( 0, 0, 0 ) );
  //colorTests.insert( "00ff00", QColor( 0, 255, 0 ) );
  //colorTests.insert( "00gg00", QColor() );
  colorTests.insert( "00ff000", QColor() );
  //colorTests.insert( "fff", QColor( 255, 255, 255 ) );
  colorTests.insert( "fff0", QColor() );

  // hex rrggbbaa colors
  colorTests.insert( "#ff00ffaa", QColor( 255, 0, 255, 170 ) );
  colorTests.insert( "#99AA0099", QColor( 153, 170, 0, 153 ) );
  colorTests.insert( "#GG0000aa", QColor() );
  colorTests.insert( "00000000", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "00ff0011", QColor( 0, 255, 0, 17 ) );
  colorTests.insert( "00gg0011", QColor() );
  colorTests.insert( "00ff00000",  QColor() );

  colorTests.insert( "0,0,0", QColor( 0, 0, 0 ) );
  colorTests.insert( "127,60,0", QColor( 127, 60, 0 ) );
  colorTests.insert( "255,255,255", QColor( 255, 255, 255 ) );
  //colorTests.insert( "256,60,0", QColor() );
  colorTests.insert( "rgb(127,60,0)", QColor( 127, 60, 0 ) );
  colorTests.insert( "rgb(255,255,255)", QColor( 255, 255, 255 ) );
  colorTests.insert( "rgb(256,60,0)", QColor() );
  colorTests.insert( " rgb(  127, 60 ,  0 ) ", QColor( 127, 60, 0 ) );
  colorTests.insert( "rgb(127,60,0);", QColor( 127, 60, 0 ) );
  colorTests.insert( "(127,60,0);", QColor( 127, 60, 0 ) );
  colorTests.insert( "(127,60,0)", QColor( 127, 60, 0 ) );
  colorTests.insert( "127,060,000", QColor( 127, 60, 0 ) );
  colorTests.insert( "0,0,0,0", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "127,60,0,0.5", QColor( 127, 60, 0, 128 ) );
  colorTests.insert( "255,255,255,0.1", QColor( 255, 255, 255, 26 ) );
  colorTests.insert( "rgba(127,60,0,1.0)", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "rgba(255,255,255,0.0)", QColor( 255, 255, 255, 0 ) );
  colorTests.insert( " rgba(  127, 60 ,  0  , 0.2 ) ", QColor( 127, 60, 0, 51 ) );
  colorTests.insert( "rgba(127,60,0,0.1);", QColor( 127, 60, 0, 26 ) );
  colorTests.insert( "(127,60,0,1);", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "(127,60,0,1.0)", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "127,060,000,1", QColor( 127, 60, 0, 255 ) );
  colorTests.insert( "0%,0%,0%", QColor( 0, 0, 0 ) );
  colorTests.insert( "50 %,60 %,0 %", QColor( 127, 153, 0 ) );
  colorTests.insert( "100%, 100%, 100%", QColor( 255, 255, 255 ) );
  colorTests.insert( "rgb(50%,60%,0%)", QColor( 127, 153, 0 ) );
  colorTests.insert( "rgb(100%, 100%, 100%)", QColor( 255, 255, 255 ) );
  colorTests.insert( " rgb(  50 % , 60 % ,  0  % ) ", QColor( 127, 153, 0 ) );
  colorTests.insert( "rgb(50%,60%,0%);", QColor( 127, 153, 0 ) );
  colorTests.insert( "(50%,60%,0%);", QColor( 127, 153, 0 ) );
  colorTests.insert( "(50%,60%,0%)", QColor( 127, 153, 0 ) );
  colorTests.insert( "050%,060%,000%", QColor( 127, 153, 0 ) );
  colorTests.insert( "0%,0%,0%,0", QColor( 0, 0, 0, 0 ) );
  colorTests.insert( "50 %,60 %,0 %,0.5", QColor( 127, 153, 0, 128 ) );
  colorTests.insert( "100%, 100%, 100%, 1.0", QColor( 255, 255, 255, 255 ) );
  colorTests.insert( "rgba(50%,60%,0%, 1.0)", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "rgba(100%, 100%, 100%, 0.0)", QColor( 255, 255, 255, 0 ) );
  colorTests.insert( " rgba(  50 % , 60 % ,  0  %, 0.5 ) ", QColor( 127, 153, 0, 128 ) );
  colorTests.insert( "rgba(50%,60%,0%,0);", QColor( 127, 153, 0, 0 ) );
  colorTests.insert( "(50%,60%,0%,1);", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "(50%,60%,0%,1.0)", QColor( 127, 153, 0, 255 ) );
  colorTests.insert( "050%,060%,000%,0", QColor( 127, 153, 0, 0 ) );

  QMap<QString, QColor >::const_iterator i = colorTests.constBegin();
  while ( i != colorTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " +  i.key() );
    QList< QColor > result = QgsSymbolLayerV2Utils::parseColorList( i.key() );
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
  list1 << QColor( QString( "blue" ) ) << QColor( QString( "red" ) ) << QColor( QString( "green" ) );
  colorListTests.append( qMakePair( QString( "blue red green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue,red,green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred\ngreen" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred green" ), list1 ) );
  colorListTests.append( qMakePair( QString( "blue\nred,green" ), list1 ) );
  QList<QColor> list2;
  list2 << QColor( QString( "#ff0000" ) ) << QColor( QString( "#00ff00" ) ) << QColor( QString( "#0000ff" ) );
  colorListTests.append( qMakePair( QString( "#ff0000 #00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000,#00ff00,#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00\n#0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00 #0000ff" ), list2 ) );
  colorListTests.append( qMakePair( QString( "#ff0000\n#00ff00,#0000ff" ), list2 ) );
  QList<QColor> list3;
  list3 << QColor( QString( "#ff0000" ) ) << QColor( QString( "#00ff00" ) ) << QColor( QString( "#0000ff" ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0) rgb(0,255,0) rgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0)\nrgb(0,255,0)\nrgb(0,0,255)" ), list3 ) );
  colorListTests.append( qMakePair( QString( "rgb(255,0,0)\nrgb(0,255,0) rgb(0,0,255)" ), list3 ) );

  QVector< QPair< QString, QList<QColor> > >::const_iterator it = colorListTests.constBegin();
  while ( it != colorListTests.constEnd() )
  {
    QgsDebugMsg( "color list string: " + ( *it ).first );
    QList< QColor > result = QgsSymbolLayerV2Utils::parseColorList(( *it ).first );
    if (( *it ).second.length() > 0 )
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

void TestQgsSymbolV2::symbolProperties()
{
  //test QgsSymbolLayerV2Utils::symbolProperties

  //make a symbol
  QgsSimpleFillSymbolLayerV2* fill = new QgsSimpleFillSymbolLayerV2();
  fill->setColor( QColor( 25, 125, 225 ) );
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, fill );

  QgsFillSymbolV2* fillSymbol2 = static_cast< QgsFillSymbolV2* >( fillSymbol->clone() );

  //test that two different symbol pointers return same properties
  QCOMPARE( QgsSymbolLayerV2Utils::symbolProperties( fillSymbol ),
            QgsSymbolLayerV2Utils::symbolProperties( fillSymbol2 ) );

  //modify one of the symbols
  fillSymbol2->symbolLayer( 0 )->setColor( QColor( 235, 135, 35 ) );
  QVERIFY( QgsSymbolLayerV2Utils::symbolProperties( fillSymbol ) !=
           QgsSymbolLayerV2Utils::symbolProperties( fillSymbol2 ) );

  delete fillSymbol;
  delete fillSymbol2;
}

QTEST_MAIN( TestQgsSymbolV2 )
#include "testqgssymbolv2.moc"
