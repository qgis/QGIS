
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
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemscalebar.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsfontutils.h"
#include "qgsproperty.h"
#include "qgsproject.h"
#include "qgspallabeling.h"

#include <QLocale>
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutScaleBar : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutScaleBar() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void singleBox();
    void singleBoxLabelBelowSegment();
    void singleBoxAlpha();
    void doubleBox();
    void doubleBoxLabelCenteredSegment();
    void numeric();
    void tick();
    void dataDefined();
    void textFormat();

  private:
    QString mReport;
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
  QgsCoordinateReferenceSystem destCRS;
  destCRS.createFromId( 4326, QgsCoordinateReferenceSystem::EpsgCrsId );
  QgsProject::instance()->setCrs( destCRS );
  QgsProject::instance()->setEllipsoid( QStringLiteral( "WGS84" ) );

  mReport = QStringLiteral( "<h1>Layout Scalebar Tests</h1>\n" );
}

void TestQgsLayoutScaleBar::cleanupTestCase()
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

void TestQgsLayoutScaleBar::init()
{

}

void TestQgsLayoutScaleBar::cleanup()
{

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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );
  scalebar->setLabelVerticalPlacement( QgsScaleBarSettings::LabelBelowSegment );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox_labelbelowsegment" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  scalebar->setStyle( QStringLiteral( "Single Box" ) );
  scalebar->setFillColor( QColor( 255, 0, 0, 100 ) );
  scalebar->setFillColor2( QColor( 0, 255, 0, 50 ) );
  scalebar->setLineColor( QColor( 0, 0, 255, 150 ) );
  scalebar->setLineWidth( 1.0 );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_singlebox_alpha" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  scalebar->setStyle( QStringLiteral( "Double Box" ) );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_doublebox" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 3 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  scalebar->setFillColor( Qt::black );
  scalebar->setFillColor2( Qt::white );
  scalebar->setLineColor( Qt::black );
  scalebar->setLineWidth( 1.0 );
  scalebar->setStyle( QStringLiteral( "Double Box" ) );

  scalebar->setLabelVerticalPlacement( QgsScaleBarSettings::LabelBelowSegment );
  scalebar->setLabelHorizontalPlacement( QgsScaleBarSettings::LabelCenteredSegment );
  scalebar->setUnitLabel( QStringLiteral( "units" ) );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_doublebox_labelcenteredsegment" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  scalebar->setStyle( QStringLiteral( "Numeric" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_numeric" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  bool result = checker.testLayout( mReport, 0, 0 );
  QVERIFY( result );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );

  scalebar->setStyle( QStringLiteral( "Line Ticks Up" ) );
  scalebar->setLineWidth( 1.0 );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_tick" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
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
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont() );
  scalebar->setTextFormat( format );
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );
  scalebar->setStyle( QStringLiteral( "Single Box" ) );

  QFont newFont = QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) );
  newFont.setPointSizeF( 12 );
  scalebar->setTextFormat( QgsTextFormat::fromQFont( newFont ) );

  scalebar->setStyle( QStringLiteral( "Numeric" ) );
  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_numeric" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  bool result = checker.testLayout( mReport, 0, 0 );
  QVERIFY( result );

  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor, QgsProperty::fromExpression( QStringLiteral( "'red'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarFillColor2, QgsProperty::fromExpression( QStringLiteral( "'blue'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineColor, QgsProperty::fromExpression( QStringLiteral( "'yellow'" ) ) );
  scalebar->dataDefinedProperties().setProperty( QgsLayoutObject::ScalebarLineWidth, QgsProperty::fromExpression( QStringLiteral( "1.2" ) ) );
  scalebar->refreshDataDefinedProperty();
  QCOMPARE( scalebar->brush().color().name(), QColor( 255, 0, 0 ).name() );
  QCOMPARE( scalebar->brush2().color().name(), QColor( 0, 0, 255 ).name() );
  QCOMPARE( scalebar->pen().color().name(), QColor( 255, 255, 0 ).name() );
  QCOMPARE( scalebar->pen().widthF(), 1.2 );
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
  scalebar->setUnits( QgsUnitTypes::DistanceMeters );
  scalebar->setUnitsPerSegment( 2000 );
  scalebar->setNumberOfSegmentsLeft( 0 );
  scalebar->setNumberOfSegments( 2 );
  scalebar->setHeight( 5 );
  scalebar->setLineWidth( 1.0 );
  scalebar->setStyle( QStringLiteral( "Single Box" ) );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setSize( 16 );
  format.dataDefinedProperties().setProperty( QgsPalLayerSettings::Color, QgsProperty::fromExpression( QStringLiteral( "case when @scale_value = 2000 then '#ff00ff' else '#ffff00' end" ) ) );
  scalebar->setTextFormat( format );

  QgsLayoutChecker checker( QStringLiteral( "layoutscalebar_textformat" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "layout_scalebar" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}


QGSTEST_MAIN( TestQgsLayoutScaleBar )
#include "testqgslayoutscalebar.moc"
