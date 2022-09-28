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
#include "qgsrenderedfeaturehandlerinterface.h"
#include "qgspallabeling.h"
#include "qgsvectorlayerlabeling.h"
#include "qgstemporalrangeobject.h"
#include "qgsfontutils.h"
#include "qgsannotationlayer.h"
#include "qgsannotationmarkeritem.h"
#include "qgslabelingresults.h"

#include <QObject>
#include "qgstest.h"

class TestQgsLayoutMap : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsLayoutMap() : QgsTest( QStringLiteral( "Layout Map Tests" ) ) {}

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void id();
    void render();
    void uniqueId(); //test if map id is adapted when doing copy paste
    void worldFileGeneration(); // test world file generation

    void mapPolygonVertices(); // test mapPolygon function with no map rotation
    void dataDefinedLayers(); //test data defined layer string
    void dataDefinedStyles(); //test data defined styles
    void dataDefinedCrs(); //test data defined crs
    void dataDefinedTemporalRange(); //test data defined temporal range's start and end values
    void rasterized();
    void layersToRender();
    void mapRotation();
    void mapItemRotation();
    void expressionContext();
    void layoutToMapCoordsTransform();
    void labelBlockingRegions();
    void testSimplificationMethod();
    void testRenderedFeatureHandler();
    void testLayeredExport();
    void testLayeredExportLabelsByLayer();
    void testTemporal();
    void testLabelResults();

  private:
    QgsRasterLayer *mRasterLayer = nullptr;
    QgsVectorLayer *mPointsLayer = nullptr;
    QgsVectorLayer *mPolysLayer = nullptr;
    QgsVectorLayer *mLinesLayer = nullptr;
};

void TestQgsLayoutMap::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  QgsFontUtils::loadStandardTestFonts( QStringList() << QStringLiteral( "Bold" ) );

  //create maplayers from testdata and add to layer registry
  const QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/landsat.tif" );
  mRasterLayer = new QgsRasterLayer( rasterFileInfo.filePath(),
                                     rasterFileInfo.completeBaseName() );
  QgsMultiBandColorRenderer *rasterRenderer = new QgsMultiBandColorRenderer( mRasterLayer->dataProvider(), 2, 3, 4 );
  mRasterLayer->setRenderer( rasterRenderer );

  const QFileInfo pointFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/points.shp" );
  mPointsLayer = new QgsVectorLayer( pointFileInfo.filePath(),
                                     pointFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  const QFileInfo polyFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/polys.shp" );
  mPolysLayer = new QgsVectorLayer( polyFileInfo.filePath(),
                                    polyFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  const QFileInfo lineFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/lines.shp" );
  mLinesLayer = new QgsVectorLayer( lineFileInfo.filePath(),
                                    lineFileInfo.completeBaseName(), QStringLiteral( "ogr" ) );

  // some layers need to be in project for data-defined layers functionality
  QgsProject::instance()->addMapLayers( QList<QgsMapLayer *>() << mRasterLayer << mPointsLayer << mPolysLayer << mLinesLayer );
}

void TestQgsLayoutMap::cleanupTestCase()
{
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

  const QString oldId = map->displayName();
  const QString newId = newMap->displayName();

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

  const QgsLayoutExporter exporter( &l );

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
  const QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;

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
  QSet<QgsMapLayer *> result = qgis::listToSet( map->layersToRender() );
  QCOMPARE( result.count(), 2 );
  map->setFollowVisibilityPresetName( QString() );

  //test malformed style string
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "5" ) ) );
  result = qgis::listToSet( map->layersToRender() );
  QCOMPARE( result, qgis::listToSet( layers ) );

  //test valid preset
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'test preset'" ) ) );
  result = qgis::listToSet( map->layersToRender() );
  QCOMPARE( result.count(), 2 );
  QVERIFY( result.contains( mLinesLayer ) );
  QVERIFY( result.contains( mPointsLayer ) );

  //test non-existent preset
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'bad preset'" ) ) );
  result = qgis::listToSet( map->layersToRender() );
  QCOMPARE( result, qgis::listToSet( layers ) );

  //test that dd layer set overrides style layers
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapStylePreset, QgsProperty::fromExpression( QStringLiteral( "'test preset'" ) ) );
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapLayers, QgsProperty::fromExpression(
        QStringLiteral( "'%1'" ).arg( mPolysLayer->name() ) ) );
  result = qgis::listToSet( map->layersToRender() );
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

