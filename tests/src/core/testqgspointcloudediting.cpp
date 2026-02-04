/***************************************************************************
     testqgspointcloudediting.cpp
     --------------------------------------
    Date                 : December 2024
    Copyright            : (C) 2024 by Stefanos Natsis
    Email                : uclaros at gmail dot com
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgspointcloudrendererregistry.h"
#include "qgstest.h"

#include <QSignalSpy>
#include <QString>

using namespace Qt::StringLiterals;

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgspointcloudlayer.h"
#include "qgsvirtualpointcloudprovider.h"
#include "qgspointcloudindex.h"
#include "qgscopcpointcloudindex.h"
#include "qgspointcloudeditingindex.h"

/**
 * \ingroup UnitTests
 * This is a unit test for point cloud editing
 */
class TestQgsPointCloudEditing : public QgsTest
{
    Q_OBJECT

  public:
    TestQgsPointCloudEditing()
      : QgsTest( u"Point Cloud Editing Tests"_s, u"pointcloud_editing"_s ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testQgsPointCloudEditingIndex();
    void testStartStopEditing();
    void testModifyAttributeValue();
    void testModifyAttributeValueInvalid();
    void testModifyAttributeValueFiltered();
    void testCommitChanges();

    void testVPCStarStopEditing();
    void testVPCModifyAttributeValue();
    void testVPCModifyAttributeValueInvalid();
    void testVPCModifyAttributeValueFiltered();
    void testVPCCommitChanges();
};

//runs before all tests
void TestQgsPointCloudEditing::initTestCase()
{
  // init QGIS's paths - true means that all path will be inited from prefix
  QgsApplication::init();
  QgsApplication::initQgis();
}

//runs after all tests
void TestQgsPointCloudEditing::cleanupTestCase()
{
  QgsApplication::exitQgis();
}


void TestQgsPointCloudEditing::testQgsPointCloudEditingIndex()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  auto i = layer->index();
  QVERIFY( i.isValid() );
  QgsPointCloudEditingIndex e = QgsPointCloudEditingIndex( i );
  QVERIFY( e.isValid() );
  QCOMPARE( i.accessType(), e.accessType() );
  QCOMPARE( i.crs(), e.crs() );
  QCOMPARE( i.pointCount(), e.pointCount() );
  QCOMPARE( i.originalMetadata(), e.originalMetadata() );
  QCOMPARE( i.attributes().count(), e.attributes().count() );
  QCOMPARE( i.attributes().pointRecordSize(), e.attributes().pointRecordSize() );
  QCOMPARE( i.extent(), e.extent() );
  QCOMPARE( i.offset(), e.offset() );
  QCOMPARE( i.scale(), e.scale() );
  QCOMPARE( i.span(), e.span() );
  QCOMPARE( i.zMax(), e.zMax() );
  QCOMPARE( i.zMin(), e.zMin() );
  QCOMPARE( i.root(), e.root() );
  QCOMPARE( i.rootNodeBounds(), e.rootNodeBounds() );
}

void TestQgsPointCloudEditing::testStartStopEditing()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );

  QVERIFY( layer->index() );
  QSignalSpy spyStart( layer.get(), &QgsMapLayer::editingStarted );
  QSignalSpy spyStop( layer.get(), &QgsMapLayer::editingStopped );
  QSignalSpy spyModify( layer.get(), &QgsMapLayer::layerModified );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 0 );
  QCOMPARE( spyModify.size(), 0 );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( layer->index().mIndex.get() ) );

  // false if already editing
  QVERIFY( !layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 0 );
  QCOMPARE( spyModify.size(), 0 );

  // stop editing
  QVERIFY( layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( layer->index().mIndex.get() ) );

  // false if already stopped
  QVERIFY( !layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );

  // start again
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 2 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( layer->index().mIndex.get() ) );

  // commit and stop editing
  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 2 );
  QCOMPARE( spyStop.size(), 2 );
  QCOMPARE( spyModify.size(), 0 );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( layer->index().mIndex.get() ) );
}

