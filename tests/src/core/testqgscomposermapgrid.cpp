/***************************************************************************
                         testqgscomposermapgrid.cpp
                         ----------------------
    begin                : August 2014
    copyright            : (C) 2014 by Nyall Dawosn, Marco Hugentobler
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
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include "qgscomposermap.h"
#include "qgscomposermapgrid.h"
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerMapGrid: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void grid(); //test if grid and grid annotation works
    void crossGrid(); //test if grid "cross" mode works
    void markerGrid(); //test if grid "marker" mode works
    void frameOnly(); //test if grid "frame/annotation" mode works
    void zebraStyle(); //test zebra map border style
    void zebraStyleSides(); //test zebra border on certain sides
    void interiorTicks(); //test interior tick mode
    void interiorTicksAnnotated(); //test interior tick mode with annotations
    void exteriorTicks(); //test exterior tick mode
    void exteriorTicksAnnotated(); //test exterior tick mode with annotations
    void interiorExteriorTicks(); //test interior & exterior tick mode
    void interiorExteriorTicksAnnotated(); //test interior & exterior tick mode with annotations
    void lineBorder(); //test line border frame mode
    void lineBorderAnnotated(); //test line border frame with annotations
    void annotationFormats(); //various tests for annotation formats

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerMapGrid::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsCoordinateReferenceSystem crs = QgsCoordinateReferenceSystem( 32633 );
  mMapSettings.setDestinationCrs( crs );
  mMapSettings.setCrsTransformEnabled( false );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposerMap->setBackgroundColor( QColor( 150, 100, 100 ) );
  mComposition->addComposerMap( mComposerMap );

  mReport = "<h1>Composer Map Grid Tests</h1>\n";
}

void TestQgsComposerMapGrid::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerMapGrid::init()
{
}

void TestQgsComposerMapGrid::cleanup()
{

}

void TestQgsComposerMapGrid::grid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setIntervalX( 2000 );
  mComposerMap->grid()->setIntervalY( 2000 );
  mComposerMap->grid()->setAnnotationEnabled( true );
  mComposerMap->grid()->setGridLineColor( QColor( 0, 255, 0 ) );
  mComposerMap->grid()->setGridLineWidth( 0.5 );
  mComposerMap->grid()->setAnnotationFont( QgsFontUtils::getStandardTestFont() );
  mComposerMap->grid()->setAnnotationPrecision( 0 );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::Disabled, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::Disabled, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Bottom );
  mComposerMap->grid()->setAnnotationDirection( QgsComposerMapGrid::Horizontal, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationDirection( QgsComposerMapGrid::Horizontal, QgsComposerMapGrid::Bottom );
  mComposerMap->grid()->setAnnotationFontColor( QColor( 255, 0, 0, 150 ) );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_Overlay );
  qWarning() << "grid annotation font: " << mComposerMap->grid()->annotationFont().toString() << " exactMatch:" << mComposerMap->grid()->annotationFont().exactMatch();
  QgsCompositionChecker checker( "composermap_grid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposerMap->grid()->setEnabled( false );
  mComposerMap->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::crossGrid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::Cross );
  mComposerMap->grid()->setCrossLength( 2.0 );
  mComposerMap->grid()->setIntervalX( 2000 );
  mComposerMap->grid()->setIntervalY( 2000 );
  mComposerMap->grid()->setAnnotationEnabled( false );
  mComposerMap->grid()->setGridLineColor( QColor( 0, 255, 0 ) );
  mComposerMap->grid()->setGridLineWidth( 0.5 );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_crossgrid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::Solid );
  mComposerMap->grid()->setEnabled( false );
  mComposerMap->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::markerGrid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::Markers );
  mComposerMap->grid()->setIntervalX( 2000 );
  mComposerMap->grid()->setIntervalY( 2000 );
  mComposerMap->grid()->setAnnotationEnabled( false );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_markergrid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::Solid );
  mComposerMap->grid()->setEnabled( false );
  mComposerMap->grid()->setAnnotationEnabled( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::frameOnly()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
  mComposerMap->grid()->setIntervalX( 2000 );
  mComposerMap->grid()->setIntervalY( 2000 );
  mComposerMap->grid()->setAnnotationEnabled( false );
  //set a frame for testing
  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::Zebra );
  mComposerMap->grid()->setFramePenSize( 0.5 );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_gridframeonly", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::Solid );
  mComposerMap->grid()->setEnabled( false );
  mComposerMap->grid()->setAnnotationEnabled( false );
  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::zebraStyle()
{
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  mComposerMap->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  mComposerMap->grid()->setAnnotationFontColor( QColor( 0, 0, 0, 0 ) );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::Zebra );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( QColor( 255, 100, 0, 200 ) );
  mComposerMap->grid()->setFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  mComposerMap->grid()->setFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  mComposerMap->grid()->setEnabled( true );

  QgsCompositionChecker checker( "composermap_zebrastyle", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::zebraStyleSides()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->grid()->setGridLineColor( QColor( 0, 0, 0 ) );
  mComposerMap->grid()->setAnnotationFontColor( QColor( 0, 0, 0, 0 ) );
  mComposerMap->grid()->setBlendMode( QPainter::CompositionMode_SourceOver );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::Zebra );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setFrameFillColor1( Qt::black );
  mComposerMap->grid()->setFrameFillColor2( Qt::white );
  mComposerMap->grid()->setEnabled( true );

  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameLeft, true );
  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameRight, false );
  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameTop, false );
  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameBottom, false );

  QgsCompositionChecker checker( "composermap_zebrastyle_left", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameTop, true );
  QgsCompositionChecker checker2( "composermap_zebrastyle_lefttop", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 0 );
  QVERIFY( testResult2 );

  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameRight, true );
  QgsCompositionChecker checker3( "composermap_zebrastyle_lefttopright", mComposition );
  bool testResult3 = checker3.testComposition( mReport, 0, 0 );
  QVERIFY( testResult3 );

  mComposerMap->grid()->setFrameSideFlag( QgsComposerMapGrid::FrameBottom, true );
  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );

}

void TestQgsComposerMapGrid::interiorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::InteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_interiorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
}

void TestQgsComposerMapGrid::interiorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::InteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
  mComposerMap->grid()->setAnnotationEnabled( true );
  mComposerMap->grid()->setAnnotationFontColor( Qt::black );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker( "composermap_interiorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker2( "composermap_interiorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 0 );
  QVERIFY( testResult2 );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
  mComposerMap->grid()->setAnnotationEnabled( false );
}

void TestQgsComposerMapGrid::exteriorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::ExteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_exteriorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
}

void TestQgsComposerMapGrid::exteriorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::ExteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
  mComposerMap->grid()->setAnnotationEnabled( true );
  mComposerMap->grid()->setAnnotationFontColor( Qt::black );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker( "composermap_exteriorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker2( "composermap_exteriorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 0 );
  QVERIFY( testResult2 );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
  mComposerMap->grid()->setAnnotationEnabled( false );
}

void TestQgsComposerMapGrid::interiorExteriorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::InteriorExteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_interiorexteriorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
}

void TestQgsComposerMapGrid::interiorExteriorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::InteriorExteriorTicks );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
  mComposerMap->grid()->setAnnotationEnabled( true );
  mComposerMap->grid()->setAnnotationFontColor( Qt::black );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker( "composermap_interiorexteriorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker2( "composermap_interiorexteriorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 0 );
  QVERIFY( testResult2 );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
  mComposerMap->grid()->setAnnotationEnabled( false );
}

void TestQgsComposerMapGrid::lineBorder()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::LineBorder );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_lineborder", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
}

void TestQgsComposerMapGrid::lineBorderAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::LineBorder );
  mComposerMap->grid()->setFrameWidth( 10 );
  mComposerMap->grid()->setFramePenSize( 1 );
  mComposerMap->grid()->setFramePenColor( Qt::black );
  mComposerMap->grid()->setEnabled( true );
  mComposerMap->grid()->setStyle( QgsComposerMapGrid::FrameAnnotationsOnly );
  mComposerMap->grid()->setAnnotationEnabled( true );
  mComposerMap->grid()->setAnnotationFontColor( Qt::black );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::InsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker( "composermap_lineborder_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 0 );
  QVERIFY( testResult );

  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Left );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Right );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Top );
  mComposerMap->grid()->setAnnotationPosition( QgsComposerMapGrid::OutsideMapFrame, QgsComposerMapGrid::Bottom );

  QgsCompositionChecker checker2( "composermap_lineborder_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 0 );
  QVERIFY( testResult2 );

  mComposerMap->grid()->setFrameStyle( QgsComposerMapGrid::NoFrame );
  mComposerMap->grid()->setAnnotationEnabled( false );
}

void TestQgsComposerMapGrid::annotationFormats()
{
  //create grids in geographic and projected coordinates
  QgsCoordinateReferenceSystem projectedCrs;
  projectedCrs.createFromSrid( 3994 );
  QgsCoordinateReferenceSystem geographicCrs;
  geographicCrs.createFromSrid( 4326 );

  QgsComposerMapGrid gridGeographic( "geographic grid", mComposerMap );
  gridGeographic.setCrs( geographicCrs );
  QgsComposerMapGrid gridProjected( "projected grid", mComposerMap );
  gridProjected.setCrs( projectedCrs );

  //decimal degrees format
  gridGeographic.setAnnotationFormat( QgsComposerMapGrid::DecimalWithSuffix );
  gridGeographic.setAnnotationPrecision( 1 );
  gridProjected.setAnnotationFormat( QgsComposerMapGrid::DecimalWithSuffix );
  gridProjected.setAnnotationPrecision( 1 );

  //normal e/w
  QCOMPARE( gridGeographic.gridAnnotationString( 90, QgsComposerMapGrid::Longitude ), QString( "90.0" ) + QChar( 176 ) + QString( "E" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 90, QgsComposerMapGrid::Longitude ), QString( "90.0E" ) );

  //0 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 0, QgsComposerMapGrid::Longitude ), QString( "0.0" ) + QChar( 176 ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, QgsComposerMapGrid::Longitude ), QString( "0.0E" ) );

  //180 degrees
  QCOMPARE( gridGeographic.gridAnnotationString( 180, QgsComposerMapGrid::Longitude ), QString( "180.0" ) + QChar( 176 ) );
  QCOMPARE( gridProjected.gridAnnotationString( 180, QgsComposerMapGrid::Longitude ), QString( "180.0E" ) );

  //normal n/s
  QCOMPARE( gridGeographic.gridAnnotationString( 45, QgsComposerMapGrid::Latitude ), QString( "45.0" ) + QChar( 176 ) + QString( "N" ) );
  QCOMPARE( gridProjected.gridAnnotationString( 45, QgsComposerMapGrid::Latitude ), QString( "45.0N" ) );

  //0 north/south
  QCOMPARE( gridGeographic.gridAnnotationString( 0, QgsComposerMapGrid::Latitude ), QString( "0.0" ) + QChar( 176 ) );
  QCOMPARE( gridProjected.gridAnnotationString( 0, QgsComposerMapGrid::Latitude ), QString( "0.0N" ) );

}



QTEST_MAIN( TestQgsComposerMapGrid )
#include "moc_testqgscomposermapgrid.cxx"
