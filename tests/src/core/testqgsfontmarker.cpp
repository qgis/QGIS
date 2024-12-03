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

/**
 * \ingroup UnitTests
 * This is a unit test for font marker symbol types.
 */
class TestQgsFontMarkerSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsFontMarkerSymbol()
      : QgsTest( QStringLiteral( "Font Marker Tests" ), QStringLiteral( "symbol_fontmarker" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void fontMarkerSymbol();
    void fontMarkerSymbolStyle();
    void fontMarkerSymbolStroke();
    void bounds();
    void fontMarkerSymbolDataDefinedProperties();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();
    void massiveFont();

  private:
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsFontMarkerSymbolLayer *mFontMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
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
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

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
}
void TestQgsFontMarkerSymbol::cleanupTestCase()
{
  delete mpPointsLayer;

  QgsApplication::exitQgis();
}

void TestQgsFontMarkerSymbol::fontMarkerSymbol()
{
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 12 );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "fontmarker", "fontmarker", mMapSettings, 30 );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolStyle()
{
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setFontStyle( QStringLiteral( "Oblique" ) );
  mFontMarkerLayer->setCharacter( QChar( 'A' ) );
  mFontMarkerLayer->setSize( 12 );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "fontmarker_style", "fontmarker_style", mMapSettings, 30 );
}

void TestQgsFontMarkerSymbol::fontMarkerSymbolDataDefinedProperties()
{
  mFontMarkerLayer->setColor( Qt::blue );
  const QFont font = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setFontFamily( font.family() );
  mFontMarkerLayer->setFontStyle( QStringLiteral( "Bold" ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FontStyle, QgsProperty::fromExpression( QStringLiteral( "'Oblique'" ) ) );
  mFontMarkerLayer->setCharacter( QChar( 'Z' ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Character, QgsProperty::fromExpression( QStringLiteral( "'A'" ) ) );
  mFontMarkerLayer->setSize( 12 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "12" ) ) );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "fontmarker_datadefinedproperties", "fontmarker_datadefinedproperties", mMapSettings, 30 );

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
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "fontmarker_outline", "fontmarker_outline", mMapSettings, 30 );
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
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 4.47214, 7.07106)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "fontmarker_bounds", "fontmarker_bounds", mMapSettings, 30 );
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
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFontMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setOpacity( 0.5 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "fontmarker_opacityddcolor", "fontmarker_opacityddcolor", mMapSettings, 30 );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
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
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mFontMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "fontmarker_ddopacity", "fontmarker_ddopacity", mMapSettings, 30 );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty() );
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
  mFontMarkerLayer->setSizeUnit( Qgis::RenderUnit::Millimeters );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 100, 350)" ) ) );
  mFontMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::LayerEnabled, QgsProperty::fromExpression( QStringLiteral( "$id in (1, 4)" ) ) ); // 3
  mFontMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "fontmarker_largesize", "fontmarker_largesize", mMapSettings, 30 );
  mFontMarkerLayer->setDataDefinedProperties( QgsPropertyCollection() );
  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsFontMarkerSymbol )
#include "testqgsfontmarker.moc"
