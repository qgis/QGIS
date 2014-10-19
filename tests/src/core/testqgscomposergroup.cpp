/***************************************************************************
                         testqgscomposergroup.cpp
                         -----------------------
    begin                : October 2014
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

#include "qgscomposeritemgroup.h"
#include "qgscomposerlabel.h"
#include "qgscomposition.h"
#include "qgscompositionchecker.h"
#include <QObject>
#include <QtTest>

class TestQgsComposerGroup: public QObject
{
    Q_OBJECT;
  private slots:
    void initTestCase();// will be called before the first testfunction is executed.
    void cleanupTestCase();// will be called after the last testfunction was executed.
    void init();// will be called before each testfunction is executed.
    void cleanup();// will be called after every testfunction.
    void createGroup(); //test grouping items
    void ungroup(); //test ungrouping items
    void deleteGroup(); //test deleting group works
    void undoRedo(); //test that group/ungroup undo/redo commands don't crash

  private:
    QgsComposition* mComposition;
    QgsMapSettings mMapSettings;
    QgsComposerLabel* mItem1;
    QgsComposerLabel* mItem2;
    QgsComposerItemGroup* mGroup;
    QString mReport;
};

void TestQgsComposerGroup::initTestCase()
{
  mComposition = new QgsComposition( mMapSettings );
  mComposition->setPaperSize( 297, 210 ); //A4 landscape

  //create some items
  mItem1 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem1 );
  mItem2 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem2 );

  mGroup = 0;

  mReport = "<h1>Composer Grouped Item Tests</h1>\n";
}

void TestQgsComposerGroup::cleanupTestCase()
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

void TestQgsComposerGroup::init()
{

}

void TestQgsComposerGroup::cleanup()
{

}

void TestQgsComposerGroup::createGroup()
{
  //group items
  QList<QgsComposerItem*> items;
  items << mItem1 << mItem2;
  mGroup = mComposition->groupItems( items );

  //check result
  QVERIFY( mGroup );
  QCOMPARE( mGroup->items().size(), 2 );
  QVERIFY( mGroup->items().contains( mItem1 ) );
  QVERIFY( mGroup->items().contains( mItem2 ) );
  QVERIFY( mItem1->isGroupMember() );
  QVERIFY( mItem2->isGroupMember() );
}

void TestQgsComposerGroup::ungroup()
{
  //test ungrouping items

  //simple tests - check that we don't crash
  mComposition->ungroupItems( 0 ); //no item

  //ungroup mGroup
  QList<QgsComposerItem*> ungroupedItems;
  ungroupedItems = mComposition->ungroupItems( mGroup );

  QCOMPARE( ungroupedItems.size(), 2 );
  QVERIFY( ungroupedItems.contains( mItem1 ) );
  QVERIFY( ungroupedItems.contains( mItem2 ) );

  QVERIFY( !mItem1->isGroupMember() );
  QVERIFY( !mItem2->isGroupMember() );

  //should also be no groups left in the composition
  QList<QgsComposerItemGroup*> groups;
  mComposition->composerItems( groups );
  QCOMPARE( groups.size(), 0 );
}

void TestQgsComposerGroup::deleteGroup()
{
  //group items
  QList<QgsComposerItem*> groupItems;
  groupItems << mItem1 << mItem2;
  mGroup = mComposition->groupItems( groupItems );
  QList<QgsComposerItem*> items;
  mComposition->composerItems( items );
  //expect initially 4 items, as paper counts as an item
  QCOMPARE( items.size(), 4 );

  //test that deleting group also removes all grouped items
  mComposition->removeComposerItem( mGroup );
  mComposition->composerItems( items );

  //expect a single item (paper item)
  QCOMPARE( items.size(), 1 );
  QVERIFY( mItem1->isRemoved() );
  QVERIFY( mItem2->isRemoved() );
}

void TestQgsComposerGroup::undoRedo()
{
#if 0 //expected fail - see #11371
  //test for crash when undo/redoing with groups

  //create some items
  mItem1 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem1 );
  mItem2 = new QgsComposerLabel( mComposition );
  mComposition->addItem( mItem2 );

  //group items
  QList<QgsComposerItem*> items;
  items << mItem1 << mItem2;
  mGroup = mComposition->groupItems( items );

  //move, and ungroup
  mGroup->beginCommand( "move" );
  mGroup->move( 10.0, 20.0 );
  mGroup->endCommand();
  mComposition->ungroupItems( mGroup );

  //undo
  mComposition->undoStack()->undo();

  //redo
  mComposition->undoStack()->redo();
#endif
}

QTEST_MAIN( TestQgsComposerGroup )
#include "moc_testqgscomposergroup.cxx"
