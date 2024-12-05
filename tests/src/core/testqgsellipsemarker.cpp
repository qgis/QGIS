/***************************************************************************
     testqgsellipsemarker.cpp
     ------------------------
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
#include "qgsellipsesymbollayer.h"
#include "qgsproperty.h"
#include "qgsmarkersymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for ellipse marker symbol types.
 */
class TestQgsEllipseMarkerSymbol : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsEllipseMarkerSymbol()
      : QgsTest( QStringLiteral( "Ellipse Marker Tests" ), QStringLiteral( "symbol_ellipsemarker" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void ellipseMarkerSymbol();

    void ellipseMarkerSymbolThirdCircle();
    void ellipseMarkerSymbolQuarterCircle();

    void ellipseMarkerSymbolPentagon();
    void ellipseMarkerSymbolHexagon();
    void ellipseMarkerSymbolOctagon();

    void ellipseMarkerSymbolStar();

    void ellipseMarkerSymbolSize();
    void ellipseMarkerSymbolBevelJoin();
    void ellipseMarkerSymbolMiterJoin();
    void ellipseMarkerSymbolRoundJoin();
    void ellipseMarkerSymbolCapStyle();
    void selected();
    void bounds();
    void opacityWithDataDefinedColor();
    void dataDefinedOpacity();

  private:
    bool mTestHasError = false;

    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPointsLayer = nullptr;
    QgsEllipseSymbolLayer *mEllipseMarkerLayer = nullptr;
    QgsMarkerSymbol *mMarkerSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
    QString mTestDataDir;
};


void TestQgsEllipseMarkerSymbol::initTestCase()
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
  mEllipseMarkerLayer = new QgsEllipseSymbolLayer();
  mMarkerSymbol = new QgsMarkerSymbol();
  mMarkerSymbol->changeSymbolLayer( 0, mEllipseMarkerLayer );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mMarkerSymbol );
  mpPointsLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPointsLayer );
}
void TestQgsEllipseMarkerSymbol::cleanupTestCase()
{
  delete mpPointsLayer;

  QgsApplication::exitQgis();
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbol()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Circle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker", "ellipsemarker", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolThirdCircle()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::ThirdCircle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_thirdcircle", "ellipsemarker_thirdcircle", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolQuarterCircle()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::QuarterCircle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_quartercircle", "ellipsemarker_quartercircle", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolPentagon()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Pentagon );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_pentagon", "ellipsemarker_pentagon", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolHexagon()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Hexagon );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_hexagon", "ellipsemarker_hexagon", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolOctagon()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Octagon );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_octagon", "ellipsemarker_octagon", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolStar()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Star );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setStrokeWidth( 0.8 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_star", "ellipsemarker_star", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolSize()
{
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  // Verify size value derived from width/height (largest value)
  QCOMPARE( mEllipseMarkerLayer->size(), 6.0 );

  mEllipseMarkerLayer->setSize( 2 );
  // Verify width / height values adjusted from setSize
  QCOMPARE( mEllipseMarkerLayer->symbolHeight(), 1.0 );
  QCOMPARE( mEllipseMarkerLayer->symbolWidth(), 2.0 );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolBevelJoin()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Triangle );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::BevelJoin );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_beveljoin", "ellipsemarker_beveljoin", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolMiterJoin()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Triangle );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::MiterJoin );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_miterjoin", "ellipsemarker_miterjoin", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolRoundJoin()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Triangle );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::RoundJoin );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_roundjoin", "ellipsemarker_roundjoin", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::ellipseMarkerSymbolCapStyle()
{
  mEllipseMarkerLayer->setColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Cross );
  mEllipseMarkerLayer->setSymbolWidth( 35 );
  mEllipseMarkerLayer->setSymbolHeight( 15 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenCapStyle( Qt::RoundCap );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "ellipsemarker_roundcap", "ellipsemarker_roundcap", mMapSettings );
}

void TestQgsEllipseMarkerSymbol::selected()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Triangle );
  mEllipseMarkerLayer->setSymbolHeight( 25 );
  mEllipseMarkerLayer->setSymbolWidth( 20 );
  mEllipseMarkerLayer->setStrokeWidth( 3 );
  mEllipseMarkerLayer->setPenJoinStyle( Qt::RoundJoin );

  mpPointsLayer->selectAll();
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool res = QGSRENDERMAPSETTINGSCHECK( "ellipsemarker_selected", "ellipsemarker_selected", mMapSettings );
  mpPointsLayer->removeSelection();
  QVERIFY( res );
}

void TestQgsEllipseMarkerSymbol::bounds()
{
  mEllipseMarkerLayer->setFillColor( Qt::blue );
  mEllipseMarkerLayer->setStrokeColor( Qt::black );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Circle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::Size, QgsProperty::fromExpression( QStringLiteral( "min(\"importance\" * 2, 6)" ) ) );
  mEllipseMarkerLayer->setStrokeWidth( 0.5 );

  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, true );
  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "ellipsemarker_bounds", "ellipsemarker_bounds", mMapSettings );
  mMapSettings.setFlag( Qgis::MapSettingsFlag::DrawSymbolBounds, false );
  QVERIFY( result );
}

void TestQgsEllipseMarkerSymbol::opacityWithDataDefinedColor()
{
  mEllipseMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mEllipseMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Circle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mEllipseMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setOpacity( 0.5 );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "ellipsemarker_opacityddcolor", "ellipsemarker_opacityddcolor", mMapSettings );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
  mMarkerSymbol->setOpacity( 1.0 );
  QVERIFY( result );
}

void TestQgsEllipseMarkerSymbol::dataDefinedOpacity()
{
  mEllipseMarkerLayer->setColor( QColor( 200, 200, 200 ) );
  mEllipseMarkerLayer->setStrokeColor( QColor( 0, 0, 0 ) );
  mEllipseMarkerLayer->setShape( QgsEllipseSymbolLayer::Circle );
  mEllipseMarkerLayer->setSymbolHeight( 3 );
  mEllipseMarkerLayer->setSymbolWidth( 6 );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'red', 'green')" ) ) );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty::fromExpression( QStringLiteral( "if(importance > 2, 'blue', 'magenta')" ) ) );
  mEllipseMarkerLayer->setStrokeWidth( 0.5 );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty::fromExpression( QStringLiteral( "if(\"Heading\" > 100, 25, 50)" ) ) );

  mMapSettings.setExtent( mpPointsLayer->extent() );
  mMapSettings.setOutputDpi( 96 );
  const bool result = QGSRENDERMAPSETTINGSCHECK( "ellipsemarker_ddopacity", "ellipsemarker_ddopacity", mMapSettings );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::FillColor, QgsProperty() );
  mEllipseMarkerLayer->setDataDefinedProperty( QgsSymbolLayer::Property::StrokeColor, QgsProperty() );
  mMarkerSymbol->setDataDefinedProperty( QgsSymbol::Property::Opacity, QgsProperty() );
  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsEllipseMarkerSymbol )
#include "testqgsellipsemarker.moc"
