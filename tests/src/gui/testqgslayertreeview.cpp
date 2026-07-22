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
#include "qgslayerdropclassifier.h"
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

//! Test drop handler which declares (via canHandleMimeData) local files with a given suffix
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

//! Test drop handler mimicking a legacy handler which only implements handleFileDrop()
class LegacyDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
    // does not reimplement canHandleMimeData(), so it relies on the base default (TRUE)
    // and claims any payload, for backward compatibility
};

//! Test drop handler which claims a custom uri provider key (browser custom uri drops)
class CustomUriDropHandler : public QgsCustomDropHandler
{
    Q_OBJECT
  public:
    QString customUriProviderKey() const override { return u"test_custom"_s; }
};

//! A layer tree view on its own tree and model, sized and shown, ready to receive events
struct LayerTreeViewFixture
{
    QgsLayerTree root;
    QgsLayerTreeModel model;
    QgsLayerTreeView view;
    //! FALSE if the view could not be exposed; tests must QVERIFY this
    bool ready = false;

    LayerTreeViewFixture()
      : model( &root )
    {
      view.setModel( &model );
      view.resize( 400, 600 );
      view.show();
      ready = QTest::qWaitForWindowExposed( &view );
    }
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

  private:
    //! Which events of a drag & drop sequence the view accepted
    struct DragDropResult
    {
        bool enterAccepted = false;
        bool moveAccepted = false;
        bool dropAccepted = false;
    };

    //! Drives a full drag enter / move / drop sequence on the view
    DragDropResult performDragAndDrop( QgsLayerTreeView &view, const QMimeData *mime, const QPoint &pos );
};

