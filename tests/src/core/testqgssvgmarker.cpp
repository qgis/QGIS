/***************************************************************************
     testqgssvgmarker.cpp
     --------------------
    Date                 : Nov 2015
    Copyright            : (C) 2015 by Nyall Dawson
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
#include <qgspathresolver.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include "qgsmarkersymbollayer.h"
#include "qgsproperty.h"
#include "qgssymbollayerutils.h"
#include "qgsmarkersymbol.h"

//qgis test includes
#include "qgsmultirenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for SVG marker symbol types.
 */
class TestQgsSvgMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsSvgMarkerSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void svgMarkerSymbol();
    void bounds();
    void boundsWidth();
    void bench();
    void anchor();
    void aspectRatio();
    void dynamicSizeWithAspectRatio();
    void dynamicWidthWithAspectRatio();
    void dynamicAspectRatio();
    void resetDefaultAspectRatio();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();
    void dynamicParameters();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsSvgMarkerSymbolLayer *mSvgMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsSvgMarkerSymbol::initTestCase()
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
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  const QString defaultSvgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/crosses/Star1.svg" ), QgsPathResolver() );

  //setup symbol
  mSvgMarkerLayer = new QgsSvgMarkerSymbolLayer( defaultSvgPath );
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mSvgMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
  mReport += QLatin1String( "<h1>SVG Marker Tests</h1>\n" );

}
void TestQgsSvgMarkerSymbol::cleanupTestCase()
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

void TestQgsSvgMarkerSymbol::svgMarkerSymbol()
{
  mReport += QLatin1String( "<h2>SVG marker symbol layer test</h2>\n" );

  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/transport/transport_airport.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setSize( 10 );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  QVERIFY( imageCheck( "svgmarker" ) );
}

void TestQgsSvgMarkerSymbol::bounds()
{
  //use a tall, narrow symbol (non-square to test calculation of height)
  mSvgMarkerLayer->setPath( mTestDataDir + "test_symbol_svg.svg" );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = imageCheck( QStringLiteral( "svgmarker_bounds" ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::boundsWidth()
{
  //use a tall, narrow symbol (non-square to test calculation of height)
  mSvgMarkerLayer->setPath( mTestDataDir + "test_symbol_svg.svg" );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyWidth, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = imageCheck( QStringLiteral( "svgmarker_bounds" ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyWidth, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::bench()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );
  QVERIFY( imageCheck( "svgmarker_bench" ) );
}

void TestQgsSvgMarkerSymbol::anchor()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/backgrounds/background_square.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setSize( 5 );
  mSvgMarkerLayer->setFixedAspectRatio( 6 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );
  mSvgMarkerLayer->setVerticalAnchorPoint( QgsMarkerSymbolLayer::Bottom );
  QVERIFY( imageCheck( "svgmarker_anchor" ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.0 );
  mSvgMarkerLayer->setVerticalAnchorPoint( QgsMarkerSymbolLayer::VCenter );
}

void TestQgsSvgMarkerSymbol::aspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setFixedAspectRatio( 0.5 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );
  QVERIFY( imageCheck( "svgmarker_aspectratio" ) );
}

void TestQgsSvgMarkerSymbol::dynamicSizeWithAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.5 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = imageCheck( QStringLiteral( "svgmarker_dynamicsize_aspectratio" ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicWidthWithAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyWidth, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.2 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = imageCheck( QStringLiteral( "svgmarker_dynamicwidth_aspectratio" ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyWidth, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyHeight, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.5 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = imageCheck( QStringLiteral( "svgmarker_dynamic_aspectratio" ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyHeight, QgsProperty() );
  mSvgMarkerLayer->setFixedAspectRatio( 0 );

  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::resetDefaultAspectRatio()
{
  // default aspect ratio must be updated as SVG path is changed
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );
  QgsSvgMarkerSymbolLayer layer( svgPath );
  QCOMPARE( layer.defaultAspectRatio(), 1.0 );
  QVERIFY( layer.preservedAspectRatio() );

  // different aspect ratio
  layer.setPath( mTestDataDir + "test_symbol_svg.svg" );
  QGSCOMPARENEAR( layer.defaultAspectRatio(), 1.58258242005, 0.0001 );
  QVERIFY( layer.preservedAspectRatio() );
  layer.setPath( svgPath );
  QCOMPARE( layer.defaultAspectRatio(), 1.0 );
  QVERIFY( layer.preservedAspectRatio() );

  layer.setFixedAspectRatio( 0.5 );
  QCOMPARE( layer.defaultAspectRatio(), 1.0 );
  QCOMPARE( layer.fixedAspectRatio(), 0.5 );
  QVERIFY( !layer.preservedAspectRatio() );

  layer.setPath( mTestDataDir + "test_symbol_svg.svg" );
  QGSCOMPARENEAR( layer.defaultAspectRatio(), 1.58258242005, 0.0001 );
  QCOMPARE( layer.fixedAspectRatio(), 0.5 );
  QVERIFY( !layer.preservedAspectRatio() );
}


void TestQgsSvgMarkerSymbol::opacityWithDataDefinedColor()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/transport/transport_airport.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSvgMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSvgMarkerLayer->setSize( 10 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSvgMarkerLayer->setStrokeWidth( 1.0 );
  mMarkerSymbol->setOpacity( 0.5 );

  const bool result = imageCheck( QStringLiteral( "svgmarker_opacityddcolor" ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
  mMarkerSymbol->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dataDefinedOpacity()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/transport/transport_airport.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSvgMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mSvgMarkerLayer->setSize( 10 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSvgMarkerLayer->setStrokeWidth( 1.0 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  const bool result = imageCheck( QStringLiteral( "svgmarker_ddopacity" ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicParameters()
{
  const QString svgPath = TEST_DATA_DIR + QStringLiteral( "/svg/test_dynamic_svg.svg" );

  const QMap<QString, QgsProperty> parameters {{QStringLiteral( "text1" ), QgsProperty::fromExpression( QStringLiteral( "1+1" ) )},
    {QStringLiteral( "text2" ), QgsProperty::fromExpression( QStringLiteral( "\"Class\"" ) )},
    {QStringLiteral( "align" ), QgsProperty::fromExpression( QStringLiteral( "'middle'" ) ) }};

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setParameters( parameters );
  const bool result = imageCheck( QStringLiteral( "svgmarker_dynamic_parameters" ) );
  mSvgMarkerLayer->setParameters( QMap<QString, QgsProperty>() );
  QVERIFY( result );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsSvgMarkerSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsMultiRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_svgmarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsSvgMarkerSymbol )
#include "testqgssvgmarker.moc"
