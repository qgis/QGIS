/***************************************************************************
     testqgsfilledmarker.cpp
     -----------------------
    Date                 : May 2016
    Copyright            : (C) 2016 by Nyall Dawson
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
#include "qgsfillsymbollayer.h"
#include "qgsproperty.h"
#include "qgsfillsymbol.h"
#include "qgsmarkersymbol.h"


/**
 * \ingroup UnitTests
 * This is a unit test for QgsFilledMarkerSymbolLayer.
 */
class TestQgsFilledMarkerSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsFilledMarkerSymbol()
      : QgsTest( QStringLiteral( "Filled Marker Tests" ), QStringLiteral( "symbol_filledmarker" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void filledMarkerSymbol();
    void dataDefinedShape();
    void bounds();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();

  private:
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsFilledMarkerSymbolLayer *mFilledMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsFilledMarkerSymbol::initTestCase()
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
  const QString pointFileName = mTestDataDir + "points.shp";
  const QFileInfo pointFileInfo( pointFileName );
  mpPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(), pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  //setup symbol
  QgsGradientFillSymbolLayer *gradientFill = new QgsGradientFillSymbolLayer();
  gradientFill->setColor( QColor( "red" ) );
  gradientFill->setColor2( QColor( "blue" ) );
  gradientFill->setGradientType( Qgis::GradientType::Linear );
  gradientFill->setGradientColorType( Qgis::GradientColorSource::SimpleTwoColor );
  gradientFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  gradientFill->setGradientSpread( Qgis::GradientSpread::Pad );
  gradientFill->setReferencePoint1( QPointF( 0, 0 ) );
  gradientFill->setReferencePoint2( QPointF( 1, 1 ) );
  QgsFillSymbol *fillSymbol = new QgsFillSymbol();
  fillSymbol->changeSymbolLayer( 0, gradientFill );

  mFilledMarkerLayer = new QgsFilledMarkerSymbolLayer();
  mFilledMarkerLayer->setSubSymbol( fillSymbol );
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mFilledMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
}
void TestQgsFilledMarkerSymbol::cleanupTestCase()
{
  delete mpPointsLayer;

  QgsApplication::exitQgis();
}

void TestQgsFilledMarkerSymbol::filledMarkerSymbol()
{
  mFilledMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mFilledMarkerLayer->setSize( 15 );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "filledmarker", "filledmarker", mMapSettings );
}

void TestQgsFilledMarkerSymbol::dataDefinedShape()
{
  mFilledMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mFilledMarkerLayer->setSize( 10 );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Name, QgsProperty::fromExpression( QStringLiteral( "if(\"class\"='Jet','square','star')" ) ) );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "filledmarker_datadefinedshape", "filledmarker_datadefinedshape", mMapSettings );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Name, QgsProperty() );
  QVERIFY( result );
}

void TestQgsFilledMarkerSymbol::bounds()
{
  mFilledMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mFilledMarkerLayer->setShape( Qgis::MarkerShape::Circle );
  mFilledMarkerLayer->setSize( 5 );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "filledmarker_bounds", "filledmarker_bounds", mMapSettings );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty() );
  QVERIFY( result );
}

void TestQgsFilledMarkerSymbol::opacityWithDataDefinedColor()
{
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setColor( QColor( 200, 200, 200 ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setColor2( QColor( 0, 0, 0 ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::SecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mMarkerSymbol->setOpacity( 0.8 );
  // set opacity on both the symbol AND sub symbol to test that both are applied
  mFilledMarkerLayer->subSymbol()->setOpacity( 0.6 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "filledmarker_opacityddcolor", "filledmarker_opacityddcolor", mMapSettings );

  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mFilledMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
  mMarkerSymbol->setOpacity( 1.0 );
  mFilledMarkerLayer->subSymbol()->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsFilledMarkerSymbol::dataDefinedOpacity()
{
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setColor( QColor( 200, 200, 200 ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setColor2( QColor( 0, 0, 0 ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::SecondaryColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "filledmarker_ddopacity", "filledmarker_ddopacity", mMapSettings );

  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  qgis::down_cast<QgsGradientFillSymbolLayer *>( mFilledMarkerLayer->subSymbol()->symbolLayer( 0 ) )->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty() );
  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsFilledMarkerSymbol )
#include "testqgsfilledmarker.moc"