void TestQgsLayoutMap::dataDefinedCrs()
{
  const QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptMove( QgsLayoutPoint( 20, 20 ) );
  map->attemptResize( QgsLayoutSize( 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( layers );
  l.addLayoutItem( map );

  //test epsg variable
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapCrs, QgsProperty::fromValue( QStringLiteral( "EPSG:2192" ) ) );
  map->refreshDataDefinedProperty( QgsLayoutObject::MapCrs );
  QCOMPARE( map->crs().authid(), QStringLiteral( "EPSG:2192" ) );

  //test proj string variable
  map->dataDefinedProperties().setProperty( QgsLayoutObject::MapCrs, QgsProperty::fromValue( QStringLiteral( "PROJ4: +proj=merc +a=6378137 +b=6378137 +lat_ts=0.0 +lon_0=0.0 +x_0=0.0 +y_0=0 +k=1.0 +units=m +nadgrids=@null +wktext  +no_defs" ) ) );
  map->refreshDataDefinedProperty( QgsLayoutObject::MapCrs );
  QCOMPARE( map->crs().toProj(), QStringLiteral( "+proj=merc +a=6378137 +b=6378137 +lat_ts=0 +lon_0=0 +x_0=0 +y_0=0 +k=1 +units=m +nadgrids=@null +wktext +no_defs" ) );
}

void TestQgsLayoutMap::dataDefinedTemporalRange()
{
  const QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;

  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptMove( QgsLayoutPoint( 20, 20 ) );
  map->attemptResize( QgsLayoutSize( 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( layers );
  l.addLayoutItem( map );

  const QDateTime dt1 = QDateTime( QDate( 2010, 1, 1 ), QTime( 0, 0, 0 ) );
  const QDateTime dt2 = QDateTime( QDate( 2020, 1, 1 ), QTime( 0, 0, 0 ) );
  map->setIsTemporal( true );
  map->dataDefinedProperties().setProperty( QgsLayoutObject::StartDateTime, QgsProperty::fromValue( dt1 ) );
  map->dataDefinedProperties().setProperty( QgsLayoutObject::EndDateTime, QgsProperty::fromValue( dt2 ) );
  map->refreshDataDefinedProperty( QgsLayoutObject::StartDateTime );
  map->refreshDataDefinedProperty( QgsLayoutObject::EndDateTime );
  QCOMPARE( map->temporalRange(), QgsDateTimeRange( dt1, dt2, true, false ) );
  const QgsMapSettings ms = map->mapSettings( map->extent(), map->rect().size(), 300, false );
  QCOMPARE( ms.temporalRange(), QgsDateTimeRange( dt1, dt2, true, false ) );
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
  const QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mLinesLayer;
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
  grid->setAnnotationTextFormat( QgsTextFormat::fromQFont( QgsFontUtils::getStandardTestFont( QStringLiteral( "Bold" ) ) ) );
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
  const QList<QgsMapLayer *> layers = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mPointsLayer << mLinesLayer;
  const QList<QgsMapLayer *> layers2 = QList<QgsMapLayer *>() << mRasterLayer << mPolysLayer << mLinesLayer;

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
  const QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
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

  // test that rotation correctly applies to restored items
  QDomDocument doc;
  QDomElement documentElement = doc.createElement( QStringLiteral( "ComposerItemClipboard" ) );
  map->writeXml( documentElement, doc, QgsReadWriteContext() );

  QgsLayoutItemMap map2( &l );
  QVERIFY( map2.readXml( documentElement.firstChildElement(), doc, QgsReadWriteContext() ) );
  QCOMPARE( map2.mapRotation(), 90.0 );
}

void TestQgsLayoutMap::mapItemRotation()
{
  QgsProject p;
  const QFileInfo rasterFileInfo( QStringLiteral( TEST_DATA_DIR ) + "/rgb256x256.png" );
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
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
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

  QgsExpression e5( QStringLiteral( "@map_crs_description" ) );
  r = e5.evaluate( &c );
  QCOMPARE( r.toString(), QString( "WGS 84" ) );

  QgsExpression e6( QStringLiteral( "@map_crs_acronym" ) );
  r = e6.evaluate( &c );
  QCOMPARE( r.toString(), QString( "longlat" ) );

  QgsExpression e6a( QStringLiteral( "@map_crs_projection" ) );
  r = e6a.evaluate( &c );
  QCOMPARE( r.toString(), QString( "Lat/long (Geodetic alias)" ) );

  QgsExpression e7( QStringLiteral( "@map_crs_proj4" ) );
  r = e7.evaluate( &c );
  QCOMPARE( r.toString(), QString( "+proj=longlat +datum=WGS84 +no_defs" ) );

  QgsExpression e8( QStringLiteral( "@map_crs_wkt" ) );
  r = e8.evaluate( &c );
  QVERIFY( r.toString().length() >= 15 );

  QgsExpression e9( QStringLiteral( "@map_crs_ellipsoid" ) );
  r = e9.evaluate( &c );
  QCOMPARE( r.toString(), QString( "EPSG:7030" ) );

  QgsVectorLayer *layer = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "A" ), QStringLiteral( "memory" ) );
  QgsVectorLayer *layer2 = new QgsVectorLayer( QStringLiteral( "Point?field=id_a:integer" ), QStringLiteral( "B" ), QStringLiteral( "memory" ) );
  map->setLayers( QList<QgsMapLayer *>() << layer << layer2 );
  QgsProject::instance()->addMapLayers( map->layers() );
  c = map->createExpressionContext();
  QgsExpression e10( QStringLiteral( "@map_layer_ids" ) );
  r = e10.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "%1,%2" ).arg( layer->id(), layer2->id() ) );
  e10 = QgsExpression( QStringLiteral( "array_foreach(@map_layers, layer_property(@element, 'name'))" ) );
  r = e10.evaluate( &c );
  QCOMPARE( r.toStringList().join( ',' ), QStringLiteral( "A,B" ) );

  QgsExpression e11( QStringLiteral( "is_layer_visible( '%1' )" ).arg( layer->id() ) );
  r = e11.evaluate( &c );
  QCOMPARE( r.toBool(), true );

  QgsExpression e12( QStringLiteral( "is_layer_visible( 'aaaaaa' )" ) );
  r = e12.evaluate( &c );
  QCOMPARE( r.toBool(), false );
}

