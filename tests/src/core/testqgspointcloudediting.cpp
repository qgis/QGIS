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
#include <QString>
#include <QSignalSpy>

//qgis includes...
#include "qgis.h"
#include "qgsapplication.h"
#include "qgspointcloudlayer.h"
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
      : QgsTest( QStringLiteral( "Point Cloud Editing Tests" ), QStringLiteral( "pointcloud_editing" ) ) {}

  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.
    void init() {}          // will be called before each testfunction is executed.
    void cleanup() {}       // will be called after every testfunction.

    void testQgsPointCloudEditingIndex();
    void testStartStopEditing();
    void testModifyAttributeValue();
    void testModifyAttributeValueInvalid();
    void testCommitChanges();
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
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );

  auto i = layer->index();
  QVERIFY( i.isValid() );
  QgsPointCloudEditingIndex e = QgsPointCloudEditingIndex( layer.get() );
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
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
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
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  QgsPointCloudCategoryList categories = QgsPointCloudRendererRegistry::classificationAttributeCategories( layer.get() );
  QgsPointCloudClassifiedRenderer *renderer = new QgsPointCloudClassifiedRenderer( QStringLiteral( "Classification" ), categories );
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
  QgsPointCloudAttribute at( QStringLiteral( "Classification" ), QgsPointCloudAttribute::UChar );
  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QVERIFY( layer->changeAttributeValue( n, { 4, 2, 0, 1, 3, 16, 5, 13, 15, 14 }, at, 1 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 1 );
  QCOMPARE( layer->undoStack()->index(), 1 );

  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_1", "classified_render_edit_1", mapSettings );

  // Change some more
  QVERIFY( layer->changeAttributeValue( n, { 42, 82, 62, 52, 72 }, at, 6 ) );
  QVERIFY( layer->isModified() );
  QCOMPARE( spy.size(), 2 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_2", "classified_render_edit_2", mapSettings );

  // Undo one edit
  layer->undoStack()->undo();
  QCOMPARE( spy.size(), 3 );
  QCOMPARE( layer->undoStack()->index(), 1 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_1", "classified_render_edit_1", mapSettings );

  // Redo edit
  layer->undoStack()->redo();
  QCOMPARE( spy.size(), 4 );
  QCOMPARE( layer->undoStack()->index(), 2 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render_edit_2", "classified_render_edit_2", mapSettings );

  // Abort editing, original points should be rendered
  QVERIFY( layer->rollBack() );
  QVERIFY( !layer->isEditable() );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 5 );
  QCOMPARE( layer->undoStack()->index(), 0 );
  QGSVERIFYRENDERMAPSETTINGSCHECK( "classified_render", "classified_render", mapSettings );
}

void TestQgsPointCloudEditing::testModifyAttributeValueInvalid()
{
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );
  QCOMPARE( layer->undoStack()->index(), 0 );

  QSignalSpy spy( layer.get(), &QgsMapLayer::layerModified );

  // invalid node
  QgsPointCloudAttribute at( QStringLiteral( "Classification" ), QgsPointCloudAttribute::UChar );
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
  at = QgsPointCloudAttribute( QStringLiteral( "X" ), QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  at = QgsPointCloudAttribute( QStringLiteral( "Y" ), QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  at = QgsPointCloudAttribute( QStringLiteral( "Z" ), QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // Wrong attribute size
  at = QgsPointCloudAttribute( QStringLiteral( "Classification" ), QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // Missing attribute
  at = QgsPointCloudAttribute( QStringLiteral( "Foo" ), QgsPointCloudAttribute::Double );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 0 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  // invalid values for standard LAZ attributes
  at = QgsPointCloudAttribute( QStringLiteral( "Intensity" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "ReturnNumber" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 16 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "NumberOfReturns" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 16 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "ScanChannel" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 4 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "ScanDirectionFlag" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "EdgeOfFlightLine" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Classification" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 256 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "UserData" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 256 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "ScanAngle" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -30'001 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 30'001 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "PointSourceId" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "GpsTime" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Synthetic" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Keypoint" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Withheld" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Overlap" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 2 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Red" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Green" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  at = QgsPointCloudAttribute( QStringLiteral( "Blue" ), QgsPointCloudAttribute::UChar );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, -1 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );
  QVERIFY( !layer->changeAttributeValue( n, { 42 }, at, 65536 ) );
  QVERIFY( !layer->isModified() );
  QCOMPARE( spy.size(), 0 );

  QCOMPARE( layer->undoStack()->index(), 0 );
}

void TestQgsPointCloudEditing::testCommitChanges()
{
  const QString dataPath = copyTestData( QStringLiteral( "point_clouds/copc/sunshine-coast.copc.laz" ) );

  std::unique_ptr<QgsPointCloudLayer> layer = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );
  QVERIFY( layer->isValid() );
  QVERIFY( layer->startEditing() );
  QVERIFY( layer->isEditable() );

  QgsPointCloudNodeId n( 0, 0, 0, 0 );
  QgsPointCloudAttribute at( QStringLiteral( "Classification" ), QgsPointCloudAttribute::UChar );

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

  // check values after change, before committing
  std::unique_ptr<QgsPointCloudBlock> block1 = layer->index().nodeData( n, request );
  const char *block1Data = block1->data();
  QCOMPARE( block1Data[0], 1 );
  QCOMPARE( block1Data[6], 2 );  // unchanged
  QCOMPARE( block1Data[11], 3 ); // unchanged
  QCOMPARE( block1Data[14], 1 );

  QVERIFY( layer->commitChanges() );
  QVERIFY( !layer->isModified() );

  // check values after committing changes
  std::unique_ptr<QgsPointCloudBlock> block2 = layer->index().nodeData( n, request );
  const char *block2Data = block2->data();
  QCOMPARE( block2Data[0], 1 );
  QCOMPARE( block2Data[6], 2 );  // unchanged
  QCOMPARE( block2Data[11], 3 ); // unchanged
  QCOMPARE( block2Data[14], 1 );

  // try to open the file as a new layer and check saved values
  std::unique_ptr<QgsPointCloudLayer> layerNew = std::make_unique<QgsPointCloudLayer>( dataPath, QStringLiteral( "layer" ), QStringLiteral( "copc" ) );

  // check values in the new layer
  std::unique_ptr<QgsPointCloudBlock> block3 = layerNew->index().nodeData( n, request );
  const char *block3Data = block3->data();
  QCOMPARE( block3Data[0], 1 );
  QCOMPARE( block3Data[6], 2 );  // unchanged
  QCOMPARE( block3Data[11], 3 ); // unchanged
  QCOMPARE( block3Data[14], 1 );
}

QGSTEST_MAIN( TestQgsPointCloudEditing )
#include "testqgspointcloudediting.moc"
