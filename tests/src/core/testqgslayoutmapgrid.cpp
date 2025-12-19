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
#include "qgsfontutils.h"
#include "qgslayout.h"
#include "qgslayoutitemmap.h"
#include "qgslayoutitemmapgrid.h"
#include "qgsmarkersymbol.h"
#include "qgsmarkersymbollayer.h"
#include "qgsproject.h"
#include "qgstest.h"

#include <QObject>

class TestQgsLayoutMapGrid : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutMapGrid()
      : QgsTest( QStringLiteral( "Layout Map Grid Tests" ), QStringLiteral( "composer_mapgrid" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void grid();                           //test if grid and grid annotation works
    void reprojected();                    //test if reprojected grid works
    void crossGrid();                      //test if grid "cross" mode works
    void markerGrid();                     //test if grid "marker" mode works
    void frameOnly();                      //test if grid "frame/annotation" mode works
    void zebraStyle();                     //test zebra map border style
    void zebraStyleSides();                //test zebra border on certain sides
    void zebraStyleMargin();               //test zebra map border style
    void zebraStyleNautical();             //test zebra map border style
    void frameDivisions();                 //test filtering frame divisions
    void annotationFilter();               //test filtering annotations
    void interiorTicks();                  //test interior tick mode
    void interiorTicksMargin();            //test interior tick mode
    void interiorTicksAnnotated();         //test interior tick mode with annotations
    void exteriorTicks();                  //test exterior tick mode
    void exteriorTicksMargin();            //test exterior tick mode
    void exteriorTicksAnnotated();         //test exterior tick mode with annotations
    void interiorExteriorTicks();          //test interior & exterior tick mode
    void interiorExteriorTicksMargin();    //test interior & exterior tick mode
    void interiorExteriorTicksAnnotated(); //test interior & exterior tick mode with annotations
    void lineBorder();                     //test line border frame mode
    void lineBorderMargin();               //test line border frame mode
    void lineBorderNautical();             //test line border frame mode
    void lineBorderAnnotated();            //test line border frame with annotations
    void annotationFormats();              //various tests for annotation formats
    void descendingAnnotations();          //test descending annotation direction
    void dataDefinedDrawAnnotation();
    void dataDefinedDrawAnnotationCountAndIndex();
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
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::HideAll, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::HideAll, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::Horizontal, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::Horizontal, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setBlendMode( QPainter::CompositionMode_Overlay );
  map->updateBoundingRect();
  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_grid" ), &l );
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
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->setFrameEnabled( false );
  map->updateBoundingRect();
  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_gridreprojected" ), &l );

  map->grid()->setEnabled( false );
  map->grid()->setCrs( crs );
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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
  map->grid()->setStyle( Qgis::MapGridStyle::LineCrosses );
  map->grid()->setCrossLength( 2.0 );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setGridLineColor( QColor( 0, 255, 0 ) );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_crossgrid" ), &l );
  map->grid()->setStyle( Qgis::MapGridStyle::Lines );
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
  static_cast<QgsSimpleMarkerSymbolLayer *>( map->grid()->markerSymbol()->symbolLayer( 0 ) )->setStrokeColor( Qt::black );
  map->updateBoundingRect();
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::Markers );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_markergrid" ), &l );
  map->grid()->setStyle( Qgis::MapGridStyle::Lines );
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
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( false );
  //set a frame for testing
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::Zebra );
  map->grid()->setFrameWidth( 2.0 );
  map->grid()->setFramePenSize( 0.5 );
  map->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  map->updateBoundingRect();
  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_gridframeonly" ), &l );
  map->grid()->setStyle( Qgis::MapGridStyle::Lines );
  map->grid()->setEnabled( false );
  map->grid()->setAnnotationEnabled( false );
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_zebrastyle" ), &l );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setFrameFillColor1( Qt::black );
  map->grid()->setFrameFillColor2( Qt::white );
  map->grid()->setEnabled( true );

  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Left, true );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Right, false );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Top, false );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom, false );
  map->updateBoundingRect();

  const bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_zebrastyle_left" ), &l );
  QVERIFY( testResult );

  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Top, true );
  map->updateBoundingRect();
  const bool testResult2 = QGSLAYOUTCHECK( QStringLiteral( "composermap_zebrastyle_lefttop" ), &l );
  QVERIFY( testResult2 );

  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Right, true );
  map->updateBoundingRect();
  const bool testResult3 = QGSLAYOUTCHECK( QStringLiteral( "composermap_zebrastyle_lefttopright" ), &l );
  QVERIFY( testResult3 );

  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom, true );
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_marginzebrastyle" ), &l );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::ZebraNautical );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  map->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  map->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  map->grid()->setEnabled( true );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_zebranauticalstyle" ), &l );
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
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::Zebra );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setFrameFillColor1( Qt::black );
  map->grid()->setFrameFillColor2( Qt::white );
  map->grid()->setEnabled( true );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Left, true );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Right, true );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Top, true );
  map->grid()->setFrameSideFlag( Qgis::MapGridFrameSideFlag::Bottom, true );
  map->updateBoundingRect();

  bool testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_rotatedframe" ), &l );
  QVERIFY( testResult );

  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::LatitudeOnly, Qgis::MapGridBorderSide::Left );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::LongitudeOnly, Qgis::MapGridBorderSide::Right );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::LatitudeOnly, Qgis::MapGridBorderSide::Top );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::LongitudeOnly, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  testResult = QGSLAYOUTCHECK( QStringLiteral( "composermap_framedivisions" ), &l );
  QVERIFY( testResult );

  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Left );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Right );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Top );
  map->grid()->setFrameDivisions( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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
  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_rotatedannotations" ), &l );

  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::HideAll, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::LongitudeOnly, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::LatitudeOnly, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::LongitudeOnly, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_filteredannotations" ), &l );

  map->grid()->setAnnotationEnabled( false );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationDisplay( Qgis::MapGridComponentVisibility::ShowAll, Qgis::MapGridBorderSide::Bottom );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_margininteriorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorticks_annotated" ), &l );

  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorticks_annotated2" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_exteriorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_marginexteriorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::ExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_exteriorticks_annotated" ), &l );

  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_exteriorticks_annotated2" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorexteriorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_margininteriorexteriorticks" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::InteriorExteriorTicks );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorexteriorticks_annotated" ), &l );

  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_interiorexteriorticks_annotated2" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_lineborder" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_marginlineborder" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::LineBorderNautical );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFrameMargin( 5 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_linebordernautical" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::LineBorder );
  map->grid()->setFrameWidth( 10 );
  map->grid()->setFramePenSize( 1 );
  map->grid()->setFramePenColor( Qt::black );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_lineborder_annotated" ), &l );

  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_lineborder_annotated2" ), &l );

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
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
  gridGeographic.setAnnotationFormat( Qgis::MapGridAnnotationFormat::DecimalWithSuffix );
  gridGeographic.setAnnotationPrecision( 1 );
  gridProjected.setAnnotationFormat( Qgis::MapGridAnnotationFormat::DecimalWithSuffix );
  gridProjected.setAnnotationPrecision( 1 );

  QgsExpressionContext expressionContext = gridGeographic.createExpressionContext();

  //normal e/w
  QCOMPARE( gridGeographic.gridAnnotationString( 90, Qgis::MapGridAnnotationType::Longitude, expressionContext, true ), QString( "90.0°E" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 90, Qgis::MapGridAnnotationType::Longitude, expressionContext, false ), QString( "90.0E" ) );

  //0 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 0, Qgis::MapGridAnnotationType::Longitude, expressionContext, true ), QString( "0.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, Qgis::MapGridAnnotationType::Longitude, expressionContext, false ), QString( "0.0E" ) );

  //180 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 180, Qgis::MapGridAnnotationType::Longitude, expressionContext, true ), QString( "180.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 180, Qgis::MapGridAnnotationType::Longitude, expressionContext, false ), QString( "180.0E" ) );

  //normal n/s
  QCOMPARE( gridGeographic.gridAnnotationString( 45, Qgis::MapGridAnnotationType::Latitude, expressionContext, true ), QString( "45.0°N" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 45, Qgis::MapGridAnnotationType::Latitude, expressionContext, false ), QString( "45.0N" ) );

  //0 north/south
  QCOMPARE( gridGeographic.gridAnnotationString( 0, Qgis::MapGridAnnotationType::Latitude, expressionContext, true ), QString( "0.0°" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, Qgis::MapGridAnnotationType::Latitude, expressionContext, false ), QString( "0.0N" ) );

  //Custom format annotations
  gridProjected.setAnnotationFormat( Qgis::MapGridAnnotationFormat::CustomFormat );
  gridProjected.setAnnotationExpression( QStringLiteral( "(@grid_number/10) || case when @grid_axis ='x' then 'a' else 'b' end" ) );
  expressionContext.lastScope()->setVariable( QStringLiteral( "grid_number" ), 45 );
  expressionContext.lastScope()->setVariable( QStringLiteral( "grid_axis" ), QStringLiteral( "y" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 45, Qgis::MapGridAnnotationType::Latitude, expressionContext, false ), QString( "4.5b" ) );
  expressionContext.lastScope()->setVariable( QStringLiteral( "grid_number" ), 35 );
  expressionContext.lastScope()->setVariable( QStringLiteral( "grid_axis" ), QStringLiteral( "x" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 35, Qgis::MapGridAnnotationType::Longitude, expressionContext, false ), QString( "3.5a" ) );
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::InsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_verticaldescending_inside" ), &l );

  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_verticaldescending_outside" ), &l );

  map->grid()->setAnnotationEnabled( false );
}

