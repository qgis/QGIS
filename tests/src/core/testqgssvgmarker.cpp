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
#include "qgsfontutils.h"

/**
 * \ingroup UnitTests
 * This is a unit test for SVG marker symbol types.
 */
class TestQgsSvgMarkerSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsSvgMarkerSymbol()
      : QgsTest( QStringLiteral( "SVG Marker Tests" ), QStringLiteral( "symbol_svgmarker" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

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
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsSvgMarkerSymbolLayer *mSvgMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsSvgMarkerSymbol::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  QgsFontUtils::loadStandardTestFonts( { QStringLiteral( "Roman" ), QStringLiteral( "Bold" ) } );

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer
  );

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
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
}

void TestQgsSvgMarkerSymbol::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsSvgMarkerSymbol::svgMarkerSymbol()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/transport/transport_airport.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setSize( 10 );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "svgmarker", "svgmarker", mMapSettings );
}

void TestQgsSvgMarkerSymbol::bounds()
{
  //use a tall, narrow symbol (non-square to test calculation of height)
  mSvgMarkerLayer->setPath( mTestDataDir + "test_symbol_svg.svg" );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_bounds", "svgmarker_bounds", mMapSettings );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::boundsWidth()
{
  //use a tall, narrow symbol (non-square to test calculation of height)
  mSvgMarkerLayer->setPath( mTestDataDir + "test_symbol_svg.svg" );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::blue );
  mSvgMarkerLayer->setStrokeWidth( 0.5 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Width, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_bounds", "svgmarker_bounds", mMapSettings );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Width, QgsProperty() );
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
  QGSVERIFYRENDERMAPSETTINGSCHECK( "svgmarker_bench", "svgmarker_bench", mMapSettings );
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
  QGSVERIFYRENDERMAPSETTINGSCHECK( "svgmarker_anchor", "svgmarker_anchor", mMapSettings );
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
  QGSVERIFYRENDERMAPSETTINGSCHECK( "svgmarker_aspectratio", "svgmarker_aspectratio", mMapSettings );
}

void TestQgsSvgMarkerSymbol::dynamicSizeWithAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.5 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_dynamicsize_aspectratio", "svgmarker_dynamicsize_aspectratio", mMapSettings );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicWidthWithAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Width, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.2 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_dynamicwidth_aspectratio", "svgmarker_dynamicwidth_aspectratio", mMapSettings );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Width, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicAspectRatio()
{
  const QString svgPath = QgsSymbolLayerUtils::svgSymbolNameToPath( QStringLiteral( "/amenity/amenity_bench.svg" ), QgsPathResolver() );

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setStrokeColor( Qt::black );
  mSvgMarkerLayer->setColor( Qt::black );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Height, QgsProperty::fromExpression( QStringLiteral( "max(\"importance\" * 5, 10)" ) ) );
  mSvgMarkerLayer->setFixedAspectRatio( 0.5 );
  mSvgMarkerLayer->setStrokeWidth( 0.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_dynamic_aspectratio", "svgmarker_dynamic_aspectratio", mMapSettings );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Height, QgsProperty() );
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
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSvgMarkerLayer->setStrokeWidth( 1.0 );
  mMarkerSymbol->setOpacity( 0.5 );

  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_opacityddcolor", "svgmarker_opacityddcolor", mMapSettings );

  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
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
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mSvgMarkerLayer->setStrokeWidth( 1.0 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_ddopacity", "svgmarker_ddopacity", mMapSettings );

  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mSvgMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty() );
  QVERIFY( result );
}

void TestQgsSvgMarkerSymbol::dynamicParameters()
{
  const QString svgPath = TEST_DATA_DIR + QStringLiteral( "/svg/test_dynamic_svg.svg" );

  const QMap<QString, QgsProperty> parameters { { QStringLiteral( "text1" ), QgsProperty::fromExpression( QStringLiteral( "1+1" ) ) }, { QStringLiteral( "text2" ), QgsProperty::fromExpression( QStringLiteral( "\"Class\"" ) ) }, { QStringLiteral( "align" ), QgsProperty::fromExpression( QStringLiteral( "'middle'" ) ) } };

  mSvgMarkerLayer->setPath( svgPath );
  mSvgMarkerLayer->setSize( 20 );
  mSvgMarkerLayer->setParameters( parameters );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "svgmarker_dynamic_parameters", "svgmarker_dynamic_parameters", mMapSettings );

  mSvgMarkerLayer->setParameters( QMap<QString, QgsProperty>() );
  QVERIFY( result );
}


QGSTEST_MAIN( TestQgsSvgMarkerSymbol )
#include "testqgssvgmarker.moc"
