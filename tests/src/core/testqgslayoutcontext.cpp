/***************************************************************************
                         testqgslayoutcontext.cpp
                         ------------------------
    begin                : November 2014
    copyright            : (C) 2014 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgis.h"
#include "qgsexpressionutils.h"
#include "qgsfeature.h"
#include "qgsgeometry.h"
#include "qgslayout.h"
#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgsproject.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QObject>
#include <QtTest/QSignalSpy>

class TestQgsLayoutContext : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutContext()
      : QgsTest( u"Layout Context Tests"_s ) {}

  private slots:

    void cleanupTestCase();
    void creation(); //test creation of QgsLayout
    void flags();    //test QgsLayout flags
    void feature();
    void layer();
    void dpi();
    void renderContextFlags();
    void textFormat();
    void boundingBoxes();
    void geometry();
    void scales();
    void simplifyMethod();
    void maskRenderSettings();
    void deprecatedFlagsRasterizePolicy();
};

void TestQgsLayoutContext::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutContext::creation()
{
  QgsLayoutRenderContext *context = new QgsLayoutRenderContext( nullptr );
  QVERIFY( context );
  delete context;
}

void TestQgsLayoutContext::flags()
{
  QgsLayoutRenderContext context( nullptr );
  const QSignalSpy spyFlagsChanged( &context, &QgsLayoutRenderContext::flagsChanged );

  //test getting and setting flags
  context.setFlags( Qgis::LayoutRenderFlags( Qgis::LayoutRenderFlag::Antialiasing | Qgis::LayoutRenderFlag::UseAdvancedEffects ) );
  // default flags, so should be no signal
  QCOMPARE( spyFlagsChanged.count(), 0 );

  QVERIFY( context.flags() == ( Qgis::LayoutRenderFlag::Antialiasing | Qgis::LayoutRenderFlag::UseAdvancedEffects ) );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::Antialiasing ) );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) );
  QVERIFY( !context.testFlag( Qgis::LayoutRenderFlag::Debug ) );
  context.setFlag( Qgis::LayoutRenderFlag::Debug );
  QCOMPARE( spyFlagsChanged.count(), 1 );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::Debug ) );
  context.setFlag( Qgis::LayoutRenderFlag::Debug, false );
  QCOMPARE( spyFlagsChanged.count(), 2 );
  QVERIFY( !context.testFlag( Qgis::LayoutRenderFlag::Debug ) );
  context.setFlag( Qgis::LayoutRenderFlag::Debug, false ); //no change
  QCOMPARE( spyFlagsChanged.count(), 2 );
  context.setFlags( Qgis::LayoutRenderFlag::Debug );
  QCOMPARE( spyFlagsChanged.count(), 3 );
}

void TestQgsLayoutContext::feature()
{
  QgsLayoutReportContext context( nullptr );

  //test removing feature
  context.setFeature( QgsFeature() );
  QVERIFY( !context.feature().isValid() );

  //test setting/getting feature
  QgsFeature testFeature;
  testFeature.initAttributes( 1 );
  testFeature.setAttribute( 0, "Test" );
  context.setFeature( testFeature );
  QCOMPARE( context.feature().attribute( 0 ), testFeature.attribute( 0 ) );
}

void TestQgsLayoutContext::layer()
{
  QgsLayoutReportContext context( nullptr );

  //test clearing layer
  context.setLayer( nullptr );
  QVERIFY( !context.layer() );

  //test setting/getting layer
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?field=id_a:integer"_s, u"A"_s, u"memory"_s );
  context.setLayer( layer );
  QCOMPARE( context.layer(), layer );

  //clear layer
  context.setLayer( nullptr );
  QVERIFY( !context.layer() );

  QgsLayout l( QgsProject::instance() );
  l.reportContext().setLayer( layer );
  //test that expression context created for layout contains report context layer scope
  const QgsExpressionContext expContext = l.createExpressionContext();
  Q_NOWARN_DEPRECATED_PUSH
  QCOMPARE( QgsExpressionUtils::getVectorLayer( expContext.variable( "layer" ), &expContext, nullptr ), layer );
  Q_NOWARN_DEPRECATED_POP

  delete layer;
}

void TestQgsLayoutContext::dpi()
{
  QgsLayoutRenderContext context( nullptr );

  const QSignalSpy spyDpiChanged( &context, &QgsLayoutRenderContext::dpiChanged );
  context.setDpi( 600 );
  QCOMPARE( context.dpi(), 600.0 );
  QCOMPARE( context.measurementConverter().dpi(), 600.0 );
  QCOMPARE( spyDpiChanged.count(), 1 );

  context.setDpi( 600 );
  QCOMPARE( spyDpiChanged.count(), 1 );
  context.setDpi( 6000 );
  QCOMPARE( spyDpiChanged.count(), 2 );
}

void TestQgsLayoutContext::renderContextFlags()
{
  QgsLayoutRenderContext context( nullptr );
  context.setFlags( Qgis::LayoutRenderFlags() );
  Qgis::RenderContextFlags flags = context.renderContextFlags();
  QVERIFY( !( flags & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( flags & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::ForceVectorOutput ) );

  context.setFlag( Qgis::LayoutRenderFlag::Antialiasing );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( flags & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::ForceVectorOutput ) );

  context.setFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::ForceVectorOutput ) );
}

void TestQgsLayoutContext::textFormat()
{
  QgsLayoutRenderContext context( nullptr );
  context.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysOutlines );
  QCOMPARE( context.textRenderFormat(), Qgis::TextRenderFormat::AlwaysOutlines );
  context.setTextRenderFormat( Qgis::TextRenderFormat::AlwaysText );
  QCOMPARE( context.textRenderFormat(), Qgis::TextRenderFormat::AlwaysText );
}

void TestQgsLayoutContext::boundingBoxes()
{
  QgsLayoutRenderContext context( nullptr );
  context.setBoundingBoxesVisible( false );
  QVERIFY( !context.boundingBoxesVisible() );
  context.setBoundingBoxesVisible( true );
  QVERIFY( context.boundingBoxesVisible() );
}

void TestQgsLayoutContext::geometry()
{
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutReportContext context( &l );

  // no feature set
  QVERIFY( context.currentGeometry().isNull() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).isNull() );

  // no layer set
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( u"LineString( 144 -38, 145 -39 )"_s ) );
  context.setFeature( f );
  QCOMPARE( context.currentGeometry().asWkt(), f.geometry().asWkt() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).isNull() );

  //with layer
  QgsVectorLayer *layer = new QgsVectorLayer( u"Point?crs=EPSG:4326&field=id_a:integer"_s, u"A"_s, u"memory"_s );
  context.setLayer( layer );

  QCOMPARE( context.currentGeometry().asWkt(), f.geometry().asWkt() );
  QVERIFY( !context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).isNull() );
  QCOMPARE( context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).asWkt( -2 ), u"LineString (2412200 2388600, 2500000 2278000)"_s );

  // should be cached
  QCOMPARE( context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).asWkt( -2 ), u"LineString (2412200 2388600, 2500000 2278000)"_s );

  // layer crs
  QCOMPARE( context.currentGeometry( layer->crs() ).asWkt(), f.geometry().asWkt() );

  // clear cache
  const QgsFeature f2;
  context.setFeature( f2 );
  QVERIFY( context.currentGeometry().isNull() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( u"EPSG:3111"_s ) ).isNull() );

  delete layer;
}

void TestQgsLayoutContext::scales()
{
  QVector<qreal> scales;
  scales << 1 << 15 << 5 << 10;

  QgsLayoutRenderContext context( nullptr );
  const QSignalSpy spyScalesChanged( &context, &QgsLayoutRenderContext::predefinedScalesChanged );
  context.setPredefinedScales( scales );

  QCOMPARE( spyScalesChanged.count(), 1 );

  // should be sorted
  QCOMPARE( context.predefinedScales(), QVector<qreal>() << 1 << 5 << 10 << 15 );

  context.setPredefinedScales( context.predefinedScales() );
  QCOMPARE( spyScalesChanged.count(), 1 );
}

void TestQgsLayoutContext::simplifyMethod()
{
  QgsLayout l( QgsProject::instance() );
  QgsLayoutRenderContext context( &l );
  // must default to no simplification
  QCOMPARE( context.simplifyMethod().simplifyHints(), Qgis::VectorRenderingSimplificationFlag::NoSimplification );
  QgsVectorSimplifyMethod simplify;
  simplify.setSimplifyHints( Qgis::VectorRenderingSimplificationFlag::GeometrySimplification );
  context.setSimplifyMethod( simplify );
  QCOMPARE( context.simplifyMethod().simplifyHints(), Qgis::VectorRenderingSimplificationFlag::GeometrySimplification );
}

void TestQgsLayoutContext::maskRenderSettings()
{
  QgsLayout l( QgsProject::instance() );
  QgsLayoutRenderContext context( &l );
  QCOMPARE( context.maskSettings().simplifyTolerance(), 0 );
  QgsMaskRenderSettings settings2;
  settings2.setSimplificationTolerance( 11 );
  context.setMaskSettings( settings2 );
  QCOMPARE( context.maskSettings().simplifyTolerance(), 11 );
}

void TestQgsLayoutContext::deprecatedFlagsRasterizePolicy()
{
  QgsLayout l( QgsProject::instance() );
  QgsLayoutRenderContext context( &l );

  // test translation of rasterize policies to flags
  context.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::ForceVector );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::ForceVectorOutput ) );
  QVERIFY( !context.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) );

  context.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::PreferVector );
  QVERIFY( !context.testFlag( Qgis::LayoutRenderFlag::ForceVectorOutput ) );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) );

  context.setRasterizedRenderingPolicy( Qgis::RasterizedRenderingPolicy::Default );
  QVERIFY( !context.testFlag( Qgis::LayoutRenderFlag::ForceVectorOutput ) );
  QVERIFY( context.testFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects ) );

  context.setFlag( Qgis::LayoutRenderFlag::ForceVectorOutput, true );
  QCOMPARE( context.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::ForceVector );
  context.setFlag( Qgis::LayoutRenderFlag::ForceVectorOutput, false );
  QCOMPARE( context.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::PreferVector );

  context.setFlag( Qgis::LayoutRenderFlag::ForceVectorOutput, true );
  context.setFlag( Qgis::LayoutRenderFlag::UseAdvancedEffects, false );
  QCOMPARE( context.rasterizedRenderingPolicy(), Qgis::RasterizedRenderingPolicy::ForceVector );
}

QGSTEST_MAIN( TestQgsLayoutContext )
#include "testqgslayoutcontext.moc"