void TestQgsLayoutMap::layoutToMapCoordsTransform()
{
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );

  QTransform t = map->layoutToMapCoordsTransform();
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).x(), 2000, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).y(), 2900, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).x(), 2500, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).y(), 2900, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).x(), 2000, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).y(), 2800, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).x(), 2500, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).y(), 2800, 1 );

  // with map rotation
  map->setMapRotation( 75 );
  t = map->layoutToMapCoordsTransform();
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).x(), 2136.998947, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).y(), 2621.459496, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).x(), 2266.408470, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).y(), 3104.422409, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).x(), 2233.591530, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).y(), 2595.577591, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).x(), 2363.001053, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).y(), 3078.540504, 1 );

  // with item rotation
  map->setItemRotation( -30 );
  t = map->layoutToMapCoordsTransform();
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).x(), 2037.867966, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 60 ) ).y(), 2708.578644, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).x(), 2391.421356, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 60 ) ).y(), 3062.132034, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).x(), 2108.578644, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 30, 100 ) ).y(), 2637.867966, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).x(), 2462.132034, 1 );
  QGSCOMPARENEAR( t.map( QPointF( 230, 100 ) ).y(), 2991.421356, 1 );
}

void TestQgsLayoutMap::labelBlockingRegions()
{
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );

  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 10, 30, 100, 200 ) );
  l.addLayoutItem( map2 );

  QgsLayoutItemMap *map3 = new QgsLayoutItemMap( &l );
  map3->attemptSetSceneRect( QRectF( 210, 70, 100, 200 ) );
  l.addLayoutItem( map3 );

  QgsLayoutItemMap *map4 = new QgsLayoutItemMap( &l );
  map4->attemptSetSceneRect( QRectF( 210, 70, 100, 200 ) );
  l.addLayoutItem( map4 );

  QVERIFY( !map->isLabelBlockingItem( map2 ) );
  QVERIFY( !map->isLabelBlockingItem( map3 ) );
  QVERIFY( !map->isLabelBlockingItem( map4 ) );
  map->addLabelBlockingItem( map2 );
  QVERIFY( map->isLabelBlockingItem( map2 ) );
  map->addLabelBlockingItem( map3 );
  QVERIFY( map->isLabelBlockingItem( map3 ) );
  map->addLabelBlockingItem( map4 );
  QVERIFY( map->isLabelBlockingItem( map4 ) );
  map->removeLabelBlockingItem( map4 );
  QVERIFY( !map->isLabelBlockingItem( map4 ) );

  QList<QgsLabelBlockingRegion> regions = map->createLabelBlockingRegions( map->mapSettings( map->extent(), map->rect().size(), 300, false ) );
  QCOMPARE( regions.count(), 2 );
  QCOMPARE( regions.at( 0 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((1950 2975, 2200 2975, 2200 2475, 1950 2475, 1950 2975, 1950 2975))" ) );
  QCOMPARE( regions.at( 1 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((2450 2875, 2700 2875, 2700 2375, 2450 2375, 2450 2875, 2450 2875))" ) );
  map->setLabelMargin( QgsLayoutMeasurement( 2, QgsUnitTypes::LayoutCentimeters ) );
  regions = map->createLabelBlockingRegions( map->mapSettings( map->extent(), map->rect().size(), 300, false ) );
  QCOMPARE( regions.count(), 2 );
  QCOMPARE( regions.at( 0 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((1950 2975, 2200 2975, 2200 2475, 1950 2475, 1950 2975, 1950 2975))" ) );
  QCOMPARE( regions.at( 1 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((2450 2875, 2700 2875, 2700 2375, 2450 2375, 2450 2875, 2450 2875))" ) );

  map2->setRotation( 45 );
  regions = map->createLabelBlockingRegions( map->mapSettings( map->extent(), map->rect().size(), 300, false ) );
  QCOMPARE( regions.count(), 2 );
  QCOMPARE( regions.at( 0 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((1950 2975, 2127 2798, 1773 2445, 1596 2621, 1950 2975, 1950 2975))" ) );
  QCOMPARE( regions.at( 1 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((2450 2875, 2700 2875, 2700 2375, 2450 2375, 2450 2875, 2450 2875))" ) );

  regions = map2->createLabelBlockingRegions( map2->mapSettings( map2->extent(), map2->rect().size(), 300, false ) );
  QVERIFY( regions.isEmpty() );

  // invisible items don't block
  map2->setVisibility( false );
  regions = map->createLabelBlockingRegions( map->mapSettings( map->extent(), map->rect().size(), 300, false ) );
  QCOMPARE( regions.count(), 1 );
  QCOMPARE( regions.at( 0 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((2450 2875, 2700 2875, 2700 2375, 2450 2375, 2450 2875, 2450 2875))" ) );

  // but they do if they were previously visible, and just temporarily hidden due to layered export
  map2->setProperty( "wasVisible", true );
  regions = map->createLabelBlockingRegions( map->mapSettings( map->extent(), map->rect().size(), 300, false ) );
  QCOMPARE( regions.count(), 2 );
  QCOMPARE( regions.at( 0 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((1950 2975, 2127 2798, 1773 2445, 1596 2621, 1950 2975, 1950 2975))" ) );
  QCOMPARE( regions.at( 1 ).geometry.asWkt( 0 ), QStringLiteral( "Polygon ((2450 2875, 2700 2875, 2700 2375, 2450 2375, 2450 2875, 2450 2875))" ) );
}

void TestQgsLayoutMap::testSimplificationMethod()
{
  const QgsRectangle extent( 2000, 2800, 2500, 2900 );
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->setCrs( QgsCoordinateReferenceSystem( QStringLiteral( "EPSG:4326" ) ) );
  map->attemptSetSceneRect( QRectF( 30, 60, 200, 100 ) );
  map->setExtent( extent );
  l.addLayoutItem( map );

  l.renderContext().mIsPreviewRender = false;
  QgsMapSettings settings = map->mapSettings( map->extent(), map->rect().size(), 300, false );
  // should default to no simplification during exports
  QCOMPARE( settings.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::NoSimplification );
  QVERIFY( !( settings.flags() & Qgis::MapSettingsFlag::UseRenderingOptimization ) );
  // set a simplification method to use
  QgsVectorSimplifyMethod method;
  method.setSimplifyHints( QgsVectorSimplifyMethod::GeometrySimplification );
  l.renderContext().setSimplifyMethod( method );

  // should still have no simplification override for preview renders
  l.renderContext().mIsPreviewRender = true;
  settings = map->mapSettings( map->extent(), map->rect().size(), 300, false );
  QCOMPARE( settings.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::NoSimplification );
  QVERIFY( settings.flags() & Qgis::MapSettingsFlag::UseRenderingOptimization );

  // for exports, we respect the layout context's simplify method
  l.renderContext().mIsPreviewRender = false;
  settings = map->mapSettings( map->extent(), map->rect().size(), 300, false );
  QCOMPARE( settings.simplifyMethod().simplifyHints(), QgsVectorSimplifyMethod::GeometrySimplification );
  QVERIFY( settings.flags() & Qgis::MapSettingsFlag::UseRenderingOptimization );
}

class TestHandler : public QgsRenderedFeatureHandlerInterface
{
  public:

    TestHandler( QList< QgsFeature > &features, QList< QgsGeometry > &geometries )
      : features( features )
      , geometries( geometries )
    {}

    void handleRenderedFeature( const QgsFeature &feature, const QgsGeometry &geom, const QgsRenderedFeatureHandlerInterface::RenderedFeatureContext & ) override
    {
      features.append( feature );
      geometries.append( geom );
    }

    QList< QgsFeature > &features;
    QList< QgsGeometry > &geometries;

};


void TestQgsLayoutMap::testRenderedFeatureHandler()
{
  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );

  QgsProject p;
  p.addMapLayer( linesLayer );

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( true );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  l.addLayoutItem( map );

  // register a handler
  QList< QgsFeature > features1;
  QList< QgsGeometry > geometries1;
  TestHandler handler1( features1, geometries1 );
  // not added yet, no crash
  map->removeRenderedFeatureHandler( nullptr );
  map->removeRenderedFeatureHandler( &handler1 );
  map->addRenderedFeatureHandler( &handler1 );

  // trigger render
  const QgsLayoutExporter exporter( &l );
  exporter.renderPageToImage( 0 );

  QCOMPARE( features1.count(), 6 );
  QCOMPARE( geometries1.count(), 6 );

  // remove handler
  map->removeRenderedFeatureHandler( &handler1 );
  features1.clear();

  // shouldn't be used anymore
  const QgsLayoutExporter exporter2( &l );
  exporter2.renderPageToImage( 0 );

  QVERIFY( features1.isEmpty() );
}

void TestQgsLayoutMap::testLayeredExport()
{
  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsProject p;
  p.addMapLayer( linesLayer );
  p.addMapLayer( pointsLayer );

  QgsLayout l( &p );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( false );
  map->setBackgroundEnabled( false );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer );
  l.addLayoutItem( map );

  QVERIFY( !map->nextExportPart() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  map->startLayeredExport();
  map->stopLayeredExport();
  QVERIFY( !map->nextExportPart() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  map->stopLayeredExport();

  map->setFrameEnabled( true );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->setBackgroundEnabled( true );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Background" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->setBackgroundEnabled( false );
  map->grid()->setEnabled( true );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->setBackgroundEnabled( false );
  map->overview()->setEnabled( true );
  map->overview()->setStackingPosition( QgsLayoutItemMapItem::StackAboveMapLabels );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overviews" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->overview()->setStackingPosition( QgsLayoutItemMapItem::StackBelowMapLabels );
  QgsLayoutItemMap *map2 = new QgsLayoutItemMap( &l );
  map2->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map2->setFrameEnabled( false );
  map2->setBackgroundEnabled( false );
  map2->setCrs( linesLayer->crs() );
  map2->zoomToExtent( linesLayer->extent() );
  map2->setLayers( QList<QgsMapLayer *>() << linesLayer );
  l.addLayoutItem( map2 );
  map->overview()->setLinkedMap( map2 );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overview" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, map->overview()->mapLayer()->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  map->overview()->setStackingPosition( QgsLayoutItemMapItem::StackAboveMapLabels );

  // add second layer
  map->setLayers( QList<QgsMapLayer *>() << linesLayer << pointsLayer );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overviews" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->setBackgroundEnabled( true );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Background" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overviews" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  // exporting by theme
  QgsMapThemeCollection::MapThemeRecord rec;
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( linesLayer )
                     );

  p.mapThemeCollection()->insert( QStringLiteral( "test preset" ), rec );
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( linesLayer )
                       << QgsMapThemeCollection::MapThemeLayerRecord( pointsLayer )
                     );
  p.mapThemeCollection()->insert( QStringLiteral( "test preset2" ), rec );
  rec.setLayerRecords( QList<QgsMapThemeCollection::MapThemeLayerRecord>()
                       << QgsMapThemeCollection::MapThemeLayerRecord( pointsLayer )
                     );
  p.mapThemeCollection()->insert( QStringLiteral( "test preset3" ), rec );

  l.renderContext().setExportThemes( QStringList() << QStringLiteral( "test preset2" ) << QStringLiteral( "test preset" ) << QStringLiteral( "test preset3" ) );


  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Background" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  // "test preset"
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset): lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  // "test preset 3"
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset3): points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset3" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset3" ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset3): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset3" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset3" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->themeToRender( QgsExpressionContext() ).isEmpty() );

  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overviews" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->overview()->setStackingPosition( QgsLayoutItemMapItem::StackBelowMapLabels );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Background" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): Overview" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, map->overview()->mapLayer()->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset2): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset2" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset2" ) );
  QVERIFY( map->nextExportPart() );
  // "test preset"
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset): lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset): Overview" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, map->overview()->mapLayer()->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  // "test preset 3"
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset3): points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset3" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset3" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset3): Overview" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, map->overview()->mapLayer()->id() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset3" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1 (test preset3): Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QCOMPARE( map->exportLayerDetails().mapTheme, QStringLiteral( "test preset3" ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset3" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->themeToRender( QgsExpressionContext() ).isEmpty() );

  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  map->overview()->setStackingPosition( QgsLayoutItemMapItem::StackAboveMapLabels );

  // but if map is already set to a particular map theme, it DOESN'T follow the theme iteration
  map->setFollowVisibilityPreset( true );
  map->setFollowVisibilityPresetName( QStringLiteral( "test preset" ) );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Background" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // "test preset"
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Labels" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Grids" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QCOMPARE( map->themeToRender( QgsExpressionContext() ), QStringLiteral( "test preset" ) );

  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Overviews" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( map->exportLayerDetails().mapTheme.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();
}

void TestQgsLayoutMap::testLayeredExportLabelsByLayer()
{
  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );
  QgsVectorLayer *pointsLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/points.shp" ),
      QStringLiteral( "points" ), QStringLiteral( "ogr" ) );
  QVERIFY( pointsLayer->isValid() );

  QgsProject p;
  p.addMapLayer( linesLayer );
  p.addMapLayer( pointsLayer );

  QgsLayout l( &p );
  l.renderContext().setFlag( QgsLayoutRenderContext::FlagRenderLabelsByMapLayer );
  l.initializeDefaults();
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( false );
  map->setBackgroundEnabled( false );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer );
  l.addLayoutItem( map );

  QVERIFY( !map->nextExportPart() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  map->startLayeredExport();
  map->stopLayeredExport();
  QVERIFY( !map->nextExportPart() );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  map->stopLayeredExport();

  map->setFrameEnabled( true );
  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // no labels!
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( !map->nextExportPart() );
  map->stopLayeredExport();

  // add second layer
  map->setLayers( QList<QgsMapLayer *>() << linesLayer << pointsLayer );
  // add labels for layers
  QgsPalLayerSettings settings;
  settings.fieldName = QStringLiteral( "Class" );
  settings.zIndex = 1;

  pointsLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  pointsLayer->setLabelsEnabled( true );

  settings.fieldName = QStringLiteral( "Name" );
  settings.placement = Qgis::LabelPlacement::Line;
  settings.zIndex = 3;
  linesLayer->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  linesLayer->setLabelsEnabled( true );

  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points (Labels)" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines (Labels)" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );

  // main annotation layer for project has content, so should be included too
  p.mainAnnotationLayer()->addItem( new QgsAnnotationMarkerItem( QgsPoint( 1, 2 ) ) );

  map->startLayeredExport();
  QVERIFY( map->nextExportPart() );
  map->createStagedRenderJob( map->extent(), QSize( 512, 512 ), 72 );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  // annotations
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Annotations" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, p.mainAnnotationLayer()->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );

  // labels
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: points (Labels)" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, pointsLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: lines (Labels)" ) );
  QCOMPARE( map->exportLayerDetails().mapLayerId, linesLayer->id() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
  QVERIFY( map->nextExportPart() );
  QCOMPARE( map->exportLayerDetails().name, QStringLiteral( "Map 1: Frame" ) );
  QVERIFY( map->exportLayerDetails().mapLayerId.isEmpty() );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Grid ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::OverviewMapExtent ) );
  QVERIFY( map->shouldDrawPart( QgsLayoutItemMap::Frame ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Background ) );
  QVERIFY( !map->shouldDrawPart( QgsLayoutItemMap::Layer ) );
}

