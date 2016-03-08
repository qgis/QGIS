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
#include <qgsfillsymbollayerv2.h>
#include "qgslinesymbollayerv2.h"
#include "qgsdatadefined.h"

//qgis test includes
#include "qgsrenderchecker.h"

/** \ingroup UnitTests
 * This is a unit test for point pattern fill symbol types.
 */
class TestQgsPointPatternFillSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsPointPatternFillSymbol()
        : mTestHasError( false )
        , mpPolysLayer( 0 )
        , mPointPatternFill( 0 )
        , mFillSymbol( 0 )
        , mSymbolRenderer( 0 )
    {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void pointPatternFillSymbol();
    void dataDefinedSubSymbol();

  private:
    bool mTestHasError;

    bool imageCheck( const QString& theType );
    QgsMapSettings mMapSettings;
    QgsVectorLayer * mpPolysLayer;
    QgsPointPatternFillSymbolLayer* mPointPatternFill;
    QgsFillSymbolV2* mFillSymbol;
    QgsSingleSymbolRendererV2* mSymbolRenderer;
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
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(),
                                     myPolyFileInfo.completeBaseName(), "ogr" );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( QgsVectorSimplifyMethod::NoSimplification );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsMapLayerRegistry::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer );

  //setup symbol
  mPointPatternFill = new QgsPointPatternFillSymbolLayer();
  mFillSymbol = new QgsFillSymbolV2();
  mFillSymbol->changeSymbolLayer( 0, mPointPatternFill );
  mSymbolRenderer = new QgsSingleSymbolRendererV2( mFillSymbol );
  mpPolysLayer->setRendererV2( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QStringList() << mpPolysLayer->id() );
  mReport += "<h1>Point Pattern Fill Tests</h1>\n";

}
void TestQgsPointPatternFillSymbol::cleanupTestCase()
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

void TestQgsPointPatternFillSymbol::pointPatternFillSymbol()
{
  mReport += "<h2>Point pattern fill symbol renderer test</h2>\n";

  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "name", "circle" );
  properties.insert( "size", "5.0" );
  QgsMarkerSymbolV2* pointSymbol = QgsMarkerSymbolV2::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "symbol_pointfill" ) );
}

void TestQgsPointPatternFillSymbol::dataDefinedSubSymbol()
{
  mReport += "<h2>Point pattern symbol data defined sub symbol test</h2>\n";

  QgsStringMap properties;
  properties.insert( "color", "0,0,0,255" );
  properties.insert( "name", "circle" );
  properties.insert( "size", "5.0" );
  QgsMarkerSymbolV2* pointSymbol = QgsMarkerSymbolV2::createSimple( properties );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( "color", new QgsDataDefined( QString( "if(\"Name\" ='Lake','#ff0000','#ff00ff')" ) ) );
  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "datadefined_subsymbol" ) );
}

//
// Private helper functions not called directly by CTest
//


bool TestQgsPointPatternFillSymbol::imageCheck( const QString& theTestType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( "symbol_pointpatternfill" );
  myChecker.setControlName( "expected_" + theTestType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( theTestType );
  mReport += myChecker.report();
  return myResultFlag;
}

QTEST_MAIN( TestQgsPointPatternFillSymbol )
#include "testqgspointpatternfillsymbol.moc"
