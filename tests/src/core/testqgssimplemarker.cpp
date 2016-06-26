/***************************************************************************
     testqgssimplemarker.cpp
     -----------------------
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
#include <QtTest/QtTest>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QApplication>
#include <QFileInfo>
#include <QDir>
#include <QDesktopServices>

//qgis includes...
#include <qgsmaprenderer.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsmaplayerregistry.h>
#include <qgssymbolv2.h>
#include <qgssinglesymbolrendererv2.h>
#include "qgsmarkersymbollayerv2.h"
#include "qgsdatadefined.h"

//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for simple marker symbol types.
 */
class TestQgsSimpleMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsSimpleMarkerSymbol()
        : mTestHasError( false )
        , mpPointsLayer( 0 )
        , mSimpleMarkerLayer( 0 )
        , mMarkerSymbol( 0 )
        , mSymbolRenderer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void simpleMarkerSymbol();
    void simpleMarkerSymbolBevelJoin();
    void simpleMarkerSymbolMiterJoin();
    void simpleMarkerSymbolRoundJoin();
    void bounds();
    void boundsWithOffset();
    void boundsWithRotation();
    void boundsWithRotationAndOffset();
    void colors();

  private:
    bool mTestHasError;

    bool imageCheck( const QString& theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPointsLayer;
    QgsSimpleMarkerSymbolLayerV2* mSimpleMarkerLayer;
    QgsMarkerSymbolV2* mMarkerSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsSimpleMarkerSymbol::initTestCase()
{
  mTestHasError = false;
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  QString pointFileName = mTestDataDir + "points.shp";
  QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                      pointFileInfo.completeBaseName(), "ogr" );

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPointsLayer );

  //setup symbol
  mSimpleMarkerLayer = new QgsSimpleMarkerSymbolLayerV2();
  mMarkerSymbol = new QgsMarkerSymbolV2();
  mMarkerSymbol->changeSymbolLayer( 0, mSimpleMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mMarkerSymbol );
  mpPointsLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPointsLayer->id() );
  mReport += "<h1>Simple Marker Tests</h1>\n";

}
void TestQgsSimpleMarkerSymbol::cleanupTestCase()
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

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbol()
{
  mReport += "<h2>Simple marker symbol layer test</h2>\n";

  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setBorderColor( Qt::black );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setOutlineWidth( 1 );
  QVERIFY( imageCheck( "simplemarker" ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolBevelJoin()
{
  mReport += "<h2>Simple marker symbol layer test</h2>\n";

  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setBorderColor( Qt::black );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setOutlineWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::BevelJoin );
  QVERIFY( imageCheck( "simplemarker_beveljoin" ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolMiterJoin()
{
  mReport += "<h2>Simple marker symbol layer test</h2>\n";

  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setBorderColor( Qt::black );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setOutlineWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::MiterJoin );
  QVERIFY( imageCheck( "simplemarker_miterjoin" ) );
}

void TestQgsSimpleMarkerSymbol::simpleMarkerSymbolRoundJoin()
{
  mReport += "<h2>Simple marker symbol layer test</h2>\n";

  mSimpleMarkerLayer->setColor( Qt::blue );
  mSimpleMarkerLayer->setBorderColor( Qt::black );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Triangle );
  mSimpleMarkerLayer->setSize( 25 );
  mSimpleMarkerLayer->setOutlineWidth( 3 );
  mSimpleMarkerLayer->setPenJoinStyle( Qt::RoundJoin );
  QVERIFY( imageCheck( "simplemarker_roundjoin" ) );
}

void TestQgsSimpleMarkerSymbol::bounds()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setBorderColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( "size", new QgsDataDefined( true, true, "min(\"importance\" * 2, 6)" ) );
  mSimpleMarkerLayer->setOutlineWidth( 0.5 );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( "simplemarker_bounds" );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mSimpleMarkerLayer->removeDataDefinedProperty( "size" );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithOffset()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setBorderColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( "offset", new QgsDataDefined( true, true, "if(importance > 2, '5,10', '10, 5')" ) );
  mSimpleMarkerLayer->setOutlineWidth( 0.5 );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( "simplemarker_boundsoffset" );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mSimpleMarkerLayer->removeDataDefinedProperty( "offset" );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithRotation()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setBorderColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( "angle", new QgsDataDefined( true, true, "importance * 20" ) );
  mSimpleMarkerLayer->setOutlineWidth( 0.5 );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( "simplemarker_boundsrotation" );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mSimpleMarkerLayer->removeDataDefinedProperty( "angle" );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::boundsWithRotationAndOffset()
{
  mSimpleMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mSimpleMarkerLayer->setBorderColor( QColor( 0, 0, 0 ) );
  mSimpleMarkerLayer->setShape( QgsSimpleMarkerSymbolLayerBase::Square );
  mSimpleMarkerLayer->setSize( 5 );
  mSimpleMarkerLayer->setDataDefinedProperty( "offset", new QgsDataDefined( true, true, "if(importance > 2, '5,10', '10, 5')" ) );
  mSimpleMarkerLayer->setDataDefinedProperty( "angle", new QgsDataDefined( true, false, QString(), "heading" ) );
  mSimpleMarkerLayer->setOutlineWidth( 0.5 );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( "simplemarker_boundsrotationoffset" );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  mSimpleMarkerLayer->removeDataDefinedProperty( "offset" );
  mSimpleMarkerLayer->removeDataDefinedProperty( "angle" );
  QVERIFY( result );
}

void TestQgsSimpleMarkerSymbol::colors()
{
  //test logic for setting/retrieving symbol color

  QgsSimpleMarkerSymbolLayerV2 marker;
  marker.setOutlineColor( QColor( 200, 200, 200 ) );
  marker.setFillColor( QColor( 100, 100, 100 ) );

  //start with a filled shape - color should be fill color
  marker.setShape( QgsSimpleMarkerSymbolLayerBase::Circle );
  QCOMPARE( marker.color(), QColor( 100, 100, 100 ) );
  marker.setColor( QColor( 150, 150, 150 ) );
  QCOMPARE( marker.fillColor(), QColor( 150, 150, 150 ) );

  //now try with a non-filled (outline only) shape - color should be outline color
  marker.setShape( QgsSimpleMarkerSymbolLayerBase::Cross );
  QCOMPARE( marker.color(), QColor( 200, 200, 200 ) );
  marker.setColor( QColor( 250, 250, 250 ) );
  QCOMPARE( marker.outlineColor(), QColor( 250, 250, 250 ) );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsSimpleMarkerSymbol::imageCheck( const QString& theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( "symbol_simplemarker" );
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( theTestType );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsSimpleMarkerSymbol )
#include "testqgssimplemarker.moc"