void TestQgsPointCloudEditing::testModifyAttributeValue()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );
  QSignalSpy spyChunkChanged( layer.get(), &QgsPointCloudLayer::chunkAttributeValuesChanged );

  QgsPointCloudCategoryList categories = QgsPointCloudRendererRegistry::classificationAttributeCategories( layer.get() );
  QgsPointCloudClassifiedRenderer *renderer = new QgsPointCloudClassifiedRenderer( u"Classification"_s, categories );
  layer->setRenderer( renderer );

  layer->renderer()->setPointSize( 2 );
  layer->renderer()->setPointSizeUnit( Qgis::RenderUnit::Millimeters );

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( layer->crs() );
  mapSettings.setExtent( QgsRectangle( 498061, 7050991, 498069, 7050999 ) );
  mapSettings.setLayers( { layer.get() } );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render", "classified_render", mapSettings );

  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );

  // Change some points, point order should not matter
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QVERIFY( layer->changeAttributeValue( n, { 4, 2, 0, 1, 3, 16, 5, 13, 15, 14 }, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 1 );
  QCOMPARE( spyChunkChanged.size(), 1 );
  QCOMPARE( layer->undoStack()->index(), 1 );

  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_1", "classified_render_edit_1", mapSettings );

  // Change some more
  QVERIFY( layer->changeAttributeValue( n, { 42, 82, 62, 52, 72 }, at, 6 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 2 );
  QCOMPARE( spyChunkChanged.size(), 2 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_2", "classified_render_edit_2", mapSettings );

  // Undo one edit
  layer->undoStack()->undo();
  QCOMPARE( spy.size(), 3 );
  QCOMPARE( spyChunkChanged.size(), 3 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_1", "classified_render_edit_1", mapSettings );

  // Undo second edit
  layer->undoStack()->undo();
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 4 );
  QCOMPARE( spyChunkChanged.size(), 4 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render", "classified_render", mapSettings );

  // Redo first edit
  layer->undoStack()->redo();
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 5 );
  QCOMPARE( spyChunkChanged.size(), 5 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_1", "classified_render_edit_1", mapSettings );

  // Redo second edit
  layer->undoStack()->redo();
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 6 );
  QCOMPARE( spyChunkChanged.size(), 6 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_2", "classified_render_edit_2", mapSettings );

  // Abort editing, original points should be rendered
  QVERIFY( layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 7 );
  QCOMPARE( spyChunkChanged.size(), 7 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render", "classified_render", mapSettings );
}

void TestQgsPointCloudEditing::testModifyAttributeValueInvalid()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  // invalid node
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n;
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // missing node
  n = QgsPointCloudNodeId( 1, 1, 1, 1 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid point ids
  n = QgsPointCloudNodeId( 0, 0, 0, 0 );
  QVERIFY( !layer->changeAttributeValue( n, { -1, 42 }, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  QVERIFY( !layer->changeAttributeValue( n, { 42, 420 }, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid attribute, X,Y,Z are read only
  at = QgsPointCloudAttribute( u"X"_s, QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  at = QgsPointCloudAttribute( u"Y"_s, QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  at = QgsPointCloudAttribute( u"Z"_s, QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // Wrong attribute size
  at = QgsPointCloudAttribute( u"Classification"_s, QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // Missing attribute
  at = QgsPointCloudAttribute( u"Foo"_s, QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid values for standard LAZ attributes
  at = QgsPointCloudAttribute( u"Intensity"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"ReturnNumber"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 16 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"NumberOfReturns"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 16 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"ScanChannel"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 4 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"ScanDirectionFlag"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"EdgeOfFlightLine"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 256 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"UserData"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 256 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"ScanAngle"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -30'001 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 30'001 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"PointSourceId"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"GpsTime"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Synthetic"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Keypoint"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Withheld"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Overlap"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Red"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Green"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( u"Blue"_s, QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  QCOMPARE( layer->undoStack()->index(), 0 );
}

void TestQgsPointCloudEditing::testModifyAttributeValueFiltered()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  QgsPointCloudCategoryList categories = QgsPointCloudRendererRegistry::classificationAttributeCategories( layer.get() );
  QgsPointCloudClassifiedRenderer *renderer = new QgsPointCloudClassifiedRenderer( u"Classification"_s, categories );
  layer->setRenderer( renderer );

  layer->renderer()->setPointSize( 2 );
  layer->renderer()->setPointSizeUnit( Qgis::RenderUnit::Millimeters );

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( layer->crs() );
  mapSettings.setExtent( QgsRectangle( 498061, 7050991, 498069, 7050999 ) );
  mapSettings.setLayers( { layer.get() } );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render", "classified_render", mapSettings );

  // Set a filter
  QVERIFY( layer->setSubsetString( u"Classification != 3"_s ) );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_filtered", "classified_render_filtered", mapSettings );

  // Change some points, some where filtered out
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QVERIFY( layer->changeAttributeValue( n, { 42, 82, 62, 52, 72 }, at, 6 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 1 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_filtered_edit", "classified_render_filtered_edit", mapSettings );

  // Commit changes and clear filter
  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isModified() );
  QVERIFY( layer->setSubsetString( QString() ) );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_filtered_edit_saved", "classified_render_filtered_edit_saved", mapSettings );
}

void TestQgsPointCloudEditing::testCommitChanges()
{
  const QString dataPath = copyTestData( u"point_clouds/copc/sunshine-coast.copc.laz"_s );

  auto layer = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );
  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );

  QgsPointCloudRequest request;
  request.setAttributes( QgsPointCloudAttributeCollection( QVector<QgsPointCloudAttribute>() << at ) );

  // check values before any changes
  std::unique_ptr<QgsPointCloudBlock> block0 = layer->index().nodeData( n, request );
  const char *block0Data = block0->data();
  QCOMPARE( block0Data[0], 2 );
  QCOMPARE( block0Data[6], 2 );
  QCOMPARE( block0Data[11], 3 );
  QCOMPARE( block0Data[14], 3 );

  // Change some points, point order should not matter
  QVERIFY( layer->changeAttributeValue( n, { 4, 2, 0, 1, 3, 16, 5, 13, 15, 14 }, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QCOMPARE( layer->undoStack()->count(), 1 );
  QCOMPARE( spy.size(), 1 );

  // check values after change, before committing
  std::unique_ptr<QgsPointCloudBlock> block1 = layer->index().nodeData( n, request );
  const char *block1Data = block1->data();
  QCOMPARE( block1Data[0], 1 );
  QCOMPARE( block1Data[6], 2 );  // unchanged
  QCOMPARE( block1Data[11], 3 ); // unchanged
  QCOMPARE( block1Data[14], 1 );

  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QCOMPARE( layer->undoStack()->count(), 0 );
  QCOMPARE( spy.size(), 2 );

  // check values after committing changes
  std::unique_ptr<QgsPointCloudBlock> block2 = layer->index().nodeData( n, request );
  const char *block2Data = block2->data();
  QCOMPARE( block2Data[0], 1 );
  QCOMPARE( block2Data[6], 2 );  // unchanged
  QCOMPARE( block2Data[11], 3 ); // unchanged
  QCOMPARE( block2Data[14], 1 );

  // try to open the file as a new layer and check saved values
  auto layerNew = std::make_unique<QgsPointCloudLayer>( dataPath, u"layer"_s, u"copc"_s );

  // check values in the new layer
  std::unique_ptr<QgsPointCloudBlock> block3 = layerNew->index().nodeData( n, request );
  const char *block3Data = block3->data();
  QCOMPARE( block3Data[0], 1 );
  QCOMPARE( block3Data[6], 2 );  // unchanged
  QCOMPARE( block3Data[11], 3 ); // unchanged
  QCOMPARE( block3Data[14], 1 );
}

void TestQgsPointCloudEditing::testVPCStarStopEditing()
{
  const QString dataPath = copyTestDataDirectory( u"point_clouds/virtual/sunshine-coast"_s );
  const QString vpcPath = dataPath + u"/combined.vpc"_s;

  auto layer = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  QVERIFY( layer->isValid() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QVERIFY( !layer->index() );
  QVERIFY( layer->isVpc() );

  layer->dataProvider()->loadSubIndex( 2 );
  layer->dataProvider()->loadSubIndex( 3 );

  QSignalSpy spyStart( layer.get(), &QgsMapLayer::editingStarted );
  QSignalSpy spyStop( layer.get(), &QgsMapLayer::editingStopped );
  QSignalSpy spyModify( layer.get(), &QgsMapLayer::layerModified );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 0 );
  QCOMPARE( spyModify.size(), 0 );

  QVector< QgsPointCloudSubIndex > subIndexes = layer->subIndexes();
  QVERIFY( !subIndexes.at( 0 ).index().isValid() );
  QVERIFY( !subIndexes.at( 1 ).index().isValid() );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 2 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 3 ).index().get() ) );

  // false if already editing
  QVERIFY( !layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 0 );
  QCOMPARE( spyModify.size(), 0 );

  // stop editing
  QVERIFY( layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );

  subIndexes = layer->subIndexes();
  QVERIFY( !subIndexes.at( 0 ).index().isValid() );
  QVERIFY( !subIndexes.at( 1 ).index().isValid() );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( subIndexes.at( 2 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( subIndexes.at( 3 ).index().get() ) );

  // false if already stopped
  QVERIFY( !layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 1 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );

  // start again
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 2 );
  QCOMPARE( spyStop.size(), 1 );
  QCOMPARE( spyModify.size(), 0 );

  subIndexes = layer->subIndexes();
  QVERIFY( !subIndexes.at( 0 ).index().isValid() );
  QVERIFY( !subIndexes.at( 1 ).index().isValid() );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 2 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 3 ).index().get() ) );

  // test the creation of editing index while open for editing
  layer->dataProvider()->loadSubIndex( 1 );
  subIndexes = layer->subIndexes();
  QVERIFY( !subIndexes.at( 0 ).index().isValid() );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 1 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 2 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsPointCloudEditingIndex *>( subIndexes.at( 3 ).index().get() ) );

  // commit and stop editing
  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spyStart.size(), 2 );
  QCOMPARE( spyStop.size(), 2 );
  QCOMPARE( spyModify.size(), 0 );

  subIndexes = layer->subIndexes();
  QVERIFY( !subIndexes.at( 0 ).index().isValid() );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( subIndexes.at( 1 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( subIndexes.at( 2 ).index().get() ) );
  QVERIFY( dynamic_cast<QgsCopcPointCloudIndex *>( subIndexes.at( 3 ).index().get() ) );
}

void TestQgsPointCloudEditing::testVPCModifyAttributeValue()
{
  const QString dataPath = copyTestDataDirectory( u"point_clouds/virtual/sunshine-coast"_s );
  const QString vpcPath = dataPath + u"/combined.vpc"_s;

  auto layer = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  for ( int i = 0; i < 4; i++ )
  {
    layer->dataProvider()->loadSubIndex( i );
  }

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );
  QSignalSpy spyChunkChanged( layer.get(), &QgsPointCloudLayer::chunkAttributeValuesChanged );

  QgsPointCloudCategoryList categories = QgsPointCloudRendererRegistry::classificationAttributeCategories( layer.get() );
  QgsPointCloudClassifiedRenderer *renderer = new QgsPointCloudClassifiedRenderer( u"Classification"_s, categories );
  layer->setRenderer( renderer );

  layer->renderer()->setPointSize( 2 );
  layer->renderer()->setPointSizeUnit( Qgis::RenderUnit::Millimeters );

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( layer->crs() );
  mapSettings.setExtent( layer->subIndexes().at( 3 ).extent() );
  mapSettings.setLayers( { layer.get() } );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_subindex3", "classified_render_vpc_subindex3", mapSettings );

  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );

  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QHash<QgsPointCloudNodeId, QVector<int> > points;
  points.insert( n, { 4, 2, 0, 1, 3, 10, 12, 20, 22 } );
  QHash<int, QHash<QgsPointCloudNodeId, QVector<int>>> mappedPoints;
  mappedPoints.insert( 3, points );
  QVERIFY( layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 1 );
  QCOMPARE( spyChunkChanged.size(), 1 );
  QCOMPARE( layer->undoStack()->index(), 1 );

  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_subindex3_edit", "classified_render_vpc_subindex3_edit", mapSettings );

  // Undo changes to subindex 3
  layer->undoStack()->undo();
  QVERIFY( !layer->isModified() ); // not correct
  QCOMPARE( spy.size(), 2 );
  QCOMPARE( spyChunkChanged.size(), 2 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_subindex3_unmodified", "classified_render_vpc_subindex3_unmodified", mapSettings );

  // change attributes from multiple indexes
  mapSettings.setExtent( QgsRectangle( 498064.5, 7050995.4, 498065.5, 7050996.3 ) );
  points.insert( n, { 40 } );
  mappedPoints.insert( 2, points );
  points.insert( n, { 0, 2 } );
  mappedPoints.insert( 3, points );
  points.insert( n, { 13 } );
  mappedPoints.insert( 1, points );
  QVERIFY( layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 3 );
  QCOMPARE( spyChunkChanged.size(), 5 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edit_1", "classified_render_vpc_all_indexes_edit_1", mapSettings );

  // change some more, from multiple indexes
  mapSettings.setExtent( QgsRectangle( 498064.5, 7050995.4, 498065.5, 7050996.3 ) );
  points.insert( n, { 64, 66 } );
  mappedPoints.insert( 0, points );
  points.insert( n, { 45 } );
  mappedPoints.insert( 2, points );
  points.insert( n, { 37 } );
  mappedPoints.insert( 3, points );
  points.insert( n, { 39 } );
  mappedPoints.insert( 1, points );
  QVERIFY( layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 4 );
  QCOMPARE( spyChunkChanged.size(), 9 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edit_2", "classified_render_vpc_all_indexes_edit_2", mapSettings );

  // Undo one edit
  layer->undoStack()->undo();
  QCOMPARE( spy.size(), 5 );
  QCOMPARE( spyChunkChanged.size(), 13 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edit_1", "classified_render_vpc_all_indexes_edit_1", mapSettings );

  // Undo second edit
  layer->undoStack()->undo();
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 6 );
  QCOMPARE( spyChunkChanged.size(), 16 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_unmodified", "classified_render_vpc_all_indexes_unmodified", mapSettings );

  // Redo first edit
  layer->undoStack()->redo();
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 7 );
  QCOMPARE( spyChunkChanged.size(), 19 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edit_1", "classified_render_vpc_all_indexes_edit_1", mapSettings );

  // Redo second edit
  layer->undoStack()->redo();
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 8 );
  QCOMPARE( spyChunkChanged.size(), 23 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edit_2", "classified_render_vpc_all_indexes_edit_2", mapSettings );

  // Abort editing, original points should be rendered
  QVERIFY( layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 9 );
  QCOMPARE( spyChunkChanged.size(), 27 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_unmodified", "classified_render_vpc_all_indexes_unmodified", mapSettings );
}

void TestQgsPointCloudEditing::testVPCModifyAttributeValueInvalid()
{
  const QString dataPath = copyTestDataDirectory( u"point_clouds/virtual/sunshine-coast"_s );
  const QString vpcPath = dataPath + u"/combined.vpc"_s;

  auto layer = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  // load only two, keep the other two not loaded for further tests
  layer->dataProvider()->loadSubIndex( 0 );
  layer->dataProvider()->loadSubIndex( 1 );

  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n;

  // invalid node
  QHash<QgsPointCloudNodeId, QVector<int> > points;
  points.insert( n, { 4, 2, 0, 1, 3, 10, 12, 20, 22 } );
  QHash<int, QHash<QgsPointCloudNodeId, QVector<int>>> mappedPoints;
  mappedPoints.insert( 0, points );

  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // missing node
  n = QgsPointCloudNodeId( 1, 1, 1, 1 );
  points.clear();
  mappedPoints.clear();
  points.insert( n, { 42 } );
  mappedPoints.insert( 0, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid subindex position
  n = QgsPointCloudNodeId( 0, 0, 0, 0 );
  points.clear();
  mappedPoints.clear();
  points.insert( n, { 1 } );
  mappedPoints.insert( -1, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid point ids
  n = QgsPointCloudNodeId( 0, 0, 0, 0 );
  points.clear();
  mappedPoints.clear();
  points.insert( n, { -1, 1 } );
  mappedPoints.insert( 1, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // valid point, not yet loaded subindex
  n = QgsPointCloudNodeId( 0, 0, 0, 0 );
  points.clear();
  mappedPoints.clear();
  points.insert( n, { 1 } );
  mappedPoints.insert( 2, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // loaded and not yet loaded subindex
  mappedPoints.insert( 0, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // Missing attribute
  at = QgsPointCloudAttribute( u"Foo"_s, QgsPointCloudAttribute::Double );
  mappedPoints.clear();
  mappedPoints.insert( 0, points );
  QVERIFY( !layer->changeAttributeValue( mappedPoints, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
}

void TestQgsPointCloudEditing::testVPCModifyAttributeValueFiltered()
{
  const QString dataPath = copyTestDataDirectory( u"point_clouds/virtual/sunshine-coast/"_s );
  const QString vpcPath = dataPath + u"/combined.vpc"_s;

  auto layer = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );
  QVERIFY( layer->isValid() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  layer->dataProvider()->loadSubIndex( 0 );
  layer->dataProvider()->loadSubIndex( 1 );
  layer->dataProvider()->loadSubIndex( 2 );
  layer->dataProvider()->loadSubIndex( 3 );

  QgsPointCloudCategoryList categories = QgsPointCloudRendererRegistry::classificationAttributeCategories( layer.get() );
  QgsPointCloudClassifiedRenderer *renderer = new QgsPointCloudClassifiedRenderer( u"Classification"_s, categories );
  layer->setRenderer( renderer );

  layer->renderer()->setPointSize( 2 );
  layer->renderer()->setPointSizeUnit( Qgis::RenderUnit::Millimeters );

  QgsMapSettings mapSettings;
  mapSettings.setOutputSize( QSize( 400, 400 ) );
  mapSettings.setOutputDpi( 96 );
  mapSettings.setDestinationCrs( layer->crs() );
  mapSettings.setExtent( QgsRectangle( 498064.5, 7050995.4, 498065.5, 7050996.3 ) );
  mapSettings.setLayers( { layer.get() } );

  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_unmodified", "classified_render_vpc_all_indexes_unmodified", mapSettings );

  QVERIFY( layer->setSubsetString( u"Classification != 3"_s ) );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_filtered", "classified_render_vpc_all_indexes_filtered", mapSettings );

  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QHash<QgsPointCloudNodeId, QVector<int> > points;
  points.insert( n, { 4, 2, 0, 1, 3, 10, 12, 20, 22 } );
  QHash<int, QHash<QgsPointCloudNodeId, QVector<int>>> mappedPoints;
  mappedPoints.insert( 0, points );
  QVERIFY( layer->changeAttributeValue( mappedPoints, at, 6 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 1 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_filtered", "classified_render_vpc_all_indexes_filtered", mapSettings ); // need an image

  // Commit changes and clear filter
  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isModified() );
  QVERIFY( layer->setSubsetString( QString() ) );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_vpc_all_indexes_edited_saved", "classified_render_vpc_all_indexes_edited_saved", mapSettings );
}

void TestQgsPointCloudEditing::testVPCCommitChanges()
{
  const QString dataPath = copyTestDataDirectory( u"point_clouds/virtual/sunshine-coast"_s );
  const QString vpcPath = dataPath + u"/combined.vpc"_s;

  auto layer = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QgsPointCloudAttribute at( u"Classification"_s, QgsPointCloudAttribute::UChar );

  QgsPointCloudRequest request;
  request.setAttributes( QgsPointCloudAttributeCollection( QVector<QgsPointCloudAttribute>() << at ) );

  const QgsPointCloudSubIndex subIndex0 = layer->subIndexes().at( 0 );
  QVERIFY( !subIndex0.index() );

  layer->dataProvider()->loadSubIndex( 0 );
  QVERIFY( layer->subIndexes().at( 0 ).index().isValid() );
  // check values before any changes
  std::unique_ptr<QgsPointCloudBlock> block0 = layer->subIndexes().at( 0 ).index().nodeData( n, request );

  const char *block0Data = block0->data();
  QCOMPARE( block0Data[0], 2 ); // will be edited to 1
  QCOMPARE( block0Data[1], 2 ); // will be edited to 1
  QCOMPARE( block0Data[2], 2 ); // will be edited to 1
  QCOMPARE( block0Data[3], 2 );
  QCOMPARE( block0Data[4], 2 ); // will be edited to 1
  QCOMPARE( block0Data[5], 2 );
  QCOMPARE( block0Data[6], 2 );
  QCOMPARE( block0Data[7], 2 );
  QCOMPARE( block0Data[8], 2 );
  QCOMPARE( block0Data[9], 1 );
  QCOMPARE( block0Data[10], 3 ); // will be edited to 1

  // Change some points, point order should not matter
  QHash<QgsPointCloudNodeId, QVector<int> > points;
  points.insert( n, { 4, 2, 0, 1, 9, 10, 12, 20, 22 } );
  QHash<int, QHash<QgsPointCloudNodeId, QVector<int>>> mappedPoints;
  mappedPoints.insert( 0, points );
  QVERIFY( layer->changeAttributeValue( mappedPoints, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QCOMPARE( layer->undoStack()->count(), 1 );
  QCOMPARE( spy.size(), 1 );

  // check values after change, before committing
  std::unique_ptr<QgsPointCloudBlock> block1 = layer->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block1Data = block1->data();

  QCOMPARE( block1Data[0], 1 ); // edited to 1
  QCOMPARE( block1Data[1], 1 ); // edited to 1
  QCOMPARE( block1Data[2], 1 ); // edited to 1
  QCOMPARE( block1Data[3], 2 );
  QCOMPARE( block1Data[4], 1 ); // edited to 1
  QCOMPARE( block1Data[5], 2 );
  QCOMPARE( block1Data[6], 2 );
  QCOMPARE( block1Data[7], 2 );
  QCOMPARE( block1Data[8], 2 );
  QCOMPARE( block1Data[9], 1 );
  QCOMPARE( block1Data[10], 1 ); // edited to 1

  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QCOMPARE( layer->undoStack()->count(), 0 );
  QCOMPARE( spy.size(), 2 );

  // check values after committing changes
  std::unique_ptr<QgsPointCloudBlock> block2 = layer->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block2Data = block2->data();

  QCOMPARE( block2Data[0], 1 );
  QCOMPARE( block2Data[1], 1 );
  QCOMPARE( block2Data[2], 1 );
  QCOMPARE( block2Data[3], 2 );
  QCOMPARE( block2Data[4], 1 );
  QCOMPARE( block2Data[5], 2 );
  QCOMPARE( block2Data[6], 2 );
  QCOMPARE( block2Data[7], 2 );
  QCOMPARE( block2Data[8], 2 );
  QCOMPARE( block2Data[9], 1 );
  QCOMPARE( block2Data[10], 1 );

  // try to open the file as a new layer and check saved values
  auto layerNew = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  QSignalSpy spy2( layerNew.get(), &QgsMapLayer::layerModified );

  // check values in the new layer
  layerNew->dataProvider()->loadSubIndex( 0 );
  std::unique_ptr<QgsPointCloudBlock> block3 = layerNew->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block3Data = block3->data();

  QCOMPARE( block3Data[0], 1 ); // will be edited to 3
  QCOMPARE( block3Data[1], 1 );
  QCOMPARE( block3Data[2], 1 ); // will be edited to 3
  QCOMPARE( block3Data[3], 2 );
  QCOMPARE( block3Data[4], 1 );
  QCOMPARE( block3Data[5], 2 ); // will be edited to 3
  QCOMPARE( block3Data[6], 2 );
  QCOMPARE( block3Data[7], 2 );
  QCOMPARE( block3Data[8], 2 );
  QCOMPARE( block3Data[9], 1 ); // will be edited to 3
  QCOMPARE( block3Data[10], 1 );

  // modify values from multiple indexes, but first check original values (above and below)
  points.insert( n, { 2, 0, 9, 5, 20, 22 } );
  mappedPoints.insert( 0, points );

  layerNew->dataProvider()->loadSubIndex( 1 );
  std::unique_ptr<QgsPointCloudBlock> block4 = layerNew->subIndexes().at( 1 ).index().nodeData( n, request );
  const char *block4Data = block4->data();

  QCOMPARE( block4Data[0], 2 );
  QCOMPARE( block4Data[1], 2 ); // will be edited to 3
  QCOMPARE( block4Data[2], 2 ); // will be edited to 3
  QCOMPARE( block4Data[3], 3 );
  QCOMPARE( block4Data[4], 2 ); // will be edited to 3
  QCOMPARE( block4Data[5], 2 ); // will be edited to 3
  QCOMPARE( block4Data[6], 2 );
  QCOMPARE( block4Data[7], 2 ); // will be edited to 3
  QCOMPARE( block4Data[8], 2 );
  QCOMPARE( block4Data[9], 2 );
  QCOMPARE( block4Data[10], 2 );

  points.insert( n, { 2, 1, 4, 5, 7, 20, 22 } );
  mappedPoints.insert( 1, points );
  QVERIFY( layerNew->isValid() );
  QVERIFY( layerNew->startEditing() );
  QVERIFY( layerNew->isEditable() );
  QVERIFY( layerNew->changeAttributeValue( mappedPoints, at, 3 ) );
  QVERIFY( layerNew->isModified() );
  QCOMPARE( layerNew->undoStack()->index(), 1 );
  QCOMPARE( layerNew->undoStack()->count(), 1 );
  QCOMPARE( spy2.size(), 1 );

  // check values after modifying
  std::unique_ptr<QgsPointCloudBlock> block5 = layerNew->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block5Data = block5->data();
  std::unique_ptr<QgsPointCloudBlock> block6 = layerNew->subIndexes().at( 1 ).index().nodeData( n, request );
  const char *block6Data = block6->data();

  QCOMPARE( block5Data[0], 3 ); // edited to 3
  QCOMPARE( block5Data[1], 1 );
  QCOMPARE( block5Data[2], 3 ); // edited to 3
  QCOMPARE( block5Data[3], 2 );
  QCOMPARE( block5Data[4], 1 );
  QCOMPARE( block5Data[5], 3 ); // edited to 3
  QCOMPARE( block5Data[6], 2 );
  QCOMPARE( block5Data[7], 2 );
  QCOMPARE( block5Data[8], 2 );
  QCOMPARE( block5Data[9], 3 ); // edited to 3
  QCOMPARE( block5Data[10], 1 );

  QCOMPARE( block6Data[0], 2 );
  QCOMPARE( block6Data[1], 3 ); // edited to 3
  QCOMPARE( block6Data[2], 3 ); // edited to 3
  QCOMPARE( block6Data[3], 3 );
  QCOMPARE( block6Data[4], 3 ); // edited to 3
  QCOMPARE( block6Data[5], 3 ); // edited to 3
  QCOMPARE( block6Data[6], 2 );
  QCOMPARE( block6Data[7], 3 ); // edited to 3
  QCOMPARE( block6Data[8], 2 );
  QCOMPARE( block6Data[9], 2 );
  QCOMPARE( block6Data[10], 2 );

  QVERIFY( layerNew->commitChanges() );
  QVERIFY( !layerNew->isModified() );
  QCOMPARE( layerNew->undoStack()->index(), 0 );
  QCOMPARE( layerNew->undoStack()->count(), 0 );
  QCOMPARE( spy2.size(), 2 );

  std::unique_ptr<QgsPointCloudBlock> block7 = layerNew->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block7Data = block7->data();
  std::unique_ptr<QgsPointCloudBlock> block8 = layerNew->subIndexes().at( 1 ).index().nodeData( n, request );
  const char *block8Data = block8->data();

  QCOMPARE( block7Data[0], 3 );
  QCOMPARE( block7Data[1], 1 );
  QCOMPARE( block7Data[2], 3 );
  QCOMPARE( block7Data[3], 2 );
  QCOMPARE( block7Data[4], 1 );
  QCOMPARE( block7Data[5], 3 );
  QCOMPARE( block7Data[6], 2 );
  QCOMPARE( block7Data[7], 2 );
  QCOMPARE( block7Data[8], 2 );
  QCOMPARE( block7Data[9], 3 );
  QCOMPARE( block7Data[10], 1 );

  QCOMPARE( block8Data[0], 2 );
  QCOMPARE( block8Data[1], 3 );
  QCOMPARE( block8Data[2], 3 );
  QCOMPARE( block8Data[3], 3 );
  QCOMPARE( block8Data[4], 3 );
  QCOMPARE( block8Data[5], 3 );
  QCOMPARE( block8Data[6], 2 );
  QCOMPARE( block8Data[7], 3 );
  QCOMPARE( block8Data[8], 2 );
  QCOMPARE( block8Data[9], 2 );
  QCOMPARE( block8Data[10], 2 );

  // try to open the file as a new layer and check saved values
  auto layerNew2 = std::make_unique<QgsPointCloudLayer>( vpcPath, u"layer"_s, u"vpc"_s );

  // check values in the new layer
  layerNew2->dataProvider()->loadSubIndex( 0 );
  layerNew2->dataProvider()->loadSubIndex( 1 );
  std::unique_ptr<QgsPointCloudBlock> block9 = layerNew2->subIndexes().at( 0 ).index().nodeData( n, request );
  const char *block9Data = block9->data();
  std::unique_ptr<QgsPointCloudBlock> block10 = layerNew2->subIndexes().at( 1 ).index().nodeData( n, request );
  const char *block10Data = block10->data();

  QCOMPARE( block9Data[0], 3 );
  QCOMPARE( block9Data[1], 1 );
  QCOMPARE( block9Data[2], 3 );
  QCOMPARE( block9Data[3], 2 );
  QCOMPARE( block9Data[4], 1 );
  QCOMPARE( block9Data[5], 3 );
  QCOMPARE( block9Data[6], 2 );
  QCOMPARE( block9Data[7], 2 );
  QCOMPARE( block9Data[8], 2 );
  QCOMPARE( block9Data[9], 3 );
  QCOMPARE( block9Data[10], 1 );

  QCOMPARE( block10Data[0], 2 );
  QCOMPARE( block10Data[1], 3 );
  QCOMPARE( block10Data[2], 3 );
  QCOMPARE( block10Data[3], 3 );
  QCOMPARE( block10Data[4], 3 );
  QCOMPARE( block10Data[5], 3 );
  QCOMPARE( block10Data[6], 2 );
  QCOMPARE( block10Data[7], 3 );
  QCOMPARE( block10Data[8], 2 );
  QCOMPARE( block10Data[9], 2 );
  QCOMPARE( block10Data[10], 2 );
}

QGSTEST_MAIN( TestQgsPointCloudEditing )
#include "testqgspointcloudediting.moc"
