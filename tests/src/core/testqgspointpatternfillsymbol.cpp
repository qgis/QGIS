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
    void offsettedPointPatternFillSymbol();
    void offsettedPointPatternFillSymbolVector();
    void dataDefinedSubSymbol();
    void zeroSpacedPointPatternFillSymbol();
    void zeroSpacedPointPatternFillSymbolVector();

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
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true );
  bool res = imageCheck( "symbol_pointfill_vector" );
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, false );
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

  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true );
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
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, false );

  QByteArray ba = buffer.data();
  QVERIFY( ba.contains( "fill=\"#ff0000\"" ) );
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
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true );
  bool res = imageCheck( "symbol_pointfill_offset_vector" );
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, false );
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
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, true );
  bool res = imageCheck( "pointfill_zero_space" );
  mMapSettings.setFlag( QgsMapSettings::ForceVectorOutput, false );
  QVERIFY( res );
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
