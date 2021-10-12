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

#include "qgslayoutrendercontext.h"
#include "qgslayoutreportcontext.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgslayout.h"
#include "qgsexpressionutils.h"
#include <QObject>
#include "qgstest.h"
#include <QtTest/QSignalSpy>

class TestQgsLayoutContext: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void creation(); //test creation of QgsLayout
    void flags(); //test QgsLayout flags
    void feature();
    void layer();
    void dpi();
    void renderContextFlags();
    void textFormat();
    void boundingBoxes();
    void geometry();
    void scales();
    void simplifyMethod();

  private:
    QString mReport;

};

void TestQgsLayoutContext::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Context Tests</h1>\n" );
}

void TestQgsLayoutContext::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsLayoutContext::init()
{

}

void TestQgsLayoutContext::cleanup()
{

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
  context.setFlags( QgsLayoutRenderContext::Flags( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects ) );
  // default flags, so should be no signal
  QCOMPARE( spyFlagsChanged.count(), 0 );

  QVERIFY( context.flags() == ( QgsLayoutRenderContext::FlagAntialiasing | QgsLayoutRenderContext::FlagUseAdvancedEffects ) );
  QVERIFY( context.testFlag( QgsLayoutRenderContext::FlagAntialiasing ) );
  QVERIFY( context.testFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects ) );
  QVERIFY( ! context.testFlag( QgsLayoutRenderContext::FlagDebug ) );
  context.setFlag( QgsLayoutRenderContext::FlagDebug );
  QCOMPARE( spyFlagsChanged.count(), 1 );
  QVERIFY( context.testFlag( QgsLayoutRenderContext::FlagDebug ) );
  context.setFlag( QgsLayoutRenderContext::FlagDebug, false );
  QCOMPARE( spyFlagsChanged.count(), 2 );
  QVERIFY( ! context.testFlag( QgsLayoutRenderContext::FlagDebug ) );
  context.setFlag( QgsLayoutRenderContext::FlagDebug, false ); //no change
  QCOMPARE( spyFlagsChanged.count(), 2 );
  context.setFlags( QgsLayoutRenderContext::FlagDebug );
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
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  context.setLayer( layer );
  QCOMPARE( context.layer(), layer );

  //clear layer
  context.setLayer( nullptr );
  QVERIFY( !context.layer() );

  QgsLayout l( QgsProject::instance() );
  l.reportContext().setLayer( layer );
  //test that expression context created for layout contains report context layer scope
  const QgsExpressionContext expContext  = l.createExpressionContext();
  QCOMPARE( QgsExpressionUtils::getVectorLayer( expContext.variable( "layer" ), nullptr ), layer );

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
  context.setFlags( QgsLayoutRenderContext::Flags() );
  Qgis::RenderContextFlags flags = context.renderContextFlags();
  QVERIFY( !( flags & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( flags & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::ForceVectorOutput ) );

  context.setFlag( QgsLayoutRenderContext::FlagAntialiasing );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & Qgis::RenderContextFlag::Antialiasing ) );
  QVERIFY( !( flags & Qgis::RenderContextFlag::UseAdvancedEffects ) );
  QVERIFY( ( flags & Qgis::RenderContextFlag::ForceVectorOutput ) );

  context.setFlag( QgsLayoutRenderContext::FlagUseAdvancedEffects );
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
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).isNull() );

  // no layer set
  QgsFeature f;
  f.setGeometry( QgsGeometry::fromWkt( QStringLiteral( "LineString( 144 -38, 145 -39 )" ) ) );
  context.setFeature( f );
  QCOMPARE( context.currentGeometry().asWkt(), f.geometry().asWkt() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).isNull() );

  //with layer
  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?crs=EPSG:4326&field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  context.setLayer( layer );

  QCOMPARE( context.currentGeometry().asWkt(), f.geometry().asWkt() );
  QVERIFY( !context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).isNull() );
  QCOMPARE( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).asWkt( 0 ), QStringLiteral( "LineString (2412169 2388563, 2500000 2277996)" ) );

  // should be cached
  QCOMPARE( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).asWkt( 0 ), QStringLiteral( "LineString (2412169 2388563, 2500000 2277996)" ) );

  // layer crs
  QCOMPARE( context.currentGeometry( layer->crs() ).asWkt(), f.geometry().asWkt() );

  // clear cache
  const QgsFeature f2;
  context.setFeature( f2 );
  QVERIFY( context.currentGeometry().isNull() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).isNull() );

  delete layer;
}

void TestQgsLayoutContext::scales()
{
  QVector< qreal > scales;
  scales << 1 << 15 << 5 << 10;

  QgsLayoutRenderContext context( nullptr );
  const QSignalSpy spyScalesChanged( &context, &QgsLayoutRenderContext::predefinedScalesChanged );
  context.setPredefinedScales( scales );

  QCOMPARE( spyScalesChanged.count(), 1 );

  // should be sorted
  QCOMPARE( context.predefinedScales(), QVector< qreal >() << 1 << 5 << 10 << 15 );

  context.setPredefinedScales( context.predefinedScales() );
  QCOMPARE( spyScalesChanged.count(), 1 );
}

void TestQgsLayoutContext::simplifyMethod()
{
  QgsLayout l( QgsProject::instance() );
  QgsLayoutRenderContext context( &l );
  // must default to no simplification
  QCOMPARE( context.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::NoSimplification );
  QgsVectorSimplifyMethod simplify;
  simplify.setSimplifyHints( QgsVectorSimplifyMethod::GeometrySimplification );
  context.setSimplifyMethod( simplify );
  QCOMPARE( context.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::GeometrySimplification );
}

QGSTEST_MAIN( TestQgsLayoutContext )
#include "testqgslayoutcontext.moc"
