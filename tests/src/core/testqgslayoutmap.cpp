/***************************************************************************
                         testqgslayoutmap.cpp
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
#include "qgsmultibandcolorrenderer.h"
#include "qgsrasterlayer.h"
#include "qgsrasterdataprovider.h"
#include "qgsvectorlayer.h"
#include "qgsvectordataprovider.h"
#include "qgsproject.h"
#include "qgsmapthemecollection.h"
#include "qgsproperty.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutitempolyline.h"
#include "qgsreadwritecontext.h"
#include <QObject>
#include "qgstest.h"

class TestQgsLayoutMap : public QObject
{
    Q_OBJECT

  public:
    TestQgsLayoutMap() = default;

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void id();
    void render();
    void uniqueId(); //test if map id is adapted when doing copy paste
    void worldFileGeneration(); // test world file generation

    void mapPolygonVertices(); // test mapPolygon function with no map rotation
    void dataDefinedLayers(); //test data defined layer string
    void dataDefinedStyles(); //test data defined styles
    void rasterized();
    void layersToRender();
    void mapRotation();
    void mapItemRotation();
    void expressionContext();

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolysLayer = nullptr;
    QgsVectorLayer *mLinesLayer = nullptr;
    QString mReport;
};

void TestQgsLayoutMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create maplayers from testdata and add to layer registry
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  QFileInfo pointFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                     pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QFileInfo polyFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" );
  mPolysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                    polyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  QFileInfo lineFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/lines.shp" );
  mLinesLayer = new QgsVectorLayer( lineFileInfo.filePath(),
                                    lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // some layers need to be in project for data-defined layers functionality
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mRasterLayer << mPointsLayer << mPolysLayer << mLinesLayer );
}

void TestQgsLayoutMap::cleanupTestCase()
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

void TestQgsLayoutMap::init()
{
#if 0
  //create composition with composer map
  mComposition = new QgsComposition( QgsProject::instance() );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposerMap = new QgsComposerMap( mComposition, 20, 20, 200, 100 );
  mComposerMap->setFrameEnabled( true );
  mComposerMap->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  mComposition->addComposerMap( mComposerMap );
#endif
  mReport = QStringLiteral( "<h1>Composer Map Tests</h1>\n" );
}

void TestQgsLayoutMap::cleanup()
{
}

void TestQgsLayoutMap::id()
{
  QgsLayout l( QgsProject::instance( ) );
  QgsLayoutItemMap *map1 = new QgsLayoutItemMap( &l );
  QCOMPARE( map1->displayName(), QStringLiteral( "Map 1" ) );
  l.addLayoutItem( map1 );

  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  QCOMPARE( map2->displayName(), QStringLiteral( "Map 2" ) );
  l.addLayoutItem( map2 );

  map1->setId( "my map" );
  QCOMPARE( map1->displayName(), QStringLiteral( "my map" ) );

  // existing name should be recycled
  l.removeLayoutItem( map1 );
  QgsLayoutItemMap *map3 = new QgsLayoutItemMap( &l );
  QCOMPARE( map3->displayName(), QStringLiteral( "Map 1" ) );
  l.addLayoutItem( map3 );

}


void TestQgsLayoutMap::render()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QgsLayoutChecker checker( QStringLiteral( "composermap_render" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );

  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutMap::uniqueId()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  l.addLayoutItem( map );

  QDomDocument doc;
  QDomElement documentElement = doc.createElement( QStringLiteral( "ComposerItemClipboard" ) );
  map->writeXml( documentElement, doc, QgsReadWriteContext() );
  l.addItemsFromXml( documentElement, doc, QgsReadWriteContext() );

  //test if both composer maps have different ids
  QgsLayoutItemMap *newMap = nullptr;
  QList<QgsLayoutItemMap *> mapList;
  l.layoutItems( mapList );
  for ( auto mapIt = mapList.constBegin() ; mapIt != mapList.constEnd(); ++mapIt )
  {
    if ( *mapIt != map )
    {
      newMap = *mapIt;
      break;
    }
  }

  QVERIFY( newMap );

  QString oldId = map->displayName();
  QString newId = newMap->displayName();

  QVERIFY( oldId != newId );
}

void TestQgsLayoutMap::worldFileGeneration()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  page2->setPageSize( "A4", QgsLayoutItemPage::Landscape );
  l.pageCollection()->addPage( page2 );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  map->setMapRotation( 30.0 );

  l.setReferenceMap( map );

  QgsLayoutExporter exporter( &l );

  double a, b, c, d, e, f;
  exporter.computeWorldFileParameters( a, b, c, d, e, f );

  QGSCOMPARENEAR( a, 4.18048, 0.001 );
  QGSCOMPARENEAR( b, 2.41331, 0.001 );
  QGSCOMPARENEAR( c, 779444, 1 );
  QGSCOMPARENEAR( d, 2.4136, 0.001 );
  QGSCOMPARENEAR( e, -4.17997, 0.001 );
  QGSCOMPARENEAR( f, 3.34241e+06, 1e+03 );

  //test with map on second page. Parameters should be the same
  map->attemptMove( QgsLayoutPoint( 20, 20 ), true, false, 1 );
  exporter.computeWorldFileParameters( a, b, c, d, e, f );

  QGSCOMPARENEAR( a, 4.18048, 0.001 );
  QGSCOMPARENEAR( b, 2.41331, 0.001 );
  QGSCOMPARENEAR( c, 779444, 1 );
  QGSCOMPARENEAR( d, 2.4136, 0.001 );
  QGSCOMPARENEAR( e, -4.17997, 0.001 );
  QGSCOMPARENEAR( f, 3.34241e+06, 1e+03 );

  //test computing parameters for specific region
  map->attemptMove( QgsLayoutPoint( 20, 20 ), true, false, 1 );
  exporter.computeWorldFileParameters( QRectF( 10, 5, 260, 200 ), a, b, c, d, e, f );

  QGSCOMPARENEAR( a, 4.18061, 0.001 );
  QGSCOMPARENEAR( b, 2.41321, 0.001 );
  QGSCOMPARENEAR( c, 773810, 1 );
  QGSCOMPARENEAR( d, 2.4137, 0.001 );
  QGSCOMPARENEAR( e, -4.1798, 0.001 );
  QGSCOMPARENEAR( f, 3.35331e+06, 1e+03 );
}


void TestQgsLayoutMap::mapPolygonVertices()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << mRasterLayer );
  l.addLayoutItem( map );

  map->setExtent( QgsRectangle( 781662.375, 3339523.125, 793062.375, 3345223.125 ) );
  QPolygonF visibleExtent = map->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( std::fabs( visibleExtent[0].x() - 781662.375 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[0].y() - 3345223.125 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[1].x() - 793062.375 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[1].y() - 3345223.125 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[2].x() - 793062.375 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[2].y() - 3339523.125 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[3].x() - 781662.375 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[3].y() - 3339523.125 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  //now test with rotated map
  map->setMapRotation( 10 );
  visibleExtent = map->visibleExtentPolygon();

  //vertices should be returned in clockwise order starting at the top-left point
  QVERIFY( std::fabs( visibleExtent[0].x() - 781254.0735015 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[0].y() - 3344190.0324834 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[1].x() - 792480.881886 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[1].y() - 3346169.62171 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[2].x() - 793470.676499 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[2].y() - 3340556.21752 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[3].x() - 782243.868114 ) < 0.001 );
  QVERIFY( std::fabs( visibleExtent[3].y() - 3338576.62829 ) < 0.001 );

  //polygon should be closed
  QVERIFY( visibleExtent.isClosed() );

  map->setMapRotation( 0 );

}

void TestQgsLayoutMap::dataDefinedLayers()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptMove( QgsLayoutPoint( 20, 20 ) );
  map->attemptResize( QgsLayoutSize( 200, 100 ) );

  map->setFrameEnabled( true );
  l.addLayoutItem( map );

  //test malformed layer set string
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression( QStringLiteral( "'x'" ) ) );
  QList<QgsMapLayer *> result = map->layersToRender();
  QVERIFY( result.isEmpty() );

  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression( QStringLiteral( "'x|'" ) ) );
  result = map->layersToRender();
  QVERIFY( result.isEmpty() );

  //test subset of valid layers
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "'%1|%2'" ).arg( mPolysLayer->name(), mRasterLayer->name() ) ) );
  result = map->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mPolysLayer ) );
  QVERIFY( result.contains( mRasterLayer ) );

  //test non-existent layer
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "'x|%1|%2'" ).arg( mLinesLayer->name(), mPointsLayer->name() ) ) );
  result = map->layersToRender();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer ) );
  QVERIFY( result.contains( mPointsLayer ) );

  //test no layers
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "''" ) ) );
  result = map->layersToRender();
  QVERIFY( result.isEmpty() );

  //test with atlas feature evaluation
  QgsVectorLayer *atlasLayer = new QgsVectorLayer( QStringLiteral( "Point?field=col1:string" ), QStringLiteral( "atlas" ), QStringLiteral( "memory" ) );
  QVERIFY( atlasLayer->isValid() );
  QgsFeature f1( atlasLayer->dataProvider()->fields(), 1 );
  f1.setAttribute( QStringLiteral( "col1" ), mLinesLayer->name() );
  QgsFeature f2( atlasLayer->dataProvider()->fields(), 1 );
  f2.setAttribute( QStringLiteral( "col1" ), mPointsLayer->name() );
  atlasLayer->dataProvider()->addFeatures( QgsFeatureList() << f1 << f2 );

  l.reportContext().setLayer( atlasLayer );
  QgsFeature f;
  QgsFeatureIterator it = atlasLayer->getFeatures();
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromField( QStringLiteral( "col1" ) ) );
  result = map->layersToRender();
  QCOMPARE( result.count(), 1 );
  QCOMPARE( result.at( 0 ), mLinesLayer );
  it.nextFeature( f );
  l.reportContext().setFeature( f );
  result = map->layersToRender();
  QCOMPARE( result.count(), 1 );
  QCOMPARE( result.at( 0 ), mPointsLayer );
  it.nextFeature( f );
  l.reportContext().setFeature( f );

  delete atlasLayer;

  //render test
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "'%1|%2'" ).arg( mPolysLayer->name(), mPointsLayer->name() ) ) );
  map->setExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsLayoutChecker checker( QStringLiteral( "composermap_ddlayers" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutMap::dataDefinedStyles()
{
  QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptMove( QgsLayoutPoint( 20, 20 ) );
  map->attemptResize( QgsLayoutSize( 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( layers );
  l.addLayoutItem( map );

  QgsMapThemeCollection::MapThemeRecord rec;
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( mPointsLayer )
                       << QgsMapThemeCollection::MapThemeLayerRecord( mLinesLayer )
                     );

  QgsProject::instance()->mapThemeCollection()->insert( QStringLiteral( "test preset" ), rec );

  // test following of preset
  map->setFollowVisibilityPreset( true );
  map->setFollowVisibilityPresetName( QStringLiteral( "test preset" ) );
  QSet<QgsMapLayer *> result = map->layersToRender().toSet();
  QCOMPARE( result.count(), 2 );
  map->setFollowVisibilityPresetName( QString() );

  //test malformed style string
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "5" ) ) );
  result = map->layersToRender().toSet();
  QCOMPARE( result, layers.toSet() );

  //test valid preset
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'test preset'" ) ) );
  result = map->layersToRender().toSet();
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer ) );
  QVERIFY( result.contains( mPointsLayer ) );

  //test non-existent preset
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'bad preset'" ) ) );
  result = map->layersToRender().toSet();
  QCOMPARE( result, layers.toSet() );

  //test that dd layer set overrides style layers
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'test preset'" ) ) );
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "'%1'" ).arg( mPolysLayer->name() ) ) );
  result = map->layersToRender().toSet();
  QCOMPARE( result.count(), 1 );
  QVERIFY( result.contains( mPolysLayer ) );
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty() );

  //render test
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'test preset'" ) ) );
  map->setExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );

  QgsLayoutChecker checker( QStringLiteral( "composermap_ddstyles" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutMap::rasterized()
{
  // test a map which must be rasterized
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptMove( QgsLayoutPoint( 20, 30 ) );
  map->attemptResize( QgsLayoutSize( 200, 100 ) );
  map->setFrameEnabled( true );
  map->setExtent( QgsRectangle( -110.0, 25.0, -90, 40.0 ) );
  QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mLinesLayer;
  map->setLayers( layers );
  map->setBackgroundColor( Qt::yellow );
  l.addLayoutItem( map );

  // add some guide lines, just for reference
  QPolygonF points;
  points << QPointF( 0, 30 ) << QPointF( 10, 30 );
  QgsLayoutItemPolyline *line1 = new QgsLayoutItemPolyline( points, &l );
  l.addLayoutItem( line1 );
  points.clear();
  points << QPointF( 0, 30 + map->rect().height() ) << QPointF( 10, 30 + map->rect().height() );
  QgsLayoutItemPolyline *line2 = new QgsLayoutItemPolyline( points, &l );
  l.addLayoutItem( line2 );
  points.clear();
  points << QPointF( 20, 0 ) << QPointF( 20, 20 );
  QgsLayoutItemPolyline *line3 = new QgsLayoutItemPolyline( points, &l );
  l.addLayoutItem( line3 );
  points.clear();
  points << QPointF( 220, 0 ) << QPointF( 220, 20 );
  QgsLayoutItemPolyline *line4 = new QgsLayoutItemPolyline( points, &l );
  l.addLayoutItem( line4 );

  // force rasterization
  QgsLayoutItemMapGrid *grid = new QgsLayoutItemMapGrid( "test", map );
  grid->setIntervalX( 10 );
  grid->setIntervalY( 10 );
  grid->setBlendMode( QPainter::CompositionMode_Darken );
  grid->setAnnotationEnabled( true );
  grid->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Left );
  grid->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Top );
  grid->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Right );
  grid->setAnnotationDisplay( QgsLayoutItemMapGrid::ShowAll, QgsLayoutItemMapGrid::Bottom );
  map->grids()->addGrid( grid );
  map->updateBoundingRect();

  QVERIFY( map->containsAdvancedEffects() );

  QgsLayoutChecker checker( QStringLiteral( "layoutmap_rasterized" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_map" ) );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );

  // try rendering again, without requiring rasterization, for comparison
  // (we can use the same test image, because CompositionMode_Darken doesn't actually have any noticeable
  // rendering differences for the black grid!)
  grid->setBlendMode( QPainter::CompositionMode_SourceOver );
  QVERIFY( !map->containsAdvancedEffects() );
  QVERIFY( checker.testLayout( mReport, 0, 0 ) );
}

void TestQgsLayoutMap::layersToRender()
{
  QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;
  QList<QgsMapLayer *> layers2 = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mLinesLayer;

  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setLayers( layers );
  l.addLayoutItem( map );

  QCOMPARE( map->layersToRender(), layers );

  // hide coverage layer
  l.reportContext().setLayer( mPointsLayer );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagHideCoverageLayer, true );
  QCOMPARE( map->layersToRender(), layers2 );

  l.renderContext().setFlag( QgsLayoutRenderContext::FlagHideCoverageLayer, false );
  QCOMPARE( map->layersToRender(), layers );
}

void TestQgsLayoutMap::mapRotation()
{
  QgsProject p;
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
  QgsRasterLayer *layer = new QgsRasterLayer( rasterFileInfo.filePath(),
      rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  layer->setRenderer( rasterRenderer );
  p.addMapLayer( layer );

  QgsLayout l( &p );
  l.initializeDefaults();

  //test map rotation
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 100, 50 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 0, -192, 256, -64 ) );
  map->setMapRotation( 90 );
  map->setLayers( QList<QgsMapLayer *>() << layer );

  QgsLayoutChecker checker( QStringLiteral( "composerrotation_maprotation" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testLayout( mReport, 0, 200 ) );
}

void TestQgsLayoutMap::mapItemRotation()
{
  QgsProject p;
  QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
  QgsRasterLayer *layer = new QgsRasterLayer( rasterFileInfo.filePath(),
      rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 1, 2, 3 );
  layer->setRenderer( rasterRenderer );
  p.addMapLayer( layer );

  QgsLayout l( &p );
  l.initializeDefaults();

  //test map rotation
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 50, 100, 50 ) );
  map->setFrameEnabled( true );
  l.addLayoutItem( map );
  map->setExtent( QgsRectangle( 0, -192, 256, -64 ) );
  map->setItemRotation( 90 );
  map->setLayers( QList<QgsMapLayer *>() << layer );

  QgsLayoutChecker checker( QStringLiteral( "composerrotation_mapitemrotation" ), &l );
  checker.setControlPathPrefix( QStringLiteral( "composer_items" ) );
  QVERIFY( checker.testLayout( mReport, 0, 200 ) );
}

void TestQgsLayoutMap::expressionContext()
{
  QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );
  map->setId( QStringLiteral( "Map_id" ) );

  QgsExpression e( QStringLiteral( "@map_scale" ) );

  QgsExpressionContext c = map->createExpressionContext();
  QVariant r = e.evaluate( &c );
  QGSCOMPARENEAR( r.toDouble(), 184764103, 100 );

  QgsExpression e2( QStringLiteral( "@map_crs" ) );
  r = e2.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:4326" ) );

  QgsExpression e3( QStringLiteral( "@map_crs_definition" ) );
  r = e3.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e4( QStringLiteral( "@map_units" ) );
  r = e4.evaluate( &c );
  QCOMPARE( r.toString(), QString( "degrees" ) );

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
  map->setLayers( QList<QgsMapLayer *>() << layer << layer2 );
  QgsProject::instance()->addMapLayers( map->layers() );
  c = map->createExpressionContext();
  QgsExpression e5( QStringLiteral( "@map_layer_ids" ) );
  r = e5.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "%1,%2" ).arg( layer->id(), layer2->id() ) );
  e5 = QgsExpression( QStringLiteral( "array_foreach(@map_layers, layer_property(@element, 'name'))" ) );
  r = e5.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "A,B" ) );

  QgsExpression e6( QStringLiteral( "is_layer_visible( '%1' )" ).arg( layer->id() ) );
  r = e6.evaluate( &c );
  QCOMPARE( r.toBool(), true );

  QgsExpression e7( QStringLiteral( "is_layer_visible( 'aaaaaa' )" ).arg( layer->id() ) );
  r = e7.evaluate( &c );
  QCOMPARE( r.toBool(), false );
}

QGSTEST_MAIN( TestQgsLayoutMap )
#include "testqgslayoutmap.moc"
