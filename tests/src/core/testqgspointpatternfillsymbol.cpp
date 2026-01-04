/***************************************************************************
     testqgspointpatternfillsymbol.cpp
     ---------------------------------
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

#include <QApplication>
#include <QBuffer>
#include <QDesktopServices>
#include <QDir>
#include <QFileInfo>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QSvgGenerator>

//qgis includes...
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgsproperty.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

//qgis test includes
#include "qgsmaprenderercustompainterjob.h"

/**
 * \ingroup UnitTests
 * This is a unit test for point pattern fill symbol types.
 */
class TestQgsPointPatternFillSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsPointPatternFillSymbol()
      : QgsTest( u"Point Pattern Fill Tests"_s, u"symbol_pointpatternfill"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void pointPatternFillSymbol();
    void pointPatternFillSymbolVector();
    void viewportPointPatternFillSymbol();
    void viewportPointPatternFillSymbolVector();
    void offsettedPointPatternFillSymbol();
    void offsettedPointPatternFillSymbolVector();
    void dataDefinedSubSymbol();
    void zeroSpacedPointPatternFillSymbol();
    void zeroSpacedPointPatternFillSymbolVector();
    void pointPatternFillNoClip();
    void pointPatternFillCompletelyWithin();
    void pointPatternFillCentroidWithin();
    void pointPatternFillDataDefinedClip();
    void pointPatternFillDataDefinedWithOpacity();
    void pointPatternRandomOffset();
    void pointPatternRandomOffsetPercent();
    void pointPatternRandomOffsetDataDefined();
    void pointPatternAngle();
    void pointPatternAngleDataDefined();
    void pointPatternAngleViewport();

  private:
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsPointPatternFillSymbolLayer *mPointPatternFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsPointPatternFillSymbol::initTestCase()
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
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), u"ogr"_s );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer
  );

  //setup symbol
  mPointPatternFill = new QgsPointPatternFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mPointPatternFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mMapSettings.setOutputDpi( 96 );
}
void TestQgsPointPatternFillSymbol::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsPointPatternFillSymbol::pointPatternFillSymbol()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill", "symbol_pointfill", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternFillSymbolVector()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 10 );
  mPointPatternFill->setDistanceY( 10 );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  const bool res = QGSRENDERMAPSETTINGSCHECK( "symbol_pointfill_vector", "symbol_pointfill_vector", mMapSettings );

  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  mPointPatternFill->setDistanceX( 15 );
  mPointPatternFill->setDistanceY( 15 );
  QVERIFY( res );

  // also confirm that output is indeed vector!
  QSvgGenerator generator;
  generator.setResolution( mMapSettings.outputDpi() );
  generator.setSize( QSize( 100, 100 ) );
  generator.setViewBox( QRect( 0, 0, 100, 100 ) );
  QBuffer buffer;
  generator.setOutputDevice( &buffer );
  QPainter p;
  p.begin( &generator );

  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  mMapSettings.setOutputSize( QSize( 100, 100 ) );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );

  properties.insert( u"color"_s, u"255,0,0,255"_s );
  pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();
  mPointPatternFill->setSubSymbol( pointSymbol );

  QgsMapRendererCustomPainterJob job( mMapSettings, &p );
  job.start();
  job.waitForFinished();
  p.end();
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );

  const QByteArray ba = buffer.data();
  QVERIFY( ba.contains( "fill=\"#ff0000\"" ) );
}

void TestQgsPointPatternFillSymbol::viewportPointPatternFillSymbol()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  layer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();
  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_viewport", "symbol_pointfill_viewport", mMapSettings );
}

void TestQgsPointPatternFillSymbol::viewportPointPatternFillSymbolVector()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  layer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_viewport_vector", "symbol_pointfill_viewport_vector", mMapSettings );

  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
}

void TestQgsPointPatternFillSymbol::offsettedPointPatternFillSymbol()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 15 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_offset", "symbol_pointfill_offset", mMapSettings );

  // With offset values greater than the pattern size (i.e. distance * 2 ), offsets values are modulos of offset against distance
  mPointPatternFill->setOffsetX( 19 );
  mPointPatternFill->setOffsetY( 19 );
  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_offset", "symbol_pointfill_offset", mMapSettings );

  mPointPatternFill->setOffsetX( 0 );
  mPointPatternFill->setOffsetY( 0 );
}

void TestQgsPointPatternFillSymbol::offsettedPointPatternFillSymbolVector()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 15 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_offset", "symbol_pointfill_offset", mMapSettings );

  // With offset values greater than the pattern size (i.e. distance * 2 ), offsets values are modulos of offset against distance
  mPointPatternFill->setOffsetX( 19 );
  mPointPatternFill->setOffsetY( 19 );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  const bool res = QGSRENDERMAPSETTINGSCHECK( "symbol_pointfill_offset_vector", "symbol_pointfill_offset_vector", mMapSettings );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  mPointPatternFill->setOffsetX( 0 );
  mPointPatternFill->setOffsetY( 0 );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::dataDefinedSubSymbol()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( u"if(\"Name\" ='Lake','#ff0000','#ff00ff')"_s ) );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( u"if(\"Name\" ='Lake',5,10)"_s ) );
  mPointPatternFill->setSubSymbol( pointSymbol );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "datadefined_subsymbol", "datadefined_subsymbol", mMapSettings );
}