TestQgsLayerTreeView::DragDropResult TestQgsLayerTreeView::performDragAndDrop( QgsLayerTreeView &view, const QMimeData *mime, const QPoint &pos )
{
  DragDropResult result;

  QDragEnterEvent enterEvent( pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier );
  view.dragEnterEvent( &enterEvent );
  result.enterAccepted = enterEvent.isAccepted();

  QDragMoveEvent moveEvent( pos, Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier );
  moveEvent.accept(); // pre-accept, so that an explicit refusal by the view is observable
  view.dragMoveEvent( &moveEvent );
  result.moveAccepted = moveEvent.isAccepted();

  // always deliver the drop, even after a refused move: the view must refuse it too
  QDropEvent dropEvent( QPointF( pos ), Qt::CopyAction, mime, Qt::LeftButton, Qt::NoModifier );
  view.dropEvent( &dropEvent );
  result.dropAccepted = dropEvent.isAccepted();

  return result;
}

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
  using PayloadType = Qgis::LayerDropPayloadType;

  QVector<QPointer<QgsCustomDropHandler>> noHandlers;

  // project file url: the file does not need to exist, the extension is enough
  QMimeData projectFileMime;
  projectFileMime.setUrls( { QUrl::fromLocalFile( u"/home/me/project.qgz"_s ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &projectFileMime, noHandlers ), PayloadType::Project );

  // dataset file url: providers recognize the extension of an existing file
  const QString pointsPath = QStringLiteral( TEST_DATA_DIR ) + u"/points.shp"_s;
  QVERIFY( QFile::exists( pointsPath ) );
  QMimeData datasetFileMime;
  datasetFileMime.setUrls( { QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &datasetFileMime, noHandlers ), PayloadType::Layers );

  // file with an extension no provider recognizes
  QTemporaryFile docFile( QDir::tempPath() + u"/XXXXXX.docx"_s );
  QVERIFY( docFile.open() );
  QMimeData invalidFileMime;
  invalidFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, noHandlers ), PayloadType::Invalid );

  // the fast, extension based check cannot judge extensionless files and directories:
  // they get the benefit of the doubt, the full check happens on drop
  QTemporaryFile extensionlessFile( QDir::tempPath() + u"/XXXXXX"_s );
  QVERIFY( extensionlessFile.open() );
  QMimeData extensionlessFileMime;
  extensionlessFileMime.setUrls( { QUrl::fromLocalFile( extensionlessFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &extensionlessFileMime, noHandlers ), PayloadType::Layers );

  QMimeData directoryMime;
  directoryMime.setUrls( { QUrl::fromLocalFile( QDir::tempPath() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &directoryMime, noHandlers ), PayloadType::Layers );

  // mixed payload: the loadable dataset wins over the unknown file
  QMimeData mixedFileMime;
  mixedFileMime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ), QUrl::fromLocalFile( pointsPath ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &mixedFileMime, noHandlers ), PayloadType::Layers );

  // a project anywhere in the payload takes precedence
  QMimeData projectAndDatasetMime;
  projectAndDatasetMime.setUrls( { QUrl::fromLocalFile( pointsPath ), QUrl::fromLocalFile( u"/home/me/project.qgs"_s ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &projectAndDatasetMime, noHandlers ), PayloadType::Project );

  // project uri (e.g. a project stored in a geopackage, dragged from the browser)
  QgsMimeDataUtils::Uri projectUri;
  projectUri.layerType = u"project"_s;
  projectUri.uri = u"/home/me/db.gpkg"_s;
  const std::unique_ptr<QMimeData> projectUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << projectUri ) );
  QCOMPARE( QgsLayerDropClassifier::classify( projectUriMime.get(), noHandlers ), PayloadType::Project );

  // layer uri: any non-project uri from QGIS is a loadable dataset
  QgsMimeDataUtils::Uri layerUri;
  layerUri.layerType = u"vector"_s;
  layerUri.providerKey = u"memory"_s;
  layerUri.uri = u"Point?field=fld:integer"_s;
  const std::unique_ptr<QMimeData> layerUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << layerUri ) );
  QCOMPARE( QgsLayerDropClassifier::classify( layerUriMime.get(), noHandlers ), PayloadType::Layers );

  // layer definition files insert layers into the tree: they classify as layers
  // and get the insertion indicator
  QTemporaryFile qlrFile( QDir::tempPath() + u"/XXXXXX.qlr"_s );
  QVERIFY( qlrFile.open() );
  QMimeData qlrFileMime;
  qlrFileMime.setUrls( { QUrl::fromLocalFile( qlrFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &qlrFileMime, noHandlers ), PayloadType::Layers );

  // payloads no provider can load, but which the application handles through custom
  // drop handlers, classify as CustomHandler instead of Invalid
  QTemporaryFile qptFile( QDir::tempPath() + u"/XXXXXX.qpt"_s );
  QVERIFY( qptFile.open() );
  QMimeData qptFileMime;
  qptFileMime.setUrls( { QUrl::fromLocalFile( qptFile.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &qptFileMime, noHandlers ), PayloadType::Invalid );

  TestDropHandler qptHandler( u"qpt"_s );
  const QVector<QPointer<QgsCustomDropHandler>> handlers { QPointer<QgsCustomDropHandler>( &qptHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( &qptFileMime, handlers ), PayloadType::CustomHandler );
  // other unloadable payloads are still refused: a modern handler declares its
  // capabilities via canHandleMimeData() and opts out of unrecognized file drops
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, handlers ), PayloadType::Invalid );

  // a legacy handler which does not reimplement canHandleMimeData() relies on the base
  // default (TRUE): for backward compatibility it claims any payload, so an otherwise
  // unloadable drop is accepted as a custom handler payload rather than refused
  LegacyDropHandler legacyHandler;
  const QVector<QPointer<QgsCustomDropHandler>> legacyHandlers { QPointer<QgsCustomDropHandler>( &legacyHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( &invalidFileMime, legacyHandlers ), PayloadType::CustomHandler );

  // a custom uri (e.g. a Processing model dragged from the browser) is dispatched to a
  // matching custom drop handler via handleCustomUriDrop(); it must not be classified as
  // a layer (which would show a misleading insertion indicator) but as CustomHandler
  QgsMimeDataUtils::Uri customUri;
  customUri.layerType = u"custom"_s;
  customUri.providerKey = u"test_custom"_s;
  customUri.uri = u"some_model"_s;
  const std::unique_ptr<QMimeData> customUriMime( QgsMimeDataUtils::encodeUriList( QgsMimeDataUtils::UriList() << customUri ) );

  // without a handler claiming its provider key, the custom uri cannot be handled at all
  QCOMPARE( QgsLayerDropClassifier::classify( customUriMime.get(), noHandlers ), PayloadType::Invalid );

  CustomUriDropHandler customUriHandler;
  const QVector<QPointer<QgsCustomDropHandler>> customUriHandlers { QPointer<QgsCustomDropHandler>( &customUriHandler ) };
  QCOMPARE( QgsLayerDropClassifier::classify( customUriMime.get(), customUriHandlers ), PayloadType::CustomHandler );
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

  LayerTreeViewFixture f;
  QVERIFY( f.ready );
  QgsLayerTreeView &view = f.view;

  // root: [ layer1, group[ layer2 ], layer3 ]
  QgsLayerTreeLayer *nodeLayer1 = f.root.addLayer( layer1.get() );
  QgsLayerTreeGroup *group = f.root.addGroup( u"group"_s );
  QgsLayerTreeLayer *nodeLayer2 = group->addLayer( layer2.get() );
  f.root.addLayer( layer3.get() );

  view.expandAll();

  QgsLayerTreeGroup *rootGroup = &f.root;

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
  const QList<QgsLayerTreeModelLegendNode *> legendNodes = f.model.layerLegendNodes( nodeLayer1 );
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

  LayerTreeViewFixture f;
  QVERIFY( f.ready );
  QgsLayerTreeView &view = f.view;

  f.root.addLayer( layer1.get() );
  QgsLayerTreeLayer *nodeLayer2 = f.root.addLayer( layer2.get() );

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

  const DragDropResult result = performDragAndDrop( view, mime.get(), dropPos );
  QVERIFY( result.enterAccepted );
  QVERIFY( result.moveAccepted );
  QVERIFY( result.dropAccepted );

  QCOMPARE( captured.group, static_cast<QgsLayerTreeGroup *>( &f.root ) );
  QCOMPARE( captured.position, 2 );

  // the insertion point is reset once the handlers have been executed
  QVERIFY( !view.datasetDropInsertionPoint().group );
}

void TestQgsLayerTreeView::testInvalidDragRefused()
{
  LayerTreeViewFixture f;
  QVERIFY( f.ready );

  QTemporaryFile docFile( QDir::tempPath() + u"/XXXXXX.docx"_s );
  QVERIFY( docFile.open() );
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( docFile.fileName() ) } );

  bool dropped = false;
  connect( &f.view, &QgsLayerTreeView::datasetsDropped, this, [&dropped]( QDropEvent * ) { dropped = true; } );

  const DragDropResult result = performDragAndDrop( f.view, &mime, QPoint( 50, 50 ) );
  // the drag enter is accepted, so that drag moves keep coming and the overlay is shown
  QVERIFY( result.enterAccepted );
  // but the moves are explicitly refused, no drop can happen
  QVERIFY( !result.moveAccepted );
  // even if a drop were delivered, it is refused and no handler runs
  QVERIFY( !result.dropAccepted );
  QVERIFY( !dropped );
  QVERIFY( !f.view.datasetDropInsertionPoint().group );
}

