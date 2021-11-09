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
#include "qgstest.h"
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
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgscolorrampimpl.h"
//qgis test includes
#include "qgsmultirenderchecker.h"
#include "qgsfillsymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for shapeburst fill types.
 */
class TestQgsShapeburst : public QObject
{
    Q_OBJECT
  public:
    TestQgsShapeburst() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void shapeburstSymbol();
    void shapeburstSymbolColors();
    void shapeburstSymbolRamp();
    void shapeburstBlur();
    void shapeburstMaxDistanceMm();
    void shapeburstMaxDistanceMapUnits();
    void shapeburstIgnoreRings();
    void shapeburstSymbolFromQml();

  private:
    bool mTestHasError =  false ;
    bool setQml( const QString &type );
    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsShapeburstFillSymbolLayer *mShapeburstFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
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

  //setup shapeburst fill
  mShapeburstFill = new QgsShapeburstFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mShapeburstFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Shapeburst Renderer Tests</h1>\n" );

}
void TestQgsShapeburst::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer test</h2>\n" );
  mShapeburstFill->setColor( QColor( "red" ) );
  mShapeburstFill->setColor2( QColor( "blue" ) );
  mShapeburstFill->setBlurRadius( 0 );
  mShapeburstFill->setUseWholeShape( true );
  QVERIFY( imageCheck( "shapeburst" ) );
}

void TestQgsShapeburst::shapeburstSymbolColors()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer color test</h2>\n" );
  mShapeburstFill->setColor( QColor( "green" ) );
  mShapeburstFill->setColor2( QColor( "white" ) );
  QVERIFY( imageCheck( "shapeburst_colors" ) );
  mShapeburstFill->setColor( QColor( "red" ) );
  mShapeburstFill->setColor2( QColor( "blue" ) );
}

void TestQgsShapeburst::shapeburstSymbolRamp()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer ramp test</h2>\n" );

  QgsGradientColorRamp *gradientRamp = new QgsGradientColorRamp( QColor( Qt::yellow ), QColor( 255, 105, 180 ) );
  QgsGradientStopsList stops;
  stops.append( QgsGradientStop( 0.5, QColor( 255, 255, 255, 0 ) ) );
  gradientRamp->setStops( stops );

  mShapeburstFill->setColorRamp( gradientRamp );
  mShapeburstFill->setColorType( Qgis::GradientColorSource::ColorRamp );
  QVERIFY( imageCheck( "shapeburst_ramp" ) );
  mShapeburstFill->setColorType( Qgis::GradientColorSource::SimpleTwoColor );
}

void TestQgsShapeburst::shapeburstBlur()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer blur test</h2>\n" );
  mShapeburstFill->setBlurRadius( 17 );
  QVERIFY( imageCheck( "shapeburst_blur" ) );
  mShapeburstFill->setBlurRadius( 0 );
}

void TestQgsShapeburst::shapeburstMaxDistanceMm()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer maximum distance MM </h2>\n" );
  mShapeburstFill->setUseWholeShape( false );
  mShapeburstFill->setMaxDistance( 3 );
  mShapeburstFill->setDistanceUnit( QgsUnitTypes::RenderMillimeters );
  QVERIFY( imageCheck( "shapeburst_maxdistance_mm" ) );
  mShapeburstFill->setUseWholeShape( true );
}

void TestQgsShapeburst::shapeburstMaxDistanceMapUnits()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer maximum distance map units</h2>\n" );
  mShapeburstFill->setUseWholeShape( false );
  mShapeburstFill->setMaxDistance( 10 );
  mShapeburstFill->setDistanceUnit( QgsUnitTypes::RenderMapUnits );
  QVERIFY( imageCheck( "shapeburst_maxdistance_mapunit" ) );
  mShapeburstFill->setUseWholeShape( true );
  mShapeburstFill->setDistanceUnit( QgsUnitTypes::RenderMillimeters );
}

void TestQgsShapeburst::shapeburstIgnoreRings()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol renderer ignore rings</h2>\n" );
  mShapeburstFill->setIgnoreRings( true );
  QVERIFY( imageCheck( "shapeburst_ignorerings" ) );
  mShapeburstFill->setIgnoreRings( false );
}

void TestQgsShapeburst::shapeburstSymbolFromQml()
{
  mReport += QLatin1String( "<h2>Shapeburst symbol from QML test</h2>\n" );
  QVERIFY( setQml( "shapeburst" ) );
  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );
  QVERIFY( imageCheck( "shapeburst_from_qml" ) );
}

//
// Private helper functions not called directly by CTest
//

bool TestQgsShapeburst::setQml( const QString &type )
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

bool TestQgsShapeburst::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_shapeburst" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  myChecker.setColorTolerance( 20 );
  const bool myResultFlag = myChecker.runTest( testType, 500 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsShapeburst )
#include "testqgsshapeburst.moc"
