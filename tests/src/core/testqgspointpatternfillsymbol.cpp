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
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>
#include <QSvgGenerator>
#include <QBuffer>

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
#include "qgslinesymbol.h"
#include "qgsmarkersymbol.h"
#include "qgsfillsymbol.h"

//qgis test includes
#include "qgsrenderchecker.h"
#include "qgsmaprenderercustompainterjob.h"

/**
 * \ingroup UnitTests
 * This is a unit test for point pattern fill symbol types.
 */
class TestQgsPointPatternFillSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsPointPatternFillSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

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
    void pointPatternRandomOffset();
    void pointPatternRandomOffsetPercent();
    void pointPatternRandomOffsetDataDefined();
    void pointPatternAngle();
    void pointPatternAngleDataDefined();
    void pointPatternAngleViewport();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type, QgsVectorLayer *layer = nullptr );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsPointPatternFillSymbolLayer *mPointPatternFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
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
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  //setup symbol
  mPointPatternFill = new QgsPointPatternFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mPointPatternFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Point Pattern Fill Tests</h1>\n" );

}
void TestQgsPointPatternFillSymbol::cleanupTestCase()
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

void TestQgsPointPatternFillSymbol::pointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "symbol_pointfill" ) );
}

void TestQgsPointPatternFillSymbol::pointPatternFillSymbolVector()
{
  mReport += QLatin1String( "<h2>Point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 10 );
  mPointPatternFill->setDistanceY( 10 );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  const bool res = imageCheck( "symbol_pointfill_vector" );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
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

  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  mMapSettings.setOutputSize( QSize( 100, 100 ) );
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );

  properties.insert( QStringLiteral( "color" ), QStringLiteral( "255,0,0,255" ) );
  pointSymbol = QgsMarkerSymbol::createSimple( properties );
  mPointPatternFill->setSubSymbol( pointSymbol );

  QgsMapRendererCustomPainterJob job( mMapSettings, &p );
  job.start();
  job.waitForFinished();
  p.end();
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );

  const QByteArray ba = buffer.data();
  QVERIFY( ba.contains( "fill=\"#ff0000\"" ) );
}

void TestQgsPointPatternFillSymbol::viewportPointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Viewport coordinate reference point pattern fill symbol renderer test</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  layer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  QVERIFY( imageCheck( "symbol_pointfill_viewport", layer.get() ) );
}

void TestQgsPointPatternFillSymbol::viewportPointPatternFillSymbolVector()
{
  mReport += QLatin1String( "<h2>Viewport coordinate reference point pattern fill symbol renderer test</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  layer->setSimplifyMethod( simplifyMethod );

  //setup gradient fill
  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  QVERIFY( imageCheck( "symbol_pointfill_viewport_vector", layer.get() ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
}

void TestQgsPointPatternFillSymbol::offsettedPointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Offsetted point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 15 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );
  QVERIFY( imageCheck( "symbol_pointfill_offset" ) );

  // With offset values greater than the pattern size (i.e. distance * 2 ), offsets values are modulos of offset against distance
  mPointPatternFill->setOffsetX( 19 );
  mPointPatternFill->setOffsetY( 19 );
  QVERIFY( imageCheck( "symbol_pointfill_offset" ) );

  mPointPatternFill->setOffsetX( 0 );
  mPointPatternFill->setOffsetY( 0 );
}

void TestQgsPointPatternFillSymbol::offsettedPointPatternFillSymbolVector()
{
  mReport += QLatin1String( "<h2>Offsetted point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 15 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );
  QVERIFY( imageCheck( "symbol_pointfill_offset" ) );

  // With offset values greater than the pattern size (i.e. distance * 2 ), offsets values are modulos of offset against distance
  mPointPatternFill->setOffsetX( 19 );
  mPointPatternFill->setOffsetY( 19 );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  const bool res = imageCheck( "symbol_pointfill_offset_vector" );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
  mPointPatternFill->setOffsetX( 0 );
  mPointPatternFill->setOffsetY( 0 );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::dataDefinedSubSymbol()
{
  mReport += QLatin1String( "<h2>Point pattern symbol data defined sub symbol test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(\"Name\" ='Lake','#ff0000','#ff00ff')" ) ) );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "if(\"Name\" ='Lake',5,10)" ) ) );
  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "datadefined_subsymbol" ) );
}

void TestQgsPointPatternFillSymbol::zeroSpacedPointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Zero distance point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 0 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );
  QVERIFY( imageCheck( "pointfill_zero_space" ) );
}

