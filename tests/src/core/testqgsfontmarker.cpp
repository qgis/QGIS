/***************************************************************************
     testqgsfontmarker.cpp
     ---------------------
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
#include "qgsmarkersymbollayer.h"
#include "qgsproperty.h"
#include "qgsfontutils.h"

//qgis test includes
#include "qgsrenderchecker.h"

/**
 * \ingroup UnitTests
 * This is a unit test for font marker symbol types.
 */
class TestQgsFontMarkerSymbol : public QObject
{
    Q_OBJECT

  public:
    TestQgsFontMarkerSymbol() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init() {} // will be called before each testfunction is executed.
    void cleanup() {} // will be called after every testfunction.

    void fontMarkerSymbol();
    void fontMarkerSymbolStroke();
    void bounds();

  private:
    bool mTestHasError =  false ;

    bool imageCheck( const QString &type );
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsFontMarkerSymbolLayer *mFontMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
    QString mReport;
};


void TestQgsFontMarkerSymbol::initTestCase()
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
                                      pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //setup symbol
  mFontMarkerLayer = new QgsFontMarkerSymbolLayer();
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mFontMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
  mReport += QLatin1String( "<h1>Font Marker Tests</h1>\n" );

}
void TestQgsFontMarkerSymbol::cleanupTestCase()
{
  QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  delete mpPointsLayer;

  QgsApplication::exitQgis();
}

void TestQgsFontMarkerSymbol::fontMarkerSymbol()
{
  mReport += QLatin1String( "<h2>Font marker symbol layer test</h2>\n" );

  mFontMarkerLayer->setColor( Qt::blue );
  QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 12 );
  QVERIFY( imageCheck( "fontmarker" ) );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolStroke()
{
  mFontMarkerLayer->setColor( Qt::blue );
  QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 30 );
  mFontMarkerLayer->setStrokeWidth( 3.5 );
  QVERIFY( imageCheck( "fontmarker_outline" ) );
}

void TestQgsFontMarkerSymbol::bounds()
{
  mFontMarkerLayer->setColor( Qt::blue );
  QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  //use a narrow character to test that width is correctly calculated
  mFontMarkerLayer->setCharacter( QChar( 'l' ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setStrokeWidth( 0 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 4.47214, 7.07106)" ) ) );

  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, true );
  bool result = imageCheck( QStringLiteral( "fontmarker_bounds" ) );
  mMapSettings.setFlag( QgsMapSettings::DrawSymbolBounds, false );
  QVERIFY( result );
}


//
// Private helper functions not called directly by CTest
//


bool TestQgsFontMarkerSymbol::imageCheck( const QString &testType )
{
  //use the QgsRenderChecker test utility class to
  //ensure the rendered output matches our control image
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QgsRenderChecker myChecker;
  myChecker.setControlPathPrefix( QStringLiteral( "symbol_fontmarker" ) );
  myChecker.setControlName( "expected_" + testType );
  myChecker.setMapSettings( mMapSettings );
  bool myResultFlag = myChecker.runTest( testType, 30 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsFontMarkerSymbol )
#include "testqgsfontmarker.moc"
