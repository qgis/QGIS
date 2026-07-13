/***************************************************************************
  testqgslayertreeview.cpp

 ---------------------
 begin                : 26.10.2020
 copyright            : (C) 2020 by Alessandro Pasotti
 email                : elpaso at itopen dot it
 ***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "qgsapplication.h"
#include "qgscategorizedsymbolrenderer.h"
#include "qgscustomdrophandler.h"
#include "qgslayertree.h"
#include "qgslayertreemodel.h"
#include "qgslayertreemodellegendnode.h"
#include "qgslayertreeview.h"
#include "qgsmimedatautils.h"
#include "qgsproviderregistry.h"
#include "qgsprovidersublayerdetails.h"
#include "qgssymbol.h"
#include "qgstest.h"
#include "qgsvectorlayer.h"

#include <QDropEvent>
#include <QMimeData>
#include <QString>
#include <QTemporaryFile>
#include <QUrl>

using namespace Qt::StringLiterals;

//! Test drop handler which claims local files with a given suffix
class TestDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
  public:
    explicit TestDropHandler( const QString &suffix )
      : mSuffix( suffix )
    {}

    bool canHandleMimeData( const QMimeData *data ) override
    {
      const QList<QUrl> urls = data->urls();
      for ( const QUrl &url : urls )
      {
        if ( url.toLocalFile().endsWith( '.' + mSuffix, Qt::CaseInsensitive ) )
          return true;
      }
      return false;
    }

  private:
    QString mSuffix;
};

class TestQgsLayerTreeView : public QObject
{
    Q_OBJECT
  private slots:
    void initTestCase();    // will be called before the first testfunction is executed.
    void cleanupTestCase(); // will be called after the last testfunction was executed.

    void testClassifyDragPayload();
    void testComputeDropTarget();
    void testDatasetDropInsertionPoint();
    void testInvalidDragRefused();
    void testCustomDragAccepted();
    void testFastCheckFalsePositive();
};

void TestQgsLayerTreeView::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();
}

void TestQgsLayerTreeView::cleanupTestCase()
{
  QgsApplication::exitQgis();
}

void TestQgsLayerTreeView::testClassifyDragPayload()
{
  using DragPayloadType = QgsLayerTreeView::DragPayloadType;

  QgsLayerTreeView view;

  // project file url: the file does not need to exist, the extension is enough
  QMimeData projectFileMime;
  projectFileMime.setUrls( { QUrl::fromLocalFile( u"/home/me/project.qgz"_s ) } );
  QCOMPARE( view.classifyDragPayload( &projectFileMime ), DragPayloadType::Project );

  // dataset file url: providers recognize the extension of an existing file
  const QString pointsPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  QVERIFY( QFile::exists( pointsPath ) );
  QMimeData datasetFileMime;
  datasetFileMime.setUrls( { QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( view.classifyDragPayload( &datasetFileMime ), DragPayloadType::Datasets );

  // file with an extension no provider recognizes
  QTemporaryFile docFile( QDir::tempPath() + u"/XXXXXX.docx"_s );
  QVERIFY( docFile.open() );
  QMimeData invalidFileMime;
  invalidFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ) } );
  QCOMPARE( view.classifyDragPayload( &invalidFileMime ), DragPayloadType::Invalid );

  // the fast, extension based check cannot judge extensionless files and directories:
  // they get the benefit of the doubt, the full check happens on drop
  QTemporaryFile extensionlessFile( QDir::tempPath() + u"/XXXXXX"_s );
  QVERIFY( extensionlessFile.open() );
  QMimeData extensionlessFileMime;
  extensionlessFileMime.setUrls( { QUrl::fromLocalFile( extensionlessFile.fileName() ) } );
  QCOMPARE( view.classifyDragPayload( &extensionlessFileMime ), DragPayloadType::Datasets );

  QMimeData directoryMime;
  directoryMime.setUrls( { QUrl::fromLocalFile( QDir::tempPath() ) } );
  QCOMPARE( view.classifyDragPayload( &directoryMime ), DragPayloadType::Datasets );

  // mixed payload: the loadable dataset wins over the unknown file
  QMimeData mixedFileMime;
  mixedFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ), QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( view.classifyDragPayload( &mixedFileMime ), DragPayloadType::Datasets );

  // a project anywhere in the payload takes precedence
  QMimeData projectAndDatasetMime;
  projectAndDatasetMime.setUrls( { QUrl::fromLocalFile( pointsPath ), QUrl::fromLocalFile( u"/home/me/project.qgs"_s ) } );
  QCOMPARE( view.classifyDragPayload( &projectAndDatasetMime ), DragPayloadType::Project );

  // project uri (e.g. a project stored in a geopackage, dragged from the browser)
  QgsMimeDataUtils::Uri projectUri;
  projectUri.layerType = u"project"_s;
  projectUri.uri = u"/home/me/db.gpkg"_s;
  const std::unique_ptr<QMimeData> projectUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << projectUri ) );
  QCOMPARE( view.classifyDragPayload( projectUriMime.get() ), DragPayloadType::Project );

  // layer uri: any non-project uri from QGIS is a loadable dataset
  QgsMimeDataUtils::Uri layerUri;
  layerUri.layerType = u"vector"_s;
  layerUri.providerKey = u"memory"_s;
  layerUri.uri = u"Point?field=fld:integer"_s;
  const std::unique_ptr<QMimeData> layerUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << layerUri ) );
  QCOMPARE( view.classifyDragPayload( layerUriMime.get() ), DragPayloadType::Datasets );

  // layer definition files insert layers into the tree: they classify as datasets
  // and get the insertion indicator
  QTemporaryFile qlrFile( QDir::tempPath() + u"/XXXXXX.qlr"_s );
  QVERIFY( qlrFile.open() );
  QMimeData qlrFileMime;
  qlrFileMime.setUrls( { QUrl::fromLocalFile( qlrFile.fileName() ) } );
  QCOMPARE( view.classifyDragPayload( &qlrFileMime ), DragPayloadType::Datasets );

  // payloads no provider can load, but which the application handles through custom
  // drop handlers, classify as Custom instead of Invalid
  QTemporaryFile qptFile( QDir::tempPath() + u"/XXXXXX.qpt"_s );
  QVERIFY( qptFile.open() );
  QMimeData qptFileMime;
  qptFileMime.setUrls( { QUrl::fromLocalFile( qptFile.fileName() ) } );
  QCOMPARE( view.classifyDragPayload( &qptFileMime ), DragPayloadType::Invalid );

  TestDropHandler qptHandler( u"qpt"_s );
  view.setCustomDropHandlers( { QPointer<QgsCustomDropHandler>( &qptHandler ) } );
  QCOMPARE( view.classifyDragPayload( &qptFileMime ), DragPayloadType::Custom );
  // other unloadable payloads are still refused
  QCOMPARE( view.classifyDragPayload( &invalidFileMime ), DragPayloadType::Invalid );
}

void TestQgsLayerTreeView::testComputeDropTarget()
{
  const std::unique_ptr<QgsVectorLayer> layer1 = std::make_unique<QgsVectorLayer>( u"Point?field=fld:integer"_s, u"layer1"_s, u"memory"_s );
  const std::unique_ptr<QgsVectorLayer> layer2 = std::make_unique<QgsVectorLayer>( u"Point?field=fld:integer"_s, u"layer2"_s, u"memory"_s );
  const std::unique_ptr<QgsVectorLayer> layer3 = std::make_unique<QgsVectorLayer>( u"Point?field=fld:integer"_s, u"layer3"_s, u"memory"_s );
  QVERIFY( layer1->isValid() );

  // give layer1 a multi-item legend, so that its legend nodes occupy tree rows of their own
  QgsCategoryList categories;
  categories << QgsRendererCategory( 1, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"one"_s ) << QgsRendererCategory( 2, QgsSymbol::defaultSymbol( Qgis::GeometryType::Point ), u"two"_s );
  layer1->setRenderer( new QgsCategorizedSymbolRenderer( u"fld"_s, categories ) );

  // root: [ layer1, group[ layer2 ], layer3 ]
  QgsLayerTree root;
  QgsLayerTreeLayer *nodeLayer1 = root.addLayer( layer1.get() );
  QgsLayerTreeGroup *group = root.addGroup( u"group"_s );
  QgsLayerTreeLayer *nodeLayer2 = group->addLayer( layer2.get() );
  root.addLayer( layer3.get() );

  QgsLayerTreeModel model( &root );
  QgsLayerTreeView view;
  view.setModel( &model );
  view.resize( 400, 600 );
  view.show();
  QVERIFY( QTest::qWaitForWindowExposed( &view ) );
  view.expandAll();

  QgsLayerTreeGroup *rootGroup = &root;

  // layer row, top half: insert above the layer
  const QRect layer1Rect = view.visualRect( view.node2index( nodeLayer1 ) );
  QVERIFY( layer1Rect.isValid() );
  QgsLayerTreeView::DropTarget target = view.computeDropTarget( QPoint( layer1Rect.center().x(), layer1Rect.top() + 1 ) );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 0 );
  QVERIFY( !target.into );

  // layer row, bottom half: insert below the layer
  target = view.computeDropTarget( QPoint( layer1Rect.center().x(), layer1Rect.bottom() - 1 ) );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 1 );
  QVERIFY( !target.into );

  // group row, top quarter: insert above the group
  const QRect groupRect = view.visualRect( view.node2index( group ) );
  QVERIFY( groupRect.isValid() );
  target = view.computeDropTarget( QPoint( groupRect.center().x(), groupRect.top() + 1 ) );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 1 );
  QVERIFY( !target.into );

  // group row, middle: insert into the group
  target = view.computeDropTarget( groupRect.center() );
  QCOMPARE( target.group, group );
  QCOMPARE( target.row, 0 );
  QVERIFY( target.into );

  // group row, bottom quarter: insert below the group
  target = view.computeDropTarget( QPoint( groupRect.center().x(), groupRect.bottom() - 1 ) );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 2 );
  QVERIFY( !target.into );

  // layer inside a group, bottom half: insert below the layer, within the group
  const QRect layer2Rect = view.visualRect( view.node2index( nodeLayer2 ) );
  QVERIFY( layer2Rect.isValid() );
  target = view.computeDropTarget( QPoint( layer2Rect.center().x(), layer2Rect.bottom() - 1 ) );
  QCOMPARE( target.group, group );
  QCOMPARE( target.row, 1 );
  QVERIFY( !target.into );

  // legend node row: insert below the layer owning the legend node
  const QList<QgsLayerTreeModelLegendNode *> legendNodes = model.layerLegendNodes( nodeLayer1 );
  QCOMPARE( legendNodes.count(), 2 );
  const QModelIndex legendIndex = view.legendNode2index( legendNodes.first() );
  QVERIFY( legendIndex.isValid() );
  const QRect legendRect = view.visualRect( legendIndex );
  QVERIFY( legendRect.isValid() );
  target = view.computeDropTarget( legendRect.center() );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 1 );
  QVERIFY( !target.into );

  // empty area below the last item: append to the root group
  target = view.computeDropTarget( QPoint( 10, view.viewport()->height() - 1 ) );
  QCOMPARE( target.group, rootGroup );
  QCOMPARE( target.row, 3 );
  QVERIFY( !target.into );
}

void TestQgsLayerTreeView::testDatasetDropInsertionPoint()
{
  const std::unique_ptr<QgsVectorLayer> layer1 = std::make_unique<QgsVectorLayer>( u"Point?field=fld:integer"_s, u"layer1"_s, u"memory"_s );
  const std::unique_ptr<QgsVectorLayer> layer2 = std::make_unique<QgsVectorLayer>( u"Point?field=fld:integer"_s, u"layer2"_s, u"memory"_s );

  QgsLayerTree root;
  root.addLayer( layer1.get() );
  QgsLayerTreeLayer *nodeLayer2 = root.addLayer( layer2.get() );

  QgsLayerTreeModel model( &root );
  QgsLayerTreeView view;
  view.setModel( &model );
  view.resize( 400, 600 );
  view.show();
  QVERIFY( QTest::qWaitForWindowExposed( &view ) );

  // outside a datasetsDropped() handler the insertion point is invalid
  QVERIFY( !view.datasetDropInsertionPoint().group );

  QgsMimeDataUtils::Uri uri;
  uri.layerType = u"vector"_s;
  uri.providerKey = u"memory"_s;
  uri.name = u"dropped"_s;
  uri.uri = u"Point?field=fld:integer"_s;
  const std::unique_ptr<QMimeData> mime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << uri ) );

  QgsLayerTreeRegistryBridge::InsertionPoint captured( nullptr, 0 );
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&captured, &view]( QDropEvent * ) { captured = view.datasetDropInsertionPoint(); } );

  // drive the full drag sequence: the payload is classified and cached on drag enter
  const QRect layer2Rect = view.visualRect( view.node2index( nodeLayer2 ) );
  QVERIFY( layer2Rect.isValid() );
  // lower half of layer2: insert below it
  const QPoint dropPos( layer2Rect.center().x(), layer2Rect.bottom() - 1 );

  QDragEnterEvent enterEvent( dropPos, Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier );
  view.dragEnterEvent( &enterEvent );
  QVERIFY( enterEvent.isAccepted() );

  QDragMoveEvent moveEvent( dropPos, Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier );
  view.dragMoveEvent( &moveEvent );
  QVERIFY( moveEvent.isAccepted() );

  QDropEvent dropEvent( QPointF( dropPos ), Qt::CopyAction, mime.get(), Qt::LeftButton, Qt::NoModifier );
  view.dropEvent( &dropEvent );
  QVERIFY( dropEvent.isAccepted() );

  QCOMPARE( captured.group, static_cast<QgsLayerTreeGroup *>( &root ) );
  QCOMPARE( captured.position, 2 );

  // the insertion point is reset once the handlers have been executed
  QVERIFY( !view.datasetDropInsertionPoint().group );
}

void TestQgsLayerTreeView::testInvalidDragRefused()
{
  QgsLayerTree root;
  QgsLayerTreeModel model( &root );
  QgsLayerTreeView view;
  view.setModel( &model );
  view.resize( 400, 600 );
  view.show();
  QVERIFY( QTest::qWaitForWindowExposed( &view ) );

  QTemporaryFile docFile( QDir::tempPath() + u"/XXXXXX.docx"_s );
  QVERIFY( docFile.open() );
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ) } );

  bool dropped = false;
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&dropped]( QDropEvent * ) { dropped = true; } );

  const QPoint pos( 50, 50 );

  // the drag enter is accepted, so that drag moves keep coming and the overlay is shown
  QDragEnterEvent enterEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dragEnterEvent( &enterEvent );
  QVERIFY( enterEvent.isAccepted() );

  // but the moves are refused, no drop can happen
  QDragMoveEvent moveEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  moveEvent.accept(); // pre-accept to prove the view explicitly refuses it
  view.dragMoveEvent( &moveEvent );
  QVERIFY( !moveEvent.isAccepted() );

  // even if a drop were delivered, it is refused and no handler runs
  QDropEvent dropEvent( QPointF( pos ), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dropEvent( &dropEvent );
  QVERIFY( !dropEvent.isAccepted() );
  QVERIFY( !dropped );
  QVERIFY( !view.datasetDropInsertionPoint().group );
}

void TestQgsLayerTreeView::testCustomDragAccepted()
{
  // payloads claimed by a custom drop handler are accepted and forwarded to the
  // datasetsDropped() signal, but expose no insertion point: the handler decides
  // what to do with the payload, it does not necessarily insert layers
  QgsLayerTree root;
  QgsLayerTreeModel model( &root );
  QgsLayerTreeView view;
  view.setModel( &model );
  view.resize( 400, 600 );
  view.show();
  QVERIFY( QTest::qWaitForWindowExposed( &view ) );

  TestDropHandler qptHandler( u"qpt"_s );
  view.setCustomDropHandlers( { QPointer<QgsCustomDropHandler>( &qptHandler ) } );

  QTemporaryFile qptFile( QDir::tempPath() + u"/XXXXXX.qpt"_s );
  QVERIFY( qptFile.open() );
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( qptFile.fileName() ) } );

  bool dropped = false;
  QgsLayerTreeRegistryBridge::InsertionPoint captured( &root, -1 );
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&dropped, &captured, &view]( QDropEvent * ) {
    dropped = true;
    captured = view.datasetDropInsertionPoint();
  } );

  const QPoint pos( 50, 50 );
  QDragEnterEvent enterEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dragEnterEvent( &enterEvent );
  QVERIFY( enterEvent.isAccepted() );

  QDragMoveEvent moveEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dragMoveEvent( &moveEvent );
  QVERIFY( moveEvent.isAccepted() );

  QDropEvent dropEvent( QPointF( pos ), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dropEvent( &dropEvent );
  QVERIFY( dropEvent.isAccepted() );
  QVERIFY( dropped );
  // no insertion point for custom handler payloads
  QVERIFY( !captured.group );
}

void TestQgsLayerTreeView::testFastCheckFalsePositive()
{
  // A file with a recognized extension but unloadable content fools the fast,
  // extension based check. Such a payload must NOT be refused by the view: the
  // complete open attempt (and its user facing error handling) happens in the
  // datasetsDropped() handler, which must therefore still be reached.
  QTemporaryFile garbageShp( QDir::tempPath() + u"/XXXXXX.shp"_s );
  QVERIFY( garbageShp.open() );
  garbageShp.write( "this is not a shapefile" );
  garbageShp.close();

  QgsLayerTree root;
  QgsLayerTreeModel model( &root );
  QgsLayerTreeView view;
  view.setModel( &model );
  view.resize( 400, 600 );
  view.show();
  QVERIFY( QTest::qWaitForWindowExposed( &view ) );

  // fast check: fooled by the extension
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( garbageShp.fileName() ) } );
  QCOMPARE( view.classifyDragPayload( &mime ), QgsLayerTreeView::DragPayloadType::Datasets );

  // complete check (as performed when actually loading): rejects the file
  QVERIFY( QgsProviderRegistry::instance()->querySublayers( garbageShp.fileName() ).isEmpty() );

  // the drop is accepted by the view and handed over to the datasetsDropped()
  // handler, where the complete check will fail with proper error feedback
  bool dropped = false;
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&dropped]( QDropEvent * ) { dropped = true; } );

  const QPoint pos( 50, 50 );
  QDragEnterEvent enterEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dragEnterEvent( &enterEvent );
  QVERIFY( enterEvent.isAccepted() );

  QDragMoveEvent moveEvent( pos, Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dragMoveEvent( &moveEvent );
  QVERIFY( moveEvent.isAccepted() );

  QDropEvent dropEvent( QPointF( pos ), Qt::CopyAction, &mime, Qt::LeftButton, Qt::NoModifier );
  view.dropEvent( &dropEvent );
  QVERIFY( dropEvent.isAccepted() );
  QVERIFY( dropped );
}

QGSTEST_MAIN( TestQgsLayerTreeView )
#include "testqgslayertreeview.moc"
