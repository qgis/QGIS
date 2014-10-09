/***************************************************************************
                         testqgscomposermultiframe.cpp
                         -----------------------
    begin                : September 2014
    copyright            : (C) 2014 by Nyall Dawson
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

#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgscomposerlabel.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerMultiFrame: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void addFrame(); //test creating new frame inherits all properties of existing frame
    void frameIsEmpty(); //test if frame is empty works
    void addRemovePage(); //test if page is added and removed for RepeatUntilFinished mode

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;
};

void TestQgsComposerMultiFrame::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  mReport = "<h1>Composer MultiFrame Tests</h1>\n";
}

void TestQgsComposerMultiFrame::cleanupTestCase()
{
  delete mComposition;

  QString myReportFile = QDir::tempPath() + QDir::separator() + "qgistest.html";
  QFile myFile( myReportFile );
  if ( myFile.open( QIODevice::WriteOnly | QIODevice::Append ) )
  {
    QTextStream myQTextStream( &myFile );
    myQTextStream << mReport;
    myFile.close();
  }
}

void TestQgsComposerMultiFrame::init()
{

}

void TestQgsComposerMultiFrame::cleanup()
{

}

void TestQgsComposerMultiFrame::addFrame()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );

  //should not be inherited
  frame1->setHidePageIfEmpty( true );

  //should be inherited
  frame1->setHideBackgroundIfEmpty( true );
  frame1->setFrameOutlineWidth( 5.0 );
  frame1->setFrameJoinStyle( Qt::RoundJoin );
  frame1->setFrameEnabled( true );
  frame1->setFrameOutlineColor( QColor( Qt::red ) );
  frame1->setBackgroundEnabled( true );
  frame1->setBackgroundColor( QColor( Qt::green ) );
  frame1->setBlendMode( QPainter::CompositionMode_ColorBurn );
  frame1->setTransparency( 50 );

  QgsComposerFrame* frame2 = htmlItem->createNewFrame( frame1, QPointF( 50, 55 ), QSizeF( 70, 120 ) );

  //check frame created in correct place
  QCOMPARE( frame2->rect().height(), 120.0 );
  QCOMPARE( frame2->rect().width(), 70.0 );
  QCOMPARE( frame2->scenePos().x(), 50.0 );
  QCOMPARE( frame2->scenePos().y(), 55.0 );

  //check frame properties
  QCOMPARE( frame2->frameOutlineWidth(), frame1->frameOutlineWidth() );
  QCOMPARE( frame2->frameOutlineColor(), frame1->frameOutlineColor() );
  QCOMPARE( frame2->frameJoinStyle(), frame1->frameJoinStyle() );
  QCOMPARE( frame2->hasBackground(), frame1->hasBackground() );
  QCOMPARE( frame2->backgroundColor(), frame1->backgroundColor() );
  QCOMPARE( frame2->blendMode(), frame1->blendMode() );
  QCOMPARE( frame2->transparency(), frame1->transparency() );

  //check non-inherited properties
  QVERIFY( !frame2->hidePageIfEmpty() );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsComposerMultiFrame::frameIsEmpty()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  QgsComposerFrame* frame2 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QString( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), false );

  //..and back again..
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( frame1->isEmpty(), false );
  QCOMPARE( frame2->isEmpty(), true );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsComposerMultiFrame::addRemovePage()
{
  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  QgsComposerFrame* frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 200 );
  htmlItem->addFrame( frame1 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  htmlItem->setResizeMode( QgsComposerMultiFrame::RepeatUntilFinished );

  //short content, so should fit in one frame
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  //should be one page
  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mComposition->numPages(), 1 );

  //long content, so we require 3 frames
  htmlItem->setHtml( QString( "<p style=\"height: 2000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 3 );
  QCOMPARE( mComposition->numPages(), 3 );

  //..and back again..
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mComposition->numPages(), 1 );


  //get a bit more complicated - add another item to page 3
  QgsComposerLabel* label1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //long content, so we require 4 pages
  htmlItem->setHtml( QString( "<p style=\"height: 3000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 4 );
  QCOMPARE( mComposition->numPages(), 4 );

  //..and back again. Since there's an item on page 3, only page 4 should be removed
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( htmlItem->frameCount(), 1 );
  QCOMPARE( mComposition->numPages(), 3 );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}


QTEST_MAIN( TestQgsComposerMultiFrame )
#include "moc_testqgscomposermultiframe.cxx"
