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

#include <QObject>
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

    void render( QgsRenderContext &, const QRectF &, const int,
                 const QStyleOptionGraphicsItem * ) override
    {

    }

    QSizeF totalSize() const override
    {
      return QSizeF();
    }

    bool writeXml( QDomElement &, QDomDocument &, bool ) const override
    {
      return true;
    }

    bool readXml( const QDomElement &, const QDomDocument &, bool ) override
    {
      return true;
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
  QPointer< TestMultiFrame > pMF( mF );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );

  // try readding
  l->addMultiFrame( mF );
  QCOMPARE( l->multiFrames().count(), 1 );

  l->removeMultiFrame( nullptr );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );

  TestMultiFrame *mF2 = new TestMultiFrame( l );
  QPointer< TestMultiFrame > pMF2( mF2 );
  l->addMultiFrame( mF2 );
  QCOMPARE( l->multiFrames().count(), 2 );
  QVERIFY( l->multiFrames().contains( mF ) );
  QVERIFY( l->multiFrames().contains( mF2 ) );
  l->removeMultiFrame( mF2 );
  QCOMPARE( l->multiFrames().count(), 1 );
  QVERIFY( l->multiFrames().contains( mF ) );

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
#if 0 //TODO
  QgsComposerHtml *htmlItem = new QgsComposerHtml( mLayout, false );
  QgsComposerFrame *frame1 = new QgsComposerFrame( mLayout, htmlItem, 0, 0, 100, 200 );
  QgsComposerFrame *frame2 = new QgsComposerFrame( mLayout, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
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
#endif
}

void TestQgsLayoutMultiFrame::addRemovePage()
{
#if 0 //TODO
  QgsComposerHtml *htmlItem = new QgsComposerHtml( mLayout, false );
  QgsComposerFrame *frame1 = new QgsComposerFrame( mLayout, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  //short content, so should fit in one frame
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  //should be one page
  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->numPages(), 1 );

  //long content, so we require 3 frames
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 2000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 3 );
  QCOMPARE( mLayout->numPages(), 3 );

  //..and back again..
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->numPages(), 1 );


  //get a bit more complicated - add another item to page 3
  QgsComposerLabel *label1 = new QgsComposerLabel( mLayout );
  mLayout->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //long content, so we require 4 pages
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 3000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 4 );
  QCOMPARE( mLayout->numPages(), 4 );

  //..and back again. Since there's an item on page 3, only page 4 should be removed
  htmlItem->setHtml( QStringLiteral( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mLayout->numPages(), 3 );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
#endif
}

void TestQgsLayoutMultiFrame::undoRedo()
{
#if 0 //TODO
  QgsComposerHtml *htmlItem = new QgsComposerHtml( mLayout, false );
  QgsComposerFrame *frame1 = new QgsComposerFrame( mLayout, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  //short content, so should fit in one frame
  htmlItem->setHtml( QStringLiteral( "<p>Test content</p>" ) );
  htmlItem->loadHtml();

  //do some combinations of undo/redo commands for both the frame and multiframe
  //to try to trigger a crash
  frame1->beginCommand( QStringLiteral( "move" ) );
  frame1->setSceneRect( QRectF( 10, 10, 20, 20 ) );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "stroke" ), QgsComposerMergeCommand::ItemStrokeWidth );
  frame1->setFrameStrokeWidth( 4.0 );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "stroke" ), QgsComposerMergeCommand::ItemStrokeWidth );
  frame1->setFrameStrokeWidth( 7.0 );
  frame1->endCommand();

  //multiframe commands
  mLayout->beginMultiFrameCommand( htmlItem, QStringLiteral( "maxbreak" ) );
  htmlItem->setMaxBreakDistance( 100 );
  mLayout->endMultiFrameCommand();

  //another frame command
  frame1->beginCommand( QStringLiteral( "bgcolor" ), QgsComposerMergeCommand::ItemOpacity );
  frame1->setBackgroundColor( QColor( 255, 255, 0 ) );
  frame1->endCommand();
  frame1->beginCommand( QStringLiteral( "bgcolor" ), QgsComposerMergeCommand::ItemOpacity );
  frame1->setBackgroundColor( QColor( 255, 0, 0 ) );
  frame1->endCommand();

  //undo changes

  //frame bg
  mLayout->undoStack()->undo();
  //multiframe max break
  mLayout->undoStack()->undo();
  //frame stroke width
  mLayout->undoStack()->undo();
  //frame move
  mLayout->undoStack()->undo();

  //check result
  QCOMPARE( htmlItem->maxBreakDistance(), 10.0 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth(), 0.3 );
  QCOMPARE( htmlItem->frame( 0 )->pos(), QPointF( 0, 0 ) );
  QCOMPARE( htmlItem->frame( 0 )->backgroundColor(), QColor( 255, 255, 255 ) );

  //now redo

  //frame move
  mLayout->undoStack()->redo();
  //frame stroke width
  mLayout->undoStack()->redo();
  //multiframe max break
  mLayout->undoStack()->redo();
  //frame bg color
  mLayout->undoStack()->redo();

  //check result
  QCOMPARE( htmlItem->maxBreakDistance(), 100.0 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth(), 7.0 );
  QCOMPARE( htmlItem->frame( 0 )->pos(), QPointF( 10, 10 ) );
  QCOMPARE( htmlItem->frame( 0 )->backgroundColor(), QColor( 255, 0, 0 ) );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