void TestQgsPointPatternFillSymbol::zeroSpacedPointPatternFillSymbolVector()
{
  mReport += QLatin1String( "<h2>Zero distance point pattern fill symbol renderer test</h2>\n" );

  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  mPointPatternFill->setDistanceX( 0 );
  mPointPatternFill->setDistanceY( 15 );
  mPointPatternFill->setOffsetX( 4 );
  mPointPatternFill->setOffsetY( 4 );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, true );
  const bool res = imageCheck( "pointfill_zero_space" );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::ForceVectorOutput, false );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternFillNoClip()
{
  mReport += QLatin1String( "<h2>Point pattern no clip mode</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::NoClipping );
  const bool res = imageCheck( "symbol_pointfill_no_clip", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternFillCompletelyWithin()
{
  mReport += QLatin1String( "<h2>Point pattern completely within</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::CompletelyWithin );
  const bool res = imageCheck( "symbol_pointfill_completely_within", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternFillCentroidWithin()
{
  mReport += QLatin1String( "<h2>Point pattern centroid within</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::CentroidWithin );
  const bool res = imageCheck( "symbol_pointfill_centroid_within", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternFillDataDefinedClip()
{
  mReport += QLatin1String( "<h2>Point pattern data defined clip</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setClipMode( Qgis::MarkerClipMode::Shape );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyMarkerClipping, QgsProperty::fromExpression( QStringLiteral( "case when $id % 4 = 0 then 'shape' when $id % 4 = 1 then 'centroid_within' when $id % 4 = 2 then 'completely_within' else 'no' end" ) ) );
  const bool res = imageCheck( "symbol_pointfill_datadefined_clip", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffset()
{
  mReport += QLatin1String( "<h2>Point pattern random offset</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setMaximumRandomDeviationX( 5 );
  pointPatternFill->setMaximumRandomDeviationY( 3 );
  pointPatternFill->setSeed( 1 );

  const bool res = imageCheck( "symbol_pointfill_random_offset", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffsetPercent()
{
  mReport += QLatin1String( "<h2>Point pattern random offset in percent</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->setMaximumRandomDeviationX( 50 );
  pointPatternFill->setMaximumRandomDeviationY( 30 );
  pointPatternFill->setRandomDeviationXUnit( QgsUnitTypes::RenderPercentage );
  pointPatternFill->setRandomDeviationYUnit( QgsUnitTypes::RenderPercentage );
  pointPatternFill->setSeed( 1 );

  const bool res = imageCheck( "symbol_pointfill_percent_random_offset", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternRandomOffsetDataDefined()
{
  mReport += QLatin1String( "<h2>Point pattern data defined random offset</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 10 );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyRandomOffsetX, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then 5 else 10 end" ) ) );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyRandomOffsetY, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then 3 else 6 end" ) ) );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyRandomSeed, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then 1 else 2 end" ) ) );

  const bool res = imageCheck( "symbol_pointfill_data_defined_random_offset", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternAngle()
{
  mReport += QLatin1String( "<h2>Point pattern angle</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setAngle( 25 );

  const bool res = imageCheck( "symbol_pointfill_angle", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternAngleDataDefined()
{
  mReport += QLatin1String( "<h2>Point pattern angle data defined</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setAngle( 25 );
  pointPatternFill->dataDefinedProperties().setProperty( QgsSymbolLayer::PropertyAngle, QgsProperty::fromExpression( QStringLiteral( "case when $id % 2 = 0 then -10 else 25 end" ) ) );

  const bool res = imageCheck( "symbol_pointfill_data_defined_angle", layer.get() );
  QVERIFY( res );
}

void TestQgsPointPatternFillSymbol::pointPatternAngleViewport()
{
  mReport += QLatin1String( "<h2>Point pattern angle viewport mode</h2>\n" );

  std::unique_ptr< QgsVectorLayer> layer = std::make_unique< QgsVectorLayer>( mTestDataDir + "polys.shp" );
  QVERIFY( layer->isValid() );

  QgsPointPatternFillSymbolLayer *pointPatternFill = new QgsPointPatternFillSymbolLayer();
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, pointPatternFill );
  layer->setRenderer( new QgsSingleSymbolRenderer( fillSymbol ) );
  QVariantMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  pointPatternFill->setSubSymbol( pointSymbol );
  pointPatternFill->setDistanceX( 10 );
  pointPatternFill->setDistanceY( 6 );
  pointPatternFill->setCoordinateReference( Qgis::SymbolCoordinateReference::Viewport );
  pointPatternFill->setAngle( 25 );

  const bool res = imageCheck( "symbol_pointfill_viewport_angle", layer.get() );
  QVERIFY( res );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsPointPatternFillSymbol::imageCheck( const QString &testType, QgsVectorLayer *layer )
{
  if ( !layer )
    layer = mpPolysLayer;

  mMapSettings.setLayers( {layer } );

  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( layer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_pointpatternfill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  const bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsPointPatternFillSymbol )
#include "testqgspointpatternfillsymbol.moc"
