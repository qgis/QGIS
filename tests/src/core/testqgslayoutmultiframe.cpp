/***************************************************************************
                         testqgslayoutmultiframe.cpp
                         ---------------------------
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

#include "qgslayoutframe.h"
#include "qgslayoutmultiframe.h"
#include "qgslayoutitemlabel.h"
#include "qgslayout.h"
#include "qgsmultirenderchecker.h"
#include "qgsapplication.h"
#include "qgsproject.h"
#include "qgslayoutitemhtml.h"
#include "qgslayoutpagecollection.h"
#include "qgslayoutundostack.h"
#include "qgsreadwritecontext.h"
#include "qgsexpressioncontextutils.h"

#include <QObject>
#include <QSignalSpy>

#include "qgstest.h"

class TestQgsLayoutMultiFrame : public QObject
{
    Q_OBJECT

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void layoutMethods();
    void addFrame(); //test creating new frame inherits all properties of existing frame
    void displayName();
    void frameIsEmpty(); //test if frame is empty works
    void addRemovePage(); //test if page is added and removed for RepeatUntilFinished mode
    void undoRedo(); //test that combinations of frame/multiframe undo/redo don't crash
    void undoRedoRemovedFrame(); //test that undo doesn't crash with removed frames
    void undoRedoRemovedFrame2();
    void registry();
    void deleteFrame();
    void writeReadXml();
    void noPageNoCrash();
    void variables();

  private:
    QgsLayout *mLayout = nullptr;
    QString mReport;
};

class TestMultiFrame : public QgsLayoutMultiFrame
{
    Q_OBJECT

  public:

    TestMultiFrame( QgsLayout *layout )
      : QgsLayoutMultiFrame( layout )
    {

    }

    int type() const override
    {

      return QgsLayoutItemRegistry::PluginItem + 1;
    }

    void render( QgsLayoutItemRenderContext &, const QRectF &, int ) override
    {

    }

    QSizeF totalSize() const override
    {
      return QSizeF();
    }

};

void TestQgsLayoutMultiFrame::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  mLayout = new QgsLayout( QgsProject::instance() );
  mLayout->initializeDefaults();

  mReport = QStringLiteral( "<h1>Layout MultiFrame Tests</h1>\n" );
}

void TestQgsLayoutMultiFrame::cleanupTestCase()
{
  delete mLayout;

  const QString myReportFile = QDir::tempPath() + "/qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }

  QgsApplication::exitQgis();
}

void TestQgsLayoutMultiFrame::init()
{

}

void TestQgsLayoutMultiFrame::cleanup()
{

}

void TestQgsLayoutMultiFrame::layoutMethods()
{
  QgsLayout *l = new QgsLayout( QgsProject::instance() );
  QVERIFY( l->multiFrames().empty() );

  l->addMultiFrame( nullptr );
  QVERIFY( l->multiFrames().empty() );
  TestMultiFrame *mF = new TestMultiFrame( l );
  const QPointer< TestMultiFrame > pMF( mF );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );
  QVERIFY( !l->multiFrameByUuid( QString() ) );
  QCOMPARE( l->multiFrameByUuid( mF->uuid() ), mF );

  // try readding
  l->addMultiFrame( mF );
  QCOMPARE( l->multiFrames().count(), 1 );

  l->removeMultiFrame( nullptr );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );

  TestMultiFrame *mF2 = new TestMultiFrame( l );
  const QPointer< TestMultiFrame > pMF2( mF2 );
  l->addMultiFrame( mF2 );
  QCOMPARE( l->multiFrames().count(), 2 );
  QVERIFY( l->multiFrames().contains( mF ) );
  QVERIFY( l->multiFrames().contains( mF2 ) );
  QCOMPARE( l->multiFrameByUuid( mF2->uuid() ), mF2 );
  l->removeMultiFrame( mF2 );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );
  QVERIFY( !l->multiFrameByUuid( mF2->uuid() ) );

  // should not be deleted
  QVERIFY( pMF2 );
  delete mF2;
  QVERIFY( !pMF2 );
  delete l;
  QVERIFY( !pMF );
}

void TestQgsLayoutMultiFrame::addFrame()
{
  TestMultiFrame *multiframe = new TestMultiFrame( mLayout );
  QCOMPARE( multiframe->frameCount(), 0 );
  QVERIFY( multiframe->frames().empty() );
  QVERIFY( !multiframe->frame( -1 ) );
  QVERIFY( !multiframe->frame( 0 ) );
  QVERIFY( !multiframe->frame( 1 ) );
  QCOMPARE( multiframe->frameIndex( nullptr ), -1 );

  multiframe->addFrame( nullptr );
  QCOMPARE( multiframe->frameCount(), 0 );
  QVERIFY( multiframe->frames().empty() );

  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, multiframe );
  QCOMPARE( multiframe->frameIndex( frame1 ), -1 );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  multiframe->addFrame( frame1 );

  QCOMPARE( multiframe->frameCount(), 1 );
  QCOMPARE( multiframe->frames().count(), 1 );
  QVERIFY( multiframe->frames().contains( frame1 ) );
  QVERIFY( !multiframe->frame( -1 ) );
  QCOMPARE( multiframe->frame( 0 ), frame1 );
  QVERIFY( !multiframe->frame( 1 ) );
  QCOMPARE( multiframe->frameIndex( frame1 ), 0 );
  QCOMPARE( frame1->multiFrame(), multiframe );

  // deferred set multiframe
  QgsLayoutFrame *frame1a = new QgsLayoutFrame( mLayout, nullptr );
  QVERIFY( !frame1a->multiFrame() );
  multiframe->addFrame( frame1a );
  QCOMPARE( frame1a->multiFrame(), multiframe );

  //should not be inherited
  frame1->setHidePageIfEmpty( true );

  //should be inherited
  frame1->setHideBackgroundIfEmpty( true );
  frame1->setFrameStrokeWidth( QgsLayoutMeasurement( 5.0 ) );
  frame1->setFrameJoinStyle( Qt::RoundJoin );
  frame1->setFrameEnabled( true );
  frame1->setFrameStrokeColor( QColor( Qt::red ) );
  frame1->setBackgroundEnabled( true );
  frame1->setBackgroundColor( QColor( Qt::green ) );
  frame1->setBlendMode( QPainter::CompositionMode_ColorBurn );
  frame1->setItemOpacity( 0.5 );

  QgsLayoutFrame *frame2 = multiframe->createNewFrame( frame1, QPointF( 50, 55 ), QSizeF( 70, 120 ) );

  //check frame created in correct place
  QCOMPARE( frame2->rect().height(), 120.0 );
  QCOMPARE( frame2->rect().width(), 70.0 );
  QCOMPARE( frame2->scenePos().x(), 50.0 );
  QCOMPARE( frame2->scenePos().y(), 55.0 );

  //check frame properties
  QCOMPARE( frame2->frameStrokeWidth(), frame1->frameStrokeWidth() );
  QCOMPARE( frame2->frameStrokeColor(), frame1->frameStrokeColor() );
  QCOMPARE( frame2->frameJoinStyle(), frame1->frameJoinStyle() );
  QCOMPARE( frame2->hasBackground(), frame1->hasBackground() );
  QCOMPARE( frame2->backgroundColor(), frame1->backgroundColor() );
  QCOMPARE( frame2->blendMode(), frame1->blendMode() );
  QCOMPARE( frame2->itemOpacity(), frame1->itemOpacity() );

  //check non-inherited properties
  QVERIFY( !frame2->hidePageIfEmpty() );

  mLayout->removeMultiFrame( multiframe );
  delete multiframe;
}

void TestQgsLayoutMultiFrame::displayName()
{
  TestMultiFrame *multiframe = new TestMultiFrame( mLayout );
  QCOMPARE( multiframe->displayName(), QStringLiteral( "<Multiframe>" ) );

  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, nullptr );
  QCOMPARE( frame1->displayName(), QStringLiteral( "<Frame>" ) );
  multiframe->addFrame( frame1 );
  QCOMPARE( frame1->displayName(), QStringLiteral( "<Multiframe>" ) );
  frame1->setId( "my frame" );
  QCOMPARE( frame1->displayName(), QStringLiteral( "my frame" ) );
}

void TestQgsLayoutMultiFrame::frameIsEmpty()
{
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( mLayout );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( mLayout, htmlItem );
  frame2->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), false );

  //..and back again..
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsLayoutMultiFrame::addRemovePage()
{
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( mLayout );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );

  //short content, so should fit in one frame
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  //should be one page
  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->pageCollection()->pageCount(), 1 );

  //long content, so we require 3 frames
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 2000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 3 );
  QCOMPARE( mLayout->pageCollection()->pageCount(), 3 );

  //..and back again..
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->pageCollection()->pageCount(), 1 );


  //get a bit more complicated - add another item to page 3
  QgsLayoutItemLabel *label1 = new QgsLayoutItemLabel( mLayout );
  mLayout->addLayoutItem( label1 );
  label1->attemptResize( QgsLayoutSize( 50, 50 ), false );

  //long content, so we require 4 pages
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 3000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 4 );
  QCOMPARE( mLayout->pageCollection()->pageCount(), 4 );

  label1->attemptMove( QgsLayoutPoint( 10, 10 ), true, false, 2 );

  //..and back again. Since there's an item on page 3, only page 4 should be removed
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->pageCollection()->pageCount(), 3 );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsLayoutMultiFrame::undoRedo()
{
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( mLayout );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );

  //short content, so should fit in one frame
  htmlItem->setHtml( QStringLiteral( "<p>Test content</p>" ) );
  htmlItem->loadHtml();

  //do some combinations of undo/redo commands for both the frame and multiframe
  //to try to trigger a crash
  frame1->beginCommand( QStringLiteral( "move" ) );
  frame1->attemptSetSceneRect( QRectF( 10, 10, 20, 20 ) );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "stroke" ), QgsLayoutItem::UndoStrokeWidth );
  frame1->setFrameStrokeWidth( QgsLayoutMeasurement( 4.0 ) );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "stroke" ), QgsLayoutItem::UndoStrokeWidth );
  frame1->setFrameStrokeWidth( QgsLayoutMeasurement( 7.0 ) );
  frame1->endCommand();

  //multiframe commands
  htmlItem->beginCommand( QStringLiteral( "maxbreak" ) );
  htmlItem->setMaxBreakDistance( 100 );
  htmlItem->endCommand();

  //another frame command
  frame1->beginCommand( QStringLiteral( "bgcolor" ), QgsLayoutItem::UndoOpacity );
  frame1->setBackgroundColor( QColor( 255, 255, 0 ) );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "bgcolor" ), QgsLayoutItem::UndoOpacity );
  frame1->setBackgroundColor( QColor( 255, 0, 0 ) );
  frame1->endCommand();

  //undo changes

  //frame bg
  mLayout->undoStack()->stack()->undo();
  //multiframe max break
  mLayout->undoStack()->stack()->undo();
  //frame stroke width
  mLayout->undoStack()->stack()->undo();
  //frame move
  mLayout->undoStack()->stack()->undo();

  //check result
  QCOMPARE( htmlItem->maxBreakDistance(), 10.0 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth().length(), 0.3 );
  QCOMPARE( htmlItem->frame( 0 )->pos(), QPointF( 0, 0 ) );
  QCOMPARE( htmlItem->frame( 0 )->backgroundColor(), QColor( 255, 255, 255 ) );

  //now redo

  //frame move
  mLayout->undoStack()->stack()->redo();
  //frame stroke width
  mLayout->undoStack()->stack()->redo();
  //multiframe max break
  mLayout->undoStack()->stack()->redo();
  //frame bg color
  mLayout->undoStack()->stack()->redo();

  //check result
  QCOMPARE( htmlItem->maxBreakDistance(), 100.0 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth().length(), 7.0 );
  QCOMPARE( htmlItem->frame( 0 )->pos(), QPointF( 10, 10 ) );
  QCOMPARE( htmlItem->frame( 0 )->backgroundColor(), QColor( 255, 0, 0 ) );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
}


void TestQgsLayoutMultiFrame::undoRedoRemovedFrame()
{
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( mLayout );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsLayoutItemHtml::ManualHtml );
  htmlItem->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );

  //long content, so should require multiple frames
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 2000px\">Test content</p>" ) );
  htmlItem->loadHtml();

  QVERIFY( htmlItem->frameCount() > 1 );

  //do a command on the first frame
  htmlItem->frame( 0 )->beginCommand( QStringLiteral( "stroke" ), QgsLayoutItem::UndoStrokeWidth );
  htmlItem->frame( 0 )->setFrameStrokeWidth( QgsLayoutMeasurement( 4.0 ) );
  htmlItem->frame( 0 )->endCommand();
  //do a command on the second frame
  htmlItem->frame( 1 )->beginCommand( QStringLiteral( "stroke" ), QgsLayoutItem::UndoStrokeWidth );
  htmlItem->frame( 1 )->setFrameStrokeWidth( QgsLayoutMeasurement( 8.0 ) );
  htmlItem->frame( 1 )->endCommand();

  //do a multiframe command which removes extra frames
  htmlItem->beginCommand( QStringLiteral( "source" ) );
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 20px\">Test content</p>" ) );
  htmlItem->endCommand();

  //wipes the second frame
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );

  auto dumpStack = [ = ]
  {
#if 0 // for debugging
    // dump stack
    for ( int i = 0; i < mLayout->undoStack()->stack()->count(); ++i )
    {
      QgsDebugMsg( QStringLiteral( "%1: %2 %3" ).arg( i ).arg( mLayout->undoStack()->stack()->command( i )->text(), i + 1 == mLayout->undoStack()->stack()->index() ? QString( "<---" ) : QString() ) );
    }
#endif
  };
  dumpStack();
  //undo changes

  //multiframe command
  mLayout->undoStack()->stack()->undo();
  dumpStack();
  //frame 2 command
  mLayout->undoStack()->stack()->undo();
  dumpStack();
  //frame 1 command
  mLayout->undoStack()->stack()->undo();
  dumpStack();
  //check result
  QVERIFY( htmlItem->frameCount() > 1 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth().length(), 0.3 );
  QCOMPARE( htmlItem->frame( 1 )->frameStrokeWidth().length(), 0.3 );

  //now redo

  //frame 1 command
  mLayout->undoStack()->stack()->redo();
  dumpStack();
  //frame 2 command
  mLayout->undoStack()->stack()->redo();
  dumpStack();

  //check result
  QVERIFY( htmlItem->frameCount() > 1 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth().length(), 4.0 );
  QCOMPARE( htmlItem->frame( 1 )->frameStrokeWidth().length(), 8.0 );

  //multiframe command
  mLayout->undoStack()->stack()->redo();
  QCOMPARE( htmlItem->frameCount(), 1 );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsLayoutMultiFrame::undoRedoRemovedFrame2()
{
  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( mLayout );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( mLayout, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );

}

void TestQgsLayoutMultiFrame::registry()
{
  // test QgsLayoutItemRegistry
  QgsLayoutItemRegistry registry;

  // empty registry
  QVERIFY( !registry.multiFrameMetadata( -1 ) );
  QVERIFY( registry.itemTypes().isEmpty() );
  QVERIFY( !registry.createMultiFrame( 1, nullptr ) );

  auto create = []( QgsLayout * layout )->QgsLayoutMultiFrame *
  {
    return new TestMultiFrame( layout );
  };
  auto resolve = []( QVariantMap & props, const QgsPathResolver &, bool )
  {
    props.clear();
  };

  const QSignalSpy spyTypeAdded( &registry, &QgsLayoutItemRegistry::multiFrameTypeAdded );

  QgsLayoutMultiFrameMetadata *metadata = new QgsLayoutMultiFrameMetadata( QgsLayoutItemRegistry::PluginItem + 1, QStringLiteral( "TestMultiFrame" ), create, resolve );
  QVERIFY( registry.addLayoutMultiFrameType( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 0 ).toInt(), QgsLayoutItemRegistry::PluginItem + 1 );
  QCOMPARE( spyTypeAdded.value( 0 ).at( 1 ).toString(), QStringLiteral( "TestMultiFrame" ) );
  // duplicate type id
  QVERIFY( !registry.addLayoutMultiFrameType( metadata ) );
  QCOMPARE( spyTypeAdded.count(), 1 );

  //retrieve metadata
  QVERIFY( !registry.multiFrameMetadata( -1 ) );
  QCOMPARE( registry.multiFrameMetadata( QgsLayoutItemRegistry::PluginItem + 1 )->visibleName(), QStringLiteral( "TestMultiFrame" ) );
  QCOMPARE( registry.itemTypes().count(), 1 );
  QCOMPARE( registry.itemTypes().value( QgsLayoutItemRegistry::PluginItem + 1 ), QStringLiteral( "TestMultiFrame" ) );
  QgsLayout l( QgsProject::instance() );
  QgsLayoutMultiFrame *item = registry.createMultiFrame( QgsLayoutItemRegistry::PluginItem + 1, &l );
  QVERIFY( item );
  QVERIFY( dynamic_cast< TestMultiFrame *>( item ) );
  QVariantMap props;
  props.insert( QStringLiteral( "a" ), 5 );
  registry.resolvePaths( 1, props, QgsPathResolver(), true );
  QCOMPARE( props.size(), 1 );
  registry.resolvePaths( QgsLayoutItemRegistry::PluginItem + 1, props, QgsPathResolver(), true );
  QVERIFY( props.isEmpty() );
}

void TestQgsLayoutMultiFrame::deleteFrame()
{
  QgsLayout l( QgsProject::instance() );
  l.initializeDefaults();

  QgsLayoutItemHtml *htmlItem = new QgsLayoutItemHtml( &l );
  QgsLayoutFrame *frame1 = new QgsLayoutFrame( &l, htmlItem );
  frame1->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame1 );
  QgsLayoutFrame *frame2 = new QgsLayoutFrame( &l, htmlItem );
  frame2->attemptSetSceneRect( QRectF( 0, 0, 100, 200 ) );
  htmlItem->addFrame( frame2 );

  QCOMPARE( htmlItem->frameCount(), 2 );
  QCOMPARE( htmlItem->frames(), QList< QgsLayoutFrame * >() << frame1 << frame2 );
  l.removeLayoutItem( frame1 );
  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( htmlItem->frames(), QList< QgsLayoutFrame * >() << frame2 );
  l.removeLayoutItem( frame2 );
  QCOMPARE( htmlItem->frameCount(), 0 );
  QVERIFY( htmlItem->frames().empty() );
}

void TestQgsLayoutMultiFrame::writeReadXml()
{
  QgsProject p;

  // create layout
  QgsLayout c( &p );
  // add an multiframe
  QgsLayoutItemHtml *html = new QgsLayoutItemHtml( &c );
  c.addMultiFrame( html );
  html->setHtml( QStringLiteral( "<blink>hi</blink>" ) );
  QgsLayoutFrame *frame = new QgsLayoutFrame( &c, html );
  frame->attemptSetSceneRect( QRectF( 1, 1, 10, 10 ) );
  c.addLayoutItem( frame );
  html->addFrame( frame );

  QCOMPARE( frame->multiFrame(), html );
  QCOMPARE( html->frameCount(), 1 );
  QCOMPARE( html->frames(), QList< QgsLayoutFrame * >() << frame );

  // save layout to xml
  QDomDocument doc;
  doc.appendChild( c.writeXml( doc, QgsReadWriteContext() ) );

  // make a new layout from xml
  QgsLayout c2( &p );
  c2.readXml( doc.childNodes().at( 0 ).toElement(), doc, QgsReadWriteContext() );
  // get table from new layout
  QList< QgsLayoutFrame * > frames2;
  c2.layoutItems( frames2 );
  QCOMPARE( frames2.count(), 1 );
  QgsLayoutFrame *frame2 = frames2.at( 0 );

  QgsLayoutItemHtml *html2 = static_cast< QgsLayoutItemHtml *>( frame2->multiFrame() );
  QVERIFY( html2 );
  QCOMPARE( html2->html(), QStringLiteral( "<blink>hi</blink>" ) );
  QCOMPARE( html2->frameCount(), 1 );
  QCOMPARE( html2->frames(), QList< QgsLayoutFrame * >() << frame2 );
}

void TestQgsLayoutMultiFrame::noPageNoCrash()
{
  QgsProject p;

  // create layout, no pages
  QgsLayout c( &p );
  // add an multiframe
  QgsLayoutItemHtml *html = new QgsLayoutItemHtml( &c );
  c.addMultiFrame( html );
  html->setContentMode( QgsLayoutItemHtml::ManualHtml );
  html->setHtml( QStringLiteral( "<div style=\"height: 2000px\">hi</div>" ) );
  QgsLayoutFrame *frame = new QgsLayoutFrame( &c, html );
  frame->attemptSetSceneRect( QRectF( 1, 1, 10, 1 ) );
  c.addLayoutItem( frame );
  html->addFrame( frame );

  html->setResizeMode( QgsLayoutMultiFrame::UseExistingFrames );
  html->recalculateFrameSizes();
  QCOMPARE( html->frameCount(), 1 );
  html->setResizeMode( QgsLayoutMultiFrame::ExtendToNextPage );
  html->recalculateFrameSizes();
  QCOMPARE( html->frameCount(), 1 );
  html->setResizeMode( QgsLayoutMultiFrame::RepeatOnEveryPage );
  html->recalculateFrameSizes();
  QCOMPARE( html->frameCount(), 1 );
  html->setResizeMode( QgsLayoutMultiFrame::RepeatUntilFinished );
  html->recalculateFrameSizes();
  QCOMPARE( html->frameCount(), 1 );
}

void TestQgsLayoutMultiFrame::variables()
{
  QgsLayout l( QgsProject::instance() );

  QgsLayoutItemHtml *html = new QgsLayoutItemHtml( &l );
  std::unique_ptr< QgsExpressionContextScope > scope( QgsExpressionContextUtils::multiFrameScope( html ) );
  const int before = scope->variableCount();

  QgsExpressionContextUtils::setLayoutMultiFrameVariable( html, QStringLiteral( "var" ), 5 );
  scope.reset( QgsExpressionContextUtils::multiFrameScope( html ) );
  QCOMPARE( scope->variableCount(), before + 1 );
  QCOMPARE( scope->variable( QStringLiteral( "var" ) ).toInt(), 5 );

  QVariantMap vars;
  vars.insert( QStringLiteral( "var2" ), 7 );
  QgsExpressionContextUtils::setLayoutMultiFrameVariables( html, vars );
  scope.reset( QgsExpressionContextUtils::multiFrameScope( html ) );
  QCOMPARE( scope->variableCount(), before + 1 );
  QVERIFY( !scope->hasVariable( QStringLiteral( "var" ) ) );
  QCOMPARE( scope->variable( QStringLiteral( "var2" ) ).toInt(), 7 );

  const QgsExpressionContext context = html->createExpressionContext();
  QCOMPARE( context.variable( QStringLiteral( "var2" ) ).toInt(), 7 );
}

QGSTEST_MAIN( TestQgsLayoutMultiFrame )
#include "testqgslayoutmultiframe.moc"
