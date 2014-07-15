/***************************************************************************
                         testqgscomposermap.cpp
                         ----------------------
    begin                : Juli 2012
    copyright            : (C) 2012 by Marco Hugentobler
    email                : marco at sourcepole dot ch
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
#include "qgsmaplayerregistry.h"
#include "qgsmaprenderer.h"
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsfontutils.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerMap: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void render(); //test if rendering of the composition with composr map is correct
    void grid(); //test if grid and grid annotation works
    void crossGrid(); //test if grid "cross" mode works
    void markerGrid(); //test if grid "marker" mode works
    void overviewMap(); //test if overview map frame works
    void overviewMapRotated(); //test if overview map frame works with rotated overview
    void overviewMapRotated2(); //test if overview map frame works with rotated map
    void overviewMapBlending(); //test if blend modes with overview map frame works
    void overviewMapInvert(); //test if invert of overview map frame works
    void uniqueId(); //test if map id is adapted when doing copy paste
    void zebraStyle(); //test zebra map border style
    void overviewMapCenter(); //test if centering of overview map frame works
    void worldFileGeneration(); // test world file generation
    void mapPolygonVertices(); // test mapPolygon function with no map rotation

  private:
    QgsComposition* mComposition;
    QgsComposerMap* mComposerMap;
    QgsMapSettings mMapSettings;
    QgsRasterLayer* mRasterLayer;
    QString mReport;
};

void TestQgsComposerMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QString( TEST_DATA_DIR ) + QDir::separator() +  "landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer* rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QgsMapLayerRegistry::instance()->addMapLayers( QList<QgsMapLayer*>() << mRasterLayer );

  //create composition with composer map
  mMapSettings.setLayers( QStringList() << mRasterLayer->id() );
  mMapSettings.setCrsTransformEnabled( false );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposition->addComposerMap( mComposerMap );

  mReport = "<h1>Composer Map Tests</h1>\n";
}

void TestQgsComposerMap::cleanupTestCase()
{
  delete mComposition;
  delete mRasterLayer;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerMap::init()
{
}

void TestQgsComposerMap::cleanup()
{

}

void TestQgsComposerMap::render()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsCompositionChecker checker( "composermap_render", mComposition );

  QVERIFY( checker.testComposition( mReport, 0, 100 ) );
}

void TestQgsComposerMap::grid()
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

void TestQgsComposerMap::crossGrid()
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

void TestQgsComposerMap::markerGrid()
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

void TestQgsComposerMap::overviewMap()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMap->setOverviewFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMap );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapRotated()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  mComposerMap->setMapRotation( 30 );
  overviewMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMap->setOverviewFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview_rotated", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMap );
  mComposerMap->setMapRotation( 0 );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapRotated2()
{
  QgsComposerMap* overviewMap = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMap->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMap );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMap->setMapRotation( 30 );
  overviewMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMap->setOverviewFrameMap( mComposerMap->id() );
  QgsCompositionChecker checker( "composermap_overview_rotated2", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMap );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapBlending()
{
  QgsComposerMap* overviewMapBlend = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapBlend->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapBlend );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMapBlend->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapBlend->setOverviewFrameMap( mComposerMap->id() );
  overviewMapBlend->setOverviewBlendMode( QPainter::CompositionMode_Multiply );

  QgsCompositionChecker checker( "composermap_overview_blending", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMapBlend );
  QVERIFY( testResult );
}

void TestQgsComposerMap::overviewMapInvert()
{
  QgsComposerMap* overviewMapInvert = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapInvert->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapInvert );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375, 3341423.125, 789262.375, 3343323.125 ) ); //zoom in
  overviewMapInvert->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapInvert->setOverviewFrameMap( mComposerMap->id() );
  overviewMapInvert->setOverviewInverted( true );

  QgsCompositionChecker checker( "composermap_overview_invert", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMapInvert );
  QVERIFY( testResult );
}

void TestQgsComposerMap::uniqueId()
{
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( "ComposerItemClipboard" );
  mComposerMap->writeXML( documentElement, doc );
  mComposition->addItemsFromXML( documentElement, doc, 0, false );

  //test if both composer maps have different ids
  const QgsComposerMap* newMap = 0;
  QList<const QgsComposerMap*> mapList = mComposition->composerMapItems();
  QList<const QgsComposerMap*>::const_iterator mapIt = mapList.constBegin();
  for ( ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if ( *mapIt != mComposerMap )
    {
      newMap = *mapIt;
      break;
    }
  }
  int oldId = mComposerMap->id();
  int newId = newMap->id();

  mComposition->removeComposerItem( const_cast<QgsComposerMap*>( newMap ) );

  QVERIFY( oldId != newId );
}

void TestQgsComposerMap::zebraStyle()
{
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

void TestQgsComposerMap::overviewMapCenter()
{
  QgsComposerMap* overviewMapCenter = new QgsComposerMap( mComposition, 20, 130, 70, 70 );
  overviewMapCenter->setFrameEnabled( true );
  mComposition->addComposerMap( overviewMapCenter );
  mComposerMap->setNewExtent( QgsRectangle( 785462.375 + 5000, 3341423.125, 789262.375 + 5000, 3343323.125 ) ); //zoom in
  mComposerMap->setGridEnabled( false );
  overviewMapCenter->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3350923.125 ) );
  overviewMapCenter->setOverviewFrameMap( mComposerMap->id() );
  overviewMapCenter->setOverviewCentered( true );

  QgsCompositionChecker checker( "composermap_overview_center", mComposition );

  bool testResult = checker.testComposition( mReport, 0, 100 );
  mComposition->removeComposerItem( overviewMapCenter );
  QVERIFY( testResult );
}

void TestQgsComposerMap::worldFileGeneration()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  mComposerMap->setMapRotation( 30.0 );

  mComposition->setGenerateWorldFile( true );
  mComposition->setWorldFileMap( mComposerMap );

  double a, b, c, d, e, f;
  mComposition->computeWorldFileParameters( a, b, c, d, e, f );

  QVERIFY( fabs( a - 4.18048 ) < 0.001 );
  QVERIFY( fabs( b - 2.41331 ) < 0.001 );
  QVERIFY( fabs( c - 779444 ) < 1 );
  QVERIFY( fabs( d - 2.4136 ) < 0.001 );
  QVERIFY( fabs( e + 4.17997 ) < 0.001 );
  QVERIFY( fabs( f - 3.34241e+06 ) < 1e+03 );

  mComposition->setGenerateWorldFile( false );
  mComposerMap->setMapRotation( 0.0 );

}

void TestQgsComposerMap::mapPolygonVertices()
{
  mComposerMap->setNewExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QPolygonF visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3345223.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793062.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3339523.125 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 781662.375 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3339523.125 ) < 0.001 );

  //now test with rotated map
  mComposerMap->setMapRotation( 10 );
  visibleExtent = mComposerMap->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( fabs( visibleExtent[0].x() - 781254.0735015 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[0].y() - 3344190.0324834 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].x() - 792480.881886 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[1].y() - 3346169.62171 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].x() - 793470.676499 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[2].y() - 3340556.21752 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].x() - 782243.868114 ) < 0.001 );
  QVERIFY( fabs( visibleExtent[3].y() - 3338576.62829 ) < 0.001 );

  mComposerMap->setMapRotation( 0 );

}

QTEST_MAIN( TestQgsComposerMap )
#include "moc_testqgscomposermap.cxx"