void TestQgsLayoutMap::testTemporal()
{
  QgsLayout l( QgsProject::instance( ) );
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  const QDateTime begin( QDate( 2020, 01, 01 ), QTime( 10, 0, 0 ), Qt::UTC );
  const QDateTime end = begin.addSecs( 3600 );

  QgsMapSettings settings = map->mapSettings( map->extent(), QSize( 512, 512 ), 72, false );
  QgsRenderContext renderContext = QgsRenderContext::fromMapSettings( settings );

  QVERIFY( !renderContext.isTemporal() );

  map->setTemporalRange( QgsDateTimeRange( begin, end ) );
  map->refresh();

  settings = map->mapSettings( map->extent(), QSize( 512, 512 ), 72, false );
  renderContext = QgsRenderContext::fromMapSettings( settings );
  QVERIFY( renderContext.isTemporal() );
  QCOMPARE( renderContext.temporalRange(), QgsDateTimeRange( begin, end, true, false ) );
}

void TestQgsLayoutMap::testLabelResults()
{
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );

  // test retrieval of labeling results
  QgsPalLayerSettings settings;

  settings.fieldName = QStringLiteral( "\"id\"" );
  settings.isExpression = true;
  settings.placement = Qgis::LabelPlacement::OverPoint;
  settings.priority = 10;
  settings.placementSettings().setAllowDegradedPlacement( true );
  settings.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::AllowOverlapIfRequired );

  QgsVectorLayer *vl2 = new QgsVectorLayer( QStringLiteral( "Point?crs=epsg:4326&field=id:integer" ), QStringLiteral( "vl" ), QStringLiteral( "memory" ) );

  QgsFeature f;
  f.setAttributes( QgsAttributes() << 1 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -6.250851540391068, 53.335006994584944 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 8888 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -21.950014487179544, 64.150023619739216 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  f.setAttributes( QgsAttributes() << 33333 );
  f.setGeometry( QgsGeometry::fromPointXY( QgsPointXY( -0.118667702475932, 51.5019405883275 ) ) );
  QVERIFY( vl2->dataProvider()->addFeature( f ) );
  vl2->updateExtents();

  p.addMapLayer( vl2 );

  vl2->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl2->setLabelsEnabled( true );

  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( false );
  map->setBackgroundEnabled( false );
  map->setCrs( vl2->crs() );
  map->zoomToExtent( vl2->extent() );
  map->setLayers( QList<QgsMapLayer *>() << vl2 );
  l.addLayoutItem( map );

  l.renderContext().mIsPreviewRender = false;
  QImage im( 600, 600, QImage::Format_ARGB32_Premultiplied );
  QPainter painter( &im );
  map->paint( &painter, nullptr, nullptr );
  painter.end();

  // retrieve label results
  std::unique_ptr< QgsLabelingResults > results = std::move( map->mExportLabelingResults );
  QVERIFY( results );
  QList<QgsLabelPosition> labels = results->allLabels();
  QCOMPARE( labels.count(), 3 );
  std::sort( labels.begin(), labels.end(), []( const QgsLabelPosition & a, const QgsLabelPosition & b )
  {
    return a.labelText.compare( b.labelText ) < 0;
  } );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QVERIFY( !labels.at( 0 ).isUnplaced );
  QCOMPARE( labels.at( 1 ).labelText, QStringLiteral( "33333" ) );
  QVERIFY( !labels.at( 1 ).isUnplaced );
  QCOMPARE( labels.at( 2 ).labelText, QStringLiteral( "8888" ) );
  QVERIFY( !labels.at( 2 ).isUnplaced );

  // with unplaced labels
  QgsVectorLayer *vl3( vl2->clone() );
  p.addMapLayer( vl3 );
  // with unplaced labels -- all vl3 labels will be unplaced, because they are conflicting with those in vl2
  settings.priority = 1;
  settings.placementSettings().setAllowDegradedPlacement( false );
  settings.placementSettings().setOverlapHandling( Qgis::LabelOverlapHandling::PreventOverlap );

  vl3->setLabeling( new QgsVectorLayerSimpleLabeling( settings ) );
  vl3->setLabelsEnabled( true );
  map->setLayers( { vl2, vl3 } );

  painter.begin( &im );
  map->paint( &painter, nullptr, nullptr );
  painter.end();

  results = std::move( map->mExportLabelingResults );
  QVERIFY( results );
  labels = results->allLabels();
  QCOMPARE( labels.count(), 6 );
  std::sort( labels.begin(), labels.end(), []( const QgsLabelPosition & a, const QgsLabelPosition & b )
  {
    return a.isUnplaced == b.isUnplaced ? a.labelText.compare( b.labelText ) < 0 : a.isUnplaced < b.isUnplaced;
  } );
  QCOMPARE( labels.at( 0 ).labelText, QStringLiteral( "1" ) );
  QVERIFY( !labels.at( 0 ).isUnplaced );
  QCOMPARE( labels.at( 1 ).labelText, QStringLiteral( "33333" ) );
  QVERIFY( !labels.at( 1 ).isUnplaced );
  QCOMPARE( labels.at( 2 ).labelText, QStringLiteral( "8888" ) );
  QVERIFY( !labels.at( 2 ).isUnplaced );
  QCOMPARE( labels.at( 3 ).labelText, QStringLiteral( "1" ) );
  QVERIFY( labels.at( 3 ).isUnplaced );
  QCOMPARE( labels.at( 4 ).labelText, QStringLiteral( "33333" ) );
  QVERIFY( labels.at( 4 ).isUnplaced );
  QCOMPARE( labels.at( 5 ).labelText, QStringLiteral( "8888" ) );
  QVERIFY( labels.at( 5 ).isUnplaced );

}

QGSTEST_MAIN( TestQgsLayoutMap )
#include "testqgslayoutmap.moc"
