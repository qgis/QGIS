
/***************************************************************************
                         testqgslayoutscalebar.cpp
                         ---------------------------
    begin                : November 2017
    copyright            : (C) 2017 by Nyall Dawson
    email                : nyall dot dawson at gmail dot com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgsfontutils.h"
#include "qgsproperty.h"
#include "qgsproject.h"
#include "qgspallabeling.h"
#include "qgsbasicnumericformat.h"
#include "qgslinesymbollayer.h"
#include "qgslayoutmanager.h"
#include "qgsprintlayout.h"
#include "qgsfillsymbollayer.h"
#include "qgshollowscalebarrenderer.h"
#include "qgsfillsymbol.h"
#include "qgslinesymbol.h"
#include <QLocale>
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutScaleBar : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutScaleBar()
      : QgsTest( QStringLiteral( "Layout Scalebar Tests" ), QStringLiteral( "layout_scalebar" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void singleBox();
    void singleBoxLineSymbol();
    void singleBoxFillSymbol();
    void singleBoxLabelBelowSegment();
    void singleBoxAlpha();
    void doubleBox();
    void doubleBoxLineSymbol();
    void doubleBoxFillSymbol();
    void doubleBoxLabelCenteredSegment();
    void numeric();
    void tick();
    void tickLineSymbol();
    void dataDefined();
    void oldDataDefinedProject();
    void textFormat();
    void numericFormat();
    void steppedLine();
    void hollow();
    void hollowDefaults();
    void tickSubdivisions();
    void methodTop();
};

void TestQgsLayoutScaleBar::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  // the scale denominator is formatted in a locale aware manner
  // so 10000 is rendered as "10,000" in C (or en_US) locale, however
  // other locales may render the number differently (e.g. "10 000" in cs_CZ)
  // so we enforce C locale to make sure we get expected result
  QLocale::setDefault( QLocale::c() );

  //reproject to WGS84
  const QgsCoordinateReferenceSystem destCRS( QStringLiteral( "EPSG:4326" ) );
  QgsProject::instance()->setCrs( destCRS );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );
}

void TestQgsLayoutScaleBar::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutScaleBar::singleBox()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_singlebox" ), &l );
}

void TestQgsLayoutScaleBar::singleBoxLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_singlebox_linesymbol" ), &l );
}

void TestQgsLayoutScaleBar::singleBoxFillSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr<QgsFillSymbol> fillSymbol = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  fillSymbolLayer->setColor2( QColor( 255, 255, 0 ) );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
  scalebar->setFillSymbol( fillSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol2 = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer2 = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer2->setColor( QColor( 0, 255, 0 ) );
  fillSymbolLayer2->setColor2( QColor( 255, 255, 255 ) );
  fillSymbol2->changeSymbolLayer( 0, fillSymbolLayer2.release() );
  scalebar->setAlternateFillSymbol( fillSymbol2.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_singlebox_fillsymbol" ), &l );
}

void TestQgsLayoutScaleBar::singleBoxLabelBelowSegment()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 18 ) ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setLabelVerticalPlacement( Qgis::ScaleBarDistanceLabelVerticalPlacement::BelowSegment );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_singlebox_labelbelowsegment" ), &l );
}

void TestQgsLayoutScaleBar::singleBoxAlpha()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setColor( QColor( 255, 0, 255 ) );
  format.setOpacity( 100.0 / 255 );
  scalebar->setTextFormat( format );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setFillColor( QColor( 255, 0, 0, 100 ) );
  scalebar->setFillColor2( QColor( 0, 255, 0, 50 ) );
  scalebar->setLineColor( QColor( 0, 0, 255, 150 ) );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_singlebox_alpha" ), &l );
}

void TestQgsLayoutScaleBar::doubleBox()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  format.setColor( Qt::black );
  scalebar->setTextFormat( format );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Double Box" ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_doublebox" ), &l );
}

void TestQgsLayoutScaleBar::doubleBoxLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Double Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_doublebox_linesymbol" ), &l );
}

void TestQgsLayoutScaleBar::doubleBoxFillSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr<QgsFillSymbol> fillSymbol = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  fillSymbolLayer->setColor2( QColor( 255, 255, 0 ) );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
  scalebar->setFillSymbol( fillSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol2 = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer2 = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer2->setColor( QColor( 0, 255, 0 ) );
  fillSymbolLayer2->setColor2( QColor( 255, 255, 255 ) );
  fillSymbol2->changeSymbolLayer( 0, fillSymbolLayer2.release() );
  scalebar->setAlternateFillSymbol( fillSymbol2.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Double Box" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_doublebox_fillsymbol" ), &l );
}

void TestQgsLayoutScaleBar::doubleBoxLabelCenteredSegment()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ), 18 ) );
  format.setColor( Qt::black );
  scalebar->setTextFormat( format );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 3 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Double Box" ) );

  scalebar->setLabelVerticalPlacement( Qgis::ScaleBarDistanceLabelVerticalPlacement::BelowSegment );
  scalebar->setLabelHorizontalPlacement( Qgis::ScaleBarDistanceLabelHorizontalPlacement::CenteredSegment );
  scalebar->setUnitLabel( QStringLiteral( "units" ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_doublebox_labelcenteredsegment" ), &l );
}

void TestQgsLayoutScaleBar::numeric()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  const QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 0 );

  scalebar->setStyle( QStringLiteral( "Numeric" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_numeric" ), &l );
}

void TestQgsLayoutScaleBar::tick()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_tick" ), &l );
}

void TestQgsLayoutScaleBar::tickLineSymbol()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol->clone() );

  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setWidth( 5 );
  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setColor( QColor( 0, 255, 0 ) );
  scalebar->setDivisionLineSymbol( lineSymbol->clone() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_tick_linesymbol" ), &l );
}

void TestQgsLayoutScaleBar::dataDefined()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  const QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 500 );
  scalebar->setNumberOfSegmentsLeft( 4 );
  scalebar->setNumberOfSegments( 6 );
  scalebar->setHeight( 40 );
  scalebar->setMinimumBarWidth( 11 );
  scalebar->setMaximumBarWidth( 13 );

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 1 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 0, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );
  scalebar->setLineSymbol( lineSymbol.release() );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 0 );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  // this is the deprecated way of doing this -- the new way is using data defined properties on the scalebar line symbol.
  // so this test is to ensure old projects/api use works correctly
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor, QgsProperty::fromExpression( QStringLiteral( "'red'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarFillColor2, QgsProperty::fromExpression( QStringLiteral( "'blue'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor, QgsProperty::fromExpression( QStringLiteral( "'yellow'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth, QgsProperty::fromExpression( QStringLiteral( "1.2*3" ) ) );

  // non-deprecated Data Defined Properties (as of QGIS 3.26)
  // The values should override the manually set values set previous so that we can test that they are correctly being applied
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarLeftSegments, QgsProperty::fromExpression( QStringLiteral( "0" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegments, QgsProperty::fromExpression( QStringLiteral( "length('Hi')" ) ) );       // basic expression -> 2
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarSegmentWidth, QgsProperty::fromExpression( QStringLiteral( "1000.0 * 2.0" ) ) );        // basic math expression -> 2
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarMinimumWidth, QgsProperty::fromExpression( QStringLiteral( "to_real('50.0')" ) ) );     // basic conversion expression -> 50
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarMaximumWidth, QgsProperty::fromExpression( QStringLiteral( "to_real('50.0') * 3" ) ) ); // basic conversion with math expression -> 150
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarHeight, QgsProperty::fromExpression( QStringLiteral( "20" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarSubdivisionHeight, QgsProperty::fromExpression( QStringLiteral( "30" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::ScalebarRightSegmentSubdivisions, QgsProperty::fromExpression( QStringLiteral( "40" ) ) );

  scalebar->refreshDataDefinedProperty();

  // test that data defined values were correctly set -- while the render test will confirm some of these, not all of the properties are used in the render
  QCOMPARE( scalebar->numberOfSegments(), 2 );
  QCOMPARE( scalebar->numberOfSegmentsLeft(), 0 );
  QCOMPARE( scalebar->unitsPerSegment(), 2000.0 );
  QCOMPARE( scalebar->minimumBarWidth(), 50.0 );
  QCOMPARE( scalebar->maximumBarWidth(), 150.0 );
  QCOMPARE( scalebar->subdivisionsHeight(), 30.0 );
  QCOMPARE( scalebar->numberOfSubdivisions(), 40 );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_datadefined" ), &l );
}

void TestQgsLayoutScaleBar::oldDataDefinedProject()
{
  QgsProject project;
  // read a project with the older data defined line width and color
  project.read( QStringLiteral( TEST_DATA_DIR ) + "/layouts/scalebar_old_datadefined.qgs" );
  QgsLayout *l = project.layoutManager()->printLayouts().at( 0 );
  QList<QgsLayoutItemScaleBar *> scaleBars;
  l->layoutItems( scaleBars );
  QgsLayoutItemScaleBar *scaleBar = scaleBars.at( 0 );

  // ensure the deprecated scalebar datadefined properties were automatically copied to the scalebar's line symbol
  QgsLineSymbol *ls = scaleBar->lineSymbol();
  QgsSimpleLineSymbolLayer *sll = dynamic_cast<QgsSimpleLineSymbolLayer *>( ls->symbolLayer( 0 ) );

  QVERIFY( sll->dataDefinedProperties().property( QgsSymbolLayer::Property::StrokeWidth ).isActive() );
  QCOMPARE( sll->dataDefinedProperties().property( QgsSymbolLayer::Property::StrokeWidth ).asExpression(), QStringLiteral( "3" ) );
  QVERIFY( sll->dataDefinedProperties().property( QgsSymbolLayer::Property::StrokeColor ).isActive() );
  QCOMPARE( sll->dataDefinedProperties().property( QgsSymbolLayer::Property::StrokeColor ).asExpression(), QStringLiteral( "'red'" ) );

  // deprecated properties should be gone
  QVERIFY( !scaleBar->dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarLineColor ).isActive() );
  QVERIFY( !scaleBar->dataDefinedProperties().property( QgsLayoutObject::DataDefinedProperty::ScalebarLineWidth ).isActive() );
}

void TestQgsLayoutScaleBar::textFormat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 16 );
  format.dataDefinedProperties().setProperty( QgsPalLayerSettings::Property::Color, QgsProperty::fromExpression( QStringLiteral( "case when @scale_value = 2000 then '#ff00ff' else '#ffff00' end" ) ) );
  scalebar->setTextFormat( format );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_textformat" ), &l );
}

void TestQgsLayoutScaleBar::numericFormat()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( true );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowPlusSign( true );
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setNumberDecimalPlaces( 1 );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_numericformat" ), &l );
}

void TestQgsLayoutScaleBar::steppedLine()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );
  scalebar->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "stepped" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_stepped" ), &l );
}

void TestQgsLayoutScaleBar::hollow()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 2 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 20 );
  scalebar->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars

  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  fillSymbolLayer->setColor2( QColor( 255, 255, 0 ) );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
  scalebar->setFillSymbol( fillSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol2 = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer2 = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer2->setColor( QColor( 0, 255, 0 ) );
  fillSymbolLayer2->setColor2( QColor( 255, 255, 255 ) );
  fillSymbol2->changeSymbolLayer( 0, fillSymbolLayer2.release() );
  scalebar->setAlternateFillSymbol( fillSymbol2.release() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "hollow" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_hollow" ), &l );
}

void TestQgsLayoutScaleBar::hollowDefaults()
{
  QgsLayout l( QgsProject::instance() );

  std::unique_ptr<QgsLayoutItemScaleBar> scalebar = std::make_unique<QgsLayoutItemScaleBar>( &l );

  // apply random symbols
  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  fillSymbolLayer->setColor2( QColor( 255, 255, 0 ) );
  fillSymbol->changeSymbolLayer( 0, fillSymbolLayer.release() );
  scalebar->setFillSymbol( fillSymbol.release() );

  std::unique_ptr<QgsFillSymbol> fillSymbol2 = std::make_unique<QgsFillSymbol>();
  std::unique_ptr<QgsGradientFillSymbolLayer> fillSymbolLayer2 = std::make_unique<QgsGradientFillSymbolLayer>();
  fillSymbolLayer2->setColor( QColor( 0, 255, 0 ) );
  fillSymbolLayer2->setColor2( QColor( 255, 255, 255 ) );
  fillSymbol2->changeSymbolLayer( 0, fillSymbolLayer2.release() );
  scalebar->setAlternateFillSymbol( fillSymbol2.release() );

  // reset to renderer defaults
  QgsHollowScaleBarRenderer renderer;
  scalebar->applyDefaultRendererSettings( &renderer );
  // should be reset to "null" fill symbols
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer *>( scalebar->fillSymbol()->symbolLayer( 0 ) )->brushStyle(), Qt::NoBrush );
  QCOMPARE( qgis::down_cast<QgsSimpleFillSymbolLayer *>( scalebar->alternateFillSymbol()->symbolLayer( 0 ) )->brushStyle(), Qt::NoBrush );
  // stroke should be unchanged
  QCOMPARE( qgis::down_cast<QgsSimpleLineSymbolLayer *>( scalebar->lineSymbol()->symbolLayer( 0 ) )->color(), QColor( 255, 0, 0 ) );
}

void TestQgsLayoutScaleBar::tickSubdivisions()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 17.923, 30.160, 18.023, 30.260 ) );

  QgsLayoutItemScaleBar *scalebar = new QgsLayoutItemScaleBar( &l );
  scalebar->attemptSetSceneRect( QRectF( 20, 180, 50, 20 ) );
  l.addLayoutItem( scalebar );
  scalebar->setLinkedMap( map );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar->setUnits( Qgis::DistanceUnit::Meters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setNumberOfSubdivisions( 4 );
  scalebar->setSubdivisionsHeight( 3 );


  std::unique_ptr<QgsLineSymbol> lineSymbol = std::make_unique<QgsLineSymbol>();
  std::unique_ptr<QgsSimpleLineSymbolLayer> lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 4 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 0, 0 ) );
  lineSymbol->changeSymbolLayer( 0, lineSymbolLayer.release() );

  lineSymbolLayer = std::make_unique<QgsSimpleLineSymbolLayer>();
  lineSymbolLayer->setWidth( 2 );
  lineSymbolLayer->setWidthUnit( Qgis::RenderUnit::Millimeters );
  lineSymbolLayer->setColor( QColor( 255, 255, 0 ) );
  lineSymbol->appendSymbolLayer( lineSymbolLayer.release() );

  scalebar->setLineSymbol( lineSymbol->clone() );

  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setWidth( 5 );
  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setColor( QColor( 0, 255, 0 ) );
  scalebar->setDivisionLineSymbol( lineSymbol->clone() );

  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setWidth( 6 );
  qgis::down_cast<QgsLineSymbolLayer *>( lineSymbol->symbolLayer( 0 ) )->setColor( QColor( 0, 0, 255 ) );
  scalebar->setSubdivisionLineSymbol( lineSymbol->clone() );

  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar->numericFormat() ) )->setShowThousandsSeparator( false );

  scalebar->setStyle( QStringLiteral( "Line Ticks Middle" ) );
  QGSVERIFYLAYOUTCHECK( QStringLiteral( "layoutscalebar_tick_subdivisions" ), &l );
}

void TestQgsLayoutScaleBar::methodTop()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  map1->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map1->setFrameEnabled( false );
  map1->setVisibility( false );
  l.addLayoutItem( map1 );
  // only scale at center of map can be calculated
  map1->setExtent( QgsRectangle( -100, -100, 100, 100 ) );
  map1->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsLayoutItemScaleBar *scalebar1 = new QgsLayoutItemScaleBar( &l );
  scalebar1->attemptSetSceneRect( QRectF( 20, 10, 50, 20 ) );
  l.addLayoutItem( scalebar1 );
  scalebar1->setLinkedMap( map1 );
  scalebar1->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar1->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar1->setUnitsPerSegment( 10000 );
  scalebar1->setNumberOfSegmentsLeft( 0 );
  scalebar1->setNumberOfSegments( 2 );
  scalebar1->setHeight( 5 );
  scalebar1->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar1->setMethod( Qgis::ScaleCalculationMethod::HorizontalMiddle );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar1->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar1->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar1->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemScaleBar *scalebar1A = new QgsLayoutItemScaleBar( &l );
  scalebar1A->attemptSetSceneRect( QRectF( 20, 30, 50, 20 ) );
  l.addLayoutItem( scalebar1A );
  scalebar1A->setLinkedMap( map1 );
  scalebar1A->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar1A->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar1A->setUnitsPerSegment( 10000 );
  scalebar1A->setNumberOfSegmentsLeft( 0 );
  scalebar1A->setNumberOfSegments( 2 );
  scalebar1A->setHeight( 5 );
  scalebar1A->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar1A->setMethod( Qgis::ScaleCalculationMethod::HorizontalAverage );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar1A->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar1A->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar1A->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 20, 20, 150, 150 ) );
  map2->setFrameEnabled( false );
  map2->setVisibility( false );
  l.addLayoutItem( map2 );
  // only scale at top of map can be calculated
  map2->setExtent( QgsRectangle( -100, -280, 100, -80 ) );
  map2->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsLayoutItemScaleBar *scalebar2 = new QgsLayoutItemScaleBar( &l );
  scalebar2->attemptSetSceneRect( QRectF( 20, 50, 50, 20 ) );
  l.addLayoutItem( scalebar2 );
  scalebar2->setLinkedMap( map2 );
  scalebar2->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar2->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar2->setUnitsPerSegment( 1000 );
  scalebar2->setNumberOfSegmentsLeft( 0 );
  scalebar2->setNumberOfSegments( 2 );
  scalebar2->setHeight( 5 );
  scalebar2->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar2->setMethod( Qgis::ScaleCalculationMethod::HorizontalTop );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar2->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar2->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar2->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemScaleBar *scalebar2A = new QgsLayoutItemScaleBar( &l );
  scalebar2A->attemptSetSceneRect( QRectF( 20, 70, 50, 20 ) );
  l.addLayoutItem( scalebar2A );
  scalebar2A->setLinkedMap( map2 );
  scalebar2A->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar2A->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar2A->setUnitsPerSegment( 1000 );
  scalebar2A->setNumberOfSegmentsLeft( 0 );
  scalebar2A->setNumberOfSegments( 2 );
  scalebar2A->setHeight( 5 );
  scalebar2A->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar2A->setMethod( Qgis::ScaleCalculationMethod::HorizontalAverage );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar2A->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar2A->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar2A->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemMap *map3 = new QgsLayoutItemMap( &l );
  map3->attemptSetSceneRect( QRectF( 20, 90, 150, 150 ) );
  map3->setFrameEnabled( false );
  map3->setVisibility( false );
  l.addLayoutItem( map3 );
  // only scale at bottom of map can be calculated
  map3->setExtent( QgsRectangle( -100, 80, 100, 280 ) );
  map3->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsLayoutItemScaleBar *scalebar3 = new QgsLayoutItemScaleBar( &l );
  scalebar3->attemptSetSceneRect( QRectF( 20, 90, 50, 20 ) );
  l.addLayoutItem( scalebar3 );
  scalebar3->setLinkedMap( map3 );
  scalebar3->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar3->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar3->setUnitsPerSegment( 1000 );
  scalebar3->setNumberOfSegmentsLeft( 0 );
  scalebar3->setNumberOfSegments( 2 );
  scalebar3->setHeight( 5 );
  scalebar3->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar3->setMethod( Qgis::ScaleCalculationMethod::HorizontalBottom );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar3->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar3->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar3->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemScaleBar *scalebar3A = new QgsLayoutItemScaleBar( &l );
  scalebar3A->attemptSetSceneRect( QRectF( 20, 110, 50, 20 ) );
  l.addLayoutItem( scalebar3A );
  scalebar3A->setLinkedMap( map3 );
  scalebar3A->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar3A->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar3A->setUnitsPerSegment( 1000 );
  scalebar3A->setNumberOfSegmentsLeft( 0 );
  scalebar3A->setNumberOfSegments( 2 );
  scalebar3A->setHeight( 5 );
  scalebar3A->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar3A->setMethod( Qgis::ScaleCalculationMethod::HorizontalAverage );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar3A->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar3A->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar3A->setStyle( QStringLiteral( "Single Box" ) );

  QgsLayoutItemMap *map4 = new QgsLayoutItemMap( &l );
  map4->attemptSetSceneRect( QRectF( 20, 90, 150, 150 ) );
  map4->setFrameEnabled( false );
  map4->setVisibility( false );
  l.addLayoutItem( map4 );
  // scale is valid everywhere
  map4->setExtent( QgsRectangle( -80, -80, 80, 80 ) );
  map4->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );

  QgsLayoutItemScaleBar *scalebar4 = new QgsLayoutItemScaleBar( &l );
  scalebar4->attemptSetSceneRect( QRectF( 20, 130, 50, 20 ) );
  l.addLayoutItem( scalebar4 );
  scalebar4->setLinkedMap( map4 );
  scalebar4->setTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() ) );
  scalebar4->setUnits( Qgis::DistanceUnit::Kilometers );
  scalebar4->setUnitsPerSegment( 5000 );
  scalebar4->setNumberOfSegmentsLeft( 0 );
  scalebar4->setNumberOfSegments( 2 );
  scalebar4->setHeight( 5 );
  scalebar4->setSubdivisionsHeight( 25 ); //ensure subdivisionsHeight is non used in non tick-style scalebars
  scalebar4->setMethod( Qgis::ScaleCalculationMethod::HorizontalAverage );
  Q_NOWARN_DEPRECATED_PUSH
  scalebar4->setLineWidth( 1.0 );
  Q_NOWARN_DEPRECATED_POP
  qgis::down_cast<QgsBasicNumericFormat *>( const_cast<QgsNumericFormat *>( scalebar4->numericFormat() ) )->setShowThousandsSeparator( false );
  scalebar4->setStyle( QStringLiteral( "Single Box" ) );

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "scalebar_method" ), &l );
}


QGSTEST_MAIN( TestQgsLayoutScaleBar )
#include "testqgslayoutscalebar.moc"
