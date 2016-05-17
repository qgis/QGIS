/***************************************************************************
     testqgsfilledmarker.cpp
     -----------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Nyall Dawson
    Email                : nyall dot dawson at gmail dot com
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
#include <QDesktopServices>

//qgis includes...
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
#include "qgsmarkersymbollayerv2.h"
#include "qgsfillsymbollayerv2.h"
#include "qgsdatadefined.h"

//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for QgsFilledMarkerSymbolLayer.
 */
class TestQgsFilledMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsFilledMarkerSymbol()
        : mTestHasError( false )
        , mpPointsLayer( nullptr )
        , mFilledMarkerLayer( nullptr )
        , mMarkerSymbol( nullptr )
        , mSymbolRenderer( nullptr )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void filledMarkerSymbol();
    void dataDefinedShape();
    void bounds();

  private:
    bool mTestHasError;

    bool imageCheck( const QString& theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPointsLayer;
    QgsFilledMarkerSymbolLayer* mFilledMarkerLayer;
    QgsMarkerSymbolV2* mMarkerSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsFilledMarkerSymbol::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  QString pointFileName = mTestDataDir + "points.shp";
  QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                      pointFileInfo.completeBaseName(), "ogr" );

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //setup symbol
  QgsGradientFillSymbolLayerV2* gradientFill = new QgsGradientFillSymbolLayerV2();
  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientType( QgsGradientFillSymbolLayerV2::Linear );
  gradientFill->setGradientColorType( QgsGradientFillSymbolLayerV2::SimpleTwoColor );
  gradientFill->setCoordinateMode( QgsGradientFillSymbolLayerV2::Feature );
  gradientFill->setGradientSpread( QgsGradientFillSymbolLayerV2::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QgsFillSymbolV2* fillSymbol = new QgsFillSymbolV2();
  fillSymbol->changeSymbolLayer( 0, gradientFill );

  mFilledMarkerLayer = new QgsFilledMarkerSymbolLayer();
  mFilledMarkerLayer->setSubSymbol( fillSymbol );
  mMarkerSymbol = new QgsMarkerSymbolV2();
  mMarkerSymbol->changeSymbolLayer( 0, mFilledMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mMarkerSymbol );
  mpPointsLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPointsLayer->id() );
  mReport += "<h1>Filled Marker Tests</h1>\n";

}
void TestQgsFilledMarkerSymbol::cleanupTestCase()
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

void TestQgsFilledMarkerSymbol::filledMarkerSymbol()
{
  mFilledMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mFilledMarkerLayer->setSize( 15 );
  QVERIFY( imageCheck( "filledmarker" ) );
}

void TestQgsFilledMarkerSymbol::dataDefinedShape()
{
  mFilledMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mFilledMarkerLayer->setSize( 10 );
  mFilledMarkerLayer->setDataDefinedProperty( "name", new QgsDataDefined( true, true, "if(\"class\"='Jet','square','star')" ) );
  bool result = imageCheck( "filledmarker_datadefinedshape" );
  mFilledMarkerLayer->removeDataDefinedProperty( "name" );
  QVERIFY( result );
}

void TestQgsFilledMarkerSymbol::bounds()
{
  mFilledMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mFilledMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mFilledMarkerLayer->setSize( 5 );
  mFilledMarkerLayer->setDataDefinedProperty( "size", new QgsDataDefined( true, true, "min(\"importance\" * 2, 6)" ) );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( "filledmarker_bounds" );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mFilledMarkerLayer->removeDataDefinedProperty( "size" );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsFilledMarkerSymbol::imageCheck( const QString& theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( "symbol_filledmarker" );
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( theTestType );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsFilledMarkerSymbol )
#include "testqgsfilledmarker.moc"
