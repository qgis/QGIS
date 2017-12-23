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

#include "qgslayoutcontext.h"
#include "qgis.h"
#include "qgsfeature.h"
#include "qgsvectorlayer.h"
#include "qgsgeometry.h"
#include "qgsproject.h"
#include "qgslayout.h"
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
    void boundingBoxes();
    void exportLayer();
    void geometry();
    void scales();

  private:
    QString mReport;

};

void TestQgsLayoutContext::initTestCase()
{
  mReport = QStringLiteral( "<h1>Layout Context Tests</h1>\n" );
}

void TestQgsLayoutContext::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
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
  QgsLayoutContext *context = new QgsLayoutContext( nullptr );
  QVERIFY( context );
  delete context;
}

void TestQgsLayoutContext::flags()
{
  QgsLayoutContext context( nullptr );
  QSignalSpy spyFlagsChanged( &context, &QgsLayoutContext::flagsChanged );

  //test getting and setting flags
  context.setFlags( QgsLayoutContext::Flags( QgsLayoutContext::FlagAntialiasing | QgsLayoutContext::FlagUseAdvancedEffects ) );
  // default flags, so should be no signal
  QCOMPARE( spyFlagsChanged.count(), 0 );

  QVERIFY( context.flags() == ( QgsLayoutContext::FlagAntialiasing | QgsLayoutContext::FlagUseAdvancedEffects ) );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagAntialiasing ) );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagUseAdvancedEffects ) );
  QVERIFY( ! context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug );
  QCOMPARE( spyFlagsChanged.count(), 1 );
  QVERIFY( context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug, false );
  QCOMPARE( spyFlagsChanged.count(), 2 );
  QVERIFY( ! context.testFlag( QgsLayoutContext::FlagDebug ) );
  context.setFlag( QgsLayoutContext::FlagDebug, false ); //no change
  QCOMPARE( spyFlagsChanged.count(), 2 );
  context.setFlags( QgsLayoutContext::FlagDebug );
  QCOMPARE( spyFlagsChanged.count(), 3 );
}

void TestQgsLayoutContext::feature()
{
  QgsLayoutContext context( nullptr );

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
  QgsLayoutContext context( nullptr );

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

  delete layer;
}

void TestQgsLayoutContext::dpi()
{
  QgsLayoutContext context( nullptr );

  QSignalSpy spyDpiChanged( &context, &QgsLayoutContext::dpiChanged );
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
  QgsLayoutContext context( nullptr );
  context.setFlags( 0 );
  QgsRenderContext::Flags flags = context.renderContextFlags();
  QVERIFY( !( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );

  context.setFlag( QgsLayoutContext::FlagAntialiasing );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( !( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );

  context.setFlag( QgsLayoutContext::FlagUseAdvancedEffects );
  flags = context.renderContextFlags();
  QVERIFY( ( flags & QgsRenderContext::Antialiasing ) );
  QVERIFY( ( flags & QgsRenderContext::UseAdvancedEffects ) );
  QVERIFY( ( flags & QgsRenderContext::ForceVectorOutput ) );
}

void TestQgsLayoutContext::boundingBoxes()
{
  QgsLayoutContext context( nullptr );
  context.setBoundingBoxesVisible( false );
  QVERIFY( !context.boundingBoxesVisible() );
  context.setBoundingBoxesVisible( true );
  QVERIFY( context.boundingBoxesVisible() );
}

void TestQgsLayoutContext::exportLayer()
{
  QgsLayoutContext context( nullptr );
  // must default to -1
  QCOMPARE( context.currentExportLayer(), -1 );
  context.setCurrentExportLayer( 1 );
  QCOMPARE( context.currentExportLayer(), 1 );
}

void TestQgsLayoutContext::geometry()
{
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutContext context( &l );

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
  QgsFeature f2;
  context.setFeature( f2 );
  QVERIFY( context.currentGeometry().isNull() );
  QVERIFY( context.currentGeometry( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:3111" ) ) ).isNull() );

  delete layer;
}

void TestQgsLayoutContext::scales()
{
  QVector< qreal > scales;
  scales << 1 << 15 << 5 << 10;

  QgsLayoutContext context( nullptr );
  context.setPredefinedScales( scales );

  // should be sorted
  QCOMPARE( context.predefinedScales(), QVector< qreal >() << 1 << 5 << 10 << 15 );
}

QGSTEST_MAIN( TestQgsLayoutContext )
#include "testqgslayoutcontext.moc"
