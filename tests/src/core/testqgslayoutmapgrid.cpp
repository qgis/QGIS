/***************************************************************************
                         testqgslayoutmapgrid.cpp
                         ----------------------
    begin                : October 2017
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
#include "qgslayoutitemmapgrid.h"
#include "qgsmarkersymbollayer.h"
#include "qgsfontutils.h"
#include "qgsproject.h"
#include <QObject>
#include "qgstest.h"
#include "qgsmarkersymbol.h"

class TestQgsLayoutMapGrid : public QgsTest
{
    Q_OBJECT

  public:

    TestQgsLayoutMapGrid() : QgsTest( QStringLiteral( "Layout Map Grid Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.

    void grid(); //test if grid and grid annotation works
    void reprojected(); //test if reprojected grid works
    void crossGrid(); //test if grid "cross" mode works
    void markerGrid(); //test if grid "marker" mode works
    void frameOnly(); //test if grid "frame/annotation" mode works
    void zebraStyle(); //test zebra map border style
    void zebraStyleSides(); //test zebra border on certain sides
    void zebraStyleMargin(); //test zebra map border style
    void zebraStyleNautical(); //test zebra map border style
    void frameDivisions(); //test filtering frame divisions
    void annotationFilter(); //test filtering annotations
    void interiorTicks(); //test interior tick mode
    void interiorTicksMargin(); //test interior tick mode
    void interiorTicksAnnotated(); //test interior tick mode with annotations
    void exteriorTicks(); //test exterior tick mode
    void exteriorTicksMargin(); //test exterior tick mode
    void exteriorTicksAnnotated(); //test exterior tick mode with annotations
    void interiorExteriorTicks(); //test interior & exterior tick mode
    void interiorExteriorTicksMargin(); //test interior & exterior tick mode
    void interiorExteriorTicksAnnotated(); //test interior & exterior tick mode with annotations
    void lineBorder(); //test line border frame mode
    void lineBorderMargin(); //test line border frame mode
    void lineBorderNautical(); //test line border frame mode
    void lineBorderAnnotated(); //test line border frame with annotations
    void annotationFormats(); //various tests for annotation formats
    void descendingAnnotations(); //test descending annotation direction
};

void TestQgsLayoutMapGrid::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );
}

void TestQgsLayoutMapGrid::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayoutMapGrid::grid()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );

  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 255, 0, 0 ) );
  format.setOpacity( 150.0 / 255.0 );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setEnabled( true );

  map->grid()->setAnnotationEnabled( true );
  map->grid()->setGridLineColor( QColor( 0, 255, 0 ) );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::HideAll, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::HideAll, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::Horizontal, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::Horizontal, QgsLayoutItemMapGrid::Bottom );
  map->grid()->setBlendMode( QPainter::CompositionMode_Overlay );
  map->updateBoundingRect();
  QgsLayoutChecker checker( QStringLiteral( "composermap_grid" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  map->grid()->setEnabled( false );
  map->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::reprojected()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( -243577.565, 2939084.773, 1215622.435, 3668684.773 ) );
  const QgsCoordinateReferenceSystem geographic = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) );
  map->grid()->setCrs( geographic );
  map->grid()->setEnabled( true );
  map->grid()->setIntervalX( 1 );
  map->grid()->setIntervalY( 1 );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->setFrameEnabled( false );
  map->updateBoundingRect();
  QgsLayoutChecker checker( QStringLiteral( "composermap_gridreprojected" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  map->grid()->setEnabled( false );
  map->grid()->setCrs( crs );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->setFrameEnabled( true );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::crossGrid()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::Cross );
  map->grid()->setCrossLength( 2.0 );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setGridLineColor( QColor( 0, 255, 0 ) );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  QgsLayoutChecker checker( QStringLiteral( "composermap_crossgrid" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  map->grid()->setStyle( QgsLayoutItemMapGrid::Solid );
  map->grid()->setEnabled( false );
  map->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::markerGrid()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  static_cast< QgsSimpleMarkerSymbolLayer * >( map->grid()->markerSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::Markers );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  QgsLayoutChecker checker( QStringLiteral( "composermap_markergrid" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  map->grid()->setStyle( QgsLayoutItemMapGrid::Solid );
  map->grid()->setEnabled( false );
  map->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::frameOnly()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( false );
  //set a frame for testing
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
  map->grid()->setFrameWidth( 2.0 );
  map->grid()->setFramePenSize( 0.5 );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  QgsLayoutChecker checker( QStringLiteral( "composermap_gridframeonly" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  map->grid()->setStyle( QgsLayoutItemMapGrid::Solid );
  map->grid()->setEnabled( false );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::zebraStyle()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_zebrastyle" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::zebraStyleSides()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setFrameFillColor1( Qt::black );
  map->grid()->setFrameFillColor2( Qt::white );
  map->grid()->setEnabled( true );

  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft, true );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameRight, false );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameTop, false );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom, false );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_zebrastyle_left" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameTop, true );
  map->updateBoundingRect();
  QgsLayoutChecker checker2( QStringLiteral( "composermap_zebrastyle_lefttop" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameRight, true );
  map->updateBoundingRect();
  QgsLayoutChecker checker3( QStringLiteral( "composermap_zebrastyle_lefttopright" ), &l );
  checker3.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult3 = checker3.testLayout( mReport, 0, 0 );
  QVERIFY( testResult3 );

  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom, true );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );

}

void TestQgsLayoutMapGrid::zebraStyleMargin()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_marginzebrastyle" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::zebraStyleNautical()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::ZebraNautical );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_zebranauticalstyle" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );

  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsLayoutMapGrid::frameDivisions()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  //rotate map, so we mix latitude and longitude coordinates on every map side
  map->setMapRotation( 45.0 );

  //setup defaults
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setFrameFillColor1( Qt::black );
  map->grid()->setFrameFillColor2( Qt::white );
  map->grid()->setEnabled( true );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameLeft, true );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameRight, true );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameTop, true );
  map->grid()->setFrameSideFlag( QgsLayoutItemMapGrid::FrameBottom, true );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_rotatedframe" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::LatitudeOnly, QgsLayoutItemMapGrid::Left );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::LongitudeOnly, QgsLayoutItemMapGrid::Right );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::LatitudeOnly, QgsLayoutItemMapGrid::Top );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::LongitudeOnly, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_framedivisions" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  testResult = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Left );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Right );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Top );
  map->grid()->setFrameDivisions( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Bottom );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->setMapRotation( 0.0 );
}

void TestQgsLayoutMapGrid::annotationFilter()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  QgsTextFormat format = QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) );
  format.setColor( QColor( 0, 0, 0, 0 ) );
  map->grid()->setAnnotationTextFormat( format );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  //rotate map, so we mix latitude and longitude coordinates on every map side
  map->setMapRotation( 45.0 );

  //setup defaults
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_rotatedannotations" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::HideAll, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::LongitudeOnly, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::LatitudeOnly, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::LongitudeOnly, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_filteredannotations" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  testResult = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationEnabled( false );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Bottom );
  map->setMapRotation( 0.0 );
}

void TestQgsLayoutMapGrid::interiorTicks()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_interiorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::interiorTicksMargin()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_margininteriorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::interiorTicksAnnotated()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_interiorticks_annotated" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_interiorticks_annotated2" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setAnnotationEnabled( false );
}

void TestQgsLayoutMapGrid::exteriorTicks()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_exteriorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::exteriorTicksMargin()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_marginexteriorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::exteriorTicksAnnotated()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_exteriorticks_annotated" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_exteriorticks_annotated2" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setAnnotationEnabled( false );
}

void TestQgsLayoutMapGrid::interiorExteriorTicks()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_interiorexteriorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::interiorExteriorTicksMargin()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_margininteriorexteriorticks" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::interiorExteriorTicksAnnotated()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_interiorexteriorticks_annotated" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_interiorexteriorticks_annotated2" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setAnnotationEnabled( false );
}

void TestQgsLayoutMapGrid::lineBorder()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_lineborder" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::lineBorderMargin()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_marginlineborder" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::lineBorderNautical()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::LineBorderNautical );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_linebordernautical" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
}

void TestQgsLayoutMapGrid::lineBorderAnnotated()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_lineborder_annotated" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_lineborder_annotated2" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setAnnotationEnabled( false );
}

void TestQgsLayoutMapGrid::annotationFormats()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  //create grids in geographic and projected coordinates
  const QgsCoordinateReferenceSystem projectedCrs( QStringLiteral( "EPSG:3994" ) );
  const QgsCoordinateReferenceSystem geographicCrs( QStringLiteral( "EPSG:4326" ) );

  QgsLayoutItemMapGrid gridGeographic( QStringLiteral( "geographic grid" ), map );
  gridGeographic.setCrs( geographicCrs );
  QgsLayoutItemMapGrid gridProjected( QStringLiteral( "projected grid" ), map );
  gridProjected.setCrs( projectedCrs );

  //decimal degrees format
  gridGeographic.setAnnotationFormat( QgsLayoutItemMapGrid::DecimalWithSuffix );
  gridGeographic.setAnnotationPrecision( 1 );
  gridProjected.setAnnotationFormat( QgsLayoutItemMapGrid::DecimalWithSuffix );
  gridProjected.setAnnotationPrecision( 1 );

  QgsExpressionContext expressionContext = gridGeographic.createExpressionContext();

  //normal e/w
  QCOMPARE( gridGeographic.gridAnnotationString( 90, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "90.0°E" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 90, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "90.0E" ) );

  //0 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 0, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "0.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "0.0E" ) );

  //180 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 180, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "180.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 180, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "180.0E" ) );

  //normal n/s
  QCOMPARE( gridGeographic.gridAnnotationString( 45, QgsLayoutItemMapGrid::Latitude, expressionContext ), QString( "45.0°N" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 45, QgsLayoutItemMapGrid::Latitude, expressionContext ), QString( "45.0N" ) );

  //0 north/south
  QCOMPARE( gridGeographic.gridAnnotationString( 0, QgsLayoutItemMapGrid::Latitude, expressionContext ), QString( "0.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, QgsLayoutItemMapGrid::Latitude, expressionContext ), QString( "0.0N" ) );

  //Custom format annotations
  gridProjected.setAnnotationFormat( QgsLayoutItemMapGrid::CustomFormat );
  gridProjected.setAnnotationExpression( QStringLiteral( "(@grid_number/10) || case when @grid_axis ='x' then 'a' else 'b' end" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 45, QgsLayoutItemMapGrid::Latitude, expressionContext ), QString( "4.5b" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 35, QgsLayoutItemMapGrid::Longitude, expressionContext ), QString( "3.5a" ) );
}

void TestQgsLayoutMapGrid::descendingAnnotations()
{
  const QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:32633" ) );
  QgsProject::instance()->setCrs( crs );
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setBackgroundColor( QColor( 150, 100, 100 ) );
  map->grid()->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
  map->grid()->setAnnotationPrecision( 0 );
  map->grid()->setIntervalX( 2000 );
  map->grid()->setIntervalY( 2000 );
  map->grid()->setGridLineWidth( 0.5 );
  map->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  map->grid()->setFrameStyle( QgsLayoutItemMapGrid::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( QgsLayoutItemMapGrid::FrameAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::InsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::VerticalDescending, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::VerticalDescending, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::VerticalDescending, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationDirection( QgsLayoutItemMapGrid::VerticalDescending, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker( QStringLiteral( "composermap_verticaldescending_inside" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult = checker.testLayout( mReport, 0, 0 );
  QVERIFY( testResult );

  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Left );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Right );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Top );
  map->grid()->setAnnotationPosition( QgsLayoutItemMapGrid::OutsideMapFrame, QgsLayoutItemMapGrid::Bottom );
  map->updateBoundingRect();

  QgsLayoutChecker checker2( QStringLiteral( "composermap_verticaldescending_outside" ), &l );
  checker2.setControlPathPrefix( QStringLiteral( "composer_mapgrid" ) );
  const bool testResult2 = checker2.testLayout( mReport, 0, 0 );
  QVERIFY( testResult2 );

  map->grid()->setAnnotationEnabled( false );
}
QGSTEST_MAIN( TestQgsLayoutMapGrid )
#include "testqgslayoutmapgrid.moc"