void TestQgsLayoutMapGrid::dataDefinedDrawAnnotation()
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );

  map->grid()->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::MapGridDrawAnnotation, QgsProperty::fromExpression( QStringLiteral( "case when @grid_axis = 'x' then @grid_number < 787000 when @grid_axis ='y' then @grid_number >= 3342000 end" ) ) );
  map->grid()->refresh();
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_dd_draw_annotation" ), &l );
}

void TestQgsLayoutMapGrid::dataDefinedDrawAnnotationCountAndIndex()
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

  map->grid()->setFrameStyle( Qgis::MapGridFrameStyle::NoFrame );
  map->grid()->setEnabled( true );
  map->grid()->setStyle( Qgis::MapGridStyle::FrameAndAnnotationsOnly );
  map->grid()->setAnnotationEnabled( true );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationDirection( Qgis::MapGridAnnotationDirection::VerticalDescending, Qgis::MapGridBorderSide::Bottom );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Left );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Right );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Top );
  map->grid()->setAnnotationPosition( Qgis::MapGridAnnotationPosition::OutsideMapFrame, Qgis::MapGridBorderSide::Bottom );

  map->grid()->setAnnotationFormat( Qgis::MapGridAnnotationFormat::CustomFormat );
  map->grid()->setAnnotationExpression( QStringLiteral( "@grid_index || '/' || @grid_count" ) );

  map->grid()->dataDefinedProperties().setProperty( QgsLayoutObject::DataDefinedProperty::MapGridDrawAnnotation, QgsProperty::fromExpression( QStringLiteral( "@grid_index > 1 and @grid_index < @grid_count" ) ) );
  map->grid()->refresh();
  map->updateBoundingRect();

  QGSVERIFYLAYOUTCHECK( QStringLiteral( "composermap_grid_variables" ), &l );
}

QGSTEST_MAIN( TestQgsLayoutMapGrid )
#include "testqgslayoutmapgrid.moc"
