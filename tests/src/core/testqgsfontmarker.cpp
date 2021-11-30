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
#include <QFontDatabase>

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
#include "qgsmarkersymbol.h"

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
    void fontMarkerSymbolStyle();
    void fontMarkerSymbolStroke();
    void bounds();
    void fontMarkerSymbolDataDefinedProperties();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();
    void massiveFont();

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
  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) << QStringLiteral( "Oblique" ) );

  //create some objects that will be used in all tests...
  const QString myDataDir( TEST_DATA_DIR ); //defined in CmakeLists.txt
  mTestDataDir = myDataDir + '/';

  //
  //create a poly layer that will be used in all tests...
  //
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
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
  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
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
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 12 );
  QVERIFY( imageCheck( "fontmarker" ) );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolStyle()
{
  mReport += QLatin1String( "<h2>Font marker symbol style layer test</h2>\n" );

  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setFontStyle( QStringLiteral( "Oblique" ) );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 12 );
  QVERIFY( imageCheck( "fontmarker_style" ) );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolDataDefinedProperties()
{
  mReport += QLatin1String( "<h2>Font marker symbol data defined properties layer test</h2>\n" );
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setFontStyle( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFontStyle, QgsProperty::fromExpression( QStringLiteral( "'Oblique'" ) ) );
  mFontMarkerLayer->setCharacter( QChar( 'Z' ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyCharacter, QgsProperty::fromExpression( QStringLiteral( "'A'" ) ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "12" ) ) );
  QVERIFY( imageCheck( "fontmarker_datadefinedproperties" ) );

  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolStroke()
{
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontStyle( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 30 );
  mFontMarkerLayer->setStrokeWidth( 3.5 );
  QVERIFY( imageCheck( "fontmarker_outline" ) );
}

void TestQgsFontMarkerSymbol::bounds()
{
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  //use a narrow character to test that width is correctly calculated
  mFontMarkerLayer->setCharacter( QChar( 'l' ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setStrokeWidth( 0 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 4.47214, 7.07106)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  const bool result = imageCheck( QStringLiteral( "fontmarker_bounds" ) );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  QVERIFY( result );
}

void TestQgsFontMarkerSymbol::opacityWithDataDefinedColor()
{
  mFontMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mFontMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  mFontMarkerLayer->setCharacter( QChar( 'X' ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFontMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setOpacity( 0.5 );

  const bool result = imageCheck( QStringLiteral( "fontmarker_opacityddcolor" ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty() );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty() );
  mMarkerSymbol->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsFontMarkerSymbol::dataDefinedOpacity()
{
  mFontMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mFontMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  mFontMarkerLayer->setCharacter( QChar( 'X' ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyFillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyStrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFontMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  const bool result = imageCheck( QStringLiteral( "fontmarker_ddopacity" ) );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::PropertyOpacity, QgsProperty() );
  QVERIFY( result );
}

void TestQgsFontMarkerSymbol::massiveFont()
{
  // test rendering a massive font
  mFontMarkerLayer->setColor( QColor( 0, 0, 0, 100 ) );
  mFontMarkerLayer->setStrokeColor( QColor( 0, 0, 0, 0 ) );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  mFontMarkerLayer->setCharacter( QChar( 'X' ) );
  mFontMarkerLayer->setSize( 200 );
  mFontMarkerLayer->setSizeUnit( QgsUnitTypes::RenderMillimeters );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertySize, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 100, 350)" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::PropertyLayerEnabled, QgsProperty::fromExpression( QStringLiteral( "$id in (1, 4)" ) ) ); // 3
  mFontMarkerLayer->setStrokeWidth( 0.5 );

  const bool result = imageCheck( QStringLiteral( "fontmarker_largesize" ) );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
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
  const bool myResultFlag = myChecker.runTest( testType, 30 );
  mReport += myChecker.report();
  return myResultFlag;
}

QGSTEST_MAIN( TestQgsFontMarkerSymbol )
#include "testqgsfontmarker.moc"
