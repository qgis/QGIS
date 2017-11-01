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
#include "qgstest.h"
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include "qgsmarkersymbollayer.h"
#include "qgsfillsymbollayer.h"
#include "qgsproperty.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for QgsFilledMarkerSymbolLayer.
 */
class TestQgsFilledMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsFilledMarkerSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void filledMarkerSymbol();
    void dataDefinedShape();
    void bounds();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsFilledMarkerSymbolLayer *mFilledMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
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
                                      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //setup symbol
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientType( QgsGradientFillSymbolLayer::Linear );
  gradientFill->setGradientColorType( QgsGradientFillSymbolLayer::SimpleTwoColor );
  gradientFill->setCoordinateMode( QgsGradientFillSymbolLayer::Feature );
  gradientFill->setGradientSpread( QgsGradientFillSymbolLayer::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );

  mFilledMarkerLayer = new QgsFilledMarkerSymbolLayer();
  mFilledMarkerLayer->setSubSymbol( fillSymbol );
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mFilledMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
  mReport += QLatin1String( "<h1>Filled Marker Tests</h1>\n" );

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

  delete mpPointsLayer;

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
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyName, QgsProperty::fromExpression( QStringLiteral( "if(\"class\"='Jet','square','star')" ) ) );
  bool result = imageCheck( QStringLiteral( "filledmarker_datadefinedshape" ) );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyName, QgsProperty() );
  QVERIFY( result );
}

void TestQgsFilledMarkerSymbol::bounds()
{
  mFilledMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mFilledMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mFilledMarkerLayer->setSize( 5 );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( QStringLiteral( "filledmarker_bounds" ) );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty() );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsFilledMarkerSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_filledmarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsFilledMarkerSymbol )
#include "testqgsfilledmarker.moc"
