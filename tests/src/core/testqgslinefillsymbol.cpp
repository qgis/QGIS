/***************************************************************************
     testqgslinefillsymbol.cpp
     -------------------------
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
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgslinesymbollayer.h"
#include "qgsproperty.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for line fill symbol types.
 */
class TestQgsLineFillSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsLineFillSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void lineFillSymbol();
    void lineFillSymbolVector();
    void viewportLineFillSymbol();
    void viewportLineFillSymbolVector();
    void lineFillSymbolOffset();
    void lineFillSymbolOffsetVector();
    void lineFillLargeOffset();
    void lineFillLargeOffsetVector();
    void lineFillNegativeAngle();
    void lineFillNegativeAngleVector();
    void lineFillClipPainter();
    void lineFillClipIntersection();
    void lineFillNoClip();
    void lineFillDataDefinedClip();

    void dataDefinedSubSymbol();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type, QgsVectorLayer *layer = nullptr, bool forceVector = false );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsLinePatternFillSymbolLayer *mLineFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsLineFillSymbol::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  const QString myDataDir( QStringLiteral( TEST_DATA_DIR ) ); //defined in CmakeLists.txt
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

  //setup gradient fill
  mLineFill = new QgsLinePatternFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mLineFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Line Fill Symbol Tests</h1>\n" );

}
void TestQgsLineFillSymbol::cleanupTestCase()
{
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mpPolysLayer;

  QgsApplication::exitQgis();
}

void TestQgsLineFillSymbol::lineFillSymbol()
{
  mReport += QLatin1String( "<h2>Line fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  mLineFill->setSubSymbol( lineSymbol );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill" ) ) );
}

