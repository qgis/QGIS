/***************************************************************************
     testqgsrasterfill.cpp
     ---------------------
    Date                 : November 2014
    Copyright            : (C) 2014 Nyall Dawson
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

// qgis includes...
#include <qgsmapsettings.h>
#include <qgsmaplayer.h>
#include <qgsvectorlayer.h>
#include <qgsapplication.h>
#include <qgsproviderregistry.h>
#include <qgsproject.h>
#include <qgssymbol.h>
#include <qgssinglesymbolrenderer.h>
#include <qgsfillsymbollayer.h>
#include "qgsfillsymbol.h"

/**
 * \ingroup UnitTests
 * This is a unit test for raster fill types.
 */
class TestQgsRasterFill : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsRasterFill()
      : QgsTest( QStringLiteral( "Raster Fill Renderer Tests" ), QStringLiteral( "symbol_rasterfill" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init();            // will be called before each testfunction is executed.
    void cleanup();         // will be called after every testfunction.

    void rasterFillSymbol();
    void coordinateMode();
    void alpha();
    void offset();
    void width();
    void widthAndHeight();
    void widthForHeight();
    void percentageHeight();

    // Tests for percentage value of size unit.
    void percentage();
    void percentageCoordinateMode();
    void percentageOffset();
    void percentageAlpha();
    void percentageWidth();

  private:
    QgsMapSettings mMapSettings;
    QgsVectorLayer *mpPolysLayer = nullptr;
    QgsRasterFillSymbolLayer *mRasterFill = nullptr;
    QgsFillSymbol *mFillSymbol = nullptr;
    QgsSingleSymbolRenderer *mSymbolRenderer = nullptr;
};


void TestQgsRasterFill::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
  QgsApplication::showSettings();

  //create some objects that will be used in all tests...
  //
  //create a poly layer that will be used in all tests...
  //
  const QString myPolysFileName = testDataPath( "polys.shp" );
  const QFileInfo myPolyFileInfo( myPolysFileName );
  mpPolysLayer = new QgsVectorLayer( myPolyFileInfo.filePath(), myPolyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QgsVectorSimplifyMethod simplifyMethod;
  simplifyMethod.setSimplifyHints( Qgis::VectorRenderingSimplificationFlags() );
  mpPolysLayer->setSimplifyMethod( simplifyMethod );

  // Register the layer with the registry
  QgsProject::instance()->addMapLayers(
    QList<QgsMapLayer *>() << mpPolysLayer
  );

  //setup raster fill
  mRasterFill = new QgsRasterFillSymbolLayer();
  mFillSymbol = new QgsFillSymbol();
  mFillSymbol->changeSymbolLayer( 0, mRasterFill );
  mSymbolRenderer = new QgsSingleSymbolRenderer( mFillSymbol );
  mpPolysLayer->setRenderer( mSymbolRenderer );

  // We only need maprender instead of mapcanvas
  // since maprender does not require a qui
  // and is more light weight
  //
  mMapSettings.setLayers( QList<QgsMapLayer *>() << mpPolysLayer );
  mMapSettings.setOutputDpi( 96 );
}

void TestQgsRasterFill::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsRasterFill::init()
{
  mRasterFill->setImageFilePath( testDataPath( QStringLiteral( "sample_image.png" ) ) );
  mRasterFill->setWidth( 30.0 );
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );
  mRasterFill->setOpacity( 1.0 );
  mRasterFill->setOffset( QPointF() );
}

void TestQgsRasterFill::cleanup()
{
}

void TestQgsRasterFill::rasterFillSymbol()
{
  mMapSettings.setExtent( mpPolysLayer->extent() );
  QGSVERIFYRENDERMAPSETTINGSCHECK( QStringLiteral( "rasterfill" ), QStringLiteral( "rasterfill" ), mMapSettings, 500, 20 );
}

void TestQgsRasterFill::coordinateMode()
{
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );

  mMapSettings.setExtent( mpPolysLayer->extent() );
  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_viewport" ), QStringLiteral( "rasterfill_viewport" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );

  QVERIFY( result );
}

void TestQgsRasterFill::alpha()
{
  mRasterFill->setOpacity( 0.5 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_alpha" ), QStringLiteral( "rasterfill_alpha" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setOpacity( 1.0 );

  QVERIFY( result );
}

void TestQgsRasterFill::offset()
{
  mRasterFill->setOffset( QPointF( 5, 10 ) );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_offset" ), QStringLiteral( "rasterfill_offset" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setOffset( QPointF() );

  QVERIFY( result );
}

void TestQgsRasterFill::width()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Millimeters );
  mRasterFill->setWidth( 5.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_width" ), QStringLiteral( "rasterfill_width" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );

  QVERIFY( result );
}

void TestQgsRasterFill::widthAndHeight()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Millimeters );
  mRasterFill->setWidth( 5.0 );

  mRasterFill->setHeight( 15.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_width_and_height" ), QStringLiteral( "rasterfill_width_and_height" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setHeight( 0 );

  QVERIFY( result );
}

void TestQgsRasterFill::widthForHeight()
{
  // width should match height respecting aspect ratio
  mRasterFill->setWidth( 0.0 );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Millimeters );
  mRasterFill->setHeight( 15.0 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_height" ), QStringLiteral( "rasterfill_height" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setHeight( 0 );

  QVERIFY( result );
}

void TestQgsRasterFill::percentageHeight()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 5.0 );
  mRasterFill->setHeight( 10 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_height_percentage" ), QStringLiteral( "rasterfill_height_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setHeight( 0 );

  QVERIFY( result );
}

void TestQgsRasterFill::percentage()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 6.3 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_percentage" ), QStringLiteral( "rasterfill_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );

  QVERIFY( result );
}

void TestQgsRasterFill::percentageCoordinateMode()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Viewport );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_viewport_percentage" ), QStringLiteral( "rasterfill_viewport_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setCoordinateMode( Qgis::SymbolCoordinateReference::Feature );

  QVERIFY( result );
}

void TestQgsRasterFill::percentageOffset()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setOffsetUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setOffset( QPointF( 12, 15 ) );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_offset_percentage" ), QStringLiteral( "rasterfill_offset_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setOffsetUnit( Qgis::RenderUnit::Millimeters );
  mRasterFill->setOffset( QPointF() );

  QVERIFY( result );
}

void TestQgsRasterFill::percentageAlpha()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 6.3 );
  mRasterFill->setOpacity( 0.5 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_alpha_percentage" ), QStringLiteral( "rasterfill_alpha_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );
  mRasterFill->setOpacity( 1.0 );

  QVERIFY( result );
}

void TestQgsRasterFill::percentageWidth()
{
  mRasterFill->setSizeUnit( Qgis::RenderUnit::Percentage );
  mRasterFill->setWidth( 3.3 );

  const bool result = QGSRENDERMAPSETTINGSCHECK(
    QStringLiteral( "rasterfill_width_percentage" ), QStringLiteral( "rasterfill_width_percentage" ),
    mMapSettings, 500, 20
  );

  mRasterFill->setSizeUnit( Qgis::RenderUnit::Pixels );
  mRasterFill->setWidth( 0 );

  QVERIFY( result );
}

QGSTEST_MAIN( TestQgsRasterFill )
#include "testqgsrasterfill.moc"