void TestQgsLayerTreeView::testCustomDragAccepted()
{
  // payloads claimed by a custom drop handler are accepted and forwarded to the
  // datasetsDropped() signal, but expose no insertion point: the handler decides
  // what to do with the payload, it does not necessarily insert layers
  LayerTreeViewFixture f;
  QVERIFY( f.ready );
  QgsLayerTreeView &view = f.view;

  TestDropHandler qptHandler( u"qpt"_s );
  view.setCustomDropHandlers( { QPointer<QgsCustomDropHandler>( &qptHandler ) } );

  QTemporaryFile qptFile( QDir::tempPath() + u"/XXXXXX.qpt"_s );
  QVERIFY( qptFile.open() );
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( qptFile.fileName() ) } );

  bool dropped = false;
  QgsLayerTreeRegistryBridge::InsertionPoint captured( &f.root, -1 );
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&dropped, &captured, &view]( QDropEvent * ) {
    dropped = true;
    captured = view.datasetDropInsertionPoint();
  } );

  const DragDropResult result = performDragAndDrop( view, &mime, QPoint( 50, 50 ) );
  QVERIFY( result.enterAccepted );
  QVERIFY( result.moveAccepted );
  QVERIFY( result.dropAccepted );
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

  LayerTreeViewFixture f;
  QVERIFY( f.ready );
  QgsLayerTreeView &view = f.view;

  // fast check: fooled by the extension
  QMimeData mime;
  mime.setUrls( { QUrl::fromLocalFile( garbageShp.fileName() ) } );
  QCOMPARE( QgsLayerDropClassifier::classify( &mime ), Qgis::LayerDropPayloadType::Layers );

  // complete check (as performed when actually loading): rejects the file
  QVERIFY( QgsProviderRegistry::instance()->querySublayers( garbageShp.fileName() ).isEmpty() );

  // the drop is accepted by the view and handed over to the datasetsDropped()
  // handler, where the complete check will fail with proper error feedback
  bool dropped = false;
  connect( &view, &QgsLayerTreeView::datasetsDropped, this, [&dropped]( QDropEvent * ) { dropped = true; } );

  const DragDropResult result = performDragAndDrop( view, &mime, QPoint( 50, 50 ) );
  QVERIFY( result.enterAccepted );
  QVERIFY( result.moveAccepted );
  QVERIFY( result.dropAccepted );
  QVERIFY( dropped );
}

QGSTEST_MAIN( TestQgsLayerTreeView )
#include "testqgslayertreeview.moc"