void TestQgsLineFillSymbol::lineFillSymbolVector()
{
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer> ( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  lineFill->setSubSymbol( lineSymbol );

  QVERIFY( imageCheck( QStringLiteral( "symbol_vector_linefill" ), layer.get(), true ) );
}

void TestQgsLineFillSymbol::viewportLineFillSymbol()
{
  mReport += QLatin1String( "<h2>Viewport coordinate reference line fill symbol renderer test</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  layer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineFill->setSubSymbol( lineSymbol );
  lineFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_viewport" ), layer.get() ) );
}

void TestQgsLineFillSymbol::viewportLineFillSymbolVector()
{
  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineFill->setSubSymbol( lineSymbol );
  lineFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_viewport" ), layer.get(), true ) );
}

void TestQgsLineFillSymbol::lineFillSymbolOffset()
{
  mReport += QLatin1String( "<h2>Line fill symbol renderer test</h2>\n" );

  mLineFill->setOffset( 0.5 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_posoffset" ) ) );

  mLineFill->setOffset( -0.5 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_negoffset" ) ) );
  mLineFill->setOffset( 0 );
}

void TestQgsLineFillSymbol::lineFillSymbolOffsetVector()
{
  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineFill->setSubSymbol( lineSymbol );

  lineFill->setOffset( 0.5 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_posoffset" ), layer.get(), true ) );

  lineFill->setOffset( -0.5 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_negoffset" ), layer.get(), true ) );
}

void TestQgsLineFillSymbol::lineFillLargeOffset()
{
  // test line fill with large offset compared to line distance
  mLineFill->setOffset( 8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_large_posoffset" ) ) );

  mLineFill->setOffset( -8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_large_negoffset" ) ) );
  mLineFill->setOffset( 0 );
}

void TestQgsLineFillSymbol::lineFillLargeOffsetVector()
{
  // test line fill with large offset compared to line distance
  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineFill->setSubSymbol( lineSymbol );

  lineFill->setOffset( 8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_large_posoffset" ), layer.get(), true ) );

  lineFill->setOffset( -8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_large_negoffset" ), layer.get(), true ) );
}

void TestQgsLineFillSymbol::lineFillNegativeAngle()
{
  mLineFill->setOffset( -8 );
  mLineFill->setDistance( 2.2 );
  mLineFill->setLineAngle( -130 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_negangle" ) ) );
  mLineFill->setOffset( 0 );
  mLineFill->setLineAngle( 45 );
  mLineFill->setDistance( 5 );
}

void TestQgsLineFillSymbol::lineFillNegativeAngleVector()
{
  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineFill->setSubSymbol( lineSymbol );

  lineFill->setOffset( -8 );
  lineFill->setDistance( 2.2 );
  lineFill->setLineAngle( -130 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_vector_negangle" ), layer.get(), true ) );
}

void TestQgsLineFillSymbol::lineFillClipPainter()
{
  // test clipping using painter path
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer> ( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  properties.clear();
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#ff0000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "3.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol->clone() );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
  lineSymbol->appendSymbolLayer( markerLine );
  markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
  lineSymbol->appendSymbolLayer( markerLine );

  lineFill->setSubSymbol( lineSymbol );
  lineFill->setDistance( 6 );
  lineFill->setClipMode( Qgis::LineClipMode::ClipPainterOnly );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_clip_painter" ), layer.get() ) );
}

void TestQgsLineFillSymbol::lineFillClipIntersection()
{
  // test clipping using intersections
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer> ( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  properties.clear();
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#ff0000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "3.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol->clone() );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
  lineSymbol->appendSymbolLayer( markerLine );
  markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
  lineSymbol->appendSymbolLayer( markerLine );

  lineFill->setSubSymbol( lineSymbol );
  lineFill->setDistance( 6 );
  lineFill->setClipMode( Qgis::LineClipMode::ClipToIntersection );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_clip_intersection" ), layer.get() ) );
}

void TestQgsLineFillSymbol::lineFillNoClip()
{
  // test no clipping
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer> ( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  properties.clear();
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#ff0000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "3.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol->clone() );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
  lineSymbol->appendSymbolLayer( markerLine );
  markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
  lineSymbol->appendSymbolLayer( markerLine );

  lineFill->setSubSymbol( lineSymbol );
  lineFill->setDistance( 6 );
  lineFill->setClipMode( Qgis::LineClipMode::NoClipping );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_clip_no" ), layer.get() ) );
}

void TestQgsLineFillSymbol::lineFillDataDefinedClip()
{
  // test data defined clipping
  std::unique_ptr< QgsVectorLayer > layer = std::make_unique< QgsVectorLayer> ( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsLinePatternFillSymbolLayer *lineFill = new QgsLinePatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, lineFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  properties.clear();
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#ff0000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "3.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  QgsMarkerLineSymbolLayer *markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol->clone() );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::FirstVertex );
  lineSymbol->appendSymbolLayer( markerLine );
  markerLine = new QgsMarkerLineSymbolLayer();
  markerLine->setSubSymbol( pointSymbol );
  markerLine->setPlacements( Qgis::MarkerLinePlacement::LastVertex );
  lineSymbol->appendSymbolLayer( markerLine );

  lineFill->setSubSymbol( lineSymbol );
  lineFill->setDistance( 6 );
  lineFill->setDataDefinedProperty(
    QgsSymbolLayer::PropertyLineClipping,
    QgsProperty::fromExpression( QStringLiteral( "case when $id % 3 =0 then 'no' when $id % 3 = 1 then 'during_render' else 'before_render' end" ) ) );

  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_clip_data_defined" ), layer.get() ) );
}

void TestQgsLineFillSymbol::dataDefinedSubSymbol()
{
  mReport += QLatin1String( "<h2>Line fill symbol data defined sub symbol test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );
  lineSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(\"Name\" ='Lake','#ff0000','#ff00ff')" ) ) );
  mLineFill->setSubSymbol( lineSymbol );
  QVERIFY( imageCheck( QStringLiteral( "datadefined_subsymbol" ) ) );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsLineFillSymbol::imageCheck( const QString &testType, QgsVectorLayer *layer, bool forceVector )
{
  if ( !layer )
    layer = mpPolysLayer;

  mMapSettings.setLayers( {layer } );

  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( layer->extent() );
  mMapSettings.setOutputDpi( 96 );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, forceVector );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_linefill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLineFillSymbol )
#include "testqgslinefillsymbol.moc"
