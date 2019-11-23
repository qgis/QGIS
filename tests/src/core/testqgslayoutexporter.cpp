/***************************************************************************
                         testqgslayoutexporter.cpp
                         -----------------
    begin                : August 2019
    copyright            : (C) 2019 by Nyall Dawson
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

#include "qgslayout.h"
#include "qgstest.h"
#include "qgsproject.h"
#include "qgslayoutexporter.h"
#include "qgslayoutitemlabel.h"
#include "qgslayoutitemshape.h"
#include "qgslayoutitemscalebar.h"
#include "qgslayoutitemmap.h"
#include "qgsvectorlayer.h"
#include "qgslayoutitemlegend.h"

class TestQgsLayoutExporter: public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void testHandleLayeredExport();

};

void TestQgsLayoutExporter::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayoutExporter::cleanupTestCase()
{
}

void TestQgsLayoutExporter::init()
{

}

void TestQgsLayoutExporter::cleanup()
{

}

void TestQgsLayoutExporter::testHandleLayeredExport()
{
  QgsProject p;
  QgsLayout l( &p );
  QgsLayoutExporter exporter( &l );

  QList< unsigned int > layerIds;
  QStringList layerNames;
  QStringList mapLayerIds;
  QgsLayout *layout = &l;
  auto exportFunc = [&layerIds, &layerNames, &mapLayerIds, layout]( unsigned int layerId, const QgsLayoutItem::ExportLayerDetail & layerDetail )->QgsLayoutExporter::ExportResult
  {
    layerIds << layerId;
    layerNames << layerDetail.name;
    mapLayerIds << layerDetail.mapLayerId;
    QImage im( 512, 512, QImage::Format_ARGB32_Premultiplied );
    QPainter p( &im );
    layout->render( &p );
    p.end();

    return QgsLayoutExporter::Success;
  };

  QList< QGraphicsItem * > items;
  QgsLayoutExporter::ExportResult res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QVERIFY( layerIds.isEmpty() );
  QVERIFY( layerNames.isEmpty() );
  QVERIFY( mapLayerIds.isEmpty() );

  // add two pages to a layout
  QgsLayoutItemPage *page1 = new QgsLayoutItemPage( &l );
  items << page1;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Page" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemPage *page2 = new QgsLayoutItemPage( &l );
  items << page2;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemLabel *label = new QgsLayoutItemLabel( &l );
  items << label;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Label" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemShape *shape = new QgsLayoutItemShape( &l );
  items << shape;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Label, Shape" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemLabel *label2 = new QgsLayoutItemLabel( &l );
  items << label2;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  // add an item which can only be used with other similar items, should break the next label into a different layer
  QgsLayoutItemScaleBar *scaleBar = new QgsLayoutItemScaleBar( &l );
  items << scaleBar;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemLabel *label3 = new QgsLayoutItemLabel( &l );
  items << label3;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  // multiple scalebars can be placed in the same layer
  QgsLayoutItemScaleBar *scaleBar2 = new QgsLayoutItemScaleBar( &l );
  items << scaleBar2;
  QgsLayoutItemScaleBar *scaleBar3 = new QgsLayoutItemScaleBar( &l );
  items << scaleBar3;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  // with an item which has sublayers
  QgsVectorLayer *linesLayer = new QgsVectorLayer( TEST_DATA_DIR + QStringLiteral( "/lines.shp" ),
      QStringLiteral( "lines" ), QStringLiteral( "ogr" ) );
  QVERIFY( linesLayer->isValid() );

  p.addMapLayer( linesLayer );

  QgsLayoutItemMap *map = new QgsLayoutItemMap( &l );
  map->attemptSetSceneRect( QRectF( 20, 20, 200, 100 ) );
  map->setFrameEnabled( false );
  map->setBackgroundEnabled( false );
  map->setCrs( linesLayer->crs() );
  map->zoomToExtent( linesLayer->extent() );
  map->setLayers( QList<QgsMapLayer *>() << linesLayer );

  items << map;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 << 6 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) << QStringLiteral( "Map 1: lines" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() << linesLayer->id() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  map->setFrameEnabled( true );
  map->setBackgroundEnabled( true );
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) << QStringLiteral( "Map 1: Background" ) << QStringLiteral( "Map 1: lines" ) << QStringLiteral( "Map 1: Frame" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() << QString() << linesLayer->id() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  // add two legends -- legends are complex and must be placed in an isolated layer
  QgsLayoutItemLegend *legend = new QgsLayoutItemLegend( &l );
  legend->setId( QStringLiteral( "my legend" ) );
  QgsLayoutItemLegend *legend2 = new QgsLayoutItemLegend( &l );
  legend2->setId( QStringLiteral( "my legend 2" ) );
  items << legend << legend2;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) << QStringLiteral( "Map 1: Background" ) << QStringLiteral( "Map 1: lines" ) << QStringLiteral( "Map 1: Frame" ) << QStringLiteral( "my legend" ) << QStringLiteral( "my legend 2" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() << QString() << linesLayer->id() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemLabel *label4 = new QgsLayoutItemLabel( &l );
  items << label4;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) << QStringLiteral( "Map 1: Background" ) << QStringLiteral( "Map 1: lines" ) << QStringLiteral( "Map 1: Frame" ) << QStringLiteral( "my legend" ) << QStringLiteral( "my legend 2" ) << QStringLiteral( "Label" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() << QString() << linesLayer->id() << QString() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  QgsLayoutItemLabel *label5 = new QgsLayoutItemLabel( &l );
  items << label5;
  res = exporter.handleLayeredExport( items, exportFunc );
  QCOMPARE( res, QgsLayoutExporter::Success );
  QCOMPARE( layerIds, QList< unsigned int >() << 1 << 2 << 3 << 4 << 5 << 6 << 7 << 8 << 9 << 10 << 11 );
  QCOMPARE( layerNames, QStringList() << QStringLiteral( "Pages" ) << QStringLiteral( "Labels, Shape" ) << QStringLiteral( "Scalebar" ) << QStringLiteral( "Label" ) << QStringLiteral( "Scalebars" ) << QStringLiteral( "Map 1: Background" ) << QStringLiteral( "Map 1: lines" ) << QStringLiteral( "Map 1: Frame" ) << QStringLiteral( "my legend" ) << QStringLiteral( "my legend 2" ) << QStringLiteral( "Labels" ) );
  QCOMPARE( mapLayerIds, QStringList() << QString() << QString() << QString() << QString() << QString() << QString() << linesLayer->id() << QString() << QString() << QString() << QString() );
  layerIds.clear();
  layerNames.clear();
  mapLayerIds.clear();

  qDeleteAll( items );
}

QGSTEST_MAIN( TestQgsLayoutExporter )
#include "testqgslayoutexporter.moc"