#endif
}


void TestQgsLayoutMultiFrame::undoRedoRemovedFrame()
{
#if 0 //TODO
  QgsComposerHtml *htmlItem = new QgsComposerHtml( mLayout, false );
  QgsComposerFrame *frame1 = new QgsComposerFrame( mLayout, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  //long content, so should require multiple frames
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 2000px\">Test content</p>" ) );
  htmlItem->loadHtml();

  QVERIFY( htmlItem->frameCount() > 1 );

  //do a command on the first frame
  htmlItem->frame( 0 )->beginCommand( QStringLiteral( "stroke" ), QgsComposerMergeCommand::ItemStrokeWidth );
  htmlItem->frame( 0 )->setFrameStrokeWidth( 4.0 );
  htmlItem->frame( 0 )->endCommand();
  //do a command on the second frame
  htmlItem->frame( 1 )->beginCommand( QStringLiteral( "stroke" ), QgsComposerMergeCommand::ItemStrokeWidth );
  htmlItem->frame( 1 )->setFrameStrokeWidth( 8.0 );
  htmlItem->frame( 1 )->endCommand();

  //do a multiframe command which removes extra frames
  mLayout->beginMultiFrameCommand( htmlItem, QStringLiteral( "source" ) );
  htmlItem->setHtml( QStringLiteral( "<p style=\"height: 20px\">Test content</p>" ) );
  mLayout->endMultiFrameCommand();

  //wipes the second frame
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );

  //undo changes

  //multiframe command
  mLayout->undoStack()->undo();
  //frame 2 command
  mLayout->undoStack()->undo();
  //frame 1 command
  mLayout->undoStack()->undo();

  //check result
  QVERIFY( htmlItem->frameCount() > 1 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth(), 0.3 );
  QCOMPARE( htmlItem->frame( 1 )->frameStrokeWidth(), 0.3 );

  //now redo

  //frame 1 command
  mLayout->undoStack()->redo();
  //frame 2 command
  mLayout->undoStack()->redo();

  //check result
  QVERIFY( htmlItem->frameCount() > 1 );
  QCOMPARE( htmlItem->frame( 0 )->frameStrokeWidth(), 4.0 );
  QCOMPARE( htmlItem->frame( 1 )->frameStrokeWidth(), 8.0 );

  //multiframe command
  mLayout->undoStack()->redo();
  QCOMPARE( htmlItem->frameCount(), 1 );

  mLayout->removeMultiFrame( htmlItem );
  delete htmlItem;
#endif
}

QGSTEST_MAIN( TestQgsLayoutMultiFrame )
#include "testqgslayoutmultiframe.moc"
