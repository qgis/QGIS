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
    void lineFillSymbolOffset();
    void lineFillLargeOffset();
    void lineFillNegativeAngle();

    void dataDefinedSubSymbol();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
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
  QString myDataDir( QStringLiteral( TEST_DATA_DIR ) ); //defined in CmakeLists.txt
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
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
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

  QgsStringMap properties;
  properties.insert( QStringLiteral( "color" ), QStringLiteral( "0,0,0,255" ) );
  properties.insert( QStringLiteral( "width" ), QStringLiteral( "1" ) );
  properties.insert( QStringLiteral( "capstyle" ), QStringLiteral( "flat" ) );
  QgsLineSymbol *lineSymbol = QgsLineSymbol::createSimple( properties );

  mLineFill->setSubSymbol( lineSymbol );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill" ) ) );
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

void TestQgsLineFillSymbol::lineFillLargeOffset()
{
  // test line fill with large offset compared to line distance
  mLineFill->setOffset( 8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_large_posoffset" ) ) );

  mLineFill->setOffset( -8 );
  QVERIFY( imageCheck( QStringLiteral( "symbol_linefill_large_negoffset" ) ) );
  mLineFill->setOffset( 0 );
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

void TestQgsLineFillSymbol::dataDefinedSubSymbol()
{
  mReport += QLatin1String( "<h2>Line fill symbol data defined sub symbol test</h2>\n" );

  QgsStringMap properties;
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


bool TestQgsLineFillSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPolysLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_linefill" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( testType );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsLineFillSymbol )
#include "testqgslinefillsymbol.moc"