void TestQgsPointPatternFillSymbol::zeroSpacedPointPatternFillSymbol()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 0 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "pointfill_zero_space", "pointfill_zero_space", mMapSettings );
}

void TestQgsPointPatternFillSymbol::zeroSpacedPointPatternFillSymbolVector()
{
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 0 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );

  mMapSettings.setLayers( { mpPolysLayer } );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  const bool res = QGSRENDERMAPSETTINGSCHECK( "pointfill_zero_space", "pointfill_zero_space", mMapSettings );
  mMapSettings.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternFillNoClip()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::NoClipping );
  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_no_clip", "symbol_pointfill_no_clip", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternFillCompletelyWithin()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::CompletelyWithin );
  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_completely_within", "symbol_pointfill_completely_within", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternFillCentroidWithin()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::CentroidWithin );
  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_centroid_within", "symbol_pointfill_centroid_within", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternFillDataDefinedClip()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::Shape );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::Property::MarkerClipping, QgsProperty::fromExpression( u"case when $id % 4 = 0 then 'shape' when $id % 4 = 1 then 'centroid_within' when $id % 4 = 2 then 'completely_within' else 'no' end"_s ) );
  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_datadefined_clip", "symbol_pointfill_datadefined_clip", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternFillDataDefinedWithOpacity()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_style"_s, u"no"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( u"if(\"Name\" ='Lake','#ff0000','#ff00ff')"_s ) );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( u"if(\"Name\" ='Lake',5,10)"_s ) );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::Shape );

  fillSymbol->setOpacity( 0.5 );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_datadefined_clip_opacity", "symbol_pointfill_datadefined_clip_opacity", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffset()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setMaximumRandomDeviationX( 5 );
  pointPatternFill->setMaximumRandomDeviationY( 3 );
  pointPatternFill->setSeed( 1 );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_random_offset", "symbol_pointfill_random_offset", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffsetPercent()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setMaximumRandomDeviationX( 50 );
  pointPatternFill->setMaximumRandomDeviationY( 30 );
  pointPatternFill->setRandomDeviationXUnit( Qgis::RenderUnit::Percentage );
  pointPatternFill->setRandomDeviationYUnit( Qgis::RenderUnit::Percentage );
  pointPatternFill->setSeed( 1 );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_percent_random_offset", "symbol_pointfill_percent_random_offset", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffsetDataDefined()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->dataDefinedProperties().setProperty( static_cast<int>( QgsSymbolLayer::Property::RandomOffsetX ), QgsProperty::fromExpression( u"case when $id % 2 = 0 then 5 else 10 end"_s ) );
  pointPatternFill->dataDefinedProperties().setProperty( static_cast<int>( QgsSymbolLayer::Property::RandomOffsetY ), QgsProperty::fromExpression( u"case when $id % 2 = 0 then 3 else 6 end"_s ) );
  pointPatternFill->dataDefinedProperties().setProperty( static_cast<int>( QgsSymbolLayer::Property::RandomSeed ), QgsProperty::fromExpression( u"case when $id % 2 = 0 then 1 else 2 end"_s ) );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_data_defined_random_offset", "symbol_pointfill_data_defined_random_offset", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternAngle()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setAngle( 25 );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_angle", "symbol_pointfill_angle", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternAngleDataDefined()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setAngle( 25 );
  pointPatternFill->dataDefinedProperties().setProperty( static_cast<int>( QgsSymbolLayer::Property::Angle ), QgsProperty::fromExpression( u"case when $id % 2 = 0 then -10 else 25 end"_s ) );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_data_defined_angle", "symbol_pointfill_data_defined_angle", mMapSettings );
}

void TestQgsPointPatternFillSymbol::pointPatternAngleViewport()
{
  auto layer = std::make_unique<QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( u"color"_s, u"0,0,0,255"_s );
  properties.insert( u"outline_color"_s, u"#000000"_s );
  properties.insert( u"name"_s, u"circle"_s );
  properties.insert( u"size"_s, u"5.0"_s );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties ).release();

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  pointPatternFill->setAngle( 25 );

  mMapSettings.setLayers( { layer.get() } );
  mMapSettings.setExtent( layer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "symbol_pointfill_viewport_angle", "symbol_pointfill_viewport_angle", mMapSettings );
}

QGSTEST_MAIN( TestQgsPointPatternFillSymbol )
#include "testqgspointpatternfillsymbol.moc"
