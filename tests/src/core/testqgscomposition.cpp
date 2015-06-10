/***************************************************************************
                         testqgscomposition.cpp
                         ----------------------
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

#include "qgsapplication.h"
#include "qgscomposition.h"
#include "qgscomposerlabel.h"
#include "qgscomposershape.h"
#include "qgscomposerarrow.h"
#include "qgscomposerhtml.h"
#include "qgscomposerframe.h"
#include "qgsmapsettings.h"

#include <QObject>
#include <QtTest/QtTest>

class TestQgsComposition : public QObject
{
    Q_OBJECT

  public:
    TestQgsComposition();

  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.

    void itemsOnPage(); //test fetching matching items on a set page
    void shouldExportPage(); //test the shouldExportPage method
    void pageIsEmpty(); //test the pageIsEmpty method

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QString mReport;
};

TestQgsComposition::TestQgsComposition()
    : mComposition( NULL )
{

}

void TestQgsComposition::initTestCase()
{
  QgsApplication::init();
  QgsApplication::initQgis();

  //create composition
  mMapSettings.setCrsTransformEnabled( true );
  mMapSettings.setMapUnits( QGis::Meters );
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape
  mComposition->setNumPages( 3 );

  mReport = "<h1>Composition Tests</h1>\n";

}

void TestQgsComposition::cleanupTestCase()
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
  QgsApplication::exitQgis();
}

void TestQgsComposition::init()
{
}

void TestQgsComposition::cleanup()
{
}

void TestQgsComposition::itemsOnPage()
{
  //add some items to the composition
  QgsComposerLabel* label1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel* label2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label2 );
  label2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel* label3 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label3 );
  label3->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerShape* shape1 = new QgsComposerShape( mComposition );
  mComposition->addComposerShape( shape1 );
  shape1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerShape* shape2 = new QgsComposerShape( mComposition );
  mComposition->addComposerShape( shape2 );
  shape2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 2 );
  QgsComposerArrow* arrow1 = new QgsComposerArrow( mComposition );
  mComposition->addComposerArrow( arrow1 );
  arrow1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );
  QgsComposerArrow* arrow2 = new QgsComposerArrow( mComposition );
  mComposition->addComposerArrow( arrow2 );
  arrow2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //fetch items - remember that these numbers include the paper item!
  QList<QgsComposerItem*> items;
  mComposition->composerItemsOnPage( items, 0 );
  //should be 4 items on page 1
  QCOMPARE( items.length(), 4 );
  mComposition->composerItemsOnPage( items, 1 );
  //should be 3 items on page 2
  QCOMPARE( items.length(), 3 );
  mComposition->composerItemsOnPage( items, 2 );
  //should be 3 items on page 3
  QCOMPARE( items.length(), 3 );

  //check fetching specific item types
  QList<QgsComposerLabel*> labels;
  mComposition->composerItemsOnPage( labels, 0 );
  //should be 2 labels on page 1
  QCOMPARE( labels.length(), 2 );
  mComposition->composerItemsOnPage( labels, 1 );
  //should be 1 label on page 2
  QCOMPARE( labels.length(), 1 );
  mComposition->composerItemsOnPage( labels, 2 );
  //should be no label on page 3
  QCOMPARE( labels.length(), 0 );

  QList<QgsComposerShape*> shapes;
  mComposition->composerItemsOnPage( shapes, 0 );
  //should be 1 shapes on page 1
  QCOMPARE( shapes.length(), 1 );
  mComposition->composerItemsOnPage( shapes, 1 );
  //should be 1 shapes on page 2
  QCOMPARE( shapes.length(), 1 );
  mComposition->composerItemsOnPage( shapes, 2 );
  //should be no shapes on page 3
  QCOMPARE( shapes.length(), 0 );

  QList<QgsComposerArrow*> arrows;
  mComposition->composerItemsOnPage( arrows, 0 );
  //should be no arrows on page 1
  QCOMPARE( arrows.length(), 0 );
  mComposition->composerItemsOnPage( arrows, 1 );
  //should be no arrows on page 2
  QCOMPARE( arrows.length(), 0 );
  mComposition->composerItemsOnPage( arrows, 2 );
  //should be 2 arrows on page 3
  QCOMPARE( arrows.length(), 2 );

  mComposition->removeComposerItem( label1 );
  mComposition->removeComposerItem( label2 );
  mComposition->removeComposerItem( label3 );
  mComposition->removeComposerItem( shape1 );
  mComposition->removeComposerItem( shape2 );
  mComposition->removeComposerItem( arrow1 );
  mComposition->removeComposerItem( arrow2 );

  //check again with removed items
  mComposition->composerItemsOnPage( labels, 0 );
  QCOMPARE( labels.length(), 0 );
  mComposition->composerItemsOnPage( labels, 1 );
  QCOMPARE( labels.length(), 0 );
  mComposition->composerItemsOnPage( labels, 2 );
  QCOMPARE( labels.length(), 0 );
}

void TestQgsComposition::shouldExportPage()
{
  mComposition->setPaperSize( 297, 200 );
  mComposition->setNumPages( 2 );

  QgsComposerHtml* htmlItem = new QgsComposerHtml( mComposition, false );
  //frame on page 1
  QgsComposerFrame* frame1 = new QgsComposerFrame( mComposition, htmlItem, 0, 0, 100, 100 );
  //frame on page 2
  QgsComposerFrame* frame2 = new QgsComposerFrame( mComposition, htmlItem, 0, 320, 100, 100 );
  frame2->setHidePageIfEmpty( true );
  htmlItem->addFrame( frame1 );
  htmlItem->addFrame( frame2 );
  htmlItem->setContentMode( QgsComposerHtml::ManualHtml );
  //short content, so frame 2 should be empty
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), false );

  //long content, so frame 2 should not be empty
  htmlItem->setHtml( QString( "<p style=\"height: 10000px\"><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), true );

  //...and back again...
  htmlItem->setHtml( QString( "<p><i>Test manual <b>html</b></i></p>" ) );
  htmlItem->loadHtml();

  QCOMPARE( mComposition->shouldExportPage( 1 ), true );
  QCOMPARE( mComposition->shouldExportPage( 2 ), false );

  mComposition->removeMultiFrame( htmlItem );
  delete htmlItem;
}

void TestQgsComposition::pageIsEmpty()
{
  //add some items to the composition
  QgsComposerLabel* label1 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label1 );
  label1->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel* label2 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label2 );
  label2->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 1 );
  QgsComposerLabel* label3 = new QgsComposerLabel( mComposition );
  mComposition->addComposerLabel( label3 );
  label3->setItemPosition( 10, 10, 50, 50, QgsComposerItem::UpperLeft, false, 3 );

  //only page 2 should be empty
  QCOMPARE( mComposition->pageIsEmpty( 1 ), false );
  QCOMPARE( mComposition->pageIsEmpty( 2 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 3 ), false );

  //remove the items
  mComposition->removeComposerItem( label1 );
  mComposition->removeComposerItem( label2 );
  mComposition->removeComposerItem( label3 );

  //expect everything to be empty now
  QCOMPARE( mComposition->pageIsEmpty( 1 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 2 ), true );
  QCOMPARE( mComposition->pageIsEmpty( 3 ), true );
}

QTEST_MAIN( TestQgsComposition )
#include "testqgscomposition.moc"
