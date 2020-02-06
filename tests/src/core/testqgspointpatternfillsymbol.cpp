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

//qgis test includes
#include "qgsrenderchecker.h"

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
    void offsettedPointPatternFillSymbol();
    void dataDefinedSubSymbol();
    void zeroSpacedPointPatternFillSymbol();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
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
  QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  QString myPolysFileName = mTestDataDir + "polys.shp";
  QFileInfo myPolyFileInfo( myPolysFileName );
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

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mReport += QLatin1String( "<h1>Point Pattern Fill Tests</h1>\n" );

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
  mReport += QLatin1String( "<h2>Point pattern fill symbol renderer test</h2>\n" );

  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );

  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "symbol_pointfill" ) );
}

void TestQgsPointPatternFillSymbol::offsettedPointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Offsetted point pattern fill symbol renderer test</h2>\n" );

  QgsStringMap properties;
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

void TestQgsPointPatternFillSymbol::dataDefinedSubSymbol()
{
  mReport += QLatin1String( "<h2>Point pattern symbol data defined sub symbol test</h2>\n" );

  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "outline_color" ), QStringLiteral( "#000000" ) );
  properties.insert( QStringLiteral( "name" ), QStringLiteral( "circle" ) );
  properties.insert( QStringLiteral( "size" ), QStringLiteral( "5.0" ) );
  QgsMarkerSymbol *pointSymbol = QgsMarkerSymbol::createSimple( properties );
  pointSymbol->symbolLayer( 0 )->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(\"Name\" ='Lake','#ff0000','#ff00ff')" ) ) );
  mPointPatternFill->setSubSymbol( pointSymbol );
  QVERIFY( imageCheck( "datadefined_subsymbol" ) );
}

void TestQgsPointPatternFillSymbol::zeroSpacedPointPatternFillSymbol()
{
  mReport += QLatin1String( "<h2>Zero distance point pattern fill symbol renderer test</h2>\n" );

  QgsStringMap properties;
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

//
// Private helper functions not called directly by CTest
//


bool TestQgsPointPatternFillSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_pointpatternfill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsPointPatternFillSymbol )
#include "testqgspointpatternfillsymbol.moc"
