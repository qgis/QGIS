/***************************************************************************
     testqgsrasterfill.cpp
     ---------------------
    Date                 : November 2014
    Copyright            : (C) 2014 Nyall Dawson
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

// qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgsfillsymbol.h"
// qgis test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for raster fill types.
 */
class TestQgsRasterFill : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterFill() : QgsTest( QStringLiteral( "Raster Fill Renderer Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init(); // will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void rasterFillSymbol();
    void coordinateMode();
    void alpha();
    void offset();
    void width();

    // Tests for percentage value of size unit.
    void percentage();
    void percentageCoordinateMode();
    void percentageOffset();
    void percentageAlpha();
    void percentageWidth();

  private:
    bool mTestHasError = false;
    bool setQml( const QString &type );
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsRasterFillSymbolLayer *mRasterFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsRasterFill::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = mTestDataDir + "polys.shp";
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  //setup raster fill
  mRasterFill = new QgsRasterFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mRasterFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
}

void TestQgsRasterFill::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterFill::init()
{
  mRasterFill->setImageFilePath( mTestDataDir + QStringLiteral( "sample_image.png" ) );
  mRasterFill->setWidth( 30.0 );
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPixels );
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  mRasterFill->setOpacity( 1.0 );
  mRasterFill->setOffset( QPointF( 0, 0 ) );
}

void TestQgsRasterFill::cleanup()
{

}

void TestQgsRasterFill::rasterFillSymbol()
{
  const bool result = imageCheck( QStringLiteral( "rasterfill" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::coordinateMode()
{
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );
  const bool result = imageCheck( QStringLiteral( "rasterfill_viewport" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::alpha()
{
  mRasterFill->setOpacity( 0.5 );
  const bool result = imageCheck( QStringLiteral( "rasterfill_alpha" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::offset()
{
  mRasterFill->setOffset( QPointF( 5, 10 ) );
  const bool result = imageCheck( QStringLiteral( "rasterfill_offset" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::width()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderMillimeters );
  mRasterFill->setWidth( 5.0 );
  const bool result = imageCheck( QStringLiteral( "rasterfill_width" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::percentage()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 6.3 );
  const bool result = imageCheck( QStringLiteral( "rasterfill_percentage" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::percentageCoordinateMode()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );
  const bool result = imageCheck( QStringLiteral( "rasterfill_viewport_percentage" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::percentageOffset()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setOffsetUnit( QgsUnitTypes::RenderPixels );
  mRasterFill->setOffset( QPointF( 12, 15 ) );
  const bool result = imageCheck( QStringLiteral( "rasterfill_offset_percentage" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::percentageAlpha()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setOpacity( 0.5 );
  const bool result = imageCheck( QStringLiteral( "rasterfill_alpha_percentage" ) );
  QVERIFY( result );
}

void TestQgsRasterFill::percentageWidth()
{
  mRasterFill->setWidthUnit( QgsUnitTypes::RenderPercentage );
  mRasterFill->setWidth( 3.3 );
  const bool result = imageCheck( QStringLiteral( "rasterfill_width_percentage" ) );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsRasterFill::setQml( const QString &type )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  const QString myFileName = mTestDataDir + "polys_" + type + "_symbol.qml";
  bool myStyleFlag = false;
  const QString error = mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
  }
  return myStyleFlag;
}

bool TestQgsRasterFill::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_rasterfill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  const bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsRasterFill )
#include "testqgsrasterfill.moc"
