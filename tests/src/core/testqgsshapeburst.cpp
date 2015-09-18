/***************************************************************************
     testqgsshapeburst.cpp
     --------------------------------------
    Date                 : 20 Jan 2008
    Copyright            : (C) 2008 by Tim Sutton
    Email                : tim @ linfiniti.com
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
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
#include <qgsfillsymbollayerv2.h>
#include <qgsvectorcolorrampv2.h>
//qgis test includes
#include "qgsmultirenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for shapeburst fill types.
 */
class TestQgsShapeburst : public QObject
{
    Q_OBJECT
  public:
    TestQgsShapeburst()
        : mTestHasError( false )
        , mpPolysLayer()
        , mShapeburstFill( 0 )
        , mFillSymbol( 0 )
        , mSymbolRenderer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {};// will be called before each testfunction is executed.
    void cleanup() {};// will be called after every testfunction.

    void shapeburstSymbol();
    void shapeburstSymbolColors();
    void shapeburstSymbolRamp();
    void shapeburstBlur();
    void shapeburstMaxDistanceMm();
    void shapeburstMaxDistanceMapUnits();
    void shapeburstIgnoreRings();
    void shapeburstSymbolFromQml();

  private:
    bool mTestHasError;
    bool setQml( QString theType );
    bool imageCheck( QString theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPolysLayer;
    QgsShapeburstFillSymbolLayerV2* mShapeburstFill;
    QgsFillSymbolV2* mFillSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsShapeburst::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + "/";

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  //setup shapeburst fill
  mShapeburstFill = new QgsShapeburstFillSymbolLayerV2();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mShapeburstFill );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mFillSymbol );
  mpPolysLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPolysLayer->id() );
  mReport += "<h1>Shapeburst Renderer Tests</h1>\n";

}
void TestQgsShapeburst::cleanupTestCase()
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

void TestQgsShapeburst::shapeburstSymbol()
{
  mReport += "<h2>Shapeburst symbol renderer test</h2>\n";
  mShapeburstFill->setColor( QColor( "red" ) );
  mShapeburstFill->setColor2( QColor( "blue" ) );
  mShapeburstFill->setBlurRadius( 0 );
  mShapeburstFill->setUseWholeShape( true );
  QVERIFY( imageCheck( "shapeburst" ) );
}

void TestQgsShapeburst::shapeburstSymbolColors()
{
  mReport += "<h2>Shapeburst symbol renderer color test</h2>\n";
  mShapeburstFill->setColor( QColor( "green" ) );
  mShapeburstFill->setColor2( QColor( "white" ) );
  QVERIFY( imageCheck( "shapeburst_colors" ) );
  mShapeburstFill->setColor( QColor( "red" ) );
  mShapeburstFill->setColor2( QColor( "blue" ) );
}

void TestQgsShapeburst::shapeburstSymbolRamp()
{
  mReport += "<h2>Shapeburst symbol renderer ramp test</h2>\n";

  QgsVectorGradientColorRampV2* gradientRamp = new QgsVectorGradientColorRampV2( QColor( Qt::yellow ), QColor( 255, 105, 180 ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( 255, 255, 255, 0 ) ) );
  gradientRamp->setStops( stops );

  mShapeburstFill->setColorRamp( gradientRamp );
  mShapeburstFill->setColorType( QgsShapeburstFillSymbolLayerV2::ColorRamp );
  QVERIFY( imageCheck( "shapeburst_ramp" ) );
  mShapeburstFill->setColorType( QgsShapeburstFillSymbolLayerV2::SimpleTwoColor );
}

void TestQgsShapeburst::shapeburstBlur()
{
  mReport += "<h2>Shapeburst symbol renderer blur test</h2>\n";
  mShapeburstFill->setBlurRadius( 17 );
  QVERIFY( imageCheck( "shapeburst_blur" ) );
  mShapeburstFill->setBlurRadius( 0 );
}

void TestQgsShapeburst::shapeburstMaxDistanceMm()
{
  mReport += "<h2>Shapeburst symbol renderer maximum distance MM </h2>\n";
  mShapeburstFill->setUseWholeShape( false );
  mShapeburstFill->setMaxDistance( 3 );
  mShapeburstFill->setDistanceUnit( QgsSymbolV2::MM );
  QVERIFY( imageCheck( "shapeburst_maxdistance_mm" ) );
  mShapeburstFill->setUseWholeShape( true );
}

void TestQgsShapeburst::shapeburstMaxDistanceMapUnits()
{
  mReport += "<h2>Shapeburst symbol renderer maximum distance map units</h2>\n";
  mShapeburstFill->setUseWholeShape( false );
  mShapeburstFill->setMaxDistance( 10 );
  mShapeburstFill->setDistanceUnit( QgsSymbolV2::MapUnit );
  QVERIFY( imageCheck( "shapeburst_maxdistance_mapunit" ) );
  mShapeburstFill->setUseWholeShape( true );
  mShapeburstFill->setDistanceUnit( QgsSymbolV2::MM );
}

void TestQgsShapeburst::shapeburstIgnoreRings()
{
  mReport += "<h2>Shapeburst symbol renderer ignore rings</h2>\n";
  mShapeburstFill->setIgnoreRings( true );
  QVERIFY( imageCheck( "shapeburst_ignorerings" ) );
  mShapeburstFill->setIgnoreRings( false );
}

void TestQgsShapeburst::shapeburstSymbolFromQml()
{
  mReport += "<h2>Shapeburst symbol from QML test</h2>\n";
  QVERIFY( setQml( "shapeburst" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );
  QVERIFY( imageCheck( "shapeburst_from_qml" ) );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsShapeburst::setQml( QString theType )
{
  //load a qml style and apply to our layer
  //the style will correspond to the renderer
  //type we are testing
  QString myFileName = mTestDataDir + "polys_" + theType + "_symbol.qml";
  bool myStyleFlag = false;
  QString error = mpPolysLayer->loadNamedStyle( myFileName, myStyleFlag );
  if ( !myStyleFlag )
  {
    qDebug( "%s", error.toLocal8Bit().constData() );
  }
  return myStyleFlag;
}

bool TestQgsShapeburst::imageCheck( QString theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( "symbol_shapeburst" );
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  bool myResultFlag = myChecker.runTest( theTestType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsShapeburst )
#include "testqgsshapeburst.moc"
