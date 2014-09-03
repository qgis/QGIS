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
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridIntervalX( 2000 );
  mComposerMap->setGridIntervalY( 2000 );
  mComposerMap->setShowGridAnnotation( true );
  mComposerMap->setGridPenWidth( 0.5 );
  mComposerMap->setGridPenColor( QColor( 0, 255, 0 ) );
  mComposerMap->setGridAnnotationFont( QgsFontUtils::getStandardTestFont() );
  mComposerMap->setGridAnnotationPrecision( 0 );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::Disabled, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Bottom );
  mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationDirection( QgsComposerMap::Horizontal, QgsComposerMap::Bottom );
  mComposerMap->setAnnotationFontColor( QColor( 255, 0, 0, 150 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_Overlay );
  qWarning() << "grid annotation font: " << mComposerMap->gridAnnotationFont().toString() << " exactMatch:" << mComposerMap->gridAnnotationFont().exactMatch();
  QgsCompositionChecker checker( "composermap_grid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposerMap->setGridEnabled( false );
  mComposerMap->setShowGridAnnotation( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::crossGrid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::Cross );
  mComposerMap->setCrossLength( 2.0 );
  mComposerMap->setGridIntervalX( 2000 );
  mComposerMap->setGridIntervalY( 2000 );
  mComposerMap->setShowGridAnnotation( false );
  mComposerMap->setGridPenWidth( 0.5 );
  mComposerMap->setGridPenColor( QColor( 0, 255, 0 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_crossgrid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposerMap->setGridStyle( QgsComposerMap::Solid );
  mComposerMap->setGridEnabled( false );
  mComposerMap->setShowGridAnnotation( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::markerGrid()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::Markers );
  mComposerMap->setGridIntervalX( 2000 );
  mComposerMap->setGridIntervalY( 2000 );
  mComposerMap->setShowGridAnnotation( false );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_markergrid", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposerMap->setGridStyle( QgsComposerMap::Solid );
  mComposerMap->setGridEnabled( false );
  mComposerMap->setShowGridAnnotation( false );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::frameOnly()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );
  mComposerMap->setGridIntervalX( 2000 );
  mComposerMap->setGridIntervalY( 2000 );
  mComposerMap->setShowGridAnnotation( false );
  //set a frame for testing
  mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );
  QgsCompositionChecker checker( "composermap_gridframeonly", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposerMap->setGridStyle( QgsComposerMap::Solid );
  mComposerMap->setGridEnabled( false );
  mComposerMap->setShowGridAnnotation( false );
  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::zebraStyle()
{
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  mComposerMap->setGridPenColor( QColor( 0, 0, 0 ) );
  mComposerMap->setAnnotationFontColor( QColor( 0, 0, 0, 0 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );

  mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( QColor( 255, 100, 0, 200 ) );
  mComposerMap->setGridFrameFillColor1( QColor( 50, 90, 50, 100 ) );
  mComposerMap->setGridFrameFillColor2( QColor( 200, 220, 100, 60 ) );
  mComposerMap->setGridEnabled( true );

  QgsCompositionChecker checker( "composermap_zebrastyle", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );
}

void TestQgsComposerMapGrid::zebraStyleSides()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setGridPenColor( QColor( 0, 0, 0 ) );
  mComposerMap->setAnnotationFontColor( QColor( 0, 0, 0, 0 ) );
  mComposerMap->setGridBlendMode( QPainter::CompositionMode_SourceOver );

  mComposerMap->setGridFrameStyle( QgsComposerMap::Zebra );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridFrameFillColor1( Qt::black );
  mComposerMap->setGridFrameFillColor2( Qt::white );
  mComposerMap->setGridEnabled( true );

  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameLeft, true );
  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameRight, false );
  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameTop, false );
  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameBottom, false );

  QgsCompositionChecker checker( "composermap_zebrastyle_left", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameTop, true );
  QgsCompositionChecker checker2( "composermap_zebrastyle_lefttop", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 100 );
  QVERIFY( testResult2 );

  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameRight, true );
  QgsCompositionChecker checker3( "composermap_zebrastyle_lefttopright", mComposition );
  bool testResult3 = checker3.testComposition( mReport, 0, 100 );
  QVERIFY( testResult3 );

  mComposerMap->mapGrids()[0]->setGridFrameSideFlag( QgsComposerMapGrid::FrameBottom, true );
  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );

}

void TestQgsComposerMapGrid::interiorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::InteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_interiorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
}

void TestQgsComposerMapGrid::interiorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::InteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );
  mComposerMap->setShowGridAnnotation( true );
  mComposerMap->setAnnotationFontColor( Qt::black );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker( "composermap_interiorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker2( "composermap_interiorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 100 );
  QVERIFY( testResult2 );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  mComposerMap->setShowGridAnnotation( false );
}

void TestQgsComposerMapGrid::exteriorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::ExteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_exteriorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
}

void TestQgsComposerMapGrid::exteriorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::ExteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );
  mComposerMap->setShowGridAnnotation( true );
  mComposerMap->setAnnotationFontColor( Qt::black );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker( "composermap_exteriorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker2( "composermap_exteriorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 100 );
  QVERIFY( testResult2 );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  mComposerMap->setShowGridAnnotation( false );
}

void TestQgsComposerMapGrid::interiorExteriorTicks()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::InteriorExteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );

  QgsCompositionChecker checker( "composermap_interiorexteriorticks", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
}

void TestQgsComposerMapGrid::interiorExteriorTicksAnnotated()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );

  mComposerMap->setGridFrameStyle( QgsComposerMap::InteriorExteriorTicks );
  mComposerMap->setGridFrameWidth( 10 );
  mComposerMap->setGridFramePenSize( 1 );
  mComposerMap->setGridFramePenColor( Qt::black );
  mComposerMap->setGridEnabled( true );
  mComposerMap->setGridStyle( QgsComposerMap::FrameAnnotationsOnly );
  mComposerMap->setShowGridAnnotation( true );
  mComposerMap->setAnnotationFontColor( Qt::black );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::InsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker( "composermap_interiorexteriorticks_annotated", mComposition );
  bool testResult = checker.testComposition( mReport, 0, 100 );
  QVERIFY( testResult );

  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Left );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Right );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Top );
  mComposerMap->setGridAnnotationPosition( QgsComposerMap::OutsideMapFrame, QgsComposerMap::Bottom );

  QgsCompositionChecker checker2( "composermap_interiorexteriorticks_annotated2", mComposition );
  bool testResult2 = checker2.testComposition( mReport, 0, 100 );
  QVERIFY( testResult2 );

  mComposerMap->setGridFrameStyle( QgsComposerMap::NoGridFrame );
  mComposerMap->setShowGridAnnotation( false );
}



QTEST_MAIN( TestQgsComposerMapGrid )
#include "moc_testqgscomposermapgrid.cxx"
